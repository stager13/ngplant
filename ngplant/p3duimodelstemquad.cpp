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

#include <ngpcore/p3dmodelstemquad.h>

#include <p3dapp.h>
#include <p3dcmdhelpers.h>
#include <p3duivisrangepanel.h>
#include <p3duimodelstemquad.h>

enum
 {
  wxID_STEM_LENGTH_CTRL = wxID_HIGHEST + 1,
  wxID_STEM_WIDTH_CTRL                    ,
  wxID_STEM_SCALING_CTRL                  ,
  wxID_SECTION_COUNT_CTRL                 ,
  wxID_CURVATURE_CTRL                     ,
  wxID_THICKNESS_CTRL
 };

BEGIN_EVENT_TABLE(P3DStemQuadPanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_STEM_LENGTH_CTRL,P3DStemQuadPanel::OnStemLengthChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_STEM_WIDTH_CTRL,P3DStemQuadPanel::OnStemWidthChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_STEM_SCALING_CTRL,P3DStemQuadPanel::OnScalingChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_SECTION_COUNT_CTRL,P3DStemQuadPanel::OnSectionCountChanged)
 EVT_P3DCURVECTRL_VALUE_CHANGED(wxID_CURVATURE_CTRL,P3DStemQuadPanel::OnCurvatureChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_THICKNESS_CTRL,P3DStemQuadPanel::OnThicknessChanged)
END_EVENT_TABLE()

                   P3DStemQuadPanel::P3DStemQuadPanel
                                      (wxWindow           *parent,
                                       P3DStemModelQuad   *model,
                                       P3DVisRangeState   *VisRangeState)
                   : P3DUIParamPanel(parent)
 {
  P3DMathNaturalCubicSpline            DefaultCurve;

  this->model = model;

  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *QuadParamsTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Quad parameters")),wxVERTICAL);
  wxFlexGridSizer  *QuadParamsGridSizer = new wxFlexGridSizer(6,2,3,3);

  QuadParamsGridSizer->AddGrowableCol(1);

  /* Length */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Length")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_STEM_LENGTH_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetLength(),0.00,100.0);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(0.1);
  spin_slider->SetLargeMove(10.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(0.01);

  QuadParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Width */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Width")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_STEM_WIDTH_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetWidth(),0.00,100.0);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(0.1);
  spin_slider->SetLargeMove(10.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(0.01);

  QuadParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Scaling */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Scaling")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  P3DCurveCtrl *ScalingCurveCtrl = new P3DCurveCtrl(this,wxID_STEM_SCALING_CTRL,*(model->GetScalingCurve()));

  P3DStemModelQuad::MakeDefaultScalingCurve(DefaultCurve);
  ScalingCurveCtrl->SetDefaultCurve(DefaultCurve);

  QuadParamsGridSizer->Add(ScalingCurveCtrl,1,wxALL | wxALIGN_RIGHT,1);

  /* Section count */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Sections")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_SECTION_COUNT_CTRL,wxSPINSLIDER_MODE_INTEGER,model->GetSectionCount(),1,1000);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  QuadParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  /* Curvature */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Curvature")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  P3DCurveCtrl *CurvatureCtrl = new P3DCurveCtrl(this,wxID_CURVATURE_CTRL,*(model->GetCurvature()));

  P3DStemModelQuad::MakeDefaultCurvatureCurve(DefaultCurve);
  CurvatureCtrl->SetDefaultCurve(DefaultCurve);

  QuadParamsGridSizer->Add(CurvatureCtrl,1,wxALL | wxALIGN_RIGHT,1);

  /* Thickness */

  QuadParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Thickness")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  spin_slider = new wxSpinSliderCtrl(this,wxID_THICKNESS_CTRL,wxSPINSLIDER_MODE_FLOAT,model->GetThickness(),0.0f,100.0f);
  spin_slider->SetStdStep(1.0);
  spin_slider->SetSmallStep(0.1);
  spin_slider->SetLargeMove(10.0);
  spin_slider->SetStdMove(1.0);
  spin_slider->SetSmallMove(0.01);

  QuadParamsGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  QuadParamsTopSizer->Add(QuadParamsGridSizer,0,wxEXPAND,0);

  TopSizer->Add(QuadParamsTopSizer,0,wxEXPAND | wxALL,1);

  VisRangePanel = new P3DVisRangePanel(this,VisRangeState);

  TopSizer->Add(VisRangePanel,0,wxEXPAND | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

typedef P3DParamEditCmdTemplate<P3DStemModelQuad,float> P3DStemQuadFloatParamEditCmd;
typedef P3DParamEditCmdTemplate<P3DStemModelQuad,unsigned int> P3DStemQuadUIntParamEditCmd;
typedef P3DParamCurveEditCmdTemplate<P3DStemModelQuad> P3DStemQuadCurveParamEditCmd;

void               P3DStemQuadPanel::OnStemLengthChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
  (new P3DStemQuadFloatParamEditCmd
        (model,
         event.GetFloatValue(),
         model->GetLength(),
         &P3DStemModelQuad::SetLength));
 }

void               P3DStemQuadPanel::OnStemWidthChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemQuadFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetWidth(),
          &P3DStemModelQuad::SetWidth));
 }

void               P3DStemQuadPanel::OnScalingChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemQuadCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetScalingCurve(),
          &P3DStemModelQuad::SetScalingCurve));
 }

void               P3DStemQuadPanel::OnSectionCountChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemQuadUIntParamEditCmd
         (model,
          event.GetIntValue(),
          model->GetSectionCount(),
          &P3DStemModelQuad::SetSectionCount));
 }

void               P3DStemQuadPanel::OnCurvatureChanged
                                      (P3DCurveCtrlEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemQuadCurveParamEditCmd
         (model,
          event.GetCurve(),
          model->GetCurvature(),
          &P3DStemModelQuad::SetCurvature));
 }

void               P3DStemQuadPanel::OnThicknessChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd
   (new P3DStemQuadFloatParamEditCmd
         (model,
          event.GetFloatValue(),
          model->GetThickness(),
          &P3DStemModelQuad::SetThickness));
 }

void               P3DStemQuadPanel::UpdateControls
                                      ()
 {
  P3DUpdateParamSpinSlider(wxID_STEM_LENGTH_CTRL,GetLength);
  P3DUpdateParamSpinSlider(wxID_STEM_WIDTH_CTRL,GetWidth);
  P3DUpdateParamSpinSlider(wxID_SECTION_COUNT_CTRL,GetSectionCount);
  P3DUpdateParamSpinSlider(wxID_THICKNESS_CTRL,GetThickness);

  P3DUpdateParamCurveCtrl(wxID_STEM_SCALING_CTRL,GetScalingCurve);
  P3DUpdateParamCurveCtrl(wxID_CURVATURE_CTRL,GetCurvature);

  VisRangePanel->UpdateControls();
 }

