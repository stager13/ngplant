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

#include <locale.h>

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
 }

#include <wx/wx.h>

#include <p3dapp.h>

#include <p3dplugluactl.h>
#include <p3dplugluahli.h>
#include <p3dplugluaui.h>
#include <p3dplugluaprefs.h>
#include <p3dpluglua.h>

static char       *SavedLocale = NULL;

static void        SetCLocale         ()
 {
  SavedLocale = strdup(setlocale(LC_ALL,NULL));

  setlocale(LC_ALL,"C");
 }

static void        RestoreLocale      ()
 {
  if (SavedLocale != NULL)
   {
    setlocale(LC_ALL,SavedLocale);
   }
 }

static int         GetTextureFileName (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *TexName;

  TexName = Control.GetArgString(1);

  Control.Commit();

   {
    std::string    FileName = P3DApp::GetApp()->GetTexFS()->Generic2System(TexName);

    if (FileName.empty())
     {
      Control.PushString(TexName);
     }
    else
     {
      Control.PushString(FileName.c_str());
     }
   }

  Control.Commit();

  return(1);
 }

static int         GetCurrentLOD      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);

  Control.PushFloat(P3DApp::GetApp()->GetLODLevel());

  Control.Commit();

  return(1);
 }

static void        DisplayErrorMessage(const wxChar       *Message)
 {
  ::wxMessageBox(Message,wxT("Error"),wxOK | wxICON_ERROR);
 }

bool               P3DPlugLuaRunScript(const char         *FileName,
                                       const P3DPlantModel*PlantModel)
 {
  lua_State                           *State;

  SetCLocale();

  State = lua_open();

  if (State != NULL)
   {
    #if NGP_LUA_VER > 50
    luaL_openlibs(State);
    #else
    luaopen_base(State);
    luaopen_table(State);
    luaopen_io(State);
    luaopen_string(State);
    luaopen_math(State);
    #endif

    P3DPlugLuaRegisterHLI(State);
    P3DPlugLuaRegisterUI(State);
    P3DPlugLuaRegisterExportPrefs(State,P3DApp::GetApp()->GetExport3DPrefs());
    P3DPlugLuaRegisterModel(State,"PlantModel",PlantModel);

    lua_register(State,"GetTextureFileName",GetTextureFileName);
    lua_register(State,"GetCurrentLOD",GetCurrentLOD);

    if (luaL_loadfile(State,FileName) || lua_pcall(State,0,0,0))
     {
      if (lua_isstring(State,-1))
       {
        DisplayErrorMessage(wxString(lua_tostring(State,-1),wxConvUTF8));
       }
      else
       {
        DisplayErrorMessage(wxT("Script error: (undefined)"));
       }
     }

    lua_close(State);

    RestoreLocale();

    return(true);
   }
  else
   {
    DisplayErrorMessage(wxT("Unable to initialize Lua environment"));

    RestoreLocale();

    return(false);
   }
 }

static
unsigned int       GetLuaArraySize    (lua_State          *State,
                                       int                 Index)
 {
  unsigned int     N;

  N = 1;

  lua_rawgeti(State,Index,N);

  while (!lua_isnil(State,-1))
   {
    lua_pop(State,1);

    lua_rawgeti(State,Index,++N);
   }

  lua_pop(State,1);

  return(N - 1);
 }

/* get table field: Table.Field and ensures that this value is a table */
static bool        GetTableTableField (lua_State          *State,
                                       const char         *Field)
 {
  wxString         Message;

  lua_pushstring(State,Field);
  lua_gettable(State,-2);

  if      (lua_isnil(State,-1))
   {
    DisplayErrorMessage(wxString::Format(wxT("Script error: field \"%s\" not found"),wxString(Field,wxConvUTF8).c_str()));

    return(false);
   }
  else if (!lua_istable(State,-1))
   {
    DisplayErrorMessage(wxString::Format(wxT("Script error: field \"%s\" must be a table"),wxString(Field,wxConvUTF8).c_str()));

    return(false);
   }

  return(true);
 }

