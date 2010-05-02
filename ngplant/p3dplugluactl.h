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

#ifndef __P3DPLUGLUACTL_H__
#define __P3DPLUGLUACTL_H__

extern "C"
 {
  #include <lua.h>
 }

class P3DPlugLUAControl
 {
  public           :

                   P3DPlugLUAControl  (lua_State          *State);

  bool             IsOk               () const;

  const char      *GetArgString       (unsigned int        ArgIndex);
  unsigned int     GetArgUInt         (unsigned int        ArgIndex);

  unsigned int     GetArgUIntOpt      (unsigned int        ArgIndex,
                                       unsigned int        DefValue);

  bool             GetArgBoolOpt      (unsigned int        ArgIndex,
                                       bool                DefValue);

  void            *GetArgUserData     (unsigned int        ArgIndex,
                                       const char         *MetaTableName);

  void             CheckArgTable      (unsigned int        ArgIndex);

  void             PushString         (const char         *Value);
  void             PushUInt           (unsigned int        Value);
  void             PushFloat          (float               Value);
  void             PushBool           (bool                Value);
  void             PushCFunction      (lua_CFunction       Value);
  void             PushNil            ();
  void             PushNewTable       ();

  void             SetTable           (int                 TableIndex);
  void             SetTable           (int                 TableIndex,
                                       int                 Key);
  void             SetTableUInt       (const char         *Key,
                                       unsigned int        Value);
  void             SetTableUInt       (int                 Key,
                                       unsigned int        Value);
  void             SetTableFloat      (const char         *Key,
                                       float               Value);
  void             SetTableFloat      (int                 Key,
                                       float               Value);
  void             SetTableString     (const char         *Key,
                                       const char         *Value);
  void             SetTableString     (int                 Key,
                                       const char         *Value);
  void             SetTableBool       (const char         *Key,
                                       bool                Value);
  void             SetTableNil        (const char         *Key);
  void             SetTableNil        (int                 Key);

  unsigned int     GetTableSizeI      (int                 TableIndex) const;

  void             RegisterUserData   (const char         *MetaTableName);
  void             SetMetaMethod      (const char         *MetaTableName,
                                       const char         *MethodName,
                                       lua_CFunction       Method);

  void            *CreateUserData     (const char         *MetaTableName,
                                       unsigned int        Size);

  void             RegisterConstant   (const char         *Name,
                                       unsigned int        Value);

  void             Commit             (); /* calls lua_error if !IsOk() */

  void             RaiseError         (const char         *Format,
                                       ...);

  void             PushErrorString    (const char         *Format,
                                       ...);

  private          :

  bool             CheckStack         (unsigned int        Count);

  lua_State       *State;
  bool             Ok;
 };

#endif

