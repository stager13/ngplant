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

#include <wx/wx.h>

#include <p3dwx.h>

#include <p3dapp.h>
#include <p3dcmdhelpers.h>
#include <p3duibalgbase.h>

enum
 {
  wxID_SPREAD_CTRL  = wxID_HIGHEST + 1,
  wxID_DENSITY_CTRL,
  wxID_DENSITYV_CTRL,
  wxID_MIN_NUMBER_CTRL,
  wxID_MAX_LIMIT_ENABLED_CTRL,
  wxID_MAX_NUMBER_CTRL,
  wxID_ROTANGLE_CTRL
 };

BEGIN_EVENT_TABLE(P3DBranchingAlgBasePanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_SPREAD_CTRL,P3DBranchingAlgBasePanel::OnSpreadChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_DENSITY_CTRL,P3DBranchingAlgBasePanel::OnDensityChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_DENSITYV_CTRL,P3DBranchingAlgBasePanel::OnDensityVChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_MIN_NUMBER_CTRL,P3DBranchingAlgBasePanel::OnMinNumberChanged)
 EVT_CHECKBOX(wxID_MAX_LIMIT_ENABLED_CTRL,P3DBranchingAlgBasePanel::OnMaxLimitEnabledChanged) EVT_SPINSLIDER_VALUE_CHANGED(wxID_MAX_NUMBER_CTRL,P3DBranchingAlgBasePanel::OnMaxNumberChanged) EVT_SPINSLIDER_VALUE_CHANGED(wxID_ROTANGLE_CTRL,P3DBranchingAlgBasePanel::OnRotAngleChanged)
END_EVENT_TABLE()

/*FIXME: copy of P3DBAlgStdMinMaxNumberEditCmd */
class P3DBAlgBaseMinMaxNumberEditCmd : public P3DEditCommand
 {
  public           :

  P3DBAlgBaseMinMaxNumberEditCmd(P3DBranchingAlgBase      *Alg,
                                 unsigned int              NewMinNumber,
                                 unsigned int              NewMaxNumber,
                                 bool                      NewMaxLimitEnabled,
                                 unsigned int              OldMinNumber,
                                 unsigned int              OldMaxNumber,
                                 bool                      OldMaxLimitEnabled)
   {
    this->Alg                = Alg;
    this->NewMinNumber       = NewMinNumber;
    this->NewMaxNumber       = NewMaxNumber;
    this->NewMaxLimitEnabled = NewMaxLimitEnabled;
    this->OldMinNumber       = OldMinNumber;
    this->OldMaxNumber       = OldMaxNumber;
    this->OldMaxLimitEnabled = OldMaxLimitEnabled;
   }

  virtual void Exec ()
   {
    Alg->SetMinNumber(NewMinNumber);
    Alg->SetMaxLimitEnabled(NewMaxLimitEnabled);
    Alg->SetMaxNumber(NewMaxNumber);
    wxGetApp().InvalidatePlant();
   }

  virtual void Undo ()
   {
    Alg->SetMinNumber(OldMinNumber);
    Alg->SetMaxLimitEnabled(OldMaxLimitEnabled);
    Alg->SetMaxNumber(OldMaxNumber);
    wxGetApp().InvalidatePlant();
   }

  private          :

  P3DBranchingAlgBase      *Alg;
  unsigned int              NewMinNumber;
  unsigned int              NewMaxNumber;
  bool                      NewMaxLimitEnabled;
  unsigned int              OldMinNumber;
  unsigned int              OldMaxNumber;
  bool                      OldMaxLimitEnabled;
 };


                   P3DBranchingAlgBasePanel::P3DBranchingAlgBasePanel
                                      (wxWindow           *Parent,
                                       P3DBranchingAlgBase*Alg)
                   : P3DUIParamPanel(Parent)
 {
  this->Alg = Alg;

  wxBoxSizer           *TopSizer        = new wxBoxSizer(wxVERTICAL);
  wxStaticBoxSizer     *ParametersSizer = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Parameters")),wxVERTICAL);
  wxFlexGridSizer      *GridSizer       = new wxFlexGridSizer(7,2,2,2);

  GridSizer->AddGrowableCol(1);

  /* Spread */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Spread")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *SpinSlider = new wxSpinSliderCtrl(this,wxID_SPREAD_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetSpread(),0.0,10.0);
  SpinSlider->SetSensitivity(0.1f,0.01f,0.2f,0.1f,0.01f);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);

  /* Density */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Density")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl(this,wxID_DENSITY_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetDensity(),0.01,100.0);
  SpinSlider->SetSensitivity(0.1f,0.01f,1.0f,0.1f,0.01f);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);


  /* DensityV */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl(this,wxID_DENSITYV_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetDensityV(),0.0,1.0);
  SpinSlider->SetSensitivity(0.1f,0.01f,0.2f,0.1f,0.01f);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);

  /* Min. number */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Min. number")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl(this,wxID_MIN_NUMBER_CTRL,wxSPINSLIDER_MODE_INTEGER,Alg->GetMinNumber(),0,1000);
  SpinSlider->SetSensitivity(1,1,2,1,1);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);

  /* Max. limit enabled */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Limit max.")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *EnabledCheckBox = new wxCheckBox(this,wxID_MAX_LIMIT_ENABLED_CTRL,wxT(""));
  EnabledCheckBox->SetValue(Alg->IsMaxLimitEnabled());

  GridSizer->Add(EnabledCheckBox,1,wxALL | wxALIGN_RIGHT,1);

  /* Max. number */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Max. number")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl(this,wxID_MAX_NUMBER_CTRL,wxSPINSLIDER_MODE_INTEGER,Alg->GetMaxNumber(),0,1000);
  SpinSlider->SetSensitivity(1,1,2,1,1);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);

  /* RotAngle */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("RotAngle")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl(this,wxID_ROTANGLE_CTRL,wxSPINSLIDER_MODE_INTEGER,(int)P3DMath::Roundf((P3DMATH_RAD2DEG(Alg->GetRotationAngle()))),0,359);
  SpinSlider->SetStdStep(1);
  SpinSlider->SetSmallStep(1);
  SpinSlider->SetLargeMove(2);
  SpinSlider->SetStdMove(1);
  SpinSlider->SetSmallMove(1);

  GridSizer->Add(SpinSlider,1,wxALL | wxALIGN_RIGHT,1);

  ParametersSizer->Add(GridSizer,0,wxGROW,0);

  TopSizer->Add(ParametersSizer,0,wxGROW | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

typedef P3DParamEditCmdTemplate<P3DBranchingAlgBase,float> P3DBAlgBaseFloatParamEditCmd;

void               P3DBranchingAlgBasePanel::OnSpreadChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetSpread(),
          &P3DBranchingAlgBase::SetSpread));
 }

