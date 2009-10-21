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

#include <p3dlog.h>

#include <p3dpluginfo.h>

                   P3DPluginInfo::P3DPluginInfo
                                      ()
 {
  Type = 0;
 }

static bool        IsSpaceChar        (char                Ch)
 {
  return((Ch == ' ') || (Ch == '\t'));
 }

static bool        ParseInfoString    (std::string        &Key,
                                       std::string        &Value,
                                       const char         *Str)
 {
  const char      *Tmp;
  const char      *End;

  while (IsSpaceChar(*Str)) Str++;

  Tmp = Str;

  while ((*Tmp > 0x20) && (*Tmp != ':')) Tmp++;

  if (Tmp == Str) return false;

  Key = std::string(Str,Tmp - Str);
  Str = Tmp;

  while (IsSpaceChar(*Str)) Str++;

  if (*Str != ':') return false;

  Str++;

  while (IsSpaceChar(*Str)) Str++;

  Tmp = Str;
  End = Str;

  while (*Tmp != 0)
   {
    if (IsSpaceChar(*Tmp))
     {
      End = Tmp; Tmp++;

      while (IsSpaceChar(*Tmp)) Tmp++;
     }
    else
     {
      End = Str;
      Tmp++;
     }
   }

  if (Tmp == Str) return false;

  if (End == Str)
   {
    Value = std::string(Str);
   }
  else
   {
    Value = std::string(Str,End - Str);
   }

  return(true);
 }

bool               P3DPluginInfo::LoadInfo
                                      (P3DInputStringStream
                                                          *SourceStream,
                                       const char         *FileName)
 {
  bool             Found;
  unsigned int     LineIndex;
  char             Buffer[256];
  std::string      Key;
  std::string      Value;
  unsigned int     TempType;

  P3DLogInfo(wxT("Searching plugin header in %s\n"),wxString(FileName,wxConvUTF8).c_str());

  Found     = false;
  LineIndex = 0;

  while ((!Found) && (LineIndex < 5) && (!SourceStream->Eof()))
   {
    SourceStream->ReadString(Buffer,sizeof(Buffer));

    Found = ParseInfoString(Key,Value,Buffer);

    if (Found)
     {
      if (Key != "ngPlant-plugin")
       {
        Found = false;
       }
     }

    LineIndex++;
   }

  if (!Found)
   {
    P3DLogInfo(wxT("not a plugin - \"ngPlant-plugin\" key not found\n"));

    return(false);
   }

  if (Found)
   {
    if      (Value == "model-export")
     {
      TempType = P3DPluginTypeModelExport;
     }
    else if (Value == "gmesh-generator")
     {
      TempType = P3DPluginTypeGMeshGenerator;
     }
    else
     {
      P3DLogInfo(wxT("not a plugin - invalid/unknown plugin type\n"));

      return(false);
     }
   }

  SourceStream->ReadString(Buffer,sizeof(Buffer));

  if (!ParseInfoString(Key,Value,Buffer))
   {
    P3DLogInfo(wxT("not a plugin - invalid info string format\n"));

    return(false);
   }

  if (Key != "menu-name")
   {
    P3DLogInfo(wxT("not a plugin - \"menu-name\" key not found\n"));

    return(false);
   }

  Type           = TempType;
  MenuName       = Value;
  this->FileName = FileName;

  P3DLogInfo(wxT("plugin header successfully parsed\n"));

  return(true);
 }

unsigned int       P3DPluginInfo::GetType
                                      () const
 {
  return(Type);
 }

const char        *P3DPluginInfo::GetMenuName
                                      () const
 {
  return(MenuName.c_str());
 }

const char        *P3DPluginInfo::GetFileName
                                      () const
 {
  return(FileName.c_str());
 }

