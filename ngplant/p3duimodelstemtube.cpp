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
#include <p3duivisrangepanel.h>
#include <p3duimodelstemtube.h>

enum
 {
  wxID_STEM_LENGTH_CTRL = wxID_HIGHEST + 1,
  wxID_STEM_LENGTHV_CTRL,
  wxID_AXIS_VARIATION_CTRL,
  wxID_LENGTH_OFFSET_INFLUENCE_CTRL,
  wxID_AXIS_RESOLUTION_CTRL,
  wxID_TRUNK_RADIUS_CTRL,
  wxID_TRUNK_PROFILE_SCALE_CTRL,
  wxID_PROFILE_RESOLUTION_CTRL,
  wxID_PHOTOTROPISM_INFLUENCE_CTRL
 };

BEGIN_EVENT_TABLE(P3DStemTubePanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_STEM_LENGTH_CTRL,P3DStemTubePanel::OnStemLengthChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_STEM_LENGTHV_CTRL,P3DStemTubePanel::OnStemLengthVChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_LENGTH_OFFSET_INFLUENCE_CTRL,P3DStemTubePanel::OnLengthOffsetInfluenceChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_AXIS_RESOLUTION_CTRL,P3DStemTubePanel::OnAxisResolutionChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_AXIS_VARIATION_CTRL,P3DStemTubePanel::OnStemAxisVariationChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_TRUNK_RADIUS_CTRL,P3DStemTubePanel::OnTrunkProfileScaleBaseChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_TRUNK_PROFILE_SCALE_CTRL,P3DStemTubePanel::OnRadiusProfileScaleChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_PROFILE_RESOLUTION_CTRL,P3DStemTubePanel::OnProfileResolutionChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_PHOTOTROPISM_INFLUENCE_CTRL,P3DStemTubePanel::OnPhototropismCurveChanged)
END_EVENT_TABLE()

                   P3DStemTubePanel::P3DStemTubePanel
                                      (wxWindow           *parent,
                                       P3DStemModelTube   *model,
                                       P3DVisRangeState   *VisRangeState)
                   : P3DUIParamPanel(parent)
 {
  P3DMathNaturalCubicSpline            DefaultCurve;

  this->model = model;

  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *AxisParamsTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Axis parameters")),wxVERTICAL);
  wxFlexGridSizer  *AxisParamsGridSizer = new wxFlexGridSizer(5,2,3,3);

  AxisParamsGridSizer->AddGrowableCol(1);

  AxisParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Length")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_STEM_LENGTH_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetLength(),0.01,100.0);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(1.0);
  spin_slider->SetStdMove(0.1);
  spin_slider->SetSmallMove(0.01);

  AxisParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  AxisParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_STEM_LENGTHV_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetLengthV(),0.0,1.0);
  spin_slider->SetStdStep(0.2);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.01);
  spin_slider->SetSmallMove(0.01);

  AxisParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  AxisParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Offset influence")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  P3DCurveCtrl *length_off_infulence_ctrl = new P3DCurveCtrl(this,wxID_LENGTH_OFFSET_INFLUENCE_CTRL,*(model->GetLengthOffsetInfuenceCurve()));

  P3DStemModelTube::MakeDefaultLengthOffsetInfuenceCurve(DefaultCurve);
  length_off_infulence_ctrl->SetDefaultCurve(DefaultCurve);

  AxisParamsGridSizer->Add(length_off_infulence_ctrl,1,wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT,1);

  AxisParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Resolution")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_AXIS_RESOLUTION_CTRL,wxSPINSLIDER_MODE_INTEGER,model->GetAxisResolution(),1.0,1000.0);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  AxisParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  AxisParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Axis variation")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_AXIS_VARIATION_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetAxisVariation(),0.0f,1.0f);
  spin_slider->SetStdStep(0.1);
  spin_slider->SetSmallStep(0.01);
  spin_slider->SetLargeMove(0.2);
  spin_slider->SetStdMove(0.01);
  spin_slider->SetSmallMove(0.01);

  AxisParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  AxisParamsTopSizer->Add(AxisParamsGridSizer,0,wxEXPAND,0);

  TopSizer->Add(AxisParamsTopSizer,0,wxEXPAND | wxALL,1);

  wxStaticBoxSizer *CrossSectionParamsTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Cross-section parameters")),wxVERTICAL);
  wxFlexGridSizer  *CrossSectionParamsGridSizer = new wxFlexGridSizer(3,2,3,3);

  CrossSectionParamsGridSizer->AddGrowableCol(1);

  CrossSectionParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Radius")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  float trunk_radius = model->GetProfileScaleBase();

  wxSpinSliderCtrl *radius_ctrl = new wxSpinSliderCtrl(this,wxID_TRUNK_RADIUS_CTRL,wxSPINSLIDER_MODE_FLOAT,trunk_radius,0.01,100.0);

  radius_ctrl->SetStdStep(1.0);
  radius_ctrl->SetSmallStep(0.1);
  radius_ctrl->SetLargeMove(1.0);
  radius_ctrl->SetStdMove(0.1);
  radius_ctrl->SetSmallMove(0.01);

  CrossSectionParamsGridSizer->Add(radius_ctrl,1,wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT,1);

  CrossSectionParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Profile")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  const P3DMathNaturalCubicSpline *curve = model->GetProfileScaleCurve();

  P3DCurveCtrl *profile_scale_ctrl = new P3DCurveCtrl(this,wxID_TRUNK_PROFILE_SCALE_CTRL,*curve);

  P3DStemModelTube::MakeDefaultProfileScaleCurve(DefaultCurve);
  profile_scale_ctrl->SetDefaultCurve(DefaultCurve);

  CrossSectionParamsGridSizer->Add(profile_scale_ctrl,1,wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT,1);

  CrossSectionParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Resolution")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_PROFILE_RESOLUTION_CTRL,wxSPINSLIDER_MODE_INTEGER,model->GetProfileResolution(),3.0,1000.0);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  CrossSectionParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  CrossSectionParamsTopSizer->Add(CrossSectionParamsGridSizer,0,wxEXPAND,0);

  TopSizer->Add(CrossSectionParamsTopSizer,0,wxEXPAND | wxALL,1);

  wxStaticBoxSizer *PhototropismParamsTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Phototropism")),wxVERTICAL);
  wxFlexGridSizer  *PhototropismParamsGridSizer = new wxFlexGridSizer(1,2,3,3);

  PhototropismParamsGridSizer->AddGrowableCol(1);

  PhototropismParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Distribution")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  P3DCurveCtrl *phototropism_curve_ctrl = new P3DCurveCtrl(this,wxID_PHOTOTROPISM_INFLUENCE_CTRL,*(model->GetPhototropismCurve()));

  P3DStemModelTube::MakeDefaultPhototropismCurve(DefaultCurve);
  phototropism_curve_ctrl->SetDefaultCurve(DefaultCurve);

  PhototropismParamsGridSizer->Add(phototropism_curve_ctrl,1,wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT,1);

  PhototropismParamsTopSizer->Add(PhototropismParamsGridSizer,0,wxEXPAND,0);

  TopSizer->Add(PhototropismParamsTopSizer,0,wxEXPAND | wxALL,1);

  VisRangePanel = new P3DVisRangePanel(this,VisRangeState);

  TopSizer->Add(VisRangePanel,0,wxEXPAND | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

typedef P3DParamEditCmdTemplate<P3DStemModelTube,float> P3DStemTubeFloatParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DStemModelTube,unsigned int> P3DStemTubeUIntParamEditCmd;
typedef P3DParamCurveEditCmdTemplate<P3DStemModelTube> P3DStemTubeCurveParamEditCmd;

void               P3DStemTubePanel::OnStemLengthChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetLength(),
          &P3DStemModelTube::SetLength));
 }

