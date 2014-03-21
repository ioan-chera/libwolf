#define _CRT_SECURE_NO_WARNINGS
#define BUILD_DLL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "libwolf.h"

#define lengthof(a) (sizeof(a) / sizeof(*(a)))

typedef struct maptype
{
	int32_t planestart[3];
	uint16_t planelength[3];
	uint16_t width, height;
	char name[16];
} maptype;

typedef struct wolf3d_LevelSet
{
	FILE* file;
	maptype mapheaderseg[WOLF3D_NUMMAPS];
	uint16_t rlewTag;
	uint16_t mapsegs[WOLF3D_NUMMAPS][WOLF3D_MAPPLANES][WOLF3D_MAPAREA];
} wolf3d_LevelSet;

wolf3d_LevelSet* wolf3d_LevelSetCreate(void)
{
	wolf3d_LevelSet *newset = calloc(1, sizeof(wolf3d_LevelSet));
	return newset;
}

static wolf3d_LoadFileResult openMapHead(FILE *f, uint16_t* newRlewTag, uint32_t* headeroffsets)
{
	if (!f)
		return wolf3d_LoadFileCannotOpen;

	if (fread(newRlewTag, 2, 1, f) != 1)
	{
		fclose(f);
		return wolf3d_LoadFileBadFormat;
	}
		
	if (fread(headeroffsets, 4, WOLF3D_NUMMAPS, f) != WOLF3D_NUMMAPS)
	{
		fclose(f);
		return wolf3d_LoadFileBadFormat;
	}

	fclose(f);

	return wolf3d_LoadFileOk;
}

static wolf3d_LoadFileResult openGameMaps(FILE *f, const uint32_t* headeroffsets, maptype* newMapheaderseg)
{
	if (!f)
		return wolf3d_LoadFileCannotOpen;

	int i;
	uint32_t pos;
	maptype *m;
	for (i = 0; i < WOLF3D_NUMMAPS; ++i)
	{
		pos = headeroffsets[i];
		if ((int32_t)pos < 0)
			continue;

		if (fseek(f, pos, SEEK_SET))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
		m = newMapheaderseg + i;
		if (fread(m->planestart, 1, sizeof(m->planestart), f) != sizeof(m->planestart))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
		if (fread(m->planelength, 1, sizeof(m->planelength), f) != sizeof(m->planelength))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
		if (fread(&m->width, 1, sizeof(m->width), f) != sizeof(m->width))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
		if (fread(&m->height, 1, sizeof(m->height), f) != sizeof(m->height))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
		if (fread(m->name, 1, sizeof(m->name), f) != sizeof(m->name))
		{
			fclose(f);
			return wolf3d_LoadFileBadFormat;
		}
	}

	return wolf3d_LoadFileOk;
}

wolf3d_LoadFileResult wolf3d_LevelSetOpenFile(wolf3d_LevelSet* levelset, const char* mapheadpath, const char* gamemapspath)
{
	FILE* f = fopen(mapheadpath, "rb");
	uint16_t newRlewTag;
	uint32_t headeroffsets[WOLF3D_NUMMAPS];
	maptype newMapheaderseg[WOLF3D_NUMMAPS];

	wolf3d_LoadFileResult result = openMapHead(f, &newRlewTag, headeroffsets);

	if (result != wolf3d_LoadFileOk)
		return result;
	
	f = fopen(gamemapspath, "rb");

	result = openGameMaps(f, headeroffsets, newMapheaderseg);
	if (result != wolf3d_LoadFileOk)
		return result;
	
	// all ok
	memcpy(levelset->mapheaderseg, newMapheaderseg, sizeof(levelset->mapheaderseg));
	wolf3d_LevelSetCloseFile(levelset);
	levelset->file = f;
	levelset->rlewTag = newRlewTag;

	return wolf3d_LoadFileOk;
}

wolf3d_LoadFileResult wolf3d_LevelSetOpenFileW(wolf3d_LevelSet* levelset, const wchar_t* mapheadpath, const wchar_t* gamemapspath)
{
#ifdef _WIN32
	FILE* f = _wfopen(mapheadpath, L"rb");
	uint16_t newRlewTag;
	uint32_t headeroffsets[WOLF3D_NUMMAPS];
	maptype newMapheaderseg[WOLF3D_NUMMAPS];

	wolf3d_LoadFileResult result = openMapHead(f, &newRlewTag, headeroffsets);

	if (result != wolf3d_LoadFileOk)
		return result;

	f = _wfopen(gamemapspath, L"rb");

	result = openGameMaps(f, headeroffsets, newMapheaderseg);
	if (result != wolf3d_LoadFileOk)
		return result;

	// all ok
	memcpy(levelset->mapheaderseg, newMapheaderseg, sizeof(levelset->mapheaderseg));
	wolf3d_LevelSetCloseFile(levelset);
	levelset->file = f;
	levelset->rlewTag = newRlewTag;
#endif
	return wolf3d_LoadFileOk;
}

