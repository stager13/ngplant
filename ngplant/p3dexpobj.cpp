/***************************************************************************

 Copyright (C) 2006  Sergey Prokhorchuk

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

#include <stdio.h>
#include <stdarg.h>

#include <ngpcore/p3dhli.h>

#include <p3dexpobj.h>
#include <p3dapp.h>

static bool        FPrintFWrap        (FILE               *Stream,
                                       const char         *Format,
                                       ...)
 {
  bool                                 Result;
  va_list                              Args;

  va_start(Args,Format);

  if (vfprintf(Stream,Format,Args) < 0)
   {
    Result = false;
   }
  else
   {
    Result = true;
   }

  va_end(Args);
 }

static bool        IsGroupVisible     (const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       const P3DPlantModel*PlantModel,
                                       unsigned int        GroupIndex,
                                       bool                ExportHiddenGroups,
                                       bool                ExportOutVisRangeGroups,
                                       float               LODLevel)
 {
  bool             Visible;

  Visible = true;

  if (!ExportOutVisRangeGroups)
   {
    if (PlantTemplate->IsLODVisRangeEnabled(GroupIndex))
     {
      float        MinLOD;
      float        MaxLOD;

      PlantTemplate->GetLODVisRange(&MinLOD,&MaxLOD,GroupIndex);

      if ((LODLevel < MinLOD) || (LODLevel > MaxLOD))
       {
        Visible = false;
       }
     }
   }

  if (Visible)
   {
    if (!ExportHiddenGroups)
     {
      const P3DBranchModel  *BranchModel;

      Visible = false;

      BranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,GroupIndex);

      if (BranchModel != 0)
       {
        const P3DMaterialInstanceSimple *MaterialInstance;

        MaterialInstance = dynamic_cast<const P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

        if (MaterialInstance != 0)
         {
          Visible = !MaterialInstance->IsHidden();
         }
       }
     }
   }

  return(Visible);
 }

static unsigned int
                   PlantTemplateGetPrimitveVertexCount
                                      (const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       unsigned int        GroupIndex,
                                       unsigned int        PrimitiveIndex)
 {
  if (PlantTemplate->GetPrimitiveType(GroupIndex,PrimitiveIndex) == P3D_QUAD)
   {
    return(4);
   }
  else
   {
    return(3);
   }
 }

static bool        ExportVAttrs       (FILE               *TargetStream,
                                       const P3DHLIPlantInstance
                                                          *PlantInstance,
                                       unsigned int        GroupIndex,
                                       unsigned int        Attr,
                                       const char         *AttrStr)
 {
  bool             Result;
  unsigned int     AttrIndex;
  unsigned int     AttrCount;
  unsigned int     ElementCount;
  float           *Buffer;
  float           *AttrValue;

  Result    = true;
  AttrCount = PlantInstance->GetVAttrCount(GroupIndex,Attr);

  if (AttrCount == 0)
   {
    return(true);
   }

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    ElementCount = 2;
   }
  else
   {
    ElementCount = 3;
   }

  Buffer = (float*)malloc(AttrCount * ElementCount * sizeof(float));

  if (Buffer == 0)
   {
    return(false);
   }

  PlantInstance->FillVAttrBuffer(Buffer,GroupIndex,Attr);

  AttrIndex = 0;
  AttrValue = Buffer;

  while ((AttrIndex < AttrCount) && (Result))
   {
    if (ElementCount == 2)
     {
      Result = FPrintFWrap(TargetStream,"%s %f %f\n",AttrStr,AttrValue[0],AttrValue[1]);
     }
    else
     {
      Result = FPrintFWrap(TargetStream,"%s %f %f %f\n",AttrStr,AttrValue[0],AttrValue[1],AttrValue[2]);
     }

    AttrValue += ElementCount;

    AttrIndex++;
   }

  free(Buffer);

  return(Result);
 }

static bool        ExportObjData      (FILE               *TargetStream,
                                       FILE               *TargetMTLStream,
                                       const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       const P3DHLIPlantInstance
                                                          *PlantInstance,
 /*FIXME: need PlantModel only to determine if group is hidden */
                                       const P3DPlantModel*PlantModel,
                                       const char         *MTLFileName,
                                       bool                ExportHiddenGroups,
                                       bool                ExportOutVisRangeGroups,
                                       float               LODLevel)
 {
  bool             Result;

  Result = true;

  Result = FPrintFWrap(TargetStream,"o plant\n");

  if (Result)
   {
    Result = FPrintFWrap(TargetStream,"mtllib %s\n",MTLFileName);
   }

  if (Result)
   {
    unsigned int             GroupIndex;
    unsigned int             GroupCount;
    unsigned int             VertexIndexOffset;
    unsigned int             NormalIndexOffset;
    unsigned int             TexCoordIndexOffset;
    unsigned int             MatIndex;

    GroupIndex = 0;
    GroupCount = PlantTemplate->GetGroupCount();

    VertexIndexOffset   = 1;
    NormalIndexOffset   = 1;
    TexCoordIndexOffset = 1;
    MatIndex            = 1;

    while ((GroupIndex < GroupCount) && (Result))
     {
      if (IsGroupVisible(PlantTemplate,
                         PlantModel,
                         GroupIndex,
                         ExportHiddenGroups,
                         ExportOutVisRangeGroups,
                         LODLevel))
       {
        Result = FPrintFWrap(TargetMTLStream,"newmtl pmat%u\n",MatIndex);

        if (Result)
         {
          float              R;
          float              G;
          float              B;

          PlantTemplate->GetMaterial(GroupIndex)->GetColor(&R,&G,&B);

          Result = FPrintFWrap(TargetMTLStream,"Kd %f %f %f\n",R,G,B);
         }

        if (Result)
         {
          Result = FPrintFWrap(TargetMTLStream,"Ns 1\n");
         }

        if (Result)
         {
          Result = FPrintFWrap(TargetStream,"g bgroup%u\n",GroupIndex);
         }

        if (Result)
         {
          Result = FPrintFWrap(TargetStream,"usemtl pmat%u\n",MatIndex++);
         }

        if (Result)
         {
          std::string   TexFileName;
          const char   *TexName;

          TexName = PlantTemplate->GetMaterial(GroupIndex)->GetTexName(P3D_TEX_DIFFUSE);

          if (TexName == 0)
           {
            TexName = "off";
           }
          else
           {
            TexFileName = P3DApp::GetApp()->GetTexFS()->Generic2System(TexName);

            if (!TexFileName.empty())
             {
              TexName = TexFileName.c_str();
             }

            Result = FPrintFWrap(TargetMTLStream,"map_Kd %s\n",TexName);
           }

          if (Result)
           {
            Result = FPrintFWrap(TargetStream,"usemap %s\n",TexName);
           }
         }

        if (Result)
         {
          Result = ExportVAttrs(TargetStream,PlantInstance,GroupIndex,
                                P3D_ATTR_VERTEX,"v");
         }

        if (Result)
         {
          Result = ExportVAttrs(TargetStream,PlantInstance,GroupIndex,
                                P3D_ATTR_NORMAL,"vn");
         }

        if (Result)
         {
          Result = ExportVAttrs(TargetStream,PlantInstance,GroupIndex,
                                P3D_ATTR_TEXCOORD0,"vt");
         }

        if (Result)
         {
          unsigned int                 PrimitiveIndex;
          unsigned int                 PrimitiveCount;
          unsigned int                 VertexCount;
          unsigned int                 VertexIndex;

          PrimitiveCount = PlantTemplate->GetPrimitiveCount(GroupIndex);
          VertexCount    = 0;

          for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
           {
            VertexCount += PlantTemplateGetPrimitveVertexCount
                            (PlantTemplate,GroupIndex,PrimitiveIndex);
           }

          unsigned int      *PosIndexBuffer;
          unsigned int      *NormalIndexBuffer;
          unsigned int      *TexCoordIndexBuffer;

          PosIndexBuffer      = (unsigned int*)malloc(sizeof(unsigned int) * VertexCount);
          NormalIndexBuffer   = (unsigned int*)malloc(sizeof(unsigned int) * VertexCount);
          TexCoordIndexBuffer = (unsigned int*)malloc(sizeof(unsigned int) * VertexCount);

          if ((PosIndexBuffer == NULL) || (NormalIndexBuffer == NULL) || (TexCoordIndexBuffer == NULL))
           {
            Result = false;
           }

          if (Result)
           {
            PlantTemplate->FillVAttrIndexBuffer
             (PosIndexBuffer,GroupIndex,P3D_ATTR_VERTEX,P3D_UNSIGNED_INT,0);
            PlantTemplate->FillVAttrIndexBuffer
             (NormalIndexBuffer,GroupIndex,P3D_ATTR_NORMAL,P3D_UNSIGNED_INT,0);
            PlantTemplate->FillVAttrIndexBuffer
             (TexCoordIndexBuffer,GroupIndex,P3D_ATTR_TEXCOORD0,P3D_UNSIGNED_INT,0);

            unsigned int     BranchIndex;
            unsigned int     BranchCount;
            unsigned int     BufferOffset;

            BranchIndex = 0;
            BranchCount = PlantInstance->GetBranchCount(GroupIndex);

            while ((BranchIndex < BranchCount) && (Result))
             {
              PrimitiveIndex = 0;
              BufferOffset   = 0;

              while ((PrimitiveIndex < PrimitiveCount) && (Result))
               {
                Result = FPrintFWrap(TargetStream,"f");

                if (Result)
                 {
                  VertexCount = PlantTemplateGetPrimitveVertexCount
                                 (PlantTemplate,GroupIndex,PrimitiveIndex);

                  VertexIndex = 0;

                  while ((VertexIndex < VertexCount) && (Result))
                   {
                    Result = FPrintFWrap(TargetStream,
                                         " %u/%u/%u",
                                         PosIndexBuffer[BufferOffset] + VertexIndexOffset,
                                         TexCoordIndexBuffer[BufferOffset] + TexCoordIndexOffset,
                                         NormalIndexBuffer[BufferOffset] + NormalIndexOffset);

                    BufferOffset++;
                    VertexIndex++;
                   }
                 }

                if (Result)
                 {
                  Result = FPrintFWrap(TargetStream,"\n");
                 }

                PrimitiveIndex++;
               }

              BranchIndex++;

              VertexIndexOffset   += PlantTemplate->GetVAttrCount(GroupIndex,P3D_ATTR_VERTEX);
              NormalIndexOffset   += PlantTemplate->GetVAttrCount(GroupIndex,P3D_ATTR_NORMAL);
              TexCoordIndexOffset += PlantTemplate->GetVAttrCount(GroupIndex,P3D_ATTR_TEXCOORD0);
             }
           }

          free(TexCoordIndexBuffer);
          free(NormalIndexBuffer);
          free(PosIndexBuffer);
         }
       }

      GroupIndex++;
     }
   }

  return(Result);
 }

