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

#include <ngpcore/p3dmodelstemtube.h>

#include <p3dapp.h>
#include <p3dcmdhelpers.h>
#include <p3duibalgstd.h>

enum
 {
  wxID_DENSITY_CTRL  = wxID_HIGHEST + 1,
  wxID_DENSITYV_CTRL,
  wxID_MULTIPLICITY_CTRL,
  wxID_MIN_OFFSET_CTRL,
  wxID_MAX_OFFSET_CTRL,
  wxID_REVANGLE_CTRL,
  wxID_REVANGLEV_CTRL,
  wxID_ROTANGLE_CTRL,
  wxID_DECLINATION_CURVE_CTRL,
  wxID_DECLINATIONV_CTRL
 };

BEGIN_EVENT_TABLE(P3DBranchingAlgStdPanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_DENSITY_CTRL,P3DBranchingAlgStdPanel::OnDensityChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_DENSITYV_CTRL,P3DBranchingAlgStdPanel::OnDensityVChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_MULTIPLICITY_CTRL,P3DBranchingAlgStdPanel::OnMultiplicityChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_MIN_OFFSET_CTRL,P3DBranchingAlgStdPanel::OnMinOffsetChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_MAX_OFFSET_CTRL,P3DBranchingAlgStdPanel::OnMaxOffsetChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_REVANGLE_CTRL,P3DBranchingAlgStdPanel::OnRevAngleChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_REVANGLEV_CTRL,P3DBranchingAlgStdPanel::OnRevAngleVChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_ROTANGLE_CTRL,P3DBranchingAlgStdPanel::OnRotAngleChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_DECLINATION_CURVE_CTRL,P3DBranchingAlgStdPanel::OnDeclinationCurveChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_DECLINATIONV_CTRL,P3DBranchingAlgStdPanel::OnDeclinationVChanged)
END_EVENT_TABLE()

                   P3DBranchingAlgStdPanel::P3DBranchingAlgStdPanel
                                      (wxWindow           *Parent,
                                       P3DBranchingAlgStd *Alg)
                   : P3DUIParamPanel(Parent)
 {
  P3DMathNaturalCubicSpline            DefaultCurve;

  this->Alg = Alg;

  wxBoxSizer           *TopSizer        = new wxBoxSizer(wxVERTICAL);
  wxStaticBoxSizer     *ParametersSizer = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Parameters")),wxVERTICAL);
  wxFlexGridSizer      *GridSizer       = new wxFlexGridSizer(10,2,2,2);

  GridSizer->AddGrowableCol(1);

  /* Density */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Density")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_DENSITY_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetDensity(),0.01,100.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(1.0);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* DensityV */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_DENSITYV_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetDensityV(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Multiplicity */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Multiplicity")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_MULTIPLICITY_CTRL,wxSPINSLIDER_MODE_INTEGER,Alg->GetMultiplicity(),1,1000);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Min. offset */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Min. offset")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_MIN_OFFSET_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetMinOffset(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Max. offset */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Max. offset")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_MAX_OFFSET_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetMaxOffset(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Rev. angle */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("RevAngle")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_REVANGLE_CTRL,wxSPINSLIDER_MODE_INTEGER,(int)P3DMath::Roundf((P3DMATH_RAD2DEG(Alg->GetRevAngle()))),0,359);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* RevAngleV */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_REVANGLEV_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetRevAngleV(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Rot. angle */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("RotAngle")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_ROTANGLE_CTRL,wxSPINSLIDER_MODE_INTEGER,(int)P3DMath::Roundf((P3DMATH_RAD2DEG(Alg->GetRotationAngle()))),0,359);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Declination */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Declination")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  const P3DMathNaturalCubicSpline *curve = Alg->GetDeclinationCurve();

  P3DCurveCtrl *DeclCurveCtrl = new P3DCurveCtrl(this,wxID_DECLINATION_CURVE_CTRL,*curve);

  P3DBranchingAlgStd::MakeDefaultDeclinationCurve(DefaultCurve);
  DeclCurveCtrl->SetDefaultCurve(DefaultCurve);

  GridSizer->Add(DeclCurveCtrl,1,wxALL | wxALIGN_RIGHT,1);

  /* DeclinationV */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_DECLINATIONV_CTRL,wxSPINSLIDER_MODE_FLOAT,Alg->GetDeclinationV(),0.0,1.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  ParametersSizer->Add(GridSizer,0,wxGROW,0);

  TopSizer->Add(ParametersSizer,0,wxGROW | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

typedef P3DParamEditCmdTemplate<P3DBranchingAlgStd,float> P3DBAlgStdFloatParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DBranchingAlgStd,unsigned int> P3DBAlgStdUIntParamEditCmd;
typedef P3DParamCurveEditCmdTemplate<P3DBranchingAlgStd> P3DBAlgStdCurveParamEditCmd;

void               P3DBranchingAlgStdPanel::OnDensityChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetDensity(),
          &P3DBranchingAlgStd::SetDensity));
 }

