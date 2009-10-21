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

#ifndef __P3DWXLISTDLG_H__
#define __P3DWXLISTDLG_H__

#include <wx/wx.h>

class P3DListDialog : public wxDialog
 {
  public           :

                   P3DListDialog      ();
                   P3DListDialog      (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Select"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  bool             Create             (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Select"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void             AddChoice          (const char         *NameStrPtr);
  int              GetSelection       () const;
  void             SetSelection       (int                 Index);

  private          :

  void             Init               ();
  void             CreateControls     ();

  wxListBox       *ListBoxCtrl;

  DECLARE_CLASS(P3DListDialog)
 };

#endif

