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

#ifndef __P3DPOBJECT_H__
#define __P3DPOBJECT_H__

#include <ngpcore/p3dhli.h>

#include <p3dshaders.h>

typedef struct
 {
  float            R,G,B;
  P3DTexHandle     DiffuseTexHandle;
  P3DTexHandle     NormalMapHandle;
  P3DShaderHandle  ShaderHandle;
  bool             TwoSided;
  bool             Transparent;
  unsigned int     BillboardMode;
  bool             AlphaCtrlEnabled;
  float            AlphaFadeIn;
  float            AlphaFadeOut;

  GLint            BiNormalLocation;

  bool             Hidden;
 } P3DMaterialData;

class P3DBranchGroupObject
 {
  public           :

                   P3DBranchGroupObject
                                      (const P3DHLIPlantTemplate
                                                          *Template,
                                       const P3DHLIPlantInstance
                                                          *Instance,
                                       unsigned int        GroupIndex,
                                       unsigned int        BranchCount,
                                       bool                Hidden,
                                       bool                UseColorArray);

                  ~P3DBranchGroupObject
                                      ();

  void             Render             (bool                Selected) const;
  void             InvalidateCamera   ();

  unsigned int     GetVertexCount     () const;
  unsigned int     GetTriangleCount   () const;

  friend class     P3DPlantObject;

  private          :

  float            CalcAlphaTestValue (float               LODLevel) const;

  void             RenderGroup        () const;
  void             RenderSelection    () const;

  float           *PosBuffer;
  float           *NormalBuffer;
  float           *BiNormalBuffer;
  float           *TexCoordBuffer;
  float           *CenterPosBuffer;
  float           *ColorBuffer;

  unsigned int    *IndexBuffer;

  unsigned int     TotalIndexCount;
  unsigned int     BranchCount;

  P3DMaterialData  MaterialData;

  float            BillboardWidth;
  float            BillboardHeight;
  float            BillboardNormal[3];

  bool             LODVisRangeEnabled;
  float            LODVisRangeMinLOD;
  float            LODVisRangeMaxLOD;

  unsigned int     VertexCount;
  unsigned int     TriangleCount;
 };

class P3DPlantObject
 {
  public           :

                   P3DPlantObject     (const P3DPlantModel*PlantModel,
                                       bool                UseColorArray);
                  ~P3DPlantObject     ();

  void             InvalidateCamera   ();
  void             Render             (const P3DPlantModel*PlantModel,
                                       bool                HighlightSelection) const;

  unsigned int     GetGroupVertexCount(unsigned int        GroupIndex) const;
  unsigned int     GetGroupTriangleCount
                                      (unsigned int        GroupIndex) const;

  unsigned int     GetTotalVertexCount() const;
  unsigned int     GetTotalTriangleCount
                                      () const;

  private          :

  unsigned int                         GroupCount;
  P3DBranchGroupObject               **Groups;

  mutable bool                         CameraModified;

  unsigned int                         TotalVertexCount;
  unsigned int                         TotalTriangleCount;
 };

#endif

