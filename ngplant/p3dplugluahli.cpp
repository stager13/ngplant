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

#include <string.h>
#include <stdlib.h>

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
 }

#include <ngpcore/p3dhli.h>
#include <ngpcore/p3diostreamadd.h>

#include <p3dmaterialstd.h>

#include <p3dplugluactl.h>
#include <p3dplugluahli.h>

#define StaticArraySize(Array) (sizeof(Array) / sizeof((Array)[0]))

static char        PlantInstanceMetaTableName[] = "NGPLUAPlantInstance_MT";
static char        BranchGroupMetaTableName[]   = "NGPLUABranchGroup_MT";

static char        ErrorMessageInvalidVAttrType[] = "invalid vertex attribute type";
static char        ErrorMessageOutOfMemory[] = "out of memory";
static char        ErrorMessageInvalidPrimitiveType[] = "invalid primitive type (must be NGP_TRIANGLE_LIST)";

class NGPLUAPlantInstance
 {
  public           :

                   NGPLUAPlantInstance()
  {
   RefCount = 1;
   Template = 0;
   Instance = 0;
   Model    = 0;
  }

                  ~NGPLUAPlantInstance()
  {
   delete Template;
   delete Instance;

   Template = 0;
   Instance = 0;
  }

  void             DecRef             ()
   {
    RefCount--;

    if (RefCount == 0)
     {
      delete this;
     }
   }

  void             AddRef             ()
   {
    RefCount++;
   }

  P3DHLIPlantTemplate                 *Template;
  P3DHLIPlantInstance                 *Instance;
  /* Model member is needed for extracting information about branch visibility */
  /* only (IsHidden()) which is available in ngPlant application only and can  */
  /* not be accessed via HLI                                                   */
  const P3DPlantModel                 *Model;
  unsigned int                         RefCount;
 };

typedef struct
 {
  NGPLUAPlantInstance                 *Instance;
  unsigned int                         Index;
 } NGPLUABranchGroup;

static int         PlantInstanceCtor  (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *ModelDesc;
  unsigned int                         Seed;
  NGPLUAPlantInstance                **UserData;

  ModelDesc = Control.GetArgString(1);
  Seed      = Control.GetArgUIntOpt(2,0);
  UserData  = (NGPLUAPlantInstance**)Control.CreateUserData(PlantInstanceMetaTableName,sizeof(NGPLUAPlantInstance*));

  if (UserData != 0)
   {
    *UserData = 0;
   }

  Control.Commit();

  try
   {
    P3DInputStringStreamString         SrcStream(ModelDesc);

    *UserData = new NGPLUAPlantInstance();

    (*UserData)->Template = new P3DHLIPlantTemplate(&SrcStream);
    (*UserData)->Instance = (*UserData)->Template->CreateInstance(Seed);
    (*UserData)->Model    = 0;
   }
  catch (const P3DException &Exception)
   {
    if ((*UserData) != 0)
     {
      (*UserData)->DecRef();

      delete (*UserData);

      (*UserData) = 0;
     }

    Control.RaiseError("%s",Exception.GetMessage());
   }

  Control.Commit();

  return(1);
 }

/*FIXME: need more accurate error checking and report */
int                P3DPlugLuaRegisterModel
                                      (lua_State          *State,
                                       const char         *VarName,
                                       const P3DPlantModel*Model)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUAPlantInstance                **UserData;

  Control.PushString(VarName);

  UserData  = (NGPLUAPlantInstance**)Control.CreateUserData(PlantInstanceMetaTableName,sizeof(NGPLUAPlantInstance*));

  if (UserData != 0)
   {
    *UserData = 0;
   }

  if (!Control.IsOk())
   {
    return(0);
   }

  try
   {
    *UserData = new NGPLUAPlantInstance();

    (*UserData)->Template = new P3DHLIPlantTemplate(Model);
    (*UserData)->Instance = (*UserData)->Template->CreateInstance(Model->GetBaseSeed());
    (*UserData)->Model    = Model;
   }
  catch (const P3DException &Exception)
   {
    if ((*UserData) != 0)
     {
      (*UserData)->DecRef();

      delete (*UserData);

      (*UserData) = 0;
     }

    return(0);
   }

  Control.SetTable(LUA_GLOBALSINDEX);

  if (Control.IsOk())
   {
    return(1);
   }
  else
   {
    return(0);
   }
 }

