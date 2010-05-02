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

#include <p3dwx.h>

class P3DNumberEntryDialog : public wxDialog
 {
  public           :

                   P3DNumberEntryDialog
                                      ();
                   P3DNumberEntryDialog
                                      (wxWindow           *Parent,
                                       int                 Mode,
                                       float               Value,
                                       float               Min,
                                       float               Max,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Enter value"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  bool             Create             (wxWindow           *Parent,
                                       int                 Mode,
                                       float               Value,
                                       float               Min,
                                       float               Max,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Enter value"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  bool             TransferDataToWindow
                                      ();
  bool             TransferDataFromWindow
                                      ();

  float            GetValue           () const
   {
    return(Value);
   }

  private          :

  void             Init               ();
  void             CreateControls     ();

  int              Mode;
  float            Value;
  float            Min;
  float            Max;

  wxTextCtrl      *EntryCtrl;

  DECLARE_CLASS(P3DNumberEntryDialog)
 };

IMPLEMENT_CLASS(P3DNumberEntryDialog,wxDialog)

                   P3DNumberEntryDialog::P3DNumberEntryDialog
                                      ()
 {
  Init();
 }

                   P3DNumberEntryDialog::P3DNumberEntryDialog
                                      (wxWindow           *Parent,
                                       int                 Mode,
                                       float               Value,
                                       float               Min,
                                       float               Max,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  Init();
  Create(Parent,Mode,Value,Min,Max,Id,Caption,Pos,Size,Style);
 }

void               P3DNumberEntryDialog::Init
                                      ()
 {
  Mode  = wxSPINSLIDER_MODE_INTEGER;
  Value = 50.0;
  Min   = 0.0;
  Max   = 100.0;
 }

bool               P3DNumberEntryDialog::Create
                                      (wxWindow           *Parent,
                                       int                 Mode,
                                       float               Value,
                                       float               Min,
                                       float               Max,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  if (!wxDialog::Create(Parent,Id,Caption,Pos,Size,Style))
   {
    return(false);
   }

  this->Mode  = Mode;
  this->Value = Value;
  this->Min   = Min;
  this->Max   = Max;

  CreateControls();

  return(true);
 }

void               P3DNumberEntryDialog::CreateControls
                                      ()
 {
  wxBoxSizer      *TopSizer = new wxBoxSizer(wxVERTICAL);

  EntryCtrl = new wxTextCtrl(this,wxID_ANY);

  TopSizer->Add(EntryCtrl,1,wxGROW | wxALL,5);

  wxBoxSizer      *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  ButtonSizer->Add(new wxButton(this,wxID_OK,wxT("Ok")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")),0,wxALL,5);

  ((wxButton*)FindWindow(wxID_OK))->SetDefault();

  TopSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxALL,5);

  EntryCtrl->SetFocus();

  SetSizer(TopSizer);
  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

bool               P3DNumberEntryDialog::TransferDataToWindow
                                      ()
 {
  wxString ValueStr;

  if (Mode == wxSPINSLIDER_MODE_INTEGER)
   {
    ValueStr.Printf(wxT("%d"),(int)(Value + 0.5f));
   }
  else
   {
    ValueStr.Printf(wxT("%.02f"),Value);
   }

  EntryCtrl->SetValue(ValueStr);
  EntryCtrl->SetSelection(-1,-1);

  return(true);
 }

bool               P3DNumberEntryDialog::TransferDataFromWindow
                                      ()
 {
  bool             Result;
  wxString         ValueStr;

  Result   = false;
  ValueStr = EntryCtrl->GetValue();

  if     (Mode == wxSPINSLIDER_MODE_INTEGER)
   {
    long TempValue;

    if (ValueStr.ToLong(&TempValue))
     {
      if ((TempValue >= Min) && (TempValue <= Max))
       {
        Value  = TempValue;
        Result = true;
       }
     }
   }
  else
   {
    double TempValue;

    if (ValueStr.ToDouble(&TempValue))
     {
      if ((TempValue >= Min) && (TempValue <= Max))
       {
        Value  = TempValue;
        Result = true;
       }
     }
   }

  if (!Result)
   {
    wxMessageBox(wxT("Invalid value"),wxT("Error"),wxOK | wxICON_ERROR);
   }

  return(Result);
 }

BEGIN_EVENT_TABLE(wxSpinSliderCtrl,wxControl)
 EVT_PAINT(wxSpinSliderCtrl::OnPaint)
 EVT_LEFT_DOWN(wxSpinSliderCtrl::OnLeftDown)
 EVT_LEFT_UP(wxSpinSliderCtrl::OnLeftUp)
 EVT_LEFT_DCLICK(wxSpinSliderCtrl::OnLeftDoubleClick)
 EVT_MOTION(wxSpinSliderCtrl::OnMouseMove)
 EVT_MOUSEWHEEL(wxSpinSliderCtrl::OnMouseWheel)
 EVT_ENTER_WINDOW(wxSpinSliderCtrl::OnMouseEnter)
 EVT_LEAVE_WINDOW(wxSpinSliderCtrl::OnMouseLeave)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxSpinSliderCtrl,wxControl)

                   wxSpinSliderCtrl::wxSpinSliderCtrl
                                      ()
 {
  Init();
 }

                   wxSpinSliderCtrl::wxSpinSliderCtrl
                                      (wxWindow           *parent,
                                       wxWindowID          id,
                                       int                 mode,
                                       float               value,
                                       float               min,
                                       float               max,
                                       const wxPoint      &pos,
                                       const wxSize       &size,
                                       long                style,
                                       const wxValidator  &validator,
                                       const wxString     &name)
 {
  Init();
  Create(parent,id,mode,value,min,max,pos,size,style,validator,name);
 }

bool               wxSpinSliderCtrl::Create
                                      (wxWindow           *parent,
                                       wxWindowID          id,
                                       int                 mode,
                                       float               value,
                                       float               min,
                                       float               max,
                                       const wxPoint      &pos,
                                       const wxSize       &size,
                                       long                style,
                                       const wxValidator  &validator,
                                       const wxString     &name)
 {
  #if defined(__WXGTK__)
   {
    if (!wxControl::Create(parent,id,pos,size,style | wxSUNKEN_BORDER,validator,name))
     {
      return(false);
     }
   }
  #elif defined(__WXMSW__)
   {
    if (!wxControl::Create(parent,id,pos,size,style | wxSTATIC_BORDER,validator,name))
     {
      return(false);
     }
   }
  #else
   {
    if (!wxControl::Create(parent,id,pos,size,style,validator,name))
     {
      return(false);
     }
   }
  #endif

  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

  if (size == wxDefaultSize)
   {
    SetSize(DoGetBestSize());
   }

/*  SetBestFittingSize(size); */

  this->mode = mode;

   if (max < min)
    {
     this->min = max;
     this->max = min;
    }
   else
    {
     this->min = min;
     this->max = max;
    }

   if      (value < this->min)
    {
     this->value = this->min;
    }
   else if (value > max)
    {
     this->value = this->max;
    }
   else
    {
     this->value = value;
    }

  return(true);
 }

void               wxSpinSliderCtrl::Init
                                      ()
 {
  mode  = wxSPINSLIDER_MODE_INTEGER;
  value = 50.0;
  min   = 0.0;
  max   = 100.0;

  std_step   = 10.0;
  small_step = 1.0;
  large_move = 10.0;
  std_move   = 1.0;
  small_move = 1.0;

  left_down = false;
  mx = my = -1;
 }

#define wxSPINSLIDER_SLIDER_WIDTH       (80)
#define wxSPINSLIDER_SLIDER_HEIGHT      (16)
#define wxSPINSLIDER_SLIDER_BUTTON_SIZE (wxSPINSLIDER_SLIDER_HEIGHT)

wxSize             wxSpinSliderCtrl::DoGetBestSize
                                      () const
 {
  return(wxSize(wxSPINSLIDER_SLIDER_WIDTH + 2 * wxSPINSLIDER_SLIDER_BUTTON_SIZE,
                wxSPINSLIDER_SLIDER_HEIGHT));
 }

void               wxSpinSliderCtrl::SetStdStep
                                      (float               step)
 {
  std_step = step;
 }

void               wxSpinSliderCtrl::SetSmallStep
                                      (float               step)
 {
  small_step = step;
 }

void               wxSpinSliderCtrl::SetLargeMove
                                      (float               step)
 {
  large_move = step;
 }

void               wxSpinSliderCtrl::SetStdMove
                                      (float               step)
 {
  std_move = step;
 }

void               wxSpinSliderCtrl::SetSmallMove
                                      (float               step)
 {
  small_move = step;
 }

void               wxSpinSliderCtrl::OnPaint
                                      (wxPaintEvent       &event)
 {
  wxPaintDC                            dc(this);
  int                                  center_x;
  int                                  center_y;
  wxCoord                              text_width,text_height;
  wxString                             buffer;
  int                                  client_width;
  int                                  client_height;

  wxSize client_size = dc.GetSize();

  client_width  = client_size.GetWidth();
  client_height = client_size.GetHeight();

  center_x = client_width  / 2;
  center_y = client_height / 2;

  dc.SetPen(*(wxThePenList->FindOrCreatePen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),1,wxSOLID)));
  dc.SetBrush(*(wxTheBrushList->FindOrCreateBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),wxSOLID)));

  dc.DrawRectangle(0,
                   0,
                   client_width,
                   client_height);

  dc.SetPen(*wxBLACK_PEN);
  dc.DrawLine(5,client_height / 2, client_height - 4, client_height / 2);
  dc.DrawLine(client_width - client_height + 5,
              client_height / 2,
              client_width  - 4,
              client_height / 2);
  dc.DrawLine(client_width - client_height / 2,
              5,
              client_width - client_height / 2 ,
              client_height - 4);

  /* draw value */

  dc.SetFont(*wxSMALL_FONT);

  if (mode == wxSPINSLIDER_MODE_INTEGER)
   {
    buffer.Printf(wxT("%d"),(int)value);
   }
  else
   {
    buffer.Printf(wxT("%.02f"),value);
   }

  #if wxCHECK_VERSION(2,8,0)
  dc.GetTextExtent(buffer,&text_width,&text_height,NULL,NULL,const_cast<wxFont*>(wxSMALL_FONT));
  #else
  dc.GetTextExtent(buffer,&text_width,&text_height,NULL,NULL,wxSMALL_FONT);
  #endif

  dc.DrawText(buffer,center_x - text_width / 2, center_y - text_height / 2);
 }

