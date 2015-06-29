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

#include <stdarg.h>

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
 }

#include <p3dplugluactl.h>

                   P3DPlugLUAControl::P3DPlugLUAControl
                                      (lua_State          *State)
 {
  this->State = State;
  Ok          = true;
 }

bool               P3DPlugLUAControl::IsOk() const
 {
  return(Ok);
 }

const char        *P3DPlugLUAControl::GetArgString
                                      (unsigned int        ArgIndex)
 {
  const char                          *Result;

  if (!Ok)
   {
    return("");
   }

  if (lua_isstring(State,ArgIndex))
   {
    Result = lua_tostring(State,ArgIndex);
   }
  else
   {
    Result = NULL;
   }

  if (Result == NULL)
   {
    Ok = false;

    PushErrorString("%d argument must be a string",ArgIndex);

    return("");
   }
  else
   {
    return(Result);
   }
 }

const char        *P3DPlugLUAControl::GetArgStringOpt
                                      (unsigned int        ArgIndex,
                                       const char         *DefValue)
 {
  const char                          *Result;

  if (!Ok)
   {
    return("");
   }

  if ((unsigned int)lua_gettop(State) < ArgIndex)
   {
    return(DefValue);
   }

  if (lua_isstring(State,ArgIndex))
   {
    Result = lua_tostring(State,ArgIndex);
   }
  else
   {
    Result = NULL;
   }

  if (Result == NULL)
   {
    Ok = false;

    PushErrorString("%d argument must be a string",ArgIndex);

    return("");
   }
  else
   {
    return(Result);
   }
 }

unsigned int       P3DPlugLUAControl::GetArgUInt
                                      (unsigned int        ArgIndex)
 {
  if (!Ok)
   {
    return(0);
   }

  if (lua_isnumber(State,ArgIndex))
   {
    return((unsigned int)(lua_tonumber(State,ArgIndex) + 0.5));
   }
  else
   {
    Ok = false;

    PushErrorString("%d argument must be an unsigned integer",ArgIndex);

    return(0);
   }
 }

unsigned int       P3DPlugLUAControl::GetArgUIntOpt
                                      (unsigned int        ArgIndex,
                                       unsigned int        DefValue)
 {
  if (!Ok)
   {
    return(0);
   }

  if ((unsigned int)lua_gettop(State) < ArgIndex)
   {
    return(DefValue);
   }

  if (lua_isnumber(State,ArgIndex))
   {
    return((unsigned int)(lua_tonumber(State,ArgIndex) + 0.5));
   }
  else
   {
    Ok = false;

    PushErrorString("%d argument must be an unsigned integer",ArgIndex);

    return(DefValue);
   }
 }

bool               P3DPlugLUAControl::GetArgBoolOpt
                                      (unsigned int        ArgIndex,
                                       bool                DefValue)
 {
  if (!Ok)
   {
    return(0);
   }

  if ((unsigned int)lua_gettop(State) < ArgIndex)
   {
    return(DefValue);
   }

  if (lua_isboolean(State,ArgIndex))
   {
    return((bool)lua_toboolean(State,ArgIndex));
   }
  else
   {
    Ok = false;

    PushErrorString("%d argument must be a boolean",ArgIndex);

    return(DefValue);
   }
 }

void              *P3DPlugLUAControl::GetArgUserData
                                      (unsigned int        ArgIndex,
                                       const char         *MetaTableName)
 {
  void                                *UData;
  void                                *Result;

  if (!Ok)
   {
    return(NULL);
   }

  Result = NULL;
  UData  = lua_touserdata(State,ArgIndex);

  if (UData != NULL)
   {
    if (lua_getmetatable(State,ArgIndex))
     {
      lua_pushstring(State,MetaTableName);
      lua_gettable(State,LUA_REGISTRYINDEX);

      if (lua_rawequal(State,-1,-2))
       {
        Result = UData;
       }

      lua_pop(State,2);
     }
   }

  if (Result == NULL)
   {
    Ok = false;

    PushErrorString("%d argument must be a %s",ArgIndex,MetaTableName);
   }

  return(Result);
 }

void               P3DPlugLUAControl::CheckArgTable
                                      (unsigned int        ArgIndex)
 {
  if (!Ok)
   {
    return;
   }

  if (lua_istable(State,ArgIndex))
   {
   }
  else
   {
    Ok = false;

    PushErrorString("%d argument must be a table",ArgIndex);
   }
 }

void               P3DPlugLUAControl::PushString
                                      (const char         *Value)
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushstring(State,Value);
   }
 }

void               P3DPlugLUAControl::PushUInt
                                      (unsigned int        Value)
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushnumber(State,Value);
   }
 }

void               P3DPlugLUAControl::PushFloat
                                      (float               Value)
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushnumber(State,Value);
   }
 }

void               P3DPlugLUAControl::PushBool
                                      (bool                Value)
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushboolean(State,Value);
   }
 }

void               P3DPlugLUAControl::PushCFunction
                                      (lua_CFunction       Value)
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushcfunction(State,Value);
   }
 }

void               P3DPlugLUAControl::PushNil
                                      ()
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_pushnil(State);
   }
 }

void               P3DPlugLUAControl::PushNewTable
                                      ()
 {
  if (!Ok)
   {
    return;
   }

  if (CheckStack(1))
   {
    lua_newtable(State);
   }
 }

void               P3DPlugLUAControl::SetTable
                                      (int                 TableIndex)
 {
  if (!Ok)
   {
    return;
   }

  if (!lua_istable(State,TableIndex))
   {
    Ok = false;

    PushErrorString("trying to set field value for non-table type");

    return;
   }

  lua_settable(State,TableIndex);
 }