static bool        GetFieldArraySize  (lua_State          *State,
                                       const char         *Field,
                                       unsigned int       *Size)
 {
  if (GetTableTableField(State,Field))
   {
    *Size  = GetLuaArraySize(State,-1);
    lua_pop(State,1);

    return(true);
   }
  else
   {
    return(false);
   }
 }

static bool        GetAttrsCount      (lua_State          *State,
                                       unsigned int       *Counts)
 {
  if (!GetFieldArraySize(State,"vertex",&Counts[P3D_ATTR_VERTEX]))
   {
    return(false);
   }

  if (!GetFieldArraySize(State,"normal",&Counts[P3D_ATTR_NORMAL]))
   {
    return(false);
   }

  if (!GetFieldArraySize(State,"texcoord0",&Counts[P3D_ATTR_TEXCOORD0]))
   {
    return(false);
   }

  if (!GetFieldArraySize(State,"tangent",&Counts[P3D_ATTR_TANGENT]))
   {
    return(false);
   }

  if (!GetFieldArraySize(State,"binormal",&Counts[P3D_ATTR_BINORMAL]))
   {
    return(false);
   }

  return(true);
 }

static bool        GetFloatFromLuaArray
                                      (lua_State          *State,
                                       unsigned int        Index,
                                       float              *Value)
 {
  lua_rawgeti(State,-1,Index);

  if (lua_isnumber(State,-1))
   {
    *Value = (float)lua_tonumber(State,-1);

    lua_pop(State,1);

    return(true);
   }
  else
   {
    DisplayErrorMessage(wxT("invalid or absent float value in array"));

    return(false);
   }
 }

static bool        GetIndexFromLuaArray
                                      (lua_State          *State,
                                       unsigned int        Index,
                                       unsigned int       *Value)
 {
  lua_rawgeti(State,-1,Index);

  if (lua_isnumber(State,-1))
   {
    *Value = (unsigned int)(lua_tonumber(State,-1) + 0.5);

    if (*Value > 0)
     {
      (*Value)--;

      lua_pop(State,1);

      return(true);
     }
    else
     {
      return(false);
     }
   }
  else
   {
    return(false);
   }
 }


static bool        FillVAttrBufferFromLuaTable
                                      (lua_State          *State,
                                       float              *Buffer,
                                       unsigned int        Dimension)
 {
  bool             Result;
  unsigned int     Index;

  Result = true;
  Index  = 1;

  lua_rawgeti(State,-1,Index);

  while ((Result) && (!lua_isnil(State,-1)))
   {
    if (lua_istable(State,-1))
     {
      for (unsigned int Dim = 1; (Dim <= Dimension) && (Result); Dim++)
       {
        Result = GetFloatFromLuaArray(State,Dim,Buffer++);
       }

      lua_pop(State,1);
      lua_rawgeti(State,-1,++Index);
     }
    else
     {
      DisplayErrorMessage(wxT("Script error: invalid vertex attribute dimension"));

      Result = false;
     }
   }

  lua_pop(State,1);

  return(Result);
 }

