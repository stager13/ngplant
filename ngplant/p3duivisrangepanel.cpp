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
#include <p3dcmdhelpers.h>
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
  spin_slider->SetStdStep(0.1f);
  spin_slider->SetSmallStep(0.01f);
  spin_slider->SetLargeMove(0.05f);
  spin_slider->SetStdMove(0.02f);
  spin_slider->SetSmallMove(0.01f);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Max. range */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Max. LOD")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_VISRANGE_MAX_CTRL,wxSPINSLIDER_MODE_FLOAT,MaxRange,0.0f,1.0f);
  spin_slider->SetStdStep(0.1f);
  spin_slider->SetSmallStep(0.01f);
  spin_slider->SetLargeMove(0.05f);
  spin_slider->SetStdMove(0.02f);
  spin_slider->SetSmallMove(0.01f);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  TopSizer->Add(GridSizer,0,wxEXPAND,0);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

namespace {

class SetVisRangeCommand : public P3DEditCommand
 {
  public           :

                   SetVisRangeCommand (P3DVisRangeState   *VisRangeState,
                                       float               NewRangeMin,
                                       float               NewRangeMax)
   {
    this->VisRangeState = VisRangeState;
    this->NewRangeMin   = NewRangeMin;
    this->NewRangeMax   = NewRangeMax;

    VisRangeState->GetRange(&OldRangeMin,&OldRangeMax);
   }

  virtual void     Exec               ()
   {
    VisRangeState->SetRange(NewRangeMin,NewRangeMax);

    P3DApp::GetApp()->InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    VisRangeState->SetRange(OldRangeMin,OldRangeMax);

    P3DApp::GetApp()->InvalidatePlant();
   }

  private          :

  P3DVisRangeState                    *VisRangeState;
  float                                NewRangeMin,NewRangeMax;
  float                                OldRangeMin,OldRangeMax;
 };
}

void               P3DVisRangePanel::OnMinChanged
                                      (wxSpinSliderEvent  &event)
 {
  float                                OldMin;
  float                                OldMax;
  float                                NewMin;
  float                                NewMax;

  State->GetRange(&OldMin,&OldMax);

  NewMin = event.GetFloatValue();
  NewMax = NewMin > OldMax ? NewMin : OldMax;

  if (NewMin > OldMax)
   {
    NewMax = NewMin;

    wxSpinSliderCtrl *MaxCtrl = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MAX_CTRL);

    MaxCtrl->SetValue(NewMax);
   }
  else
   {
    NewMax = OldMax;
   }

  P3DApp::GetApp()->ExecEditCmd
   (new SetVisRangeCommand(State,NewMin,NewMax));
 }

void               P3DVisRangePanel::OnMaxChanged
                                      (wxSpinSliderEvent  &event)
 {
  float                                OldMin;
  float                                OldMax;
  float                                NewMin;
  float                                NewMax;

  State->GetRange(&OldMin,&OldMax);

  NewMax = event.GetFloatValue();

  if (NewMax < OldMin)
   {
    NewMin = NewMax;

    wxSpinSliderCtrl *MinCtrl = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MIN_CTRL);

    MinCtrl->SetValue(NewMin);
   }
  else
   {
    NewMin = OldMin;
   }

  P3DApp::GetApp()->ExecEditCmd
   (new SetVisRangeCommand(State,NewMin,NewMax));
 }

typedef P3DParamEditCmdTemplate<P3DVisRangeState,bool> P3DVisRangeBoolParamEditCmd;

void               P3DVisRangePanel::OnEnabledChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DVisRangeBoolParamEditCmd
         (State,
          event.IsChecked(),
          State->IsEnabled(),
          &P3DVisRangeState::SetState));
 }

#define model State

void               P3DVisRangePanel::UpdateControls
                                      ()
 {
  float                                MinValue;
  float                                MaxValue;
  wxSpinSliderCtrl                    *SpinSlider;

  State->GetRange(&MinValue,&MaxValue);

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MIN_CTRL);

  if (SpinSlider != NULL)
   {
    SpinSlider->SetValue(MinValue);
   }

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(wxID_VISRANGE_MAX_CTRL);

  if (SpinSlider != NULL)
   {
    SpinSlider->SetValue(MaxValue);
   }

  P3DUpdateParamCheckBox(wxID_VISRANGE_ENABLED_CTRL,IsEnabled);
 }

#undef model

