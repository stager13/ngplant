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

#ifndef __P3DUIOPTGENERAL_H__
#define __P3DUIOPTGENERAL_H__

#include <wx/wx.h>

#include <p3dwx.h>
#include <p3duiparampanel.h>

class P3DOptGeneralPanel : public P3DUIParamPanel
 {
  public           :

                   P3DOptGeneralPanel (wxWindow           *parent);

  void             OnSeedChanged      (wxSpinSliderEvent  &event);
  void             OnLODChanged       (wxSpinSliderEvent  &event);
  void             OnRandomnessStateChanged
                                      (wxCommandEvent     &event);
  void             OnAuthorChanged    (wxCommandEvent     &event);
  void             OnLicenseNameChanged
                                      (wxCommandEvent     &event);
  void             OnLicenseURLChanged(wxCommandEvent     &event);
  void             OnPlantInfoURLChanged
                                      (wxCommandEvent     &event);

  virtual void     UpdateControls     ();

  private          :

  wxSizer         *CreateRandomnessBox();
  wxSizer         *CreateLODBox       ();
  wxSizer         *CreateModelInfoBox ();

  wxTextCtrl      *CreateInfoTextCtrl (int                 id,
                                       const char         *value);

  void             UpdateTextCtrl     (int                 id,
                                       const char         *value);

  static
  wxString         InfoValueToText    (const char         *value);
  static
  const char      *TextToInfoValue    (const wxString     &text);

  DECLARE_EVENT_TABLE();
 };

#endif

