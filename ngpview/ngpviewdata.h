/***************************************************************************

 Copyright (C) 2007  Sergey Prokhorchuk

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

***************************************************************************/

#ifndef __NGPVIEWDATA_H__
#define __NGPVIEWDATA_H__

#include <ngput/p3dglext.h>

#include <vector>

typedef struct
 {
  /* Mesh parameters */

  void            *PosBuffer;
  void            *NormalBuffer;
  void            *TexCoordBuffer;
  unsigned int    *IndexBuffer;
  unsigned int     IndexCount;

  /* Material parameters */

  float            R,G,B;
  GLuint           TexHandle;
  bool             DoubleSided;
  bool             Transparent;
  bool             Billboard;
  unsigned int     BillboardMode;

  /* Billboard parameters (used only if Billboard is true)*/

  float            BillboardWidth;
  float            BillboardHeight;
 } NGPViewSubMeshData;

class NGPViewMeshData
 {
  public           :

                   NGPViewMeshData    (unsigned int        GroupCount);
                  ~NGPViewMeshData    ();

  float                                BBoxMin[3];
  float                                BBoxMax[3];
  std::vector<NGPViewSubMeshData>      SubMeshes;
 };

#endif

