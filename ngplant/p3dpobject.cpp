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

#include <stdlib.h>

#ifdef P3D_TIMINGS_ENABLED
 #include <time.h>
#endif

#include <stdexcept>

#include <ngput/p3dglext.h>

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dhli.h>

#include <p3dapp.h>
#include <p3dpobject.h>

static void       *P3DMallocEx        (size_t              Size)
 {
  void            *Result;

  Result = malloc(Size);

  if (Result == 0)
   {
    throw std::bad_alloc();
   }

  return(Result);
 }

static void        UpdateBillboardsInfo
                                      (float              *PosBuffer,
                                       const float        *CenterPosBuffer,
                                       float              *BillboardNormal,
                                       unsigned int        BillboardMode,
                                       float               BillboardWidth,
                                       float               BillboardHeight,
                                       unsigned int        BranchCount)
 {
  GLfloat                              ModelViewMatrix[16];
  float                                HalfWidth;
  float                                HalfHeight;
  float                                Up[3];
  float                                Right[3];

  glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);

  HalfWidth  = BillboardWidth  * 0.5f;
  HalfHeight = BillboardHeight * 0.5f;

  Right[0] = ModelViewMatrix[0] * HalfWidth;
  Right[1] = ModelViewMatrix[4] * HalfWidth;
  Right[2] = ModelViewMatrix[8] * HalfWidth;

  Up[1] = ModelViewMatrix[5] * HalfHeight;

  if (BillboardMode == P3D_BILLBOARD_MODE_CYLINDRICAL)
   {
    Up[0] = Up[2] = 0.0f;
   }
  else
   {
    Up[0] = ModelViewMatrix[1] * HalfHeight;
    Up[2] = ModelViewMatrix[9] * HalfHeight;
   }

  BillboardNormal[0] = ModelViewMatrix[2];
  BillboardNormal[1] = ModelViewMatrix[6];
  BillboardNormal[2] = ModelViewMatrix[10];

  for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
   {
    PosBuffer[0] = CenterPosBuffer[0] - Up[0] - Right[0];
    PosBuffer[1] = CenterPosBuffer[1] - Up[1] - Right[1];
    PosBuffer[2] = CenterPosBuffer[2] - Up[2] - Right[2];

    PosBuffer += 3; CenterPosBuffer += 3;

    PosBuffer[0] = CenterPosBuffer[0] - Up[0] + Right[0];
    PosBuffer[1] = CenterPosBuffer[1] - Up[1] + Right[1];
    PosBuffer[2] = CenterPosBuffer[2] - Up[2] + Right[2];

    PosBuffer += 3; CenterPosBuffer += 3;

    PosBuffer[0] = CenterPosBuffer[0] + Up[0] - Right[0];
    PosBuffer[1] = CenterPosBuffer[1] + Up[1] - Right[1];
    PosBuffer[2] = CenterPosBuffer[2] + Up[2] - Right[2];

    PosBuffer += 3; CenterPosBuffer += 3;

    PosBuffer[0] = CenterPosBuffer[0] + Up[0] + Right[0];
    PosBuffer[1] = CenterPosBuffer[1] + Up[1] + Right[1];
    PosBuffer[2] = CenterPosBuffer[2] + Up[2] + Right[2];

    PosBuffer += 3; CenterPosBuffer += 3;
   }
 }


                   P3DBranchGroupObject::P3DBranchGroupObject
                                      (const P3DHLIPlantTemplate
                                                          *Template,
                                       const P3DHLIPlantInstance
                                                          *Instance,
                                       unsigned int        GroupIndex,
                                       unsigned int        BranchCount,
                                       bool                Hidden,
                                       bool                UseColorArray)
 {
  const P3DMaterialDef                *MaterialDef;
  unsigned int                         BranchVAttrCount;
  unsigned int                         BranchIndexCount;
  unsigned int                         TotalVAttrCount;

  PosBuffer       = 0;
  NormalBuffer    = 0;
  BiNormalBuffer  = 0;
  TexCoordBuffer  = 0;
  IndexBuffer     = 0;
  CenterPosBuffer = 0;
  ColorBuffer     = 0;

  TotalIndexCount = 0;

  this->BranchCount = BranchCount;

  MaterialDef = Template->GetMaterial(GroupIndex);

  MaterialDef->GetColor(&MaterialData.R,&MaterialData.G,&MaterialData.B);

  MaterialData.TwoSided         = MaterialDef->IsDoubleSided();
  MaterialData.Transparent      = MaterialDef->IsTransparent();
  MaterialData.BillboardMode    = MaterialDef->GetBillboardMode();
  MaterialData.AlphaCtrlEnabled = MaterialDef->IsAlphaCtrlEnabled();
  MaterialData.AlphaFadeIn      = MaterialDef->GetAlphaFadeIn();
  MaterialData.AlphaFadeOut     = MaterialDef->GetAlphaFadeOut();

  if (MaterialDef->GetTexName(P3D_TEX_DIFFUSE) != 0)
   {
    MaterialData.DiffuseTexHandle =
     P3DApp::GetApp()->GetTexManager()->GetHandleByGenericName
      (MaterialDef->GetTexName(P3D_TEX_DIFFUSE));
   }
  else
   {
    MaterialData.DiffuseTexHandle = P3DTexHandleNULL;
   }

  if (MaterialDef->GetTexName(P3D_TEX_NORMAL_MAP) != 0)
   {
    MaterialData.NormalMapHandle =
     P3DApp::GetApp()->GetTexManager()->GetHandleByGenericName
      (MaterialDef->GetTexName(P3D_TEX_NORMAL_MAP));
   }
  else
   {
    MaterialData.NormalMapHandle = P3DTexHandleNULL;
   }

  MaterialData.ShaderHandle =
   P3DApp::GetApp()->GetShaderManager()->GenShader
    (MaterialData.DiffuseTexHandle != P3DTexHandleNULL,
     MaterialData.NormalMapHandle != P3DTexHandleNULL,
     MaterialData.TwoSided);

  MaterialData.BiNormalLocation = -1;

  if (MaterialData.ShaderHandle != P3DShaderHandleNULL)
   {
    GLhandleARB    ProgHandle;

    ProgHandle = P3DApp::GetApp()->GetShaderManager()->GetProgramHandle
                  (MaterialData.ShaderHandle);

    if (ProgHandle != 0)
     {
      MaterialData.BiNormalLocation = glGetAttribLocationARB(ProgHandle,"ngp_BiNormal");
     }
   }

  MaterialData.Hidden = Hidden;

  LODVisRangeEnabled = Template->IsLODVisRangeEnabled(GroupIndex);
  Template->GetLODVisRange(&LODVisRangeMinLOD,&LODVisRangeMaxLOD,GroupIndex);

  BillboardWidth = BillboardHeight = 0.0f;

  if (BranchCount == 0)
   {
    VertexCount   = 0;
    TriangleCount = 0;

    return;
   }

  BranchVAttrCount = Template->GetVAttrCountI(GroupIndex);
  BranchIndexCount = Template->GetIndexCount(GroupIndex,P3D_TRIANGLE_LIST);
  TotalVAttrCount  = BranchVAttrCount * BranchCount;
  TotalIndexCount  = BranchIndexCount * BranchCount;

  TriangleCount = TotalIndexCount / 3;
  VertexCount   = Template->GetVAttrCount(GroupIndex,P3D_ATTR_VERTEX) * BranchCount;

  try
   {
    PosBuffer      = (float*)P3DMallocEx(sizeof(float) * 3 * TotalVAttrCount);
    NormalBuffer   = (float*)P3DMallocEx(sizeof(float) * 3 * TotalVAttrCount);
    TexCoordBuffer = (float*)P3DMallocEx(sizeof(float) * 2 * TotalVAttrCount);

    if (MaterialData.BiNormalLocation != -1)
     {
      BiNormalBuffer = (float*)P3DMallocEx(sizeof(float) * 3 * TotalVAttrCount);
     }

    IndexBuffer = (unsigned int*)P3DMallocEx(sizeof(unsigned int) * TotalIndexCount);

    if (MaterialData.BillboardMode != P3D_BILLBOARD_MODE_NONE)
     {
      CenterPosBuffer = (float*)P3DMallocEx(sizeof(float) * 3 * TotalVAttrCount);

      Template->GetBillboardSize(&BillboardWidth,&BillboardHeight,GroupIndex);
     }

    if (UseColorArray)
     {
      ColorBuffer = (float*)P3DMallocEx(sizeof(float) * 3 * TotalVAttrCount);

      for (unsigned int VAttrIndex = 0; VAttrIndex < TotalVAttrCount; VAttrIndex++)
       {
        ColorBuffer[VAttrIndex * 3]     = MaterialData.R;
        ColorBuffer[VAttrIndex * 3 + 1] = MaterialData.G;
        ColorBuffer[VAttrIndex * 3 + 2] = MaterialData.B;
       }
     }

    void            *Data      = IndexBuffer;
    unsigned int     IndexBase = 0;

    for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
     {
      Template->FillIndexBuffer
       (Data,GroupIndex,P3D_TRIANGLE_LIST,P3D_UNSIGNED_INT,IndexBase);

      IndexBase += BranchVAttrCount;

      Data = ((char*)Data) + BranchIndexCount * sizeof(unsigned int);
     }
   }
  catch (...)
   {
    free(ColorBuffer);
    free(CenterPosBuffer);
    free(IndexBuffer);
    free(TexCoordBuffer);
    free(BiNormalBuffer);
    free(NormalBuffer);
    free(PosBuffer);

    throw;
   }
 }


                   P3DBranchGroupObject::~P3DBranchGroupObject
                                      ()
 {
  if (MaterialData.ShaderHandle != P3DShaderHandleNULL)
   {
    P3DApp::GetApp()->GetShaderManager()->FreeShader(MaterialData.ShaderHandle);
   }

  if (MaterialData.DiffuseTexHandle != P3DTexHandleNULL)
   {
    P3DApp::GetApp()->GetTexManager()->FreeTexture(MaterialData.DiffuseTexHandle);
   }

  if (MaterialData.NormalMapHandle != P3DTexHandleNULL)
   {
    P3DApp::GetApp()->GetTexManager()->FreeTexture(MaterialData.NormalMapHandle);
   }

  free(ColorBuffer);
  free(CenterPosBuffer);
  free(IndexBuffer);
  free(TexCoordBuffer);
  free(BiNormalBuffer);
  free(NormalBuffer);
  free(PosBuffer);
 }

