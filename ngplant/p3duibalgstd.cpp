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
                   : wxPanel(Parent)
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

void               P3DBranchingAlgStdPanel::OnDensityChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetDensity(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnDensityVChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetDensityV(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnMultiplicityChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetMultiplicity(event.GetIntValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnRevAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetRevAngle(P3DMATH_DEG2RAD((float)(event.GetIntValue())));

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnRevAngleVChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetRevAngleV(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnRotAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetRotationAngle(P3DMATH_DEG2RAD((float)(event.GetIntValue())));

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnMinOffsetChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetMinOffset(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnMaxOffsetChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetMaxOffset(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnDeclinationCurveChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  Alg->SetDeclinationCurve(event.GetCurve());

  wxGetApp().InvalidatePlant();
 }

void               P3DBranchingAlgStdPanel::OnDeclinationVChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetDeclinationV(event.GetFloatValue());

  wxGetApp().InvalidatePlant();
 }