static bool        FillVAttrBuffersFromLuaTables
                                      (lua_State          *State,
                                       P3DGMeshData       *MeshData,
                                       bool                Indexed)
 {
  bool             Result;

  if (Indexed)
   {
    Result = GetTableTableField(State,"attrs");
   }
  else
   {
    Result = GetTableTableField(State,"vattrs");
   }

  if (Result)
   {
    Result = GetTableTableField(State,"vertex");

    if (Result)
     {
      Result = FillVAttrBufferFromLuaTable
                (State,
                 Indexed ? MeshData->GetVAttrBufferI(P3D_ATTR_VERTEX):
                           MeshData->GetVAttrBuffer(P3D_ATTR_VERTEX),
                 3);

      lua_pop(State,1);
     }

    if (Result)
     {
      Result = GetTableTableField(State,"normal");

      if (Result)
       {
        Result = FillVAttrBufferFromLuaTable
                  (State,
                   Indexed ? MeshData->GetVAttrBufferI(P3D_ATTR_NORMAL):
                             MeshData->GetVAttrBuffer(P3D_ATTR_NORMAL),
                   3);

        lua_pop(State,1);
       }
     }

    if (Result)
     {
      Result = GetTableTableField(State,"texcoord0");

      if (Result)
       {
        Result = FillVAttrBufferFromLuaTable
                  (State,
                   Indexed ? MeshData->GetVAttrBufferI(P3D_ATTR_TEXCOORD0):
                             MeshData->GetVAttrBuffer(P3D_ATTR_TEXCOORD0),
                   2);

        lua_pop(State,1);
       }
     }

    if (Result)
     {
      Result = GetTableTableField(State,"tangent");

      if (Result)
       {
        Result = FillVAttrBufferFromLuaTable
                  (State,
                   Indexed ? MeshData->GetVAttrBufferI(P3D_ATTR_TANGENT):
                             MeshData->GetVAttrBuffer(P3D_ATTR_TANGENT),
                   3);

        lua_pop(State,1);
       }
     }

    if (Result)
     {
      Result = GetTableTableField(State,"binormal");

      if (Result)
       {
        Result = FillVAttrBufferFromLuaTable
                  (State,
                   Indexed ? MeshData->GetVAttrBufferI(P3D_ATTR_BINORMAL):
                             MeshData->GetVAttrBuffer(P3D_ATTR_BINORMAL),
                   3);

        lua_pop(State,1);
       }
     }

    lua_pop(State,1);
   }

  return(Result);
 }

static bool        FillPrimitiveIndexBufferFromLuaTable
                                      (lua_State          *State,
                                       P3DGMeshData       *MeshData)
 {
  bool             Result;
  unsigned int    *PrimitiveBuffer;
  unsigned int    *IndexBuffers[P3D_GMESH_MAX_ATTRS];
  unsigned int     AttrIndex;
  unsigned int     PrimitiveIndex;

  Result = GetTableTableField(State,"vindices");

  if (Result)
   {
    PrimitiveBuffer = MeshData->GetPrimitiveBuffer();

    for (AttrIndex = 0; AttrIndex < P3D_GMESH_MAX_ATTRS; AttrIndex++)
     {
      IndexBuffers[AttrIndex] = MeshData->GetIndexBuffer(AttrIndex);
     }

    PrimitiveIndex = 1;

    lua_rawgeti(State,-1,PrimitiveIndex);

    while ((Result) && (!lua_isnil(State,-1)))
     {
      if (lua_istable(State,-1))
       {
        unsigned int VertexIndex;

        VertexIndex = 1;

        lua_rawgeti(State,-1,VertexIndex);

        while ((Result) && (!lua_isnil(State,-1)))
         {
          if (VertexIndex >= 5)
           {
            Result = false;

            DisplayErrorMessage(wxT("Script error: primitive with more than four vertices generated"));
           }

          if (Result)
           {
            if (lua_istable(State,-1))
             {
              Result = GetIndexFromLuaArray
                        (State,1,IndexBuffers[P3D_ATTR_VERTEX]++);

              if (Result)
               {
                Result = GetIndexFromLuaArray
                          (State,2,IndexBuffers[P3D_ATTR_NORMAL]++);
               }

              if (Result)
               {
                Result = GetIndexFromLuaArray
                          (State,3,IndexBuffers[P3D_ATTR_TEXCOORD0]++);
               }

              if (Result)
               {
                Result = GetIndexFromLuaArray
                          (State,4,IndexBuffers[P3D_ATTR_TANGENT]++);
               }

              if (Result)
               {
                Result = GetIndexFromLuaArray
                          (State,5,IndexBuffers[P3D_ATTR_BINORMAL]++);
               }

              if (!Result)
               {
                DisplayErrorMessage(wxT("Script error: invalid or absent vertex attribute index"));
               }
             }
            else
             {
              Result = false;

              DisplayErrorMessage(wxT("Script error: primitive vertex description must be an array of attribute indices"));
             }
           }

          lua_pop(State,1);
          lua_rawgeti(State,-1,++VertexIndex);
         }

        if (Result)
         {
          if      (VertexIndex == 4)
           {
            *PrimitiveBuffer = P3D_TRIANGLE;
           }
          else if (VertexIndex == 5)
           {
            *PrimitiveBuffer = P3D_QUAD;
           }
          else
           {
            Result = false;

            DisplayErrorMessage(wxT("Script error: primitive must be a triangle or a quad"));
           }
         }

        PrimitiveBuffer++;

        lua_pop(State,1);
       }
      else
       {
        Result = false;

        DisplayErrorMessage(wxT("Script error: primitive description must be an array"));
       }

      lua_pop(State,1);
      lua_rawgeti(State,-1,++PrimitiveIndex);
     }

    lua_pop(State,1);

    lua_pop(State,1);
   }

  return(Result);
 }