float              P3DBranchGroupObject::CalcAlphaTestValue
                                      (float               LODLevel) const
 {
  const float      DefaultTestValue = 0.5f;
  float            AlphaTestValue;
  float            VisLODRange;
  float            LODFraction;

  if (!MaterialData.AlphaCtrlEnabled) return DefaultTestValue;

  VisLODRange = LODVisRangeMaxLOD - LODVisRangeMinLOD;

  if (VisLODRange < 0.001f) return DefaultTestValue;

  LODFraction = (LODLevel - LODVisRangeMinLOD) / VisLODRange;

  if (MaterialData.AlphaFadeOut < MaterialData.AlphaFadeIn)
   {
    if      (LODFraction < MaterialData.AlphaFadeOut)
     {
      AlphaTestValue = LODFraction / MaterialData.AlphaFadeOut;
     }
    else if (LODFraction > MaterialData.AlphaFadeIn)
     {
      AlphaTestValue = 1.0f - (LODFraction - MaterialData.AlphaFadeIn) / (1.0f - MaterialData.AlphaFadeIn);
     }
    else
     {
      AlphaTestValue = 0.99f;
     }
   }
  else
   {
    if      (LODFraction < MaterialData.AlphaFadeIn)
     {
      AlphaTestValue = 1.0f - LODFraction / MaterialData.AlphaFadeIn;
     }
    else if (LODFraction > MaterialData.AlphaFadeOut)
     {
      AlphaTestValue = (LODFraction - MaterialData.AlphaFadeOut) / (1.0f - MaterialData.AlphaFadeOut);
     }
    else
     {
      AlphaTestValue = 0.01f;
     }
   }

  return(AlphaTestValue);
 }

