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

#include <ngpcore/p3dmodel.h>

#include <p3dapp.h>
#include <p3duivisrangepanel.h>

enum
 {
  wxID_VISRANGE_MIN_CTRL = wxID_HIGHEST + 5100,
  wxID_VISRANGE_MAX_CTRL                      ,
  wxID_VISRANGE_ENABLED_CTRL
 };

BEGIN_EVENT_TABLE(P3DVisRangePanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_VISRANGE_MIN_CTRL,P3DVisRangePanel::OnMinChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_VISRANGE_MAX_CTRL,P3DVisRangePanel::OnMaxChanged)
 EVT_CHECKBOX(wxID_VISRANGE_ENABLED_CTRL,P3DVisRangePanel::OnEnabledChanged)
END_EVENT_TABLE()

                   P3DVisRangePanel::P3DVisRangePanel
                                      (wxWindow           *Parent,
                                       P3DVisRangeState   *VisRangeState)
                   : wxPanel(Parent)
 {
  float            MinRange;
  float            MaxRange;

  State = VisRangeState;

  State->GetRange(&MinRange,&MaxRange);

  wxStaticBoxSizer *TopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Visibility range")),wxVERTICAL);
  wxFlexGridSizer  *GridSizer = new wxFlexGridSizer(3,2,3,3);

  GridSizer->AddGrowableCol(1);

  /* Enabled */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Enabled")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *EnabledCheckBox = new wxCheckBox(this,wxID_VISRANGE_ENABLED_CTRL,wxT(""));
  EnabledCheckBox->SetValue(State->IsEnabled());

  GridSizer->Add(EnabledCheckBox,1,wxALL | wxALIGN_RIGHT,1);

  /* Min. range */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Min. LOD")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_VISRANGE_MIN_CTRL,wxSPINSLIDER_MODE_FLOAT,MinRange,0.0f,1.0f);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.05);
  spin_slider->SetStdMove(0.02);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Max. range */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Max. LOD")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_VISRANGE_MAX_CTRL,wxSPINSLIDER_MODE_FLOAT,MaxRange,0.0f,1.0f);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.05);
  spin_slider->SetStdMove(0.02);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  TopSizer->Add(GridSizer,0,wxEXPAND,0);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DVisRangePanel::OnMinChanged
                                      (wxSpinSliderEvent  &event)
 {
  float                                MinValue;
  float                                OldMin;
  float                                OldMax;

  State->GetRange(&OldMin,&OldMax);

  MinValue = event.GetFloatValue();

  if (MinValue > OldMax)
   {
    wxSpinSliderCtrl *MaxCtrl = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MAX_CTRL);

    MaxCtrl->SetValue(MinValue);

    State->SetRange(MinValue,MinValue);
   }
  else
   {
    State->SetRange(MinValue,OldMax);
   }

  wxGetApp().InvalidatePlant();
 }

void               P3DVisRangePanel::OnMaxChanged
                                      (wxSpinSliderEvent  &event)
 {
  float                                MaxValue;
  float                                OldMin;
  float                                OldMax;

  State->GetRange(&OldMin,&OldMax);

  MaxValue = event.GetFloatValue();

  if (MaxValue < OldMin)
   {
    wxSpinSliderCtrl *MinCtrl = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MIN_CTRL);

    MinCtrl->SetValue(MaxValue);

    State->SetRange(MaxValue,MaxValue);
   }
  else
   {
    State->SetRange(OldMin,MaxValue);
   }

  wxGetApp().InvalidatePlant();
 }

void               P3DVisRangePanel::OnEnabledChanged
                                      (wxCommandEvent     &event)
 {
  if (event.IsChecked())
   {
    State->SetState(true);
   }
  else
   {
    State->SetState(false);
   }

  wxGetApp().InvalidatePlant();
 }

