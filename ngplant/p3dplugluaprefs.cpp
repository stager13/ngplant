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

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
 }

#include <p3dplugluactl.h>
#include <p3dplugluaprefs.h>

extern void        P3DPlugLuaRegisterExportPrefs
                                      (lua_State          *State,
                                       const P3DExport3DPrefs
                                                          *Prefs)
 {
  P3DPlugLUAControl                    Control(State);

  Control.PushString("ExportPreferences");
  Control.PushNewTable();
  Control.SetTableUInt("HiddenGroupsExportMode",Prefs->HiddenGroupsExportMode);
  Control.SetTableUInt("OutVisRangeExportMode",Prefs->OutVisRangeExportMode);
  Control.SetTable(LUA_GLOBALSINDEX);

  Control.RegisterConstant("NGP_ALWAYS",P3D_ALWAYS);
  Control.RegisterConstant("NGP_NEVER",P3D_NEVER);
  Control.RegisterConstant("NGP_ASK",P3D_ASK);

  Control.Commit();
 }