static uint16_t READWORD(uint8_t **ptr)
{
	uint16_t val = (*ptr)[0] | (*ptr)[1] << 8;
	*ptr += 2;
	return val;
}

//
// CAL_CarmackExpand
//
// Length is the length of the EXPANDED data
//
#define NEARTAG 0xa7
#define FARTAG  0xa8

static void CAL_CarmackExpand(uint8_t *source, uint16_t *dest, int length)
{
	uint16_t ch, chhigh, count, offset;
	uint8_t *inptr;
	uint16_t *copyptr, *outptr;

	length /= 2;

	inptr = (uint8_t *)source;
	outptr = dest;

	while (length > 0)
	{
		ch = READWORD(&inptr);
		chhigh = ch >> 8;
		if (chhigh == NEARTAG)
		{
			count = ch & 0xff;
			if (!count)
			{                     // have to insert a word containing the tag byte
				ch |= *inptr++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *inptr++;
				copyptr = outptr - offset;
				length -= count;
				if (length < 0)
					return;

				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch & 0xff;
			if (!count)
			{                     // have to insert a word containing the tag byte
				ch |= *inptr++;
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = READWORD(&inptr);
				copyptr = dest + offset;
				length -= count;
				if (length<0)
					return;

				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else
		{
			*outptr++ = ch;
			length--;
		}
	}
}

//
// CAL_RLEWexpand
//
// length is EXPANDED length
// IOANCH: possibly added endian swapping, but commented out, since it may be
// the output of Carmack expansion, which already does swapping
//
static void CAL_RLEWexpand(uint16_t *source, uint16_t *dest, int32_t length, uint16_t rlewtag)
{
	uint16_t value, count, i;
	uint16_t *end = dest + length / sizeof(uint16_t);

	//
	// expand it
	//
	do
	{
		value = /*SwapUShort(*/*source++/*)*/;
		if (value != rlewtag)
		{
			//
			// uncompressed
			//
			*dest++ = value;
		}
		else
		{
			//
			// compressed string
			//
			count = /*SwapUShort(*/*source++/*)*/;
			value = /*SwapUShort(*/*source++/*)*/;
			for (i = 1; i <= count; i++)
				*dest++ = value;
		}
	} while (dest < end);
}

enum
{
	BUFFERSIZE = 0x1000
};
wolf3d_LoadFileResult wolf3d_LevelSetLoadMap(wolf3d_LevelSet* levelset, unsigned mapnum)
{
	int32_t bufferseg[BUFFERSIZE / 4];

	if (!levelset->file)
		return wolf3d_LoadFileCannotOpen;

	unsigned size = WOLF3D_MAPAREA * 2;
	int plane;
	int32_t pos;
	int32_t compressed;
	int32_t expanded;
	uint16_t* dest;
	uint16_t* source;
	uint16_t* buffer2seg = NULL;
	void* bigbufferseg = NULL;
	for (plane = 0; plane < WOLF3D_MAPPLANES; ++plane)
	{
		pos = levelset->mapheaderseg[mapnum].planestart[plane];
		compressed = levelset->mapheaderseg[mapnum].planelength[plane];
		dest = levelset->mapsegs[mapnum][plane];
		if (fseek(levelset->file, pos, SEEK_SET))
		{
			return wolf3d_LoadFileBadFormat;
		}
		if (compressed <= BUFFERSIZE)
			source = (uint16_t*)bufferseg;
		else
		{
			bigbufferseg = malloc(compressed);
			if (!bigbufferseg)
				return wolf3d_LoadFileBadFormat;
			source = (uint16_t*)bigbufferseg;
		}
		if (fread(source, 1, compressed, levelset->file) != compressed)
		{
			free(bigbufferseg);
			return wolf3d_LoadFileBadFormat;
		}
		expanded = *source;
		++source;
		buffer2seg = malloc(expanded);
		if (!buffer2seg)
		{
			free(bigbufferseg);
			return wolf3d_LoadFileBadFormat;
		}

		CAL_CarmackExpand((uint8_t*)source, buffer2seg, expanded);
		CAL_RLEWexpand(buffer2seg + 1, dest, size, levelset->rlewTag);
		free(buffer2seg);
		free(bigbufferseg);
	}

	return wolf3d_LoadFileOk;
}

uint16_t* wolf3d_LevelSetGetMap(wolf3d_LevelSet* levelset, unsigned mapnum, unsigned plane)
{
	if (mapnum >= WOLF3D_NUMMAPS || plane >= WOLF3D_MAPPLANES)
		return NULL;
	return levelset->mapsegs[mapnum][plane];
}

void wolf3d_LevelSetCloseFile(wolf3d_LevelSet* pointer)
{
	if (pointer->file)
		fclose(pointer->file);
	pointer->file = NULL;
}

void wolf3d_LevelSetDestroy(wolf3d_LevelSet* pointer)
{
	if (!pointer)
		return;
	if (pointer->file)
		fclose(pointer->file);
	free(pointer);
}