static int         PlantInstanceDtor  (lua_State          *State)
 {
  NGPLUAPlantInstance                **UserData;

  UserData = (NGPLUAPlantInstance**)lua_touserdata(State,1);

  if (UserData != NULL)
   {
    if ((*UserData) != 0)
     {
      (*UserData)->DecRef();
     }
   }

  return(0);
 }

static int         PlantInstanceGetGroupCount
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUAPlantInstance                **PlantInstance;

  PlantInstance = (NGPLUAPlantInstance**)Control.GetArgUserData(1,PlantInstanceMetaTableName);
  Control.Commit();

  Control.PushUInt((*PlantInstance)->Template->GetGroupCount());

  Control.Commit();

  return(1);
 }

static int         PlantInstanceGetGroup
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUAPlantInstance                **PlantInstance;
  unsigned int                         GroupCount;
  unsigned int                         GroupIndex;
  NGPLUABranchGroup                   *BranchGroup;

  PlantInstance = (NGPLUAPlantInstance**)Control.GetArgUserData(1,PlantInstanceMetaTableName);
  Control.Commit();
  GroupIndex    = Control.GetArgUInt(2);
  Control.Commit();

  GroupCount = (*PlantInstance)->Template->GetGroupCount();

  if ((GroupIndex < 1) || (GroupIndex > GroupCount))
   {
    Control.RaiseError("Branch group index must be in range 1 .. %d",GroupCount);

    Control.Commit();
   }

  BranchGroup = (NGPLUABranchGroup*)Control.CreateUserData(BranchGroupMetaTableName,sizeof(NGPLUABranchGroup));

  Control.Commit();

  BranchGroup->Instance = *PlantInstance;
  BranchGroup->Index    = GroupIndex - 1;

  (*PlantInstance)->AddRef();

  return(1);
 }

static int         PlantInstanceGetBoundingBox
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUAPlantInstance                **PlantInstance;
  float                                BBox[6];

  PlantInstance = (NGPLUAPlantInstance**)Control.GetArgUserData(1,PlantInstanceMetaTableName);
  Control.Commit();

  (*PlantInstance)->Instance->GetBoundingBox(BBox,&BBox[3]);

  for (unsigned int Index = 0; Index < 6; Index++)
   {
    Control.PushFloat(BBox[Index]);
   }

  Control.Commit();

  return(6);
 }

static luaL_reg   PlantInstanceMethods[] =
 {
  { "GetGroupCount" , PlantInstanceGetGroupCount  },
  { "GetGroup"      , PlantInstanceGetGroup       },
  { "GetBoundingBox", PlantInstanceGetBoundingBox }
 };

static
lua_CFunction      FindFunc           (luaL_reg           *Funcs,
                                       unsigned int        Count,
                                       const char         *Name)
 {
  for (unsigned int Index = 0; Index < Count; Index++)
   {
    if (strcmp(Funcs[Index].name,Name) == 0)
     {
      return(Funcs[Index].func);
     }
   }

  return(NULL);
 }

