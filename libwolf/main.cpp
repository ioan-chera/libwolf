// libwolf.cpp : Defines the entry point for the console application.
//

#if 0

#include "stdafx.h"
#include "libwolf.h"

int _tmain(int argc, _TCHAR* argv[])
{
	wolf3d_LevelSetRef set = wolf3d_LevelSetCreate();

	wolf3d_LoadFileResult result = wolf3d_LevelSetOpenFile(set, "c:\\users\\ioan_chera\\dos\\wolf3d\\maphead.wl6", "c:\\users\\ioan_chera\\dos\\wolf3d\\gamemaps.wl6");
	if (result)
	{
		printf("%d\n", result);
		return result;
	}
	wolf3d_LevelSetLoadMap(set, 44);
	uint16_t* map = wolf3d_LevelSetGetMap(set, 44, 0);

	for (unsigned u = 0; u < WOLF3D_MAPAREA; ++u)
		printf("%2x", map[u]);

	wolf3d_LevelSetDestroy(set);

	getchar();

	return 0;
}

#endif