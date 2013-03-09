/***************************************************************************

 Copyright (C) 2013  Sergey Prokhorchuk

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
 }

#include <ngput/p3dospath.h>

#include <p3dplugluactl.h>
#include <p3dplugluafs.h>

static int         OSPathJoin         (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Path1;
  const char                          *Path2;

  Path1 = Control.GetArgString(1);
  Path2 = Control.GetArgString(2);

  Control.Commit();

  std::string Path = P3DPathName::JoinPaths(Path1,Path2);

  Control.PushString(Path.c_str());

  Control.Commit();

  return(1);
 }

static int         OSPathBaseName     (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Path;

  Path = Control.GetArgString(1);

  Control.Commit();

  std::string BaseName = P3DPathName::BaseName(Path);

  Control.PushString(BaseName.c_str());

  Control.Commit();

  return(1);
 }

static int         OSFileExists       (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *FileName;
  struct stat                          FileStat;

  FileName = Control.GetArgString(1);

  Control.Commit();

  Control.PushBool(stat(FileName,&FileStat) == 0);

  Control.Commit();

  return(1);
 }

#define FileCopyBufferSize (16 * 1024)

static bool        CopyFile           (const char         *SrcFileName,
                                       const char         *DstFileName)
 {
  bool   Ok = false;
  FILE  *SrcFile;
  FILE  *DstFile;
  void  *Buf;

  Buf = malloc(FileCopyBufferSize);

  if (Buf != NULL)
   {
    SrcFile = fopen(SrcFileName,"rb");

    if (SrcFile != NULL)
     {
      DstFile = fopen(DstFileName,"wb");

      if (DstFile != NULL)
       {
        Ok = true;

        size_t N;

        while ((N = fread(Buf,1,FileCopyBufferSize,SrcFile)) > 0 && Ok)
         {
          if (fwrite(Buf,1,N,DstFile) < N)
           {
            Ok = false;
           }
         }

        Ok = Ok && (!ferror(SrcFile));

        fclose(DstFile);
       }

      fclose(SrcFile);
     }

    free(Buf);
   }

  return Ok;
 }

static int         OSFileCopy         (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *SrcFileName;
  const char                          *DstFileName;

  SrcFileName = Control.GetArgString(1);
  DstFileName = Control.GetArgString(2);

  Control.Commit();

  Control.PushBool(CopyFile(SrcFileName,DstFileName));

  Control.Commit();

  return(1);
 }


extern void        P3DPlugLuaRegisterFS
                                      (lua_State          *State)
 {
  lua_register(State,"OSPathJoin",OSPathJoin);
  lua_register(State,"OSPathBaseName",OSPathBaseName);
  lua_register(State,"OSFileExists",OSFileExists);
  lua_register(State,"OSFileCopy",OSFileCopy);
 }