static int         PlantInstanceMetaIndex
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Key;
  lua_CFunction                        Func;

  Control.GetArgUserData(1,PlantInstanceMetaTableName);
  Key           = Control.GetArgString(2);
  Control.Commit();

  Func = FindFunc(PlantInstanceMethods,StaticArraySize(PlantInstanceMethods),Key);

  if (Func != NULL)
   {
    Control.PushCFunction(Func);
   }
  else
   {
    Control.RaiseError("undefined method '%s'",Key);
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetMaterial
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  const P3DMaterialDef                *MaterialDef;
  float                                R,G,B;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  MaterialDef = BranchGroup->Instance->Template->GetMaterial(BranchGroup->Index);

  MaterialDef->GetColor(&R,&G,&B);

  Control.PushNewTable();
  Control.PushString("Color");
  Control.PushNewTable();
  Control.SetTableFloat("R",R);
  Control.SetTableFloat("G",G);
  Control.SetTableFloat("B",B);
  Control.SetTable(-3);

  Control.PushString("TexNames");
  Control.PushNewTable();

  for (int TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    if (MaterialDef->GetTexName(TexLayer) != NULL)
     {
      Control.SetTableString(TexLayer,MaterialDef->GetTexName(TexLayer));
     }
    else
     {
      Control.SetTableNil(TexLayer);
     }
   }

  Control.SetTable(-3);

  Control.SetTableBool("DoubleSided",MaterialDef->IsDoubleSided());
  Control.SetTableBool("Transparent",MaterialDef->IsTransparent());
  Control.SetTableBool("Billboard",MaterialDef->IsBillboard());
  Control.SetTableUInt("BillboardMode",MaterialDef->GetBillboardMode());
  Control.SetTableBool("AlphaCtrlEnabled",MaterialDef->IsAlphaCtrlEnabled());
  Control.SetTableFloat("AlphaFadeIn",MaterialDef->GetAlphaFadeIn());
  Control.SetTableFloat("AlphaFadeOut",MaterialDef->GetAlphaFadeOut());

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetName (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  Control.PushString
   (BranchGroup->Instance->Template->GetGroupName(BranchGroup->Index));

  Control.Commit();

  return(1);
 }

static int         BranchGroupIsHidden(lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  bool                                 Hidden;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  Hidden = true;

  if (BranchGroup->Instance->Model != 0)
   {
    const P3DBranchModel              *BranchModel;

    BranchModel = P3DPlantModel::GetBranchModelByIndex(BranchGroup->Instance->Model,BranchGroup->Index);

    if (BranchModel != 0)
     {
      const P3DMaterialInstanceSimple *MaterialInstance;

      MaterialInstance = dynamic_cast<const P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

      if (MaterialInstance != 0)
       {
        Hidden = MaterialInstance->IsHidden();
       }
     }
   }

  Control.PushBool(Hidden);

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetBranchCount
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  Control.PushUInt(BranchGroup->Instance->Instance->GetBranchCount(BranchGroup->Index));

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetBillboardSize
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  try
   {
    float          Width,Height;

    BranchGroup->Instance->Template->GetBillboardSize(&Width,&Height,BranchGroup->Index);

    Control.PushFloat(Width);
    Control.PushFloat(Height);
   }
  catch (P3DException       &Error)
   {
    Control.RaiseError("%s",Error.GetMessage());
   }

  Control.Commit();

  return(2);
 }

static int         BranchGroupIsCloneable
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  bool                                 AllowScaling;

  BranchGroup  = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  AllowScaling = Control.GetArgBoolOpt(2,false);

  Control.Commit();

  try
   {
    Control.PushBool(BranchGroup->Instance->Template->IsCloneable
                      (BranchGroup->Index,AllowScaling));
   }
  catch (P3DException       &Error)
   {
    Control.RaiseError("%s",Error.GetMessage());
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupIsLODVisRangeEnabled
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  Control.PushBool(BranchGroup->Instance->Template->IsLODVisRangeEnabled(BranchGroup->Index));

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetLODVisRange
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  float                                MinLOD;
  float                                MaxLOD;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  BranchGroup->Instance->Template->
   GetLODVisRange(&MinLOD,&MaxLOD,BranchGroup->Index);

  Control.PushFloat(MinLOD);
  Control.PushFloat(MaxLOD);

  Control.Commit();

  return(2);
 }

static int         BranchGroupGetVAttrCount
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Attr;
  bool                                 Total;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Attr        = Control.GetArgUInt(2);
  Total       = Control.GetArgBoolOpt(3,false);

  Control.Commit();

  if (Attr > P3D_ATTR_BINORMAL)
   {
    Control.RaiseError(ErrorMessageInvalidVAttrType);
    Control.Commit();
   }

  if (Total)
   {
    Control.PushUInt
     (BranchGroup->Instance->Instance->GetVAttrCount(BranchGroup->Index,Attr));
   }
  else
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetVAttrCount(BranchGroup->Index,Attr));
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetCloneVAttrBuffer
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Attr;
  unsigned int                         AttrItemCount;
  unsigned int                         TotalAttrCount;
  float                               *AttrBuffer;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Attr        = Control.GetArgUInt(2);

  Control.Commit();

  if (Attr > P3D_ATTR_BINORMAL)
   {
    Control.RaiseError(ErrorMessageInvalidVAttrType);
    Control.Commit();
   }

  Control.PushNewTable();

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    AttrItemCount = 2;
   }
  else
   {
    AttrItemCount = 3;
   }

  TotalAttrCount = BranchGroup->Instance->Template->GetVAttrCount(BranchGroup->Index,Attr);

  if (TotalAttrCount > 0)
   {
    AttrBuffer = (float*)malloc(sizeof(float) * TotalAttrCount * AttrItemCount);

    if (AttrBuffer != NULL)
     {
      float                           *Ptr;

      Ptr = AttrBuffer;

      BranchGroup->Instance->Template->FillCloneVAttrBuffer
       (AttrBuffer,BranchGroup->Index,Attr);

      for (unsigned int AttrIndex = 0; AttrIndex < TotalAttrCount; AttrIndex++)
       {
        Control.PushNewTable();
        Control.SetTableFloat(1,*Ptr); ++Ptr;
        Control.SetTableFloat(2,*Ptr); ++Ptr;

        if (AttrItemCount == 3)
         {
          Control.SetTableFloat(3,*Ptr); ++Ptr;
         }

        Control.SetTable(-2,AttrIndex + 1);
       }

      free(AttrBuffer);
     }
    else
     {
      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetCloneTransformBuffer
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         BranchCount;
  float                               *TranslationBuffer;
  float                               *OrientationBuffer;
  float                               *ScaleBuffer;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);

  Control.Commit();

  Control.PushNewTable();
  Control.PushNewTable();
  Control.PushNewTable();

  BranchCount = BranchGroup->Instance->Instance->GetBranchCount(BranchGroup->Index);

  if (BranchCount > 0)
   {
    TranslationBuffer = (float*)malloc(sizeof(float) * BranchCount * 3);
    OrientationBuffer = (float*)malloc(sizeof(float) * BranchCount * 4);
    ScaleBuffer       = (float*)malloc(sizeof(float) * BranchCount);

    if (TranslationBuffer != NULL && OrientationBuffer != NULL && ScaleBuffer != NULL)
     {
      float  *TPtr;
      float  *OPtr;
      float  *SPtr;

      TPtr = TranslationBuffer;
      OPtr = OrientationBuffer;
      SPtr = ScaleBuffer;

      BranchGroup->Instance->Instance->FillCloneTransformBuffer
       (TranslationBuffer,OrientationBuffer,ScaleBuffer,BranchGroup->Index);

      for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
       {
        Control.PushNewTable();
        Control.SetTableFloat(1,*TPtr); ++TPtr;
        Control.SetTableFloat(2,*TPtr); ++TPtr;
        Control.SetTableFloat(3,*TPtr); ++TPtr;

        Control.SetTable(-4,BranchIndex + 1);

        Control.PushNewTable();
        Control.SetTableFloat(1,*OPtr); ++OPtr;
        Control.SetTableFloat(2,*OPtr); ++OPtr;
        Control.SetTableFloat(3,*OPtr); ++OPtr;
        Control.SetTableFloat(4,*OPtr); ++OPtr;

        Control.SetTable(-3,BranchIndex + 1);

        Control.PushNewTable();
        Control.SetTableFloat(1,*SPtr); ++SPtr;

        Control.SetTable(-2,BranchIndex + 1);
       }

      free(TranslationBuffer);
      free(OrientationBuffer);
      free(ScaleBuffer);
     }
    else
     {
      free(TranslationBuffer);
      free(OrientationBuffer);
      free(ScaleBuffer);

      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(3);
 }

static int         BranchGroupGetVAttrBuffer
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Attr;
  unsigned int                         AttrItemCount;
  unsigned int                         TotalAttrCount;
  float                               *AttrBuffer;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Attr        = Control.GetArgUInt(2);

  Control.Commit();

  if (Attr > P3D_ATTR_BINORMAL)
   {
    Control.RaiseError(ErrorMessageInvalidVAttrType);
    Control.Commit();
   }

  Control.PushNewTable();

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    AttrItemCount = 2;
   }
  else
   {
    AttrItemCount = 3;
   }

  TotalAttrCount = BranchGroup->Instance->Instance->GetVAttrCount(BranchGroup->Index,Attr);

  if (TotalAttrCount > 0)
   {
    AttrBuffer = (float*)malloc(sizeof(float) * TotalAttrCount * AttrItemCount);

    if (AttrBuffer != NULL)
     {
      float                           *Ptr;

      Ptr = AttrBuffer;

      BranchGroup->Instance->Instance->FillVAttrBuffer
       (AttrBuffer,BranchGroup->Index,Attr);

      for (unsigned int AttrIndex = 0; AttrIndex < TotalAttrCount; AttrIndex++)
       {
        Control.PushNewTable();
        Control.SetTableFloat(1,*Ptr); ++Ptr;
        Control.SetTableFloat(2,*Ptr); ++Ptr;

        if (AttrItemCount == 3)
         {
          Control.SetTableFloat(3,*Ptr); ++Ptr;
         }

        Control.SetTable(-2,AttrIndex + 1);
       }

      free(AttrBuffer);
     }
    else
     {
      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetPrimitiveCount
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  bool                                 Total;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Total       = Control.GetArgBoolOpt(2,true);

  Control.Commit();

  if (Total)
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetPrimitiveCount(BranchGroup->Index) *
       BranchGroup->Instance->Instance->GetBranchCount(BranchGroup->Index));
   }
  else
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetPrimitiveCount(BranchGroup->Index));
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetPrimitiveType
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Index;
  bool                                 Total;
  unsigned int                         Result;

  Result = 0;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Index       = Control.GetArgUInt(2) - 1;
  Total       = Control.GetArgBoolOpt(3,true);

  Control.Commit();

  if (Total)
   {
    Index %= BranchGroup->Instance->Template->GetPrimitiveCount(BranchGroup->Index);
   }

  try
   {
    Result = BranchGroup->Instance->Template->GetPrimitiveType(BranchGroup->Index,Index);
   }
  catch (P3DException                 &Error)
   {
    Control.RaiseError("%s",Error.GetMessage());
   }

  Control.PushUInt(Result);

  Control.Commit();

  return(1);
 }

