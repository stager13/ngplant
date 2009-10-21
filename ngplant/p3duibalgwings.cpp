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

#include <p3dapp.h>
#include <p3duibalgwings.h>

enum
 {
  wxID_ROTANGLE_CTRL  = wxID_HIGHEST + 1
 };

BEGIN_EVENT_TABLE(P3DBranchingAlgWingsPanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_ROTANGLE_CTRL,P3DBranchingAlgWingsPanel::OnRotAngleChanged)
END_EVENT_TABLE()

                   P3DBranchingAlgWingsPanel::P3DBranchingAlgWingsPanel
                                      (wxWindow           *Parent,
                                       P3DBranchingAlgWings
                                                          *Alg)
                   : wxPanel(Parent)
 {
  this->Alg = Alg;

  wxBoxSizer           *TopSizer        = new wxBoxSizer(wxVERTICAL);
  wxStaticBoxSizer     *ParametersSizer = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Parameters")),wxVERTICAL);
  wxFlexGridSizer      *GridSizer       = new wxFlexGridSizer(1,2,2,2);

  GridSizer->AddGrowableCol(1);

  /* Rot. angle */

  GridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("RotAngle")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_ROTANGLE_CTRL,wxSPINSLIDER_MODE_INTEGER,(int)(P3DMath::Roundf(P3DMATH_RAD2DEG(Alg->GetRotationAngle()))),0,359);
  spin_slider->SetStdStep(1);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(2);
  spin_slider->SetStdMove(1);
  spin_slider->SetSmallMove(1);

  GridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  ParametersSizer->Add(GridSizer,0,wxGROW,0);

  TopSizer->Add(ParametersSizer,0,wxGROW | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DBranchingAlgWingsPanel::OnRotAngleChanged
                                      (wxSpinSliderEvent  &event)
 {
  Alg->SetRotationAngle(P3DMATH_DEG2RAD((float)(event.GetIntValue())));

  wxGetApp().InvalidatePlant();
 }

