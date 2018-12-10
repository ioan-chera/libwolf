/*
 LibWolf: Wolf3D utility library
 Copyright (C) 2018  Ioan Chera

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef libwolf_hpp
#define libwolf_hpp

#include "libwolf.h"

namespace wolf3d
{
    //
    // C++ wrapper over the C base
    //
    class LevelSet
    {
    public:
        LevelSet()
        {
            mRef = wolf3d_LevelSetCreate();
            // don't care about malloc failure
        }
        wolf3d_LoadFileResult openFile(const char *mapheadpath, const char *gamemapspath)
        {
            return wolf3d_LevelSetOpenFile(mRef, mapheadpath, gamemapspath);
        }
        wolf3d_LoadFileResult openFileW(const wchar_t *mapheadpath, const wchar_t *gamemapspath)
        {
            return wolf3d_LevelSetOpenFileW(mRef, mapheadpath, gamemapspath);
        }
        uint16_t *getMap(unsigned mapnum, unsigned plane)
        {
            return wolf3d_LevelSetGetMap(mRef, mapnum, plane);
        }
        void closeFile()
        {
            wolf3d_LevelSetCloseFile(mRef);
        }
        ~LevelSet()
        {
            closeFile();
            wolf3d_LevelSetDestroy(mRef);
        }
    private:
        wolf3d_LevelSetRef mRef;
    };
}

#endif /* libwolf_h */
