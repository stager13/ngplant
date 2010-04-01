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

#ifndef __P3DUIBALGSTD_H__
#define __P3DUIBALGSTD_H__

#include <wx/wx.h>

#include <p3dwx.h>
#include <p3dwxcurvectrl.h>
#include <p3duiparampanel.h>

#include <ngpcore/p3dbalgstd.h>

class P3DBranchingAlgStdPanel : public P3DUIParamPanel
 {
  public           :

                   P3DBranchingAlgStdPanel
                                      (wxWindow           *Parent,
                                       P3DBranchingAlgStd *Alg);

  void             OnDensityChanged   (wxSpinSliderEvent  &event);
  void             OnDensityVChanged  (wxSpinSliderEvent  &event);
  void             OnMultiplicityChanged
                                      (wxSpinSliderEvent  &event);
  void             OnRevAngleChanged  (wxSpinSliderEvent  &event);
  void             OnRevAngleVChanged (wxSpinSliderEvent  &event);
  void             OnRotAngleChanged  (wxSpinSliderEvent  &event);
  void             OnMinOffsetChanged (wxSpinSliderEvent  &event);
  void             OnMaxOffsetChanged (wxSpinSliderEvent  &event);
  void             OnDeclinationCurveChanged
                                      (P3DCurveCtrlEvent  &event);
  void             OnDeclinationVChanged
                                      (wxSpinSliderEvent  &event);

  virtual void     UpdateControls     ();

  private          :

  P3DBranchingAlgStd                  *Alg;

  DECLARE_EVENT_TABLE();
 };

#endif