bool               wxSpinSliderCtrl::PointInIncButton
                                      (long                x,
                                       long                y) const
 {
  wxSize                               client_size   = GetClientSize();
  int                                  client_width  = client_size.GetWidth();
  int                                  client_height = client_size.GetHeight();

  if      (y < 0)
   {
    return(false);
   }
  else if (y > client_height)
   {
    return(false);
   }
  else if (x > client_width)
   {
    return(false);
   }
  else if (x < (client_width - client_height))
   {
    return(false);
   }

  return(true);
 }

bool               wxSpinSliderCtrl::PointInDecButton
                                      (long                x,
                                       long                y) const
 {
  wxSize                               client_size   = GetClientSize();
  int                                  client_width  = client_size.GetWidth();
  int                                  client_height = client_size.GetHeight();

  if      (y < 0)
   {
    return(false);
   }
  else if (y > client_height)
   {
    return(false);
   }
  else if (x > client_height)
   {
    return(false);
   }
  else if (x < 0)
   {
    return(false);
   }

  return(true);
 }

bool               wxSpinSliderCtrl::PointInSlider
                                      (long                x,
                                       long                y) const
 {
  wxSize                               client_size   = GetClientSize();
  int                                  client_width  = client_size.GetWidth();
  int                                  client_height = client_size.GetHeight();

  if      (y < 0)
   {
    return(false);
   }
  else if (y > client_height)
   {
    return(false);
   }
  else if (x < client_height)
   {
    return(false);
   }
  else if (x > (client_width - client_height))
   {
    return(false);
   }

  return(true);
 }

