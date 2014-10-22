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

#ifndef __P3DLICENSEDIALOG_H__
#define __P3DLICENSEDIALOG_H__

#include <wx/wx.h>

class P3DLicenseDialog : public wxDialog
 {
  public           :

                   P3DLicenseDialog   ();
                   P3DLicenseDialog   (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Choose a Creative Commons license"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxDEFAULT_DIALOG_STYLE);

  void             Init               ();

  bool             Create             (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Choose a Creative Commons license"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxDEFAULT_DIALOG_STYLE);

  wxString         GetLicenseName     () const;
  wxString         GetLicenseURL      () const;

  private          :

  enum { ADAPTATION_MODE_ALLOW,
         ADAPTATION_MODE_DISALLOW,
         ADAPTATION_MODE_ALLOW_SIMILAR };

  void             CreateControls     ();

  wxSizer         *CreateInfoBox      ();
  wxSizer         *CreateCCBox        ();
  wxSizer         *CreateOkCancelBox  ();

  void             UpdateLicenseName  ();

  void             OnAdaptationModeChanged (wxCommandEvent &event);
  void             OnCommercialModeChanged (wxCommandEvent &event);

  static wxString  GetLicenseName     (int                 AdaptationMode,
                                       bool                AllowCommercialUses);
  static wxString  GetLicenseURL      (int                 AdaptationMode,
                                       bool                AllowCommercialUses);

  wxStaticText    *NameText;

  int              AdaptationMode;
  bool             AllowCommercialUses;

  DECLARE_EVENT_TABLE()
 };

#endif