void               P3DBranchingAlgBasePanel::OnDensityChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetDensity(),
          &P3DBranchingAlgBase::SetDensity));
 }

void               P3DBranchingAlgBasePanel::OnDensityVChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetDensityV(),
          &P3DBranchingAlgBase::SetDensityV));
 }

void               P3DBranchingAlgBasePanel::OnMinNumberChanged
                                      (wxSpinSliderEvent  &event)
 {
  unsigned int OldMinNumber = Alg->GetMinNumber();
  unsigned int OldMaxNumber = Alg->GetMaxNumber();
  unsigned int NewMinNumber = event.GetIntValue();
  bool         LimitEnabled = Alg->IsMaxLimitEnabled();
  unsigned int NewMaxNumber;

  if (OldMaxNumber < NewMinNumber)
   {
    NewMaxNumber = NewMinNumber;

    wxSpinSliderCtrl *SpinSlider = (wxSpinSliderCtrl*)FindWindow(wxID_MAX_NUMBER_CTRL);

    if (SpinSlider != NULL)
     {
      SpinSlider->SetValue(NewMaxNumber);
     }
   }
  else
   {
    NewMaxNumber = OldMaxNumber;
   }

  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseMinMaxNumberEditCmd
         (Alg,
          NewMinNumber,NewMaxNumber,LimitEnabled,
          OldMinNumber,OldMaxNumber,LimitEnabled));
 }

void               P3DBranchingAlgBasePanel::OnMaxLimitEnabledChanged
                                      (wxCommandEvent     &event)
 {
  unsigned int MinNumber = Alg->GetMinNumber();
  unsigned int MaxNumber = Alg->GetMaxNumber();

  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseMinMaxNumberEditCmd
         (Alg,
          MinNumber,MaxNumber,event.IsChecked(),
          MinNumber,MaxNumber,Alg->IsMaxLimitEnabled()));
 }

void               P3DBranchingAlgBasePanel::OnMaxNumberChanged
                                      (wxSpinSliderEvent  &event)
 {
  unsigned int OldMinNumber = Alg->GetMinNumber();
  unsigned int OldMaxNumber = Alg->GetMaxNumber();
  unsigned int NewMaxNumber = event.GetIntValue();
  bool         LimitEnabled = Alg->IsMaxLimitEnabled();
  unsigned int NewMinNumber;

  if (OldMinNumber > NewMaxNumber)
   {
    NewMinNumber = NewMaxNumber;

    wxSpinSliderCtrl *SpinSlider = (wxSpinSliderCtrl*)FindWindow(wxID_MIN_NUMBER_CTRL);

    if (SpinSlider != NULL)
     {
      SpinSlider->SetValue(NewMinNumber);
     }
   }
  else
   {
    NewMinNumber = OldMinNumber;
   }

  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseMinMaxNumberEditCmd
         (Alg,
          NewMinNumber,NewMaxNumber,LimitEnabled,
          OldMinNumber,OldMaxNumber,LimitEnabled));
 }

void               P3DBranchingAlgBasePanel::OnRotAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DBAlgBaseFloatParamEditCmd
         (Alg,
          P3DMATH_DEG2RAD((float)(event.GetIntValue())),
          Alg->GetRotationAngle(),
          &P3DBranchingAlgBase::SetRotationAngle));
 }

#define model Alg

void               P3DBranchingAlgBasePanel::UpdateControls
                                      ()
 {
  P3DUpdateParamSpinSliderDegrees(wxID_ROTANGLE_CTRL,GetRotationAngle);
 }

#undef model

