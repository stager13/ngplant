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

#include <wx/wx.h>

#include <p3dwxlistdlg.h>

IMPLEMENT_CLASS(P3DListDialog,wxDialog)

                   P3DListDialog::P3DListDialog
                                      ()
 {
  Init();
 }

                   P3DListDialog::P3DListDialog
                                      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  Init();
  Create(Parent,Id,Caption,Pos,Size,Style);
 }

bool               P3DListDialog::Create
                                      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  if (!wxDialog::Create(Parent,Id,Caption,Pos,Size,Style))
   {
    return(false);
   }

  CreateControls();

  return(true);
 }

void               P3DListDialog::AddChoice
                                      (const char         *NameStrPtr)
 {
  ListBoxCtrl->Append(wxString(NameStrPtr,wxConvUTF8));
 }

int                P3DListDialog::GetSelection
                                      () const
 {
  return(ListBoxCtrl->GetSelection());
 }

void               P3DListDialog::SetSelection
                                      (int                 Index)
 {
  ListBoxCtrl->SetSelection(Index);
 }

void               P3DListDialog::Init()
 {
 }

void               P3DListDialog::CreateControls
                                      ()
 {
  wxBoxSizer      *TopSizer = new wxBoxSizer(wxVERTICAL);

  ListBoxCtrl = new wxListBox(this,
                              wxID_ANY,
                              wxDefaultPosition,
                              wxDLG_UNIT(this,wxSize(16 * 8,8 * 8)),
                              0,
                              NULL,
                              wxLB_SINGLE);

  TopSizer->Add(ListBoxCtrl,1,wxGROW | wxALL,5);

  wxBoxSizer      *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  ButtonSizer->Add(new wxButton(this,wxID_OK,wxT("Ok")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")),0,wxALL,5);

  ((wxButton*)FindWindow(wxID_OK))->SetDefault();

  TopSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxALL,5);

  SetSizer(TopSizer);
  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

