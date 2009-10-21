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

#ifndef __P3DUIMODELSTEMQUAD_H__
#define __P3DUIMODELSTEMQUAD_H__

#include <wx/wx.h>

#include <p3dwx.h>
#include <p3dwxcurvectrl.h>

#include <ngpcore/p3dmodelstemquad.h>

class P3DStemQuadPanel : public wxPanel
 {
  public           :

                   P3DStemQuadPanel   (wxWindow           *parent,
                                       P3DStemModelQuad   *model,
                                       P3DVisRangeState   *VisRangeState);

  void             OnStemLengthChanged(wxSpinSliderEvent  &event);
  void             OnStemWidthChanged (wxSpinSliderEvent  &event);

  void             OnScalingChanged   (P3DCurveCtrlEvent  &event);

  void             OnSectionCountChanged
                                      (wxSpinSliderEvent  &event);
  void             OnCurvatureChanged (P3DCurveCtrlEvent  &event);
  void             OnThicknessChanged (wxSpinSliderEvent  &event);

  private          :

  P3DStemModelQuad                    *model;

  DECLARE_EVENT_TABLE();
 };

#endif