static
unsigned int       CalcVAttrVertexCount
                                      (const P3DHLIPlantTemplate
                                                          *Template,
                                       unsigned int        GroupIndex)
 {
  unsigned int                         Result;
  unsigned int                         PrimitiveIndex;
  unsigned int                         PrimitiveCount;

  PrimitiveIndex = 0;
  PrimitiveCount = Template->GetPrimitiveCount(GroupIndex);

  Result = 0;

  for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
   {
    if (Template->GetPrimitiveType(GroupIndex,PrimitiveIndex) == P3D_QUAD)
     {
      Result += 4;
     }
    else
     {
      Result += 3;
     }
   }

  return(Result);
 }

static int         BranchGroupGetVAttrIndexBuffer
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Attr;
  bool                                 Total;
  unsigned int                         Base;
  unsigned int                         BranchCount;
  unsigned int                         BranchAttrCount;
  unsigned int                         BranchPrimitiveCount;
  unsigned int                         BranchVertexCount;
  unsigned int                        *IndexBuffer;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Attr        = Control.GetArgUInt(2);
  Total       = Control.GetArgBoolOpt(3,true);
  Base        = Control.GetArgUIntOpt(4,1);

  Control.Commit();

  if (Attr > P3D_ATTR_BINORMAL)
   {
    Control.RaiseError(ErrorMessageInvalidVAttrType);
    Control.Commit();
   }

  Control.PushNewTable();

  if (Total)
   {
    BranchCount = BranchGroup->Instance->Instance->GetBranchCount
                   (BranchGroup->Index);
   }
  else
   {
    BranchCount = 1;
   }

  BranchVertexCount = CalcVAttrVertexCount(BranchGroup->Instance->Template,
                                           BranchGroup->Index);

  if ((BranchVertexCount > 0) && (BranchCount > 0))
   {
    BranchAttrCount = BranchGroup->Instance->Template->GetVAttrCount
                     (BranchGroup->Index,Attr);

    BranchPrimitiveCount = BranchGroup->Instance->Template->GetPrimitiveCount
                            (BranchGroup->Index);

    IndexBuffer = (unsigned int*)malloc(sizeof(unsigned int) * BranchVertexCount);

    if (IndexBuffer != NULL)
     {
      unsigned int           PrimitiveCounter;

      PrimitiveCounter = 1;

      for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
       {
        unsigned int                  *Ptr;

        BranchGroup->Instance->Template->FillVAttrIndexBuffer
         (IndexBuffer,BranchGroup->Index,Attr,P3D_UNSIGNED_INT,Base);

        Ptr = IndexBuffer;

        for (unsigned int PrimitiveIndex = 0;
             PrimitiveIndex < BranchPrimitiveCount;
             PrimitiveIndex++)
         {
          Control.PushNewTable();
          Control.SetTableUInt(1,*Ptr); ++Ptr;
          Control.SetTableUInt(2,*Ptr); ++Ptr;
          Control.SetTableUInt(3,*Ptr); ++Ptr;

          if (BranchGroup->Instance->Template->GetPrimitiveType
              (BranchGroup->Index,PrimitiveIndex) == P3D_QUAD)
           {
            Control.SetTableUInt(4,*Ptr); ++Ptr;
           }

          Control.SetTable(-2,PrimitiveCounter);

          ++PrimitiveCounter;
         }

        Base += BranchAttrCount;
       }

      free(IndexBuffer);
     }
    else
     {
      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetVAttrCountI
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  bool                                 Total;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Total       = Control.GetArgBoolOpt(2,false);

  Control.Commit();

  if (Total)
   {
    Control.PushUInt
     (BranchGroup->Instance->Instance->GetVAttrCountI(BranchGroup->Index));
   }
  else
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetVAttrCountI(BranchGroup->Index));
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetVAttrBufferI
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         Attr;
  unsigned int                         AttrItemCount;
  unsigned int                         TotalAttrCount;
  float                               *AttrBuffer;
  P3DHLIVAttrBuffers                   VAttrBuffers;

  BranchGroup = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  Attr        = Control.GetArgUInt(2);

  Control.Commit();

  if      ((Attr == P3D_ATTR_VERTEX)  ||
           (Attr == P3D_ATTR_NORMAL)  ||
           (Attr == P3D_ATTR_TANGENT) ||
           (Attr == P3D_ATTR_BINORMAL))
   {
    AttrItemCount = 3;
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    AttrItemCount = 2;
   }
  else if (Attr == P3D_ATTR_BILLBOARD_POS)
   {
    if (BranchGroup->Instance->Template->GetMaterial(BranchGroup->Index)->IsBillboard())
     {
      AttrItemCount = 3;
     }
    else
     {
      Control.RaiseError(ErrorMessageInvalidVAttrType);
      Control.Commit();
     }
   }
  else
   {
    Control.RaiseError(ErrorMessageInvalidVAttrType);
    Control.Commit();
   }

  Control.PushNewTable();

  TotalAttrCount = BranchGroup->Instance->Instance->GetVAttrCountI(BranchGroup->Index);

  if (TotalAttrCount > 0)
   {
    AttrBuffer = (float*)malloc(sizeof(float) * TotalAttrCount * AttrItemCount);

    if (AttrBuffer != NULL)
     {
      float                           *Ptr;

      VAttrBuffers.AddAttr(Attr,AttrBuffer,0,sizeof(float) * AttrItemCount);

      BranchGroup->Instance->Instance->FillVAttrBuffersI
       (&VAttrBuffers,BranchGroup->Index);

      Ptr = AttrBuffer;

      for (unsigned int AttrIndex = 0; AttrIndex < TotalAttrCount; AttrIndex++)
       {
        Control.PushNewTable();
        Control.SetTableFloat(1,*Ptr); ++Ptr;
        Control.SetTableFloat(2,*Ptr); ++Ptr;

        if (AttrItemCount == 3)
         {
          Control.SetTableFloat(3,*Ptr); ++Ptr;
         }

        Control.SetTable(-2,AttrIndex + 1);
       }

      free(AttrBuffer);
     }
    else
     {
      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetIndexCount
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         PrimitiveType;
  bool                                 Total;

  BranchGroup   = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  PrimitiveType = Control.GetArgUInt(2);
  Total         = Control.GetArgBoolOpt(3,true);

  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    Control.RaiseError(ErrorMessageInvalidPrimitiveType);
   }

  Control.Commit();

  if (Total)
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetIndexCount(BranchGroup->Index,P3D_TRIANGLE_LIST) *
      BranchGroup->Instance->Instance->GetBranchCount(BranchGroup->Index));
   }
  else
   {
    Control.PushUInt
     (BranchGroup->Instance->Template->GetIndexCount(BranchGroup->Index,P3D_TRIANGLE_LIST));
   }

  Control.Commit();

  return(1);
 }