static bool        FillIndexBufferFromLuaTable
                                      (lua_State          *State,
                                       P3DGMeshData       *MeshData)
 {
  bool             Result;
  unsigned int    *IndexBuffer;
  unsigned int     TriangleIndex;

  Result = GetTableTableField(State,"indices");

  if (Result)
   {
    IndexBuffer = MeshData->GetIndexBufferI();

    TriangleIndex = 1;

    lua_rawgeti(State,-1,TriangleIndex);

    while ((Result) && (!lua_isnil(State,-1)))
     {
      if (lua_istable(State,-1))
       {
        Result = GetIndexFromLuaArray(State,1,IndexBuffer++);

        if (Result)
         {
          Result = GetIndexFromLuaArray(State,2,IndexBuffer++);
         }

        if (Result)
         {
          Result = GetIndexFromLuaArray(State,3,IndexBuffer++);
         }

        if (!Result)
         {
          DisplayErrorMessage(wxT("Script error: invalid or absent vertex index in triangle list"));
         }
       }
      else
       {
        Result = false;

        DisplayErrorMessage(wxT("Script error: triangle description must be an array of three indices"));
       }

      lua_pop(State,1);
      lua_rawgeti(State,-1,++TriangleIndex);
     }

    lua_pop(State,1);

    lua_pop(State,1);
   }

  return(Result);
 }