bool               P3DModelExportOBJ  (const char         *FileName,
                                       const char         *MTLFileName,
                                       P3DPlantModel      *Model,
                                       bool                ExportHiddenGroups,
                                       bool                ExportOutVisRangeGroups,
                                       float               LODLevel)
 {
  bool                                 Result;
  char                                *SavedLocale;
  FILE                                *TargetStream;
  FILE                                *TargetMTLStream;

  Result = true;

  TargetStream    = fopen(FileName,"wt");
  TargetMTLStream = fopen(MTLFileName,"wt");

  if ((TargetStream == NULL) || (TargetMTLStream == NULL))
   {
    Result = false;
   }

  if (Result)
   {
    P3DHLIPlantTemplate        PlantTemplate(Model);
    P3DHLIPlantInstance       *PlantInstance;

    PlantInstance = PlantTemplate.CreateInstance();

    SavedLocale = setlocale(LC_ALL,NULL);

    if (SavedLocale != NULL)
     {
      SavedLocale = strdup(SavedLocale);
     }

    setlocale(LC_ALL,"C");

    Result = ExportObjData( TargetStream,
                            TargetMTLStream,
                           &PlantTemplate,
                            PlantInstance,
                            Model,
                            MTLFileName,
                            ExportHiddenGroups,
                            ExportOutVisRangeGroups,
                            LODLevel);

    if (SavedLocale != NULL)
     {
      setlocale(LC_ALL,SavedLocale);
     }

    delete PlantInstance;
   }

  if (TargetStream != NULL)
   {
    fclose(TargetStream);
   }

  if (TargetMTLStream != NULL)
   {
    fclose(TargetMTLStream);
   }

  return(Result);
 }

