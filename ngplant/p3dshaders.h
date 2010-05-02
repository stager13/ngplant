/***************************************************************************

 Copyright (C) 2007  Sergey Prokhorchuk

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

#ifndef __P3DSHADERS_H__
#define __P3DSHADERS_H__

#include <vector>


#include <ngput/p3dglext.h>

typedef unsigned int     P3DShaderHandle;

#define P3DShaderHandleNULL (0)

typedef struct
 {
  unsigned int     RefCount;
  bool             ProgramReady;
  GLhandleARB      ProgramHandle;
  bool             HaveDiffuseTex;
  bool             HaveNormalMap;
  bool             TwoSided;
 } P3DShaderManagerEntry;

class P3DShaderManager
 {
  public           :

                   P3DShaderManager   ();
                  ~P3DShaderManager   ();

  P3DShaderHandle  GenShader          (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided);
  void             FreeShader         (P3DShaderHandle     ShaderHandle);

  GLhandleARB      GetProgramHandle   (P3DShaderHandle     ShaderHandle) const;

  void             DisableShaders     ();

  private          :

  P3DShaderHandle  GetUnusedSlot      ();
  P3DShaderHandle  FindByProps        (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const;

  bool             ShadersEnabled;

  mutable
  std::vector<P3DShaderManagerEntry>   ShaderSet;
 };

#endif