void               P3DBranchingAlgStdPanel::OnDensityVChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetDensityV(),
          &P3DBranchingAlgStd::SetDensityV));
 }

void               P3DBranchingAlgStdPanel::OnMultiplicityChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdUIntParamEditCmd
         (Alg,
          event.GetIntValue(),
          Alg->GetMultiplicity(),
          &P3DBranchingAlgStd::SetMultiplicity));
 }

void               P3DBranchingAlgStdPanel::OnRevAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          P3DMATH_DEG2RAD((float)(event.GetIntValue())),
          Alg->GetRevAngle(),
          &P3DBranchingAlgStd::SetRevAngle));
 }

void               P3DBranchingAlgStdPanel::OnRevAngleVChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetRevAngleV(),
          &P3DBranchingAlgStd::SetRevAngleV));
 }

void               P3DBranchingAlgStdPanel::OnRotAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          P3DMATH_DEG2RAD((float)(event.GetIntValue())),
          Alg->GetRotationAngle(),
          &P3DBranchingAlgStd::SetRotationAngle));
 }

void               P3DBranchingAlgStdPanel::OnMinOffsetChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetMinOffset(),
          &P3DBranchingAlgStd::SetMinOffset));
 }

void               P3DBranchingAlgStdPanel::OnMaxOffsetChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetMaxOffset(),
          &P3DBranchingAlgStd::SetMaxOffset));
 }

void               P3DBranchingAlgStdPanel::OnDeclinationCurveChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdCurveParamEditCmd
         (Alg,
          event.GetCurve(),
          Alg->GetDeclinationCurve(),
          &P3DBranchingAlgStd::SetDeclinationCurve));
 }

void               P3DBranchingAlgStdPanel::OnDeclinationVChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DBAlgStdFloatParamEditCmd
         (Alg,
          event.GetFloatValue(),
          Alg->GetDeclinationV(),
          &P3DBranchingAlgStd::SetDeclinationV));
 }

#define model Alg

void               P3DBranchingAlgStdPanel::UpdateControls
                                      ()
 {
  P3DUpdateParamSpinSlider(wxID_DENSITY_CTRL,GetDensity);
  P3DUpdateParamSpinSlider(wxID_DENSITYV_CTRL,GetDensityV);
  P3DUpdateParamSpinSlider(wxID_MULTIPLICITY_CTRL,GetMultiplicity);
  P3DUpdateParamSpinSlider(wxID_MIN_OFFSET_CTRL,GetMinOffset);
  P3DUpdateParamSpinSlider(wxID_MAX_OFFSET_CTRL,GetMaxOffset);
  P3DUpdateParamSpinSliderDegrees(wxID_REVANGLE_CTRL,GetRevAngle);
  P3DUpdateParamSpinSlider(wxID_REVANGLEV_CTRL,GetRevAngleV);
  P3DUpdateParamSpinSliderDegrees(wxID_ROTANGLE_CTRL,GetRotationAngle);
  P3DUpdateParamCurveCtrl(wxID_DECLINATION_CURVE_CTRL,GetDeclinationCurve);
  P3DUpdateParamSpinSlider(wxID_DECLINATIONV_CTRL,GetDeclinationV);
 }

#undef model

