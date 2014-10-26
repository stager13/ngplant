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

#ifndef __P3DIDEVFS_H__
#define __P3DIDEVFS_H__

#include <string>
#include <vector>

class P3DIDEVFS
 {
  public           :

  std::string      System2Generic     (const char         *FileName);
  std::string      Generic2System     (const char         *FileName);

  const char      *GetModelPath       () const;
  void             SetModelPath       (const char         *DirName);

  unsigned int     GetEntryCount      () const;
  const char      *GetEntry           (unsigned int        EntryIndex) const;
  void             AppendEntry        (const char         *DirName);
  void             RemoveEntry        (unsigned int        EntryIndex);

  void             Load               (const char         *FileName);
  void             Save               (const char         *FileName);

  private          :

  void             ClearEntries       ();
  std::string      JoinPathComponents (const std::string  &path,
                                       const std::string  &name);

  std::string                          ModelPath;
  std::vector<std::string>             Entries;

  static const char                   *DIR_CHAR;
  static const char                   *SYS_DIR_CHAR;
 };

#endif