static int         BranchGroupGetIndexBuffer
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  NGPLUABranchGroup                   *BranchGroup;
  unsigned int                         PrimitiveType;
  bool                                 Total;
  unsigned int                         Base;
  unsigned int                         BranchCount;
  unsigned int                         BranchIndexCount;
  unsigned int                        *IndexBuffer;

  BranchGroup   = (NGPLUABranchGroup*)Control.GetArgUserData(1,BranchGroupMetaTableName);
  PrimitiveType = Control.GetArgUInt(2);
  Total         = Control.GetArgBoolOpt(3,true);
  Base          = Control.GetArgUIntOpt(4,1);

  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    Control.RaiseError(ErrorMessageInvalidPrimitiveType);
   }

  Control.Commit();

  Control.PushNewTable();

  if (Total)
   {
    BranchCount = BranchGroup->Instance->Instance->GetBranchCount
                   (BranchGroup->Index);
   }
  else
   {
    BranchCount = 1;
   }

  BranchIndexCount = BranchGroup->Instance->Template->GetIndexCount
                      (BranchGroup->Index,P3D_TRIANGLE_LIST);

  if ((BranchIndexCount > 0) && (BranchCount > 0))
   {
    IndexBuffer = (unsigned int*)malloc(sizeof(unsigned int) * BranchIndexCount);

    if (IndexBuffer != NULL)
     {
      unsigned int                     BranchVertexCount;
      unsigned int                     IndexCounter;

      BranchVertexCount = BranchGroup->Instance->Template->GetVAttrCountI
                           (BranchGroup->Index);

      IndexCounter = 1;

      for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
       {
        unsigned int                  *Ptr;

        BranchGroup->Instance->Template->FillIndexBuffer
         (IndexBuffer,BranchGroup->Index,P3D_TRIANGLE_LIST,P3D_UNSIGNED_INT,Base);

        Ptr = IndexBuffer;

        for (unsigned int Index = 0; Index < BranchIndexCount; Index++)
         {
          Control.SetTableUInt(IndexCounter,*Ptr);

          Ptr++;
          IndexCounter++;
         }

        Base += BranchVertexCount;
       }

      free(IndexBuffer);
     }
    else
     {
      Control.RaiseError(ErrorMessageOutOfMemory);
     }
   }

  Control.Commit();

  return(1);
 }