static
P3DGMeshData      *CreateGMeshDataFromLuaTable
                                      (lua_State          *State)
 {
  P3DGMeshData    *MeshData = 0;
  unsigned int     VAttrCount[P3D_GMESH_MAX_ATTRS];
  unsigned int     PrimitiveCount;
  unsigned int     IndexCount;
  unsigned int     VAttrCountI[P3D_GMESH_MAX_ATTRS];
  unsigned int     IndexCountI;
  unsigned int     AttrIndex;

  if      (lua_isnil(State,-1))
   {
    /* operation canceled by user */

    return(0);
   }
  else if (!lua_istable(State,-1))
   {
    DisplayErrorMessage(wxT("Script error: result must be a table"));

    return(0);
   }

  if (GetTableTableField(State,"vattrs"))
   {
    if (!GetAttrsCount(State,VAttrCount))
     {
      return(0);
     }

    for (AttrIndex = 0; AttrIndex < P3D_GMESH_MAX_ATTRS; AttrIndex++)
     {
      if (VAttrCount[AttrIndex] == 0)
       {
        DisplayErrorMessage(wxT("Script error: vattr count must be greater than zero"));

        return(0);
       }
     }

    lua_pop(State,1);
   }
  else
   {
    return(0);
   }

  if (GetTableTableField(State,"vindices"))
   {
    unsigned int   PrimitiveIndex;

    PrimitiveCount = GetLuaArraySize(State,-1);

    if (PrimitiveCount == 0)
     {
      DisplayErrorMessage(wxT("Script error: no primitives found"));

      return(0);
     }

    IndexCount = 0;

    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      lua_rawgeti(State,-1,PrimitiveIndex + 1);

      if (lua_istable(State,-1))
       {
        IndexCount += GetLuaArraySize(State,-1);
       }
      else
       {
        DisplayErrorMessage(wxT("Script error: \"vindices\" field structure is broken (primitve indices must be in array)"));

        return(0);
       }

      lua_pop(State,1);
     }

    lua_pop(State,1);
   }
  else
   {
    return(0);
   }

  if (GetTableTableField(State,"attrs"))
   {
    if (!GetAttrsCount(State,VAttrCountI))
     {
      return(0);
     }

    for (AttrIndex = 0; AttrIndex < P3D_GMESH_MAX_ATTRS; AttrIndex++)
     {
      if      (VAttrCountI[AttrIndex] == 0)
       {
        DisplayErrorMessage(wxT("Script error: attr count must be greater than zero"));

        return(0);
       }
      else if (VAttrCountI[AttrIndex] != VAttrCountI[0])
       {
        DisplayErrorMessage(wxT("Script error: attr count for all attributes must be the same"));

        return(0);
       }
     }

    lua_pop(State,1);
   }
  else
   {
    return(0);
   }

  if (GetFieldArraySize(State,"indices",&IndexCountI))
   {
    if (IndexCountI > 0)
     {
      IndexCountI *= 3; /* three verts per triangle */
     }
    else
     {
      DisplayErrorMessage(wxT("Script error: no indexed primitives found"));

      return(0);
     }
   }
  else
   {
    return(0);
   }

  try
   {
    MeshData = new P3DGMeshData(VAttrCount,PrimitiveCount,IndexCount,VAttrCountI[0],IndexCountI);
   }
  catch (...)
   {
    DisplayErrorMessage(wxT("Script error: unable to allocate gmesh data"));

    return(0);
   }

  bool Result = true;

  Result = FillVAttrBuffersFromLuaTables(State,MeshData,false);

  if (Result)
   {
    Result = FillVAttrBuffersFromLuaTables(State,MeshData,true);
   }

  if (Result)
   {
    Result = FillPrimitiveIndexBufferFromLuaTable(State,MeshData);
   }

  if (Result)
   {
    Result = FillIndexBufferFromLuaTable(State,MeshData);
   }

  if (!Result)
   {
    delete MeshData;

    MeshData = 0;
   }

  return(MeshData);
 }

extern
P3DGMeshData      *P3DPlugLuaRunGMeshGenerator
                                      (const char         *FileName)
 {
  lua_State                           *State;
  P3DGMeshData                        *Result = 0;

  SetCLocale();

  State = lua_open();

  if (State != NULL)
   {
    #if NGP_LUA_VER > 50
    luaL_openlibs(State);
    #else
    luaopen_base(State);
    luaopen_table(State);
    luaopen_io(State);
    luaopen_string(State);
    luaopen_math(State);
    #endif

    P3DPlugLuaRegisterUI(State);

    if (luaL_loadfile(State,FileName) || lua_pcall(State,0,1,0))
     {
      if (lua_isstring(State,-1))
       {
        DisplayErrorMessage(wxString(lua_tostring(State,-1),wxConvUTF8));
       }
      else
       {
        DisplayErrorMessage(wxT("Script error: (undefined)"));
       }
     }
    else
     {
      Result = CreateGMeshDataFromLuaTable(State);
     }

    lua_close(State);

    RestoreLocale();
   }
  else
   {
    DisplayErrorMessage(wxT("Unable to initialize Lua environment"));

    RestoreLocale();
   }

  return(Result);
 }