void               wxSpinSliderCtrl::OnLeftDown
                                      (wxMouseEvent       &event)
 {
  bool                                 changed;
  float                                delta;

  left_down = true;
  mx = event.GetX();
  my = event.GetY();

  changed = false;

  if      (PointInDecButton(event.GetX(),event.GetY()))
   {
    if (event.ShiftDown())
     {
      delta = -small_step;
     }
    else
     {
      delta = -std_step;
     }

    changed = true;
   }
  else if (PointInIncButton(event.GetX(),event.GetY()))
   {
    if (event.ShiftDown())
     {
      delta = small_step;
     }
    else
     {
      delta = std_step;
     }

    changed = true;
   }

  if (changed)
   {
    if (ChangeValue(delta))
     {
      Refresh();

      SendChangedEvent();
     }
   }
 }

void               wxSpinSliderCtrl::OnLeftDoubleClick
                                      (wxMouseEvent       &event)
 {
  bool                                 changed;
  float                                delta;

  left_down = false;
  mx = event.GetX();
  my = event.GetY();

  changed = false;

  if      (PointInDecButton(event.GetX(),event.GetY()))
   {
    if (event.ShiftDown())
     {
      delta = -small_step;
     }
    else
     {
      delta = -std_step;
     }

    changed = true;
   }
  else if (PointInIncButton(event.GetX(),event.GetY()))
   {
    if (event.ShiftDown())
     {
      delta = small_step;
     }
    else
     {
      delta = std_step;
     }

    changed = true;
   }
  else
   {
    P3DNumberEntryDialog               NumberDialog(GetParent(),mode,value,min,max);

    NumberDialog.Centre();

    if (NumberDialog.ShowModal() == wxID_OK)
     {
      delta = NumberDialog.GetValue() - value;

      changed = true;
     }
   }

  if (changed)
   {
    if (ChangeValue(delta))
     {
      Refresh();

      SendChangedEvent();
     }
   }
 }

