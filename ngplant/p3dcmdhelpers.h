/***************************************************************************

 Copyright (C) 2008  Sergey Prokhorchuk

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

#ifndef __P3DCMDHELPERS_H__
#define __P3DCMDHELPERS_H__

#include <wx/wx.h>

#include <ngpcore/p3dmathspline.h>

#include <p3dcmdqueue.h>
#include <p3dapp.h>

template<class M,class T>
class P3DParamEditCmdTemplate : public P3DEditCommand
 {
  public           :

  typedef void (M::* SetParamMethod)(T Value);

                   P3DParamEditCmdTemplate
                                      (M                  *Model,
                                       T                   NewVal,
                                       T                   OldVal,
                                       SetParamMethod      Setter)
   {
    this->Model  = Model;
    this->OldVal = OldVal;
    this->NewVal = NewVal;
    this->Setter = Setter;
   }

  virtual void     Exec               ()
   {
    (Model->*Setter)(NewVal);
    wxGetApp().InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    (Model->*Setter)(OldVal);
    wxGetApp().InvalidatePlant();
   }

  private          :

  M               *Model;
  T                NewVal;
  T                OldVal;
  SetParamMethod   Setter;
 };

template<class M>
class P3DParamCurveEditCmdTemplate : public P3DEditCommand
 {
  public           :

  typedef void (M::* SetParamMethod)(const P3DMathNaturalCubicSpline *Value);

                   P3DParamCurveEditCmdTemplate
                                      (M                  *Model,
                                       const P3DMathNaturalCubicSpline *NewVal,
                                       const P3DMathNaturalCubicSpline *OldVal,
                                       SetParamMethod      Setter)
   {
    this->Model  = Model;
    this->OldVal.CopyFrom(*OldVal);
    this->NewVal.CopyFrom(*NewVal);
    this->Setter = Setter;
   }

  virtual void     Exec               ()
   {
    (Model->*Setter)(&NewVal);
    wxGetApp().InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    (Model->*Setter)(&OldVal);
    wxGetApp().InvalidatePlant();
   }

  private          :

  M               *Model;
  P3DMathNaturalCubicSpline                NewVal;
  P3DMathNaturalCubicSpline                OldVal;
  SetParamMethod   Setter;
 };

#define P3DUpdateParamSpinSlider(Id,Getter)       \
 {                                                \
  wxSpinSliderCtrl *SpinSlider;                   \
                                                  \
  SpinSlider = (wxSpinSliderCtrl*)FindWindow(Id); \
                                                  \
  if (SpinSlider != NULL)                         \
   {                                              \
    SpinSlider->SetValue((model)->Getter());      \
   }                                              \
 }

#define P3DUpdateParamSpinSliderExt(Model,Id,Getter) \
 {                                                   \
  wxSpinSliderCtrl *SpinSlider;                      \
                                                     \
  SpinSlider = (wxSpinSliderCtrl*)FindWindow(Id);    \
                                                     \
  if (SpinSlider != NULL)                            \
   {                                                 \
    SpinSlider->SetValue((Model)->Getter());         \
   }                                                 \
 }

#define P3DUpdateParamSpinSliderDegrees(Id,Getter)\
 {                                                \
  wxSpinSliderCtrl *SpinSlider;                   \
                                                  \
  SpinSlider = (wxSpinSliderCtrl*)FindWindow(Id); \
                                                  \
  if (SpinSlider != NULL)                         \
   {                                              \
    SpinSlider->SetValue((int)P3DMath::Roundf(P3DMATH_RAD2DEG((model)->Getter()))); \
   }                                              \
 }

#define P3DUpdateParamCurveCtrl(Id,Getter)        \
 {                                                \
  P3DCurveCtrl     *CurveCtrl;                    \
                                                  \
  CurveCtrl = (P3DCurveCtrl*)FindWindow(Id);      \
                                                  \
  if (CurveCtrl != NULL)                          \
   {                                              \
    CurveCtrl->SetCurve(*((model)->Getter()));    \
   }                                              \
 }

#define P3DUpdateParamCheckBox(Id,Getter)         \
 {                                                \
  wxCheckBox       *CheckBox;                     \
                                                  \
  CheckBox = (wxCheckBox*)FindWindow(Id);         \
                                                  \
  if (CheckBox != NULL)                           \
   {                                              \
    CheckBox->SetValue((model)->Getter());        \
   }                                              \
 }

#endif

