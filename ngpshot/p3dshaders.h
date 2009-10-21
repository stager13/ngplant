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

/* FIXME: Very similar to ngplant/p3dshaders module */

#include <vector>

#include <ngput/p3dglext.h>

typedef struct
 {
  GLhandleARB      ProgramHandle;
  bool             HaveDiffuseTex;
  bool             HaveNormalMap;
  bool             TwoSided;
 } P3DShaderEntry;

class P3DShaderLoader
 {
  public           :

                   P3DShaderLoader    (const char         *VertexProgramSrc,
                                       const char         *FragmentProgramSrc);
                  ~P3DShaderLoader    ();

  GLhandleARB      GetProgramHandle   (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const;

  private          :

  GLhandleARB      FindByProps        (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const;

  mutable
  std::vector<P3DShaderEntry>          ShaderSet;

  const char                          *VertexProgramSrc;
  const char                          *FragmentProgramSrc;
 };

#endif

