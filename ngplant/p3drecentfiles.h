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

#ifndef __P3DRECENTFILES_H__
#define __P3DRECENTFILES_H__

#include <string>
#include <vector>

#include <wx/wx.h>

#define P3D_RECENT_FILES_MAX (10)

class P3DRecentFiles
 {
  public           :

                   P3DRecentFiles     (int                 startId);

  wxMenu          *CreateMenu         () const;
  void             UpdateMenu         (wxMenu             *menu) const;

  void             OnFileSaved        (const char         *name);
  void             OnFileOpened       (const char         *name);

  bool             IsEmpty            () const;
  const char      *GetFileName        (unsigned int        index) const;

  private          :

  void             LoadConfig         ();
  void             SaveConfig         () const;

  void             AddToRecentFiles   (const char         *name);
  void             PrependEntry       (const std::string  &name);
  void             RemoveEntry        (const std::string  &name);

  void             ClearMenu          (wxMenu             *menu) const;
  void             FillMenu           (wxMenu             *menu) const;

  static
  std::string      GetAbsoluteFileName(const char         *name);

  int                                  startId;
  std::vector<std::string>             files;
 };

#endif