void               wxSpinSliderCtrl::OnLeftUp
                                      (wxMouseEvent       &event)
 {
  mx = event.GetX();
  my = event.GetY();
  left_down = false;
 }

void               wxSpinSliderCtrl::OnMouseMove
                                      (wxMouseEvent       &event)
 {
  long                                 dx;
  long                                 dy;
  float                                delta;

  dx = event.GetX() - mx;
  dy = event.GetY() - my;

  mx += dx;
  my += dy;

  if ((PointInSlider(mx,my)) && (dx != 0))
   {
    if (event.LeftIsDown())
     {
      if      (event.ShiftDown())
       {
        delta = small_move * ((float)dx);
       }
      else if ((dx > 5) || (dx < -5))
       {
        delta = large_move * ((float)dx);
       }
      else
       {
        delta = std_move * ((float)dx);
       }

      if (ChangeValue(delta))
       {
        Refresh();

        SendChangedEvent();
       }
     }
   }
 }

void               wxSpinSliderCtrl::OnMouseWheel
                                      (wxMouseEvent       &event)
 {
  float                               delta;

  delta = ((float)event.GetWheelRotation()) / 120.0f;

  if (ChangeValue(delta * std_move))
   {
    Refresh();

    SendChangedEvent();
   }
 }

void               wxSpinSliderCtrl::OnMouseEnter
                                      (wxMouseEvent       &event)
 {
  SetFocus();
 }

void               wxSpinSliderCtrl::OnMouseLeave
                                      (wxMouseEvent       &event)
 {
  wxWindow                            *Parent;

  Parent = GetParent();

  if (Parent != 0)
   {
    Parent->SetFocus();
   }
 }

bool               wxSpinSliderCtrl::ChangeValue
                                      (float               delta)
 {
  float                                new_value;

  new_value = value + delta;

  if (new_value > max)
   {
    new_value = max;
   }
  else if (new_value < min)
   {
    new_value = min;
   }

  if (mode == wxSPINSLIDER_MODE_INTEGER)
   {
    if (((int)new_value) != ((int)value))
     {
      value = new_value;

      return(true);
     }
   }
  else
   {
    if (new_value != value)
     {
      value = new_value;

      return(true);
     }
   }

  return(false);
 }

bool               wxSpinSliderCtrl::SetValue
                                      (float               Value)
 {
  if (Value < min)
   {
    Value = min;
   }
  else if (Value > max)
   {
    Value = max;
   }

  if (mode == wxSPINSLIDER_MODE_INTEGER)
   {
    if (((int)Value) != ((int)(this->value)))
     {
      this->value = Value;

      Refresh();

      return(true);
     }
   }
  else
   {
    if (Value != this->value)
     {
      this->value = Value;

      Refresh();

      return(true);
     }
   }

  return(false);
 }

void               wxSpinSliderCtrl::SendChangedEvent
                                      (void)
 {
  wxSpinSliderEvent                    event(wxEVT_SPINSLIDER_VALUE_CHANGED,GetId(),value);

  event.SetEventObject(this);

  GetEventHandler()->ProcessEvent(event);
 }

DEFINE_EVENT_TYPE(wxEVT_SPINSLIDER_VALUE_CHANGED)

IMPLEMENT_DYNAMIC_CLASS(wxSpinSliderEvent,wxCommandEvent)

