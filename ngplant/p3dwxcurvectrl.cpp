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
#include <wx/filename.h>

#include <ngpcore/p3dsplineio.h>

#include <p3dappprefs.h>

#include <p3dwxcurvectrl.h>

enum
 {
  P3D_RESET_TO_DEFAULT = wxID_HIGHEST + 1,
  P3D_ID_EXPORT                          ,
  P3D_ID_IMPORT
 };

unsigned int  P3DCurveCtrl::BestWidth  = P3DCurveCtrlMinWidth;
unsigned int  P3DCurveCtrl::BestHeight = P3DCurveCtrlMinHeight;

class P3DCurveCtrlDialog : public wxDialog
 {
  public           :

                   P3DCurveCtrlDialog ();
                   P3DCurveCtrlDialog (wxWindow           *Parent,
                                       const P3DMathNaturalCubicSpline
                                                          &Curve,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Curve editor"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  bool             Create             (wxWindow           *Parent,
                                       const P3DMathNaturalCubicSpline
                                                          &Curve,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Curve editor"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  const P3DMathNaturalCubicSpline
                  &GetCurve           () const;

  void             SetDefaultCurve    (const P3DMathNaturalCubicSpline
                                                          &Curve);

  void             OnExportButtonClicked
                                      (wxCommandEvent     &Event);
  void             OnImportButtonClicked
                                      (wxCommandEvent     &Event);

  private          :

  void             CreateControls     (const P3DMathNaturalCubicSpline
                                                          &Curve);

  P3DCurveCtrl    *CurveCtrl;

  DECLARE_CLASS(P3DCurveCtrlDialog)
  DECLARE_EVENT_TABLE()
 };

BEGIN_EVENT_TABLE(P3DCurveCtrlDialog,wxDialog)
 EVT_BUTTON(P3D_ID_EXPORT,P3DCurveCtrlDialog::OnExportButtonClicked)
 EVT_BUTTON(P3D_ID_IMPORT,P3DCurveCtrlDialog::OnImportButtonClicked)
END_EVENT_TABLE()

IMPLEMENT_CLASS(P3DCurveCtrlDialog,wxDialog)

                   P3DCurveCtrlDialog::P3DCurveCtrlDialog
                                      ()
 {
 }

                   P3DCurveCtrlDialog::P3DCurveCtrlDialog
                                      (wxWindow           *Parent,
                                       const P3DMathNaturalCubicSpline
                                                          &Curve,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  Create(Parent,Curve,Id,Caption,Pos,Size,Style);
 }

bool               P3DCurveCtrlDialog::Create
                                      (wxWindow           *Parent,
                                       const P3DMathNaturalCubicSpline
                                                          &Curve,
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

  CreateControls(Curve);

  return(true);
 }

void               P3DCurveCtrlDialog::CreateControls
                                      (const P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  wxBoxSizer      *TopSizer = new wxBoxSizer(wxVERTICAL);

  #if wxCHECK_VERSION(2,6,0)
  CurveCtrl = new P3DCurveCtrl(this,wxID_ANY,Curve,wxDefaultPosition,wxSize(320,160),wxFULL_REPAINT_ON_RESIZE);
  CurveCtrl->SetMinSize(wxSize(320,160));
  #else
  CurveCtrl = new P3DCurveCtrl(this,wxID_ANY,Curve,wxDefaultPosition,wxSize(320,160));
  #endif

  CurveCtrl->EnableEditDialog(false);

  TopSizer->Add(CurveCtrl,1,wxGROW | wxALL,5);

  wxBoxSizer      *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  ButtonSizer->Add(new wxButton(this,P3D_ID_IMPORT,wxT("Import...")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,P3D_ID_EXPORT,wxT("Export...")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_OK,wxT("Ok")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")),0,wxALL,5);

  ((wxButton*)FindWindow(wxID_OK))->SetDefault();

  TopSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxALL,5);

  SetSizer(TopSizer);
  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

const P3DMathNaturalCubicSpline
                  &P3DCurveCtrlDialog::GetCurve
                                      () const
 {
  return(CurveCtrl->GetCurve());
 }

void               P3DCurveCtrlDialog::SetDefaultCurve
                                      (const P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  CurveCtrl->SetDefaultCurve(Curve);
 }

void               P3DCurveCtrlDialog::OnExportButtonClicked
                                      (wxCommandEvent     &Event)
 {
  wxString       FileNameStr;

  FileNameStr = ::wxFileSelector(wxT("File name"),wxT(""),wxT(""),wxT(".ngc"),wxT("*.ngc"),wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (!FileNameStr.empty())
   {
    wxFileName FileName(FileNameStr);

    if (!FileName.HasExt())
     {
      FileName.SetExt(wxT("ngc"));
     }

    try
     {
      P3DOutputStringStreamFile          TargetStream;

      TargetStream.Open(FileName.GetFullPath().mb_str());

      P3DExportSplineCurve(&TargetStream,&CurveCtrl->GetCurve());

      TargetStream.Close();
     }
    catch (P3DException &Exception)
     {
      ::wxMessageBox(wxString(Exception.GetMessage(),wxConvUTF8),
                     wxT("Error"),wxOK | wxICON_ERROR);
     }
   }
 }

void               P3DCurveCtrlDialog::OnImportButtonClicked(wxCommandEvent     &Event)
 {
  wxString       FileName;

  FileName = ::wxFileSelector(wxT("File name"),wxT(""),wxT(""),wxT(".ngc"),wxT("*.ngc"),wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (!FileName.empty())
   {
    try
     {
      P3DMathNaturalCubicSpline        Spline;
      P3DInputStringStreamFile         SourceStream;

      SourceStream.Open(FileName.mb_str());

      P3DImportSplineCurve(&Spline,&SourceStream);

      SourceStream.Close();

      CurveCtrl->SetCurve(Spline);
     }
    catch (P3DException &Exception)
     {
      ::wxMessageBox(wxString(Exception.GetMessage(),wxConvUTF8),
                     wxT("Error"),wxOK | wxICON_ERROR);
     }
   }
 }

BEGIN_EVENT_TABLE(P3DCurveCtrl,wxWindow)
 EVT_PAINT(P3DCurveCtrl::OnPaint)
 EVT_LEFT_DOWN(P3DCurveCtrl::OnLeftDown)
 EVT_LEFT_DCLICK(P3DCurveCtrl::OnLeftDblClick)
 EVT_LEFT_UP(P3DCurveCtrl::OnLeftUp)
 EVT_MOTION(P3DCurveCtrl::OnMouseMove)
 EVT_RIGHT_DOWN(P3DCurveCtrl::OnRightDown)
 EVT_LEAVE_WINDOW(P3DCurveCtrl::OnLeaveWindow)
 EVT_ERASE_BACKGROUND(P3DCurveCtrl::onEraseBackground)
 EVT_MENU(P3D_RESET_TO_DEFAULT,P3DCurveCtrl::OnResetToDefault)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(P3DCurveCtrl,wxControl)

                   P3DCurveCtrl::P3DCurveCtrl
                                      ()
 {
  Init();
 }

                   P3DCurveCtrl::P3DCurveCtrl
                                      (wxWindow           *parent,
                                       wxWindowID          id,
                                       const P3DMathNaturalCubicSpline
                                                          &curve,
                                       const wxPoint      &pos,
                                       const wxSize       &size,
                                       long                style,
                                       const wxValidator  &validator,
                                       const wxString     &name)
 {
  Init();
  Create(parent,id,curve,pos,size,style,validator,name);
 }

bool               P3DCurveCtrl::Create
                                      (wxWindow           *parent,
                                       wxWindowID          id,
                                       const P3DMathNaturalCubicSpline
                                                          &curve,
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

  this->curve = curve;

  return(true);
 }

void               P3DCurveCtrl::SetCurve
                                      (const P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  this->curve = Curve;

  Refresh();
 }

void               P3DCurveCtrl::SetDefaultCurve
                                      (const P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  HaveDefaultCurve = true;
  DefaultCurve.CopyFrom(Curve);
 }

void               P3DCurveCtrl::Init ()
 {
  ActiveControlPoint = -1;
  HaveDefaultCurve = false;
  DialogEnabled    = true;
 }

wxSize             P3DCurveCtrl::DoGetBestSize
                                      () const
 {
  return(wxSize(BestWidth,BestHeight));
 }

void               P3DCurveCtrl::OnLeftDown
                                      (wxMouseEvent       &event)
 {
  CalcTransform(GetClientSize());

  MousePosX = event.GetX();
  MousePosY = event.GetY();

  ActiveControlPoint = GetCPByMousePos(MousePosX,MousePosY);
 }

void               P3DCurveCtrl::OnLeftDblClick
                                      (wxMouseEvent       &event)
 {
  if (DialogEnabled)
   {
    P3DCurveCtrlDialog    Dialog(GetParent(),curve);

    if (HaveDefaultCurve)
     {
      Dialog.SetDefaultCurve(DefaultCurve);
     }

    Dialog.Centre();

    if (Dialog.ShowModal() == wxID_OK)
     {
      curve = Dialog.GetCurve();

      Refresh();

      SendChangedEvent();
     }
   }
 }

void               P3DCurveCtrl::OnRightDown
                                      (wxMouseEvent       &event)
 {
  CalcTransform(GetClientSize());

  MousePosX = event.GetX();
  MousePosY = event.GetY();

  ActiveControlPoint = GetCPByMousePos(MousePosX,MousePosY);

  if ((ActiveControlPoint != -1) &&
      (ActiveControlPoint != 0) &&
      (ActiveControlPoint != (curve.GetCPCount() - 1)))
   {
    curve.DelCP(ActiveControlPoint);

    ActiveControlPoint = -1;

    Refresh();

    SendChangedEvent();
   }
  else
   {
    if (HaveDefaultCurve)
     {
      wxMenu                           PopupMenu;

      PopupMenu.Append(P3D_RESET_TO_DEFAULT,wxT("Reset to default"));

      this->PopupMenu(&PopupMenu,event.GetPosition());
     }
   }
 }

void               P3DCurveCtrl::OnLeftUp
                                      (wxMouseEvent       &event)
 {
  ActiveControlPoint = -1;
 }

void               P3DCurveCtrl::OnLeaveWindow
                                      (wxMouseEvent       &event)
 {
  ActiveControlPoint = -1;
 }

void               P3DCurveCtrl::OnMouseMove
                                      (wxMouseEvent       &event)
 {
  float                                cp_x;
  float                                cp_y;
  int                                  dx,dy;

  CalcTransform(GetClientSize());

  if (ActiveControlPoint != -1)
   {
    dx = event.GetX() - MousePosX;
    dy = event.GetY() - MousePosY;

    MousePosX += dx;
    MousePosY += dy;

    if (ActiveControlPoint == 0)
     {
      cp_x = 0.0f;
     }
    else if (ActiveControlPoint == (curve.GetCPCount() - 1))
     {
      cp_x = 1.0f;
     }
    else
     {
      cp_x = RegionToCurveX(MousePosX);
     }

    cp_y = RegionToCurveY(MousePosY);

    if (cp_y < 0.0f)
     {
      cp_y = 0.0f;
     }
    else if (cp_y > 1.0f)
     {
      cp_y = 1.0f;
     }

    curve.UpdateCP(cp_x,cp_y,ActiveControlPoint);

    Refresh();

    SendChangedEvent();
   }
  else
   {
    if (event.LeftIsDown())
     {
      cp_x = RegionToCurveX(MousePosX);
      cp_y = RegionToCurveY(MousePosY);

      if ((cp_x < 0.0f) || (cp_x > 1.0f) || (cp_y < 0.0f) || (cp_y > 1.0f))
       {
        return;
       }

      int cy = CurveToRegionY(curve.GetValue(cp_x));

      dy = cy - MousePosY;

      if ((dy > -3) && (dy < 3))
       {
        curve.AddCP(cp_x,cp_y);

        dx = event.GetX() - MousePosX;
        dy = event.GetY() - MousePosY;

        MousePosX += dx;
        MousePosY += dy;

        ActiveControlPoint = GetCPByMousePos(MousePosX,MousePosY);

        Refresh();

        SendChangedEvent();
       }
     }
   }
 }

int                P3DCurveCtrl::GetCPByMousePos
                                      (int                 x,
                                       int                 y) const
 {
  float                                cp_x;
  float                                cp_y;
  int                                  dx;
  int                                  dy;

  for (unsigned int i = 0; i < curve.GetCPCount(); i++)
   {
    cp_x = curve.GetCPX(i);
    cp_y = curve.GetCPY(i);

    dx = x - CurveToRegionX(cp_x);
    dy = y - CurveToRegionY(cp_y);

    if ((dx > -3) && (dx < 3) && (dy > -3) && (dy < 3))
     {
      return(i);
     }
   }

  return(-1);
 }

#define P3DCURVECTRL_BORDER_SIZE (5)

void               P3DCurveCtrl::OnPaint
                                      (wxPaintEvent       &event)
 {
  wxPaintDC                            dc(this);
  int                                  reg_width,reg_height;
  float                                x0,y0,y1;

  wxSize client_size = dc.GetSize();

  CalcTransform(client_size);

  reg_width  = client_size.GetX() - P3DCURVECTRL_BORDER_SIZE * 2;
  reg_height = client_size.GetY() - P3DCURVECTRL_BORDER_SIZE * 2;

  dc.SetPen(*(wxThePenList->FindOrCreatePen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),1,wxSOLID)));
  dc.SetBrush(*(wxTheBrushList->FindOrCreateBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),wxSOLID)));

  dc.DrawRectangle(0,0,client_size.GetX(),P3DCURVECTRL_BORDER_SIZE);
  dc.DrawRectangle(0,client_size.GetY() - P3DCURVECTRL_BORDER_SIZE,client_size.GetX(),P3DCURVECTRL_BORDER_SIZE);
  dc.DrawRectangle(0,P3DCURVECTRL_BORDER_SIZE,P3DCURVECTRL_BORDER_SIZE,reg_height);
  dc.DrawRectangle(client_size.GetX() - P3DCURVECTRL_BORDER_SIZE,P3DCURVECTRL_BORDER_SIZE,P3DCURVECTRL_BORDER_SIZE,reg_height);

  /* clear region */

  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(*wxBLACK_BRUSH);
  dc.DrawRectangle(P3DCURVECTRL_BORDER_SIZE,
                   P3DCURVECTRL_BORDER_SIZE,
                   reg_width,
                   reg_height);


  /* draw curve */

  dc.SetPen(*wxWHITE_PEN);

  y0 = curve.GetValue(0.0f);
  y0 = CurveToRegionY(y0);

  for (int x = 1; x <= reg_width; x++)
   {
    y1 = curve.GetValue((float)x / reg_width);

    y1 = CurveToRegionY(y1);

    dc.DrawLine(x - 1 + P3DCURVECTRL_BORDER_SIZE,(int)y0,x + P3DCURVECTRL_BORDER_SIZE,(int)y1);

    y0 = y1;
   }


  dc.SetPen(*wxRED_PEN);
  dc.SetBrush(*wxRED_BRUSH);

  /* draw CP's */

  for (unsigned int i = 0; i < curve.GetCPCount(); i++)
   {
    x0 = curve.GetCPX(i);
    y0 = curve.GetCPY(i);
    dc.DrawCircle(CurveToRegionX(x0),CurveToRegionY(y0),3);
   }
 }

void               P3DCurveCtrl::onEraseBackground
                                      (wxEraseEvent       &event)
 {
 }

void               P3DCurveCtrl::OnResetToDefault
                                      (wxCommandEvent     &event)
 {
  if (HaveDefaultCurve)
   {
    curve.CopyFrom(DefaultCurve);

    Refresh();

    SendChangedEvent();
   }
 }

/* from curve to window */
void               P3DCurveCtrl::CalcTransform
                                      (const wxSize       &client_size)
 {
  int                                  reg_width,reg_height;

  reg_width  = client_size.GetX() - P3DCURVECTRL_BORDER_SIZE * 2;
  reg_height = client_size.GetY() - P3DCURVECTRL_BORDER_SIZE * 2;

  ta_x = (float)reg_width;
  tb_x = P3DCURVECTRL_BORDER_SIZE;
  ta_y = -((float)reg_height);
  tb_y = (float)(reg_height + P3DCURVECTRL_BORDER_SIZE - 1);
 }

float              P3DCurveCtrl::RegionToCurveX
                                      (int                 x) const
 {
  return(((float)x - tb_x) / ta_x);
 }

float              P3DCurveCtrl::RegionToCurveY
                                      (int                 y) const
 {
  return(((float)y - tb_y) / ta_y);
 }

int                P3DCurveCtrl::CurveToRegionX
                                      (float               x) const
 {
  return((int)(x * ta_x + tb_x));
 }

int                P3DCurveCtrl::CurveToRegionY
                                      (float               y) const
 {
  return((int)(y * ta_y + tb_y));
 }

void               P3DCurveCtrl::SendChangedEvent
                                      (void)
 {
  P3DCurveCtrlEvent                    event(wxEVT_P3DCURVECTRL_CURVE_CHANGED,GetId(),&curve);

  event.SetEventObject(this);

  GetEventHandler()->ProcessEvent(event);
 }

DEFINE_EVENT_TYPE(wxEVT_P3DCURVECTRL_CURVE_CHANGED)

IMPLEMENT_DYNAMIC_CLASS(P3DCurveCtrlEvent,wxCommandEvent)

