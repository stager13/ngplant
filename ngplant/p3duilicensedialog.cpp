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

#include <wx/wx.h>

#include <p3duilicensedialog.h>

enum
 {
  wxID_ADAPTATION_MODE_RADIOBOX = wxID_HIGHEST + 1,
  wxID_COMMERCIAL_MODE_RADIOBOX
 };

BEGIN_EVENT_TABLE(P3DLicenseDialog,wxDialog)
 EVT_RADIOBOX(wxID_ADAPTATION_MODE_RADIOBOX,P3DLicenseDialog::OnAdaptationModeChanged)
 EVT_RADIOBOX(wxID_COMMERCIAL_MODE_RADIOBOX,P3DLicenseDialog::OnCommercialModeChanged)
END_EVENT_TABLE()

                   P3DLicenseDialog::P3DLicenseDialog
                                      ()
 {
  Init();
 }

                   P3DLicenseDialog::P3DLicenseDialog
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

void               P3DLicenseDialog::Init
                                      ()
 {
  AdaptationMode      = ADAPTATION_MODE_ALLOW;
  AllowCommercialUses = true;
 }

bool               P3DLicenseDialog::Create
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

void               P3DLicenseDialog::CreateControls
                                      ()
 {
  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  TopSizer->Add(CreateInfoBox(),0,wxEXPAND | wxALL,5);
  TopSizer->Add(CreateCCBox(),1,wxEXPAND | wxALL,5);
  TopSizer->Add(CreateOkCancelBox(),0,wxALIGN_RIGHT | wxALL,5);

  SetSizerAndFit(TopSizer);

  UpdateLicenseName();
 }

wxSizer           *P3DLicenseDialog::CreateInfoBox
                                      ()
 {
  wxBoxSizer *Sizer = new wxBoxSizer(wxVERTICAL);

  // the most wide license is used here for making dialog wide enough
  NameText = new wxStaticText(this,wxID_ANY,GetLicenseName(ADAPTATION_MODE_DISALLOW,false));

  wxFont TextFont = NameText->GetFont();

  TextFont.SetWeight(wxFONTWEIGHT_BOLD);

  NameText->SetFont(TextFont);

  Sizer->Add(NameText,1,wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL,0);

  return Sizer;
 }

wxSizer           *P3DLicenseDialog::CreateCCBox
                                      ()
 {
  wxBoxSizer *CCSizer = new wxBoxSizer(wxVERTICAL);

  wxString AdaptaionModeNames[] =
   {
    wxT("Yes"),wxT("No"),wxT("Yes, as long as others share alike ")
   };

  wxRadioBox *AdaptationModeRadioBox =
   new wxRadioBox(this,wxID_ADAPTATION_MODE_RADIOBOX,
                  wxT("Allow adaptations of your work to be shared?"),
                  wxDefaultPosition,wxDefaultSize,
                  WXSIZEOF(AdaptaionModeNames),AdaptaionModeNames,1);

  CCSizer->Add(AdaptationModeRadioBox,0,wxEXPAND | wxALL,5);

  wxString CommercialModeNames[] =
   {
    wxT("Yes"),wxT("No")
   };

  wxRadioBox *CommercialModeRadioBox =
   new wxRadioBox(this,wxID_COMMERCIAL_MODE_RADIOBOX,
                  wxT("Allow commercial uses of your work?"),
                  wxDefaultPosition,wxDefaultSize,
                  WXSIZEOF(CommercialModeNames),CommercialModeNames,1);

  CCSizer->Add(CommercialModeRadioBox,0,wxEXPAND | wxALL,5);

  return CCSizer;
 }

wxSizer           *P3DLicenseDialog::CreateOkCancelBox
                                      ()
 {
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

  sizer->Add(new wxButton(this,wxID_OK,wxT("Ok")));
  sizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")));

  return sizer;
 }

void               P3DLicenseDialog::UpdateLicenseName
                                      ()
 {
  NameText->SetLabel(GetLicenseName());

  Layout();
 }

void               P3DLicenseDialog::OnAdaptationModeChanged
                                      (wxCommandEvent     &event)
 {
  AdaptationMode = event.GetInt();

  UpdateLicenseName();
 }

void               P3DLicenseDialog::OnCommercialModeChanged
                                      (wxCommandEvent     &event)
 {
  AllowCommercialUses = event.GetInt() == 0;

  UpdateLicenseName();
 }

wxString           P3DLicenseDialog::GetLicenseName
                                      () const
 {
  return GetLicenseName(AdaptationMode,AllowCommercialUses);
 }

wxString           P3DLicenseDialog::GetLicenseName
                                      (int                 AdaptationMode,
                                       bool                AllowCommercialUses)
 {
  wxString         Name;

  Name = wxT("Attribution");

  if (!AllowCommercialUses)
   {
    Name += wxT("-NonCommercial");
   }

  if (AdaptationMode == ADAPTATION_MODE_DISALLOW)
   {
    Name += wxT("-NoDerivatives");
   }
  else if (AdaptationMode == ADAPTATION_MODE_ALLOW_SIMILAR)
   {
    Name += wxT("-ShareAlike");
   }

  Name += wxT(" 4.0 International");

  return Name;
 }

wxString           P3DLicenseDialog::GetLicenseURL
                                      () const
 {
  return GetLicenseURL(AdaptationMode,AllowCommercialUses);
 }

wxString           P3DLicenseDialog::GetLicenseURL
                                      (int                 AdaptationMode,
                                       bool                AllowCommercialUses)
 {
  wxString URL;

  URL = wxT("http://creativecommons.org/licenses/by");

  if (!AllowCommercialUses)
   {
    URL += wxT("-nc");
   }

  if (AdaptationMode == ADAPTATION_MODE_DISALLOW)
   {
    URL += wxT("-nd");
   }
  else if (AdaptationMode == ADAPTATION_MODE_ALLOW_SIMILAR)
   {
    URL += wxT("-sa");
   }

  URL += wxT("/4.0/legalcode");

  return URL;
 }

