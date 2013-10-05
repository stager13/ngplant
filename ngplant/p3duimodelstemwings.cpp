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

#include <ngpcore/p3dmodelstemwings.h>

#include <p3dapp.h>
#include <p3dcmdhelpers.h>
#include <p3duivisrangepanel.h>
#include <p3duimodelstemwings.h>

enum
 {
  wxID_WINGS_ANGLE_CTRL = wxID_HIGHEST + 1,
  wxID_STEM_WIDTH_CTRL                    ,
  wxID_SECTION_COUNT_CTRL                 ,
  wxID_CURVATURE_CTRL                     ,
  wxID_THICKNESS_CTRL                     ,
  wxID_WIDTH_THICKNESS_SCALING_CTRL
 };

BEGIN_EVENT_TABLE(P3DStemWingsPanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_WINGS_ANGLE_CTRL,P3DStemWingsPanel::OnWingsAngleChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_STEM_WIDTH_CTRL,P3DStemWingsPanel::OnStemWidthChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_SECTION_COUNT_CTRL,P3DStemWingsPanel::OnSectionCountChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_CURVATURE_CTRL,P3DStemWingsPanel::OnCurvatureChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_THICKNESS_CTRL,P3DStemWingsPanel::OnThicknessChanged)
 EVT_CHECKBOX(wxID_WIDTH_THICKNESS_SCALING_CTRL,P3DStemWingsPanel::OnWidthThicknessScalingChanged)
END_EVENT_TABLE()

                   P3DStemWingsPanel::P3DStemWingsPanel
                                      (wxWindow           *parent,
                                       P3DStemModelWings  *model,
                                       P3DVisRangeState   *VisRangeState)
                   : P3DUIParamPanel(parent)
 {
  P3DMathNaturalCubicSpline            DefaultCurve;

  this->model = model;

  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *WingsParamsTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Wings parameters")),wxVERTICAL);
  wxFlexGridSizer  *WingsParamsGridSizer = new wxFlexGridSizer(7,2,3,3);

  WingsParamsGridSizer->AddGrowableCol(1);

  /* Wings Angle */

  /*
  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Wings angle")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_WINGS_ANGLE_CTRL,wxSPINSLIDER_MODE_INTEGER,(int)(P3DMATH_RAD2DEG(model->GetWingsAngle())),-90,90);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(1.0);
  spin_slider->SetLargeMove(5.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(1.0);

  WingsParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);
  */
  /* Width */

  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Width")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_STEM_WIDTH_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetWidth(),0.00,100.0);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(0.1);
  spin_slider->SetLargeMove(10.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(0.01);

  WingsParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Section count */

  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Sections")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_SECTION_COUNT_CTRL,wxSPINSLIDER_MODE_INTEGER,model->GetSectionCount(),1,1000);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  WingsParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Curvature */

  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Curvature")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  P3DCurveCtrl *CurvatureCtrl = new P3DCurveCtrl(this,wxID_CURVATURE_CTRL,*(model->GetCurvature()));

  P3DStemModelWings::MakeDefaultCurvatureCurve(DefaultCurve);
  CurvatureCtrl->SetDefaultCurve(DefaultCurve);

  WingsParamsGridSizer->Add(CurvatureCtrl,1,wxALL | wxALIGN_RIGHT,1);

  /* Thickness */

  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Thickness")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_THICKNESS_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetThickness(),0.0f,100.0f);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(0.1);
  spin_slider->SetLargeMove(10.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(0.01);

  WingsParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Width/thickness scaling checkbox */

  WingsParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Apply scaling")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *WidthThicknessScalingCheckBox = new wxCheckBox(this,wxID_WIDTH_THICKNESS_SCALING_CTRL,wxT(""));
  WidthThicknessScalingCheckBox->SetValue(model->IsWidthScalingEnabled());

  WingsParamsGridSizer->Add(WidthThicknessScalingCheckBox,1,wxALIGN_RIGHT,0);

  WingsParamsTopSizer->Add(WingsParamsGridSizer,0,wxEXPAND,0);

  TopSizer->Add(WingsParamsTopSizer,0,wxEXPAND | wxALL,1);

  VisRangePanel = new P3DVisRangePanel(this,VisRangeState);

  TopSizer->Add(VisRangePanel,0,wxEXPAND | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DStemWingsPanel::OnWingsAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  model->SetWingsAngle(event.GetFloatValue());

  P3DApp::GetApp()->InvalidatePlant();
 }

typedef P3DParamEditCmdTemplate<P3DStemModelWings,float> P3DStemWingsFloatParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DStemModelWings,unsigned int> P3DStemWingsUIntParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DStemModelWings,bool> P3DStemWingsBoolParamEditCmd;
typedef P3DParamCurveEditCmdTemplate<P3DStemModelWings> P3DStemWingsCurveParamEditCmd;

void               P3DStemWingsPanel::OnStemWidthChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
  (new P3DStemWingsFloatParamEditCmd
        (model,
         event.GetFloatValue(),
         model->GetWidth(),
         &P3DStemModelWings::SetWidth));
 }

void               P3DStemWingsPanel::OnSectionCountChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
  (new P3DStemWingsUIntParamEditCmd
        (model,
         event.GetIntValue(),
         model->GetSectionCount(),
         &P3DStemModelWings::SetSectionCount));
 }

void               P3DStemWingsPanel::OnCurvatureChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemWingsCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetCurvature(),
          &P3DStemModelWings::SetCurvature));
 }

void               P3DStemWingsPanel::OnThicknessChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
  (new P3DStemWingsFloatParamEditCmd
        (model,
         event.GetFloatValue(),
         model->GetThickness(),
         &P3DStemModelWings::SetThickness));
 }

void               P3DStemWingsPanel::OnWidthThicknessScalingChanged
                                      (wxCommandEvent     &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemWingsBoolParamEditCmd
         (model,
          event.IsChecked(),
          model->IsWidthScalingEnabled(),
          &P3DStemModelWings::EnableWidthScaling));
 }

void               P3DStemWingsPanel::UpdateControls
                                      ()
 {
  P3DUpdateParamSpinSlider(wxID_STEM_WIDTH_CTRL,GetWidth);
  P3DUpdateParamSpinSlider(wxID_SECTION_COUNT_CTRL,GetSectionCount);
  P3DUpdateParamSpinSlider(wxID_THICKNESS_CTRL,GetThickness);

  P3DUpdateParamCurveCtrl(wxID_CURVATURE_CTRL,GetCurvature);

  VisRangePanel->UpdateControls();
 }

