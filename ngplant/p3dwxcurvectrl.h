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

#ifndef __P3DWXCURVECTRL_H__
#define __P3DWXCURVECTRL_H__

#include <wx/wx.h>

#include <ngpcore/p3dmathspline.h>

#define P3DCurveCtrlMinWidth          (140)
#define P3DCurveCtrlMinHeight         (40)

class P3DCurveCtrl : public wxControl
 {
  public           :

                   P3DCurveCtrl       ();
                   P3DCurveCtrl       (wxWindow           *parent,
                                       wxWindowID          id,
                                       const P3DMathNaturalCubicSpline
                                                          &curve,
                                       const wxPoint      &pos = wxDefaultPosition,
                                       const wxSize       &size = wxDefaultSize,
                                       long                style = 0,
                                       const wxValidator  &validator = wxDefaultValidator,
                                       const wxString     &name= wxTextCtrlNameStr);

  virtual bool     Create             (wxWindow           *parent,
                                       wxWindowID          id,
                                       const P3DMathNaturalCubicSpline
                                                          &curve,
                                       const wxPoint      &pos = wxDefaultPosition,
                                       const wxSize       &size = wxDefaultSize,
                                       long                style = 0,
                                       const wxValidator  &validator = wxDefaultValidator,
                                       const wxString     &name= wxTextCtrlNameStr);

  void             SetCurve           (const P3DMathNaturalCubicSpline
                                                          &Curve);

  void             SetDefaultCurve    (const P3DMathNaturalCubicSpline
                                                          &Curve);

  void             Init               ();

  virtual wxSize   DoGetBestSize      () const;

  void             OnPaint            (wxPaintEvent       &event);
  void             OnLeftDown         (wxMouseEvent       &event);
  void             OnLeftDblClick     (wxMouseEvent       &event);
  void             OnRightDown        (wxMouseEvent       &event);
  void             OnLeftUp           (wxMouseEvent       &event);
  void             OnMouseMove        (wxMouseEvent       &event);
  void             OnLeaveWindow      (wxMouseEvent       &event);
  void             onEraseBackground  (wxEraseEvent       &event);
  void             OnResetToDefault   (wxCommandEvent     &event);

  const P3DMathNaturalCubicSpline
                  &GetCurve           () const
   {
    return(curve);
   }

  void             EnableEditDialog   (bool                Enable)
   {
    DialogEnabled = Enable;
   }

  static unsigned int                  BestWidth;
  static unsigned int                  BestHeight;

  private          :

  enum
   {
    BORDER_SIZE = 5,
    CONTROL_POINT_RADIUS = 3,
    SELECTION_TOLERANCE_IN_PX = CONTROL_POINT_RADIUS
   };

  enum { INVALID_CONTROL_POINT = -1 };

  /* from curve to window */
  void             CalcTransform      (const wxSize       &client_size);

  int              GetCPByMousePos    (int                 x,
                                       int                 y) const;

  float            RegionToCurveX     (int                 x) const;
  float            RegionToCurveY     (int                 y) const;
  int              CurveToRegionX     (float               x) const;
  int              CurveToRegionY     (float               y) const;

  void             SendChangedEvent   (void);

  P3DMathNaturalCubicSpline            curve;

  bool                                 HaveDefaultCurve;
  P3DMathNaturalCubicSpline            DefaultCurve;

  float                                ta_x,tb_x;
  float                                ta_y,tb_y;
  int                                  ActiveControlPoint;
  long                                 MousePosX;
  long                                 MousePosY;

  bool                                 DialogEnabled;

  DECLARE_DYNAMIC_CLASS(P3DCurveCtrl)
  DECLARE_EVENT_TABLE()
 };

class P3DCurveCtrlEvent : public wxCommandEvent
 {
  public           :

                   P3DCurveCtrlEvent  (wxEventType         commandType = wxEVT_NULL,
                                       int                 id = 0,
                                       const P3DMathNaturalCubicSpline
                                                          *curve = 0)
                   : wxCommandEvent(commandType,id) { this->curve = curve; };

  const P3DMathNaturalCubicSpline
                  *GetCurve           () const { return(curve); };

  virtual wxEvent *Clone              () const { return new P3DCurveCtrlEvent(*this); }

  private          :

  const P3DMathNaturalCubicSpline    *curve;

  DECLARE_DYNAMIC_CLASS(P3DCurveCtrlEvent)
 };

typedef void (wxEvtHandler::*P3DCurveCtrlEventFunction)(P3DCurveCtrlEvent&);

BEGIN_DECLARE_EVENT_TYPES()
 DECLARE_EVENT_TYPE(wxEVT_P3DCURVECTRL_CURVE_CHANGED,wxEVT_USER_FIRST + 1101)
END_DECLARE_EVENT_TYPES()

#define EVT_P3DCURVECTRL_VALUE_CHANGED(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_P3DCURVECTRL_CURVE_CHANGED,id,-1,(wxObjectEventFunction)(wxEventFunction)(P3DCurveCtrlEventFunction) & fn, (wxObject*) NULL),

#endif

