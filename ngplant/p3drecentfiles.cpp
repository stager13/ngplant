/***************************************************************************

 Copyright (C) 2014  Sergey Prokhorchuk

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

#include <algorithm>

#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/filename.h>

#include "p3drecentfiles.h"

              P3DRecentFiles::P3DRecentFiles    (int            startId)
 {
  this->startId = startId;

  LoadConfig();
 }

wxMenu       *P3DRecentFiles::CreateMenu        () const
 {
  wxMenu *menu = new wxMenu();

  FillMenu(menu);

  return menu;
 }

void          P3DRecentFiles::UpdateMenu        (wxMenu        *menu) const
 {
  ClearMenu(menu);
  FillMenu(menu);
 }

void          P3DRecentFiles::ClearMenu         (wxMenu        *menu) const
 {
  size_t n = menu->GetMenuItemCount();

  while (n > 0)
   {
    menu->Destroy(menu->FindItemByPosition(--n));
   }
 }

void          P3DRecentFiles::FillMenu          (wxMenu        *menu) const
 {
  int id = startId;

  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end();
       ++it)
   {
    menu->Append(id++,wxString(it->c_str(),wxConvUTF8));
   }
 }

void          P3DRecentFiles::OnFileSaved       (const char    *name)
 {
  AddToRecentFiles(name);
 }

void          P3DRecentFiles::OnFileOpened      (const char    *name)
 {
  AddToRecentFiles(name);
 }

void          P3DRecentFiles::AddToRecentFiles  (const char    *name)
 {
  std::string absFileName = GetAbsoluteFileName(name);

  RemoveEntry(absFileName);
  PrependEntry(absFileName);

  if (files.size() > P3D_RECENT_FILES_MAX)
   {
    files.pop_back();
   }

  SaveConfig();
 }

bool          P3DRecentFiles::IsEmpty           () const
 {
  return files.empty();
 }

const char   *P3DRecentFiles::GetFileName       (unsigned int   index) const
 {
  if (index < files.size())
   {
    return files[index].c_str();
   }
  else
   {
    return NULL;
   }
 }

void          P3DRecentFiles::LoadConfig        ()
 {
  wxConfigBase                        *Cfg;

  Cfg = wxConfigBase::Get();

  if (Cfg->HasGroup(wxT("/RecentFiles")))
   {
    Cfg->SetPath(wxT("/RecentFiles"));

    bool     Cont;
    wxString EntryName;
    wxString FileName;
    long     Cookie;

    Cont = Cfg->GetFirstEntry(EntryName,Cookie);

    while (Cont)
     {
      if (Cfg->Read(EntryName,&FileName))
       {
        files.push_back(std::string(FileName.mb_str()));
       }

      Cont = Cfg->GetNextEntry(EntryName,Cookie);
     }

    Cfg->SetPath(wxT("/"));
   }
 }

void          P3DRecentFiles::SaveConfig        () const
 {
  wxConfigBase                        *Cfg;

  Cfg = wxConfigBase::Get();

  Cfg->DeleteGroup(wxT("/RecentFiles"));
  Cfg->SetPath(wxT("/RecentFiles"));

  for (unsigned int Index = 0; Index < files.size(); Index++)
   {
    wxString EntryName = wxString::Format(wxT("File%u"),Index);

    Cfg->Write(EntryName,wxString(files[Index].c_str(),wxConvUTF8));
   }

  Cfg->SetPath(wxT("/"));
  Cfg->Flush();
 }

void          P3DRecentFiles::PrependEntry      (const std::string  &name)
 {
  files.insert(files.begin(),name);
 }

void          P3DRecentFiles::RemoveEntry       (const std::string  &name)
 {
  std::vector<std::string>::iterator it = std::find(files.begin(),files.end(),name);

  if (it != files.end())
   {
    files.erase(it);
   }
 }

std::string   P3DRecentFiles::GetAbsoluteFileName
                                                (const char         *name)
 {
  wxFileName absFileName(wxString(name,wxConvUTF8));

  absFileName.MakeAbsolute();

  return std::string(absFileName.GetFullPath().mb_str());
 }

