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

#include <stdio.h>
#include <stdlib.h>

#include <ngpcore/p3dhli.h>

#include <ngpviewdata.h>

                   NGPViewMeshData::NGPViewMeshData
                                      (unsigned int        GroupCount)
 {
  unsigned int                         GroupIndex;
  NGPViewSubMeshData                   SubMesh;

  SubMesh.PosBuffer      = 0;
  SubMesh.NormalBuffer   = 0;
  SubMesh.TexCoordBuffer = 0;
  SubMesh.IndexBuffer    = 0;

  SubMesh.R = 0.0f;
  SubMesh.G = 0.0f;
  SubMesh.B = 0.0f;

  SubMesh.TexHandle     = 0;
  SubMesh.DoubleSided   = false;
  SubMesh.Transparent   = false;
  SubMesh.Billboard     = false;
  SubMesh.BillboardMode = P3D_BILLBOARD_MODE_NONE;

  SubMesh.BillboardWidth  = 0.0f;
  SubMesh.BillboardHeight = 0.0f;

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    SubMeshes.push_back(SubMesh);
   }

  BBoxMin[0] = BBoxMin[1] = BBoxMin[2] = 0.0f;
  BBoxMax[0] = BBoxMax[1] = BBoxMax[2] = 0.0f;
 }

                   NGPViewMeshData::~NGPViewMeshData
                                      ()
 {
  std::vector<NGPViewSubMeshData>::iterator
                                       Iter = SubMeshes.begin();

  while (Iter != SubMeshes.end())
   {
    if (Iter->PosBuffer != 0)
     {
      free(Iter->PosBuffer);
     }

    if (Iter->NormalBuffer != 0)
     {
      free(Iter->NormalBuffer);
     }

    if (Iter->TexCoordBuffer != 0)
     {
      free(Iter->TexCoordBuffer);
     }

    if (Iter->IndexBuffer != 0)
     {
      free(Iter->IndexBuffer);
     }

    ++Iter;
   }
 }