static luaL_reg   BranchGroupMethods[] =
 {
  { "GetName"             , BranchGroupGetName              },
  { "GetMaterial"         , BranchGroupGetMaterial          },
  { "IsHidden"            , BranchGroupIsHidden             },
  { "GetBranchCount"      , BranchGroupGetBranchCount       },
  { "GetBillboardSize"    , BranchGroupGetBillboardSize     },
  { "IsCloneable"         , BranchGroupIsCloneable          },
  { "GetCloneVAttrBuffer" , BranchGroupGetCloneVAttrBuffer  },
  { "GetCloneTransformBuffer", BranchGroupGetCloneTransformBuffer },
  { "IsLODVisRangeEnabled", BranchGroupIsLODVisRangeEnabled },
  { "GetLODVisRange"      , BranchGroupGetLODVisRange       },
  { "GetVAttrCount"       , BranchGroupGetVAttrCount        },
  { "GetVAttrBuffer"      , BranchGroupGetVAttrBuffer       },
  { "GetPrimitiveCount"   , BranchGroupGetPrimitiveCount    },
  { "GetPrimitiveType"    , BranchGroupGetPrimitiveType     },
  { "GetVAttrIndexBuffer" , BranchGroupGetVAttrIndexBuffer  },
  { "GetVAttrCountI"      , BranchGroupGetVAttrCountI       },
  { "GetVAttrBufferI"     , BranchGroupGetVAttrBufferI      },
  { "GetIndexCount"       , BranchGroupGetIndexCount        },
  { "GetIndexBuffer"      , BranchGroupGetIndexBuffer       }
 };