void               P3DStemTubePanel::OnStemLengthVChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetLengthV(),
          &P3DStemModelTube::SetLengthV));
 }

void               P3DStemTubePanel::OnStemAxisVariationChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetAxisVariation(),
          &P3DStemModelTube::SetAxisVariation));
 }

void               P3DStemTubePanel::OnLengthOffsetInfluenceChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetLengthOffsetInfuenceCurve(),
          &P3DStemModelTube::SetLengthOffsetInfluenceCurve));
 }

void               P3DStemTubePanel::OnAxisResolutionChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeUIntParamEditCmd
         (model,
          event.GetIntValue(),
          model->GetAxisResolution(),
          &P3DStemModelTube::SetAxisResolution));
 }

void               P3DStemTubePanel::OnTrunkProfileScaleBaseChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetProfileScaleBase(),
          &P3DStemModelTube::SetProfileScaleBase));
 }

void               P3DStemTubePanel::OnRadiusProfileScaleChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetProfileScaleCurve(),
          &P3DStemModelTube::SetProfileScaleCurve));
 }

void               P3DStemTubePanel::OnProfileResolutionChanged
                                      (wxSpinSliderEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeUIntParamEditCmd
         (model,
          event.GetIntValue(),
          model->GetProfileResolution(),
          &P3DStemModelTube::SetProfileResolution));
 }

void               P3DStemTubePanel::OnPhototropismCurveChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  wxGetApp().ExecEditCmd
   (new P3DStemTubeCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetPhototropismCurve(),
          &P3DStemModelTube::SetPhototropismCurve));
 }

void               P3DStemTubePanel::UpdateControls
                                      ()
 {
  P3DUpdateParamSpinSlider(wxID_STEM_LENGTH_CTRL,GetLength);
  P3DUpdateParamSpinSlider(wxID_STEM_LENGTHV_CTRL,GetLengthV);
  P3DUpdateParamSpinSlider(wxID_AXIS_VARIATION_CTRL,GetAxisVariation);
  P3DUpdateParamCurveCtrl(wxID_LENGTH_OFFSET_INFLUENCE_CTRL,GetLengthOffsetInfuenceCurve);
  P3DUpdateParamSpinSlider(wxID_AXIS_RESOLUTION_CTRL,GetAxisResolution);
  P3DUpdateParamSpinSlider(wxID_TRUNK_RADIUS_CTRL,GetProfileScaleBase);
  P3DUpdateParamCurveCtrl(wxID_TRUNK_PROFILE_SCALE_CTRL,GetProfileScaleCurve);
  P3DUpdateParamSpinSlider(wxID_PROFILE_RESOLUTION_CTRL,GetProfileResolution);
  P3DUpdateParamCurveCtrl(wxID_PHOTOTROPISM_INFLUENCE_CTRL,GetPhototropismCurve);

  VisRangePanel->UpdateControls();
 }

