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

#ifndef __P3DUIMATERIALSTD_H__
#define __P3DUIMATERIALSTD_H__

#include <wx/wx.h>

#include <p3dwx.h>
#include <p3duiparampanel.h>

#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemquad.h>

#include <p3dmaterialstd.h>

class P3DMaterialStdPanel : public P3DUIParamPanel
 {
  public           :

                   P3DMaterialStdPanel(wxWindow           *Parent,
                                       P3DMaterialInstanceSimple
                                                          *Material,
                                       P3DStemModel       *StemModel);

  void             OnBaseColorClicked (wxCommandEvent     &Event);

  void             OnTexLayerClicked  (wxCommandEvent     &event);

  void             OnBaseTexClicked   (wxCommandEvent     &event);
  void             OnRemoveTexClicked (wxCommandEvent     &event);

  void             OnUScaleChanged    (wxSpinSliderEvent  &event);
  void             OnVModeChanged     (wxCommandEvent     &event);
  void             OnVScaleChanged    (wxSpinSliderEvent  &event);

  void             OnDoubleSidedChanged
                                      (wxCommandEvent     &event);
  void             OnTransparentChanged
                                      (wxCommandEvent     &event);
  void             OnBillboardModeChanged
                                      (wxCommandEvent     &event);

  void             OnAlphaCtrlEnabledChanged
                                      (wxCommandEvent     &event);
  void             OnAlphaFadeInChanged
                                      (wxSpinSliderEvent  &event);
  void             OnAlphaFadeOutChanged
                                      (wxSpinSliderEvent  &event);

  void             UpdateControls     ();

  private          :

  P3DMaterialInstanceSimple           *Material;
  P3DStemModelTube                    *StemModelTube;
  P3DStemModelQuad                    *StemModelQuad;
  wxBitmapButton                      *BaseTexButton;
  wxBitmapButton                      *RemoveTexButton;

  unsigned int                         ActiveTexLayer;

  DECLARE_EVENT_TABLE();
 };

#endif