static int         BranchGroupDtor    (lua_State          *State)
 {
  NGPLUABranchGroup                   *BranchGroup;

  BranchGroup = (NGPLUABranchGroup*)lua_touserdata(State,1);

  if (BranchGroup != NULL)
   {
    BranchGroup->Instance->DecRef();
   }

  return(0);
 }

static int         BranchGroupMetaIndex
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Key;
  lua_CFunction                        Func;

  Control.GetArgUserData(1,BranchGroupMetaTableName);

  Key = Control.GetArgString(2);

  Control.Commit();

  Func = FindFunc(BranchGroupMethods,StaticArraySize(BranchGroupMethods),Key);

  if (Func != NULL)
   {
    Control.PushCFunction(Func);
   }
  else
   {
    Control.RaiseError("undefined method '%s'",Key);
   }

  Control.Commit();

  return(1);
 }

extern void        P3DPlugLuaRegisterHLI
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);

  Control.RegisterUserData(PlantInstanceMetaTableName);
  Control.SetMetaMethod(PlantInstanceMetaTableName,"__gc",PlantInstanceDtor);
  Control.SetMetaMethod(PlantInstanceMetaTableName,"__index",PlantInstanceMetaIndex);

  Control.RegisterUserData(BranchGroupMetaTableName);
  Control.SetMetaMethod(BranchGroupMetaTableName,"__gc",BranchGroupDtor);
  Control.SetMetaMethod(BranchGroupMetaTableName,"__index",BranchGroupMetaIndex);

  Control.RegisterConstant("NGP_ATTR_VERTEX",P3D_ATTR_VERTEX);
  Control.RegisterConstant("NGP_ATTR_NORMAL",P3D_ATTR_NORMAL);
  Control.RegisterConstant("NGP_ATTR_TEXCOORD0",P3D_ATTR_TEXCOORD0);
  Control.RegisterConstant("NGP_ATTR_TANGENT",P3D_ATTR_TANGENT);
  Control.RegisterConstant("NGP_ATTR_BINORMAL",P3D_ATTR_BINORMAL);
  Control.RegisterConstant("NGP_ATTR_BILLBOARD_POS",P3D_ATTR_BILLBOARD_POS);

  Control.RegisterConstant("NGP_BILLBOARD_MODE_NONE",P3D_BILLBOARD_MODE_NONE);
  Control.RegisterConstant("NGP_BILLBOARD_MODE_SPHERICAL",P3D_BILLBOARD_MODE_SPHERICAL);
  Control.RegisterConstant("NGP_BILLBOARD_MODE_CYLINDRICAL",P3D_BILLBOARD_MODE_CYLINDRICAL);

  Control.RegisterConstant("NGP_TRIANGLE",P3D_TRIANGLE);
  Control.RegisterConstant("NGP_TRIANGLE_LIST",P3D_TRIANGLE_LIST);
  Control.RegisterConstant("NGP_TRIANGLE_STRIP",P3D_TRIANGLE_STRIP);
  Control.RegisterConstant("NGP_QUAD",P3D_QUAD);

  Control.RegisterConstant("NGP_TEX_DIFFUSE",P3D_TEX_DIFFUSE);
  Control.RegisterConstant("NGP_TEX_NORMAL_MAP",P3D_TEX_NORMAL_MAP);
  Control.RegisterConstant("NGP_TEX_AUX0",P3D_TEX_AUX0);
  Control.RegisterConstant("NGP_TEX_AUX1",P3D_TEX_AUX1);

  Control.Commit();

  lua_register(State,"NGPPlantInstance",PlantInstanceCtor);
 }

