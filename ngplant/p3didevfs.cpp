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

#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/filename.h>

#include <ngpcore/p3diostream.h>
#include <ngpcore/p3dmodel.h>
#include <ngput/p3dospath.h>
#include <p3didevfs.h>

const char        *P3DIDEVFS::DIR_CHAR = "/";

std::string        P3DIDEVFS::System2Generic
                                      (const char         *FileName)
 {
  unsigned int                         EntryIndex;
  unsigned int                         EntryCount;
  bool                                 EntryFound;
  P3DPathName                          SystemFileName(FileName);

  if (!SystemFileName.MakeAbsolute())
   {
    return(std::string(""));
   }

  EntryFound = false;

  if (!ModelPath.empty())
   {
    P3DPathName BasePath(ModelPath.c_str());

    if (SystemFileName.MakeRelativeTo(&BasePath))
     {
      EntryFound = true;
     }
   }

  EntryIndex = 0;
  EntryCount = Entries.size();

  while ((EntryIndex < EntryCount) && (!EntryFound))
   {
    P3DPathName                       BasePath(Entries[EntryIndex].c_str());

    if (SystemFileName.MakeRelativeTo(&BasePath))
     {
      EntryFound = true;
     }

    if (!EntryFound)
     {
      EntryIndex++;
     }
   }

  if (!EntryFound)
   {
    return(std::string(""));
   }

  if (!SystemFileName.ToUNIX())
   {
    return(std::string(""));
   }

  return(std::string(SystemFileName.c_str()));
 }

static bool        P3DFileExists      (const char         *FileName)
 {
  FILE            *TempStream;

  TempStream = fopen(FileName,"rb");

  if (TempStream != NULL)
   {
    fclose(TempStream);

    return(true);
   }
  else
   {
    return(false);
   }
 }

std::string        P3DIDEVFS::Generic2System
                                      (const char         *FileName)
 {
  P3DPathName                          SystemFileName(FileName);
  unsigned int                         EntryIndex;
  unsigned int                         EntryCount;
  std::string                          FullPath;

  if (SystemFileName.IsAbsolute())
   {
    return(std::string(""));
   }

  if (!ModelPath.empty())
   {
    FullPath = JoinPathComponents(ModelPath,SystemFileName.c_str());

    if (P3DFileExists(FullPath.c_str()))
     {
      return(FullPath);
     }
   }

  EntryIndex = 0;
  EntryCount = Entries.size();

  while (EntryIndex < EntryCount)
   {
    FullPath = JoinPathComponents(Entries[EntryIndex],SystemFileName.c_str());

    if (P3DFileExists(FullPath.c_str()))
     {
      return(FullPath);
     }

    EntryIndex++;
   }

  return(std::string(""));
 }

const char        *P3DIDEVFS::GetModelPath
                                      () const
 {
  if (!ModelPath.empty())
   {
    return ModelPath.c_str();
   }
  else
   {
    return 0;
   }
 }

void               P3DIDEVFS::SetModelPath
                                      (const char         *DirName)
 {
  if (DirName != 0)
   {
    ModelPath = JoinPathComponents(DirName,P3D_LOCAL_TEXTURES_PATH);
   }
  else
   {
    ModelPath = "";
   }
 }

unsigned int       P3DIDEVFS::GetEntryCount
                                      () const
 {
  return(Entries.size());
 }

const char        *P3DIDEVFS::GetEntry(unsigned int        EntryIndex) const
 {
  if (EntryIndex < GetEntryCount())
   {
    return(Entries[EntryIndex].c_str());
   }
  else
   {
    return(0);
   }
 }

void               P3DIDEVFS::AppendEntry
                                      (const char         *DirName)
 {
  P3DPathName                          SystemFileName(DirName);

  if (SystemFileName.MakeAbsolute())
   {
    SystemFileName.Normalize();

    Entries.push_back(SystemFileName.c_str());
   }
 }

void               P3DIDEVFS::RemoveEntry
                                      (unsigned int        EntryIndex)
 {
  if (EntryIndex < GetEntryCount())
   {
    Entries.erase(Entries.begin() + EntryIndex);
   }
 }

void               P3DIDEVFS::Load    (const char         *FileName)
 {
  P3DInputStringStreamFile             SourceStream;
  char                                 PathName[256];

  SourceStream.Open(FileName);

  while (!SourceStream.Eof())
   {
    SourceStream.ReadString(PathName,sizeof(PathName));

    AppendEntry(PathName);
   }

  SourceStream.Close();
 }

void               P3DIDEVFS::Save    (const char         *FileName)
 {
 }

void               P3DIDEVFS::ClearEntries
                                      ()
 {
  Entries.clear();
 }

std::string        P3DIDEVFS::JoinPathComponents
                                      (const std::string  &path,
                                       const std::string  &name)
 {
  return path + std::string(DIR_CHAR) + name;
 }