void               P3DBranchGroupObject::Render
                                      () const
 {
  if (PosBuffer == 0)      return;
  if (MaterialData.Hidden) return;

  float LODLevel;

  LODLevel = P3DApp::GetApp()->GetLODLevel();

  if ((LODVisRangeEnabled) &&
      ((LODLevel < LODVisRangeMinLOD) || (LODLevel > LODVisRangeMaxLOD)))
   return;

  /* material setup */

  if (ColorBuffer == 0)
   {
    glColor3f(MaterialData.R,MaterialData.G,MaterialData.B);
   }

  if (MaterialData.TwoSided)
   {
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
   }
  else
   {
    glEnable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
   }

  if (MaterialData.Transparent)
   {
    float          AlphaTestValue;

    /*FIXME: need more generic way to get current LOD level */
    AlphaTestValue = CalcAlphaTestValue(P3DApp::GetApp()->GetLODLevel());

    glAlphaFunc(GL_GREATER,AlphaTestValue);
    glEnable(GL_ALPHA_TEST);
   }
  else
   {
    glDisable(GL_ALPHA_TEST);
   }

  if (MaterialData.DiffuseTexHandle == P3DTexHandleNULL)
   {
    glDisable(GL_TEXTURE_2D);
   }
  else
   {
    glEnable(GL_TEXTURE_2D);

    glBindTexture
     (GL_TEXTURE_2D,P3DApp::GetApp()->GetTexManager()->GetGLHandle(MaterialData.DiffuseTexHandle));
   }

  GLhandleARB      ProgHandle;

  ProgHandle = 0;

  if (MaterialData.ShaderHandle != P3DShaderHandleNULL)
   {
    ProgHandle = P3DApp::GetApp()->GetShaderManager()->GetProgramHandle
                  (MaterialData.ShaderHandle);

    if (ProgHandle != 0)
     {
      glUseProgramObjectARB(ProgHandle);

      glEnable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
     }

    if (MaterialData.NormalMapHandle != P3DTexHandleNULL)
     {
      glActiveTexture(GL_TEXTURE1);
      glEnable(GL_TEXTURE_2D);

      glBindTexture
       (GL_TEXTURE_2D,P3DApp::GetApp()->GetTexManager()->GetGLHandle(MaterialData.NormalMapHandle));
      glActiveTexture(GL_TEXTURE0);
     }
   }

  /* verts setup */

  glVertexPointer(3,GL_FLOAT,0,PosBuffer);
  glEnableClientState(GL_VERTEX_ARRAY);

  if (MaterialData.BillboardMode == P3D_BILLBOARD_MODE_NONE)
   {
    glNormalPointer(GL_FLOAT,0,NormalBuffer);
    glEnableClientState(GL_NORMAL_ARRAY);
   }
  else
   {
    glNormal3fv(BillboardNormal);
   }

  glTexCoordPointer(2,GL_FLOAT,0,TexCoordBuffer);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  if (MaterialData.BiNormalLocation != -1)
   {
    glEnableVertexAttribArrayARB(MaterialData.BiNormalLocation);
    glVertexAttribPointerARB(MaterialData.BiNormalLocation,3,GL_FLOAT,0,GL_FALSE,BiNormalBuffer);
   }

  if (ColorBuffer != 0)
   {
    glColorPointer(3,GL_FLOAT,0,ColorBuffer);
    glEnableClientState(GL_COLOR_ARRAY);
   }

  glDrawElements(GL_TRIANGLES,TotalIndexCount,GL_UNSIGNED_INT,IndexBuffer);

  if (ColorBuffer != 0)
   {
    glDisableClientState(GL_COLOR_ARRAY);
   }

  if (MaterialData.BiNormalLocation != -1)
   {
    glDisableVertexAttribArrayARB(MaterialData.BiNormalLocation);
   }

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if (MaterialData.BillboardMode == P3D_BILLBOARD_MODE_NONE)
   {
    glDisableClientState(GL_NORMAL_ARRAY);
   }

  glDisableClientState(GL_VERTEX_ARRAY);

  if (ProgHandle != 0)
   {
    glUseProgramObjectARB(0);

    glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
   }

  if (MaterialData.ShaderHandle != P3DShaderHandleNULL)
   {
    if (MaterialData.NormalMapHandle != P3DTexHandleNULL)
     {
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D,0);
      glActiveTexture(GL_TEXTURE0);
     }
   }
 }

