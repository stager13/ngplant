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

#ifndef __P3DWX_H__
#define __P3DWX_H__

#include <wx/wx.h>

#define wxSPINSLIDER_MODE_INTEGER (0)
#define wxSPINSLIDER_MODE_FLOAT   (1)

class wxSpinSliderCtrl : public wxControl
 {
  public           :

                   wxSpinSliderCtrl   ();
                   wxSpinSliderCtrl   (wxWindow           *parent,
                                       wxWindowID          id,
                                       int                 mode,
                                       float               value,
                                       float               min,
                                       float               max,
                                       const wxPoint      &pos = wxDefaultPosition,
                                       const wxSize       &size = wxDefaultSize,
                                       long                style = 0,
                                       const wxValidator  &validator = wxDefaultValidator,
                                       const wxString     &name= wxTextCtrlNameStr);

  virtual bool     Create             (wxWindow           *parent,
                                       wxWindowID          id,
                                       int                 mode,
                                       float               value,
                                       float               min,
                                       float               max,
                                       const wxPoint      &pos = wxDefaultPosition,
                                       const wxSize       &size = wxDefaultSize,
                                       long                style = 0,
                                       const wxValidator  &validator = wxDefaultValidator,
                                       const wxString     &name= wxTextCtrlNameStr);

  void             SetStdStep         (float               step);
  void             SetSmallStep       (float               step);
  void             SetLargeMove       (float               step);
  void             SetStdMove         (float               step);
  void             SetSmallMove       (float               step);

  void             SetSensitivity     (float               stdStep,
                                       float               smallStep,
                                       float               largeMove,
                                       float               stdMove,
                                       float               smallMove);

  bool             SetValue           (float               Value);
  float            GetValue           () const;

  void             Init               ();

  virtual wxSize   DoGetBestSize      () const;

  void             OnPaint            (wxPaintEvent       &event);
  void             OnLeftDown         (wxMouseEvent       &event);
  void             OnLeftDoubleClick  (wxMouseEvent       &event);
  void             OnLeftUp           (wxMouseEvent       &event);
  void             OnMouseMove        (wxMouseEvent       &event);
  void             OnMouseWheel       (wxMouseEvent       &event);
  void             OnMouseEnter       (wxMouseEvent       &event);
  void             OnMouseLeave       (wxMouseEvent       &event);

  private          :

  bool             PointInIncButton   (long                x,
                                       long                y) const;
  bool             PointInDecButton   (long                x,
                                       long                y) const;
  bool             PointInSlider      (long                x,
                                       long                y) const;

  bool             ChangeValue        (float               delta);

  void             SendChangedEvent   (void);

  int              mode;
  float            value;
  float            min;
  float            max;
  float            std_step;
  float            small_step;
  float            large_move;
  float            std_move;
  float            small_move;

  bool             left_down;
  long             mx,my;

  DECLARE_DYNAMIC_CLASS(wxSpinSliderCtrl)
  DECLARE_EVENT_TABLE()
 };

class wxSpinSliderEvent : public wxCommandEvent
 {
  public           :

                   wxSpinSliderEvent  (wxEventType         commandType = wxEVT_NULL,
                                       int                 id = 0,
                                       float               value = 0.0f)
                   : wxCommandEvent(commandType,id) { this->value = value; };

  int              GetIntValue        () const { return((int)value); };
  float            GetFloatValue      () const { return(value); };

  virtual wxEvent *Clone              () const { return new wxSpinSliderEvent(*this); }

  private          :

  float            value;

  DECLARE_DYNAMIC_CLASS(wxSpinSliderEvent)
 };

typedef void (wxEvtHandler::*wxSpinSliderEventFunction)(wxSpinSliderEvent&);

BEGIN_DECLARE_EVENT_TYPES()
 DECLARE_EVENT_TYPE(wxEVT_SPINSLIDER_VALUE_CHANGED,wxEVT_USER_FIRST + 1100)
END_DECLARE_EVENT_TYPES()

#define EVT_SPINSLIDER_VALUE_CHANGED(id,fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_SPINSLIDER_VALUE_CHANGED,id,-1,(wxObjectEventFunction)(wxEventFunction)(wxSpinSliderEventFunction) & fn, (wxObject*) NULL),

#endif