void               P3DPlugLUAControl::SetTable
                                      (int                 TableIndex,
                                       int                 Key)
 {
  if (!Ok)
   {
    return;
   }

  if (!lua_istable(State,TableIndex))
   {
    Ok = false;

    PushErrorString("trying to set field value for non-table type");

    return;
   }

  lua_rawseti(State,TableIndex,Key);
 }

void               P3DPlugLUAControl::SetTableUInt
                                      (const char         *Key,
                                       unsigned int        Value)
 {
  PushString(Key);
  PushUInt(Value);
  SetTable(-3);
 }

void               P3DPlugLUAControl::SetTableUInt
                                      (int                 Key,
                                       unsigned int        Value)
 {
  PushUInt(Value);
  SetTable(-2,Key);
 }

void               P3DPlugLUAControl::SetTableFloat
                                      (const char         *Key,
                                       float               Value)
 {
  PushString(Key);
  PushFloat(Value);
  SetTable(-3);
 }

void               P3DPlugLUAControl::SetTableFloat
                                      (int                 Key,
                                       float               Value)
 {
  PushFloat(Value);
  SetTable(-2,Key);
 }

void               P3DPlugLUAControl::SetTableString
                                      (const char         *Key,
                                       const char         *Value)
 {
  PushString(Key);
  PushString(Value);
  SetTable(-3);
 }

void               P3DPlugLUAControl::SetTableString
                                      (int                 Key,
                                       const char         *Value)
 {
  PushString(Value);
  SetTable(-2,Key);
 }

void               P3DPlugLUAControl::SetTableBool
                                      (const char         *Key,
                                       bool                Value)
 {
  PushString(Key);
  PushBool(Value);
  SetTable(-3);
 }

void               P3DPlugLUAControl::SetTableNil
                                      (const char         *Key)
 {
  PushString(Key);
  PushNil();
  SetTable(-3);
 }

void               P3DPlugLUAControl::SetTableNil
                                      (int                 Key)
 {
  PushNil();
  SetTable(-2,Key);
 }

unsigned int       P3DPlugLUAControl::GetTableSizeI
                                      (int                 TableIndex) const
 {
  unsigned int     Size;
  bool             Done;

  Size = 0;
  Done = false;

  while (!Done)
   {
    lua_rawgeti(State,TableIndex,Size + 1);

    if (lua_isnil(State,-1))
     {
      Done = true;
     }
    else
     {
      Size++;
     }

    lua_pop(State,1);
   }

  return(Size);
 }

void               P3DPlugLUAControl::RegisterConstant
                                      (const char         *Name,
                                       unsigned int        Value)
 {
  PushString(Name);
  PushUInt(Value);
  SetTable(LUA_GLOBALSINDEX);
 }

bool               P3DPlugLUAControl::CheckStack
                                      (unsigned int        Count)
 {
  if (lua_checkstack(State,Count))
   {
    return(true);
   }
  else
   {
    /*FIXME: if Count > 1 then it is possible that where is place for error string */
    /* I have to check that case and push error string if there is space */
    Ok = false;

    return(false);
   }
 }

void               P3DPlugLUAControl::PushErrorString
                                      (const char         *Format,
                                       ...)
 {
  va_list                              VArgs;

  va_start(VArgs,Format);

  luaL_where(State,1);
  lua_pushvfstring(State,Format,VArgs);

  va_end(VArgs);

  lua_concat(State,2);
 }

void               P3DPlugLUAControl::RaiseError
                                      (const char         *Format,
                                       ...)
 {
  va_list                              VArgs;

  if (!Ok)
   {
    return;
   }

  va_start(VArgs,Format);

  luaL_where(State,1);
  lua_pushvfstring(State,Format,VArgs);

  va_end(VArgs);

  lua_concat(State,2);

  Ok = false;
 }


void               P3DPlugLUAControl::RegisterUserData
                                      (const char         *MetaTableName)
 {
  if (!Ok)
   {
    return;
   }

  /*FIXME: check for errors */
  luaL_newmetatable(State,MetaTableName);
  lua_pop(State,1);
 }

void              *P3DPlugLUAControl::CreateUserData
                                      (const char         *MetaTableName,
                                       unsigned int        Size)
 {
  void                                *Result;

  if (!Ok)
   {
    return(NULL);
   }

  Result = NULL;

  if (CheckStack(2))
   {
    /*FIXME: check for errors */
    Result = lua_newuserdata(State,Size);

    luaL_getmetatable(State,MetaTableName);
    lua_setmetatable(State,-2);
   }

  return(Result);
 }

void               P3DPlugLUAControl::SetMetaMethod
                                      (const char         *MetaTableName,
                                       const char         *MethodName,
                                       lua_CFunction       Method)
 {
  if (!Ok)
   {
    return;
   }

  lua_pushstring(State,MetaTableName);
  lua_gettable(State,LUA_REGISTRYINDEX);

  if (lua_istable(State,-1))
   {
    if (CheckStack(2))
     {
      lua_pushstring(State,MethodName);
      lua_pushcfunction(State,Method);
      lua_settable(State,-3);

      lua_pop(State,1);
     }
   }
  else
   {
    lua_pop(State,1);

    Ok = false;

    PushErrorString("metatable %s is undefined",MetaTableName);
   }
 }

void               P3DPlugLUAControl::Commit
                                      ()
 {
  if (!Ok)
   {
    lua_error(State);
   }
 }

