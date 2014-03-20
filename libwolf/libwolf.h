#ifndef LIBWOLF_H_
#define LIBWOLF_H_

#include <stdint.h>

#ifdef _WIN32
#ifdef BUILD_DLL
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	WOLF3D_NUMMAPS = 60,
	WOLF3D_MAPPLANES = 2,
	WOLF3D_MAPSIZE = 64,
	WOLF3D_MAPAREA = WOLF3D_MAPSIZE * WOLF3D_MAPSIZE,
};

typedef enum wolf3d_LoadFileResult
{
	wolf3d_LoadFileOk,
	wolf3d_LoadFileCannotOpen,
	wolf3d_LoadFileBadFormat,
} wolf3d_LoadFileResult;

typedef struct wolf3d_LevelSet *wolf3d_LevelSetRef;

DECLSPEC wolf3d_LevelSetRef wolf3d_LevelSetCreate(void);
DECLSPEC wolf3d_LoadFileResult wolf3d_LevelSetOpenFile(wolf3d_LevelSetRef levelset, const char* mapheadpath, const char* gamemapspath);
DECLSPEC wolf3d_LoadFileResult wolf3d_LevelSetLoadMap(wolf3d_LevelSetRef levelset, unsigned mapnum);
DECLSPEC uint16_t* wolf3d_LevelSetGetMap(wolf3d_LevelSetRef levelset, unsigned mapnum, unsigned plane);
DECLSPEC void wolf3d_LevelSetCloseFile(wolf3d_LevelSetRef pointer);
DECLSPEC void wolf3d_LevelSetDestroy(wolf3d_LevelSetRef pointer);

#ifdef __cplusplus
}
#endif

#endif