void               P3DBranchGroupObject::InvalidateCamera
                                      ()
 {
  if (PosBuffer == 0) return;
  if (MaterialData.BillboardMode == P3D_BILLBOARD_MODE_NONE) return;

  UpdateBillboardsInfo(PosBuffer,
                       CenterPosBuffer,
                       BillboardNormal,
                       MaterialData.BillboardMode,
                       BillboardWidth,
                       BillboardHeight,
                       BranchCount);
 }

unsigned int       P3DBranchGroupObject::GetVertexCount
                                      () const
 {
  return(VertexCount);
 }

unsigned int       P3DBranchGroupObject::GetTriangleCount
                                      () const
 {
  return(TriangleCount);
 }

                   P3DPlantObject::P3DPlantObject
                                      (const P3DPlantModel*PlantModel,
                                       bool                UseColorArray)
 {
  P3DHLIPlantTemplate                  Template(PlantModel);
  P3DHLIPlantInstance                 *Instance;
  unsigned int                         GroupIndex;

  #ifdef P3D_TIMINGS_ENABLED
  clock_t                              StartTime;
  clock_t                              EndTime;

  StartTime = clock();
  #endif

  CameraModified = false;
  TotalVertexCount   = 0;
  TotalTriangleCount = 0;

  Template.SetDummiesEnabled(P3DApp::GetApp()->IsDummyVisible());

  GroupCount = Template.GetGroupCount();

  if (GroupCount == 0)
   {
    Groups = 0;

    return;
   }
  else
   {
    Groups = (P3DBranchGroupObject**)P3DMallocEx(sizeof(P3DBranchGroupObject*) * GroupCount);
   }

  unsigned int    *BranchCounts;

  BranchCounts = 0;

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    Groups[GroupIndex] = 0;
   }

  Instance = 0;

  try
   {
    Instance = Template.CreateInstance();

    BranchCounts = new unsigned int[GroupCount];

    Instance->GetBranchCountMulti(BranchCounts);

    for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
     {
      bool                             Hidden;
      const P3DBranchModel            *BranchModel;

      Hidden = true;

      BranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,GroupIndex);

      if (BranchModel != 0)
       {
        const P3DMaterialInstanceSimple *MaterialInstance;

        MaterialInstance = dynamic_cast<const P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

        if (MaterialInstance != 0)
         {
          Hidden = MaterialInstance->IsHidden();
         }
       }

      Groups[GroupIndex] = new P3DBranchGroupObject
                                (&Template,
                                  Instance,
                                  GroupIndex,
                                  BranchCounts[GroupIndex],
                                  Hidden,
                                  UseColorArray);

      TotalVertexCount   += Groups[GroupIndex]->GetVertexCount();
      TotalTriangleCount += Groups[GroupIndex]->GetTriangleCount();
     }

    P3DHLIVAttrBufferSet            *Buffers = 0;

    try
     {
      Buffers = new P3DHLIVAttrBufferSet[GroupCount];

      for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
       {
        for (unsigned int AttrIndex = 0; AttrIndex < P3D_MAX_ATTRS; AttrIndex++)
         {
          Buffers[GroupIndex][AttrIndex] = 0;
         }

        if (Groups[GroupIndex]->MaterialData.BillboardMode == P3D_BILLBOARD_MODE_NONE)
         {
          Buffers[GroupIndex][P3D_ATTR_VERTEX]        = Groups[GroupIndex]->PosBuffer;
          Buffers[GroupIndex][P3D_ATTR_BILLBOARD_POS] = 0;
         }
        else
         {
          Buffers[GroupIndex][P3D_ATTR_VERTEX]        = 0;
          Buffers[GroupIndex][P3D_ATTR_BILLBOARD_POS] = Groups[GroupIndex]->CenterPosBuffer;
         }

        Buffers[GroupIndex][P3D_ATTR_NORMAL]    = Groups[GroupIndex]->NormalBuffer;
        Buffers[GroupIndex][P3D_ATTR_TEXCOORD0] = Groups[GroupIndex]->TexCoordBuffer;

        if (Groups[GroupIndex]->MaterialData.BiNormalLocation != -1)
         {
          Buffers[GroupIndex][P3D_ATTR_BINORMAL] = Groups[GroupIndex]->BiNormalBuffer;
         }
       }

      Instance->FillVAttrBuffersIMulti(Buffers);

      for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
       {
        if (Groups[GroupIndex]->MaterialData.BillboardMode != P3D_BILLBOARD_MODE_NONE)
         {
          UpdateBillboardsInfo
           (Groups[GroupIndex]->PosBuffer,
            Groups[GroupIndex]->CenterPosBuffer,
            Groups[GroupIndex]->BillboardNormal,
            Groups[GroupIndex]->MaterialData.BillboardMode,
            Groups[GroupIndex]->BillboardWidth,
            Groups[GroupIndex]->BillboardHeight,
            Groups[GroupIndex]->BranchCount);
         }
       }
     }
    catch (...)
     {
      delete Buffers;

      throw;
     }

    delete Buffers;
   }
  catch (...)
   {
    for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
     {
      delete Groups[GroupIndex];
     }

    free(Groups);

    delete BranchCounts;
    delete Instance;

    throw;
   }

  delete BranchCounts;
  delete Instance;

  #ifdef P3D_TIMINGS_ENABLED
  EndTime = clock();

  double TimeUsed = ((double)(EndTime - StartTime)) / CLOCKS_PER_SEC;

  printf("gen. time: %.04f\n",TimeUsed);
  #endif
 }

                   P3DPlantObject::~P3DPlantObject
                                      ()
 {
  for (unsigned int GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    delete Groups[GroupIndex];
   }

  free(Groups);
 }

void               P3DPlantObject::InvalidateCamera
                                      ()
 {
  CameraModified = true;
 }

void               P3DPlantObject::Render
                                      () const
 {
  unsigned int     GroupIndex;

  if (CameraModified)
   {
    for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
     {
      Groups[GroupIndex]->InvalidateCamera();
     }

    CameraModified = false;
   }

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    Groups[GroupIndex]->Render();
   }
 }

unsigned int       P3DPlantObject::GetGroupVertexCount
                                      (unsigned int        GroupIndex) const
 {
  if (GroupIndex < GroupCount)
   {
    return(Groups[GroupIndex]->GetVertexCount());
   }
  else
   {
    return(0);
   }
 }

unsigned int       P3DPlantObject::GetGroupTriangleCount
                                      (unsigned int        GroupIndex) const
 {
  if (GroupIndex < GroupCount)
   {
    return(Groups[GroupIndex]->GetTriangleCount());
   }
  else
   {
    return(0);
   }
 }

unsigned int       P3DPlantObject::GetTotalVertexCount
                                      () const
 {
  return(TotalVertexCount);
 }

unsigned int       P3DPlantObject::GetTotalTriangleCount
                                      () const
 {
  return(TotalTriangleCount);
 }

