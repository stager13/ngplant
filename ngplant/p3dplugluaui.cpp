/***************************************************************************

 Copyright (C) 2007  Sergey Prokhorchuk

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

extern "C"
 {
  #include <lua.h>
  #include <lauxlib.h>
 }

#include <string>
#include <vector>

#include <wx/wx.h>

#include <p3dplugluactl.h>
#include <p3dplugluaui.h>

static wxString    GetTableStringOpt  (lua_State          *State,
                                       int                 TableIndex,
                                       const char         *Key)
 {
  wxString         Result;

  lua_pushstring(State,Key);
  lua_gettable(State,TableIndex);

  if (lua_isstring(State,-1))
   {
    Result = wxString(lua_tostring(State,-1),wxConvUTF8);
   }

  lua_pop(State,1);

  return(Result);
 }

static wxString    GetTableString     (lua_State          *State,
                                       int                 TableIndex,
                                       const char         *Key)
 {
  wxString         Result;

  lua_pushstring(State,Key);
  lua_gettable(State,TableIndex);

  if (lua_isstring(State,-1))
   {
    Result = wxString(lua_tostring(State,-1),wxConvUTF8);
   }
  else
   {
    ::wxMessageBox(wxString::Format(wxT("Parameter description does not have %s field"),wxString(Key,wxConvUTF8).c_str()),wxT("Error"),wxICON_ERROR | wxOK);
   }

  lua_pop(State,1);

  return(Result);
 }

class P3DPlugParamCtrl
 {
  public           :

                   P3DPlugParamCtrl   (const wxString     &Name)
   {
    this->Name = Name;
   }

  virtual         ~P3DPlugParamCtrl   () {}

  virtual wxWindow*CreateControl      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       lua_State          *State) = 0;

  virtual void     RegisterValue      (lua_State          *State) = 0;

  protected        :

  wxString         Name;
 };

class P3DPlugParamNumberCtrl : public P3DPlugParamCtrl
 {
  public           :

                   P3DPlugParamNumberCtrl
                                      (const wxString     &Name)
                   : P3DPlugParamCtrl(Name)
   {
    TextCtrl = 0;
   }

  virtual wxWindow*CreateControl      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       lua_State          *State);

  virtual void     RegisterValue      (lua_State          *State);

  private          :

  wxTextCtrl      *TextCtrl;
 };

class P3DPlugParamChoiceCtrl : public P3DPlugParamCtrl
 {
  public           :

                   P3DPlugParamChoiceCtrl
                                      (const wxString     &Name)
                   : P3DPlugParamCtrl(Name)
   {
    ChoiceCtrl = 0;
   }

  virtual wxWindow*CreateControl      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       lua_State          *State);

  virtual void     RegisterValue      (lua_State          *State);

  private          :

  wxChoice        *ChoiceCtrl;
 };

wxWindow          *P3DPlugParamNumberCtrl::CreateControl
                                      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       lua_State          *State)
 {
  wxString Default = GetTableStringOpt(State,lua_gettop(State),"default");

  TextCtrl = new wxTextCtrl(Parent,Id,Default);

  return(TextCtrl);
 }

void               P3DPlugParamNumberCtrl::RegisterValue
                                      (lua_State          *State)
 {
  if (TextCtrl == 0)
   {
    return;
   }

  P3DPlugLUAControl                    Control(State);

  Control.SetTableString(Name.mb_str(),TextCtrl->GetValue().mb_str());
 }

wxWindow          *P3DPlugParamChoiceCtrl::CreateControl
                                      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       lua_State          *State)
 {
  ChoiceCtrl = new wxChoice(Parent,Id);

  lua_pushstring(State,"choices");
  lua_gettable(State,-2);

  if (lua_istable(State,-1))
   {
    bool          Done  = false;
    unsigned int  Index = 1;

    while (!Done)
     {
      lua_rawgeti(State,-1,Index);

      if      (lua_isnil(State,-1))
       {
        Done = true;
       }
      else if (lua_isstring(State,-1))
       {
        ChoiceCtrl->Append(wxString(lua_tostring(State,-1),wxConvUTF8));
       }

      lua_pop(State,1);

      Index++;
     }
   }

  lua_pop(State,1);

  ChoiceCtrl->SetSelection(0);

  return(ChoiceCtrl);
 }

void               P3DPlugParamChoiceCtrl::RegisterValue
                                      (lua_State          *State)
 {
  if (ChoiceCtrl == 0)
   {
    return;
   }

  P3DPlugLUAControl                    Control(State);

  Control.SetTableString(Name.mb_str(),ChoiceCtrl->GetStringSelection().mb_str());
 }


class P3DPlugParamDialog : public wxDialog
 {
  public           :

                   P3DPlugParamDialog ();
                   P3DPlugParamDialog (wxWindow           *Parent,
                                       lua_State          *State,
                                       int                 ParamsTblIndex,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Parameters"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
  virtual         ~P3DPlugParamDialog ();

  void             Init               ();

  bool             Create             (wxWindow           *Parent,
                                       lua_State          *State,
                                       int                 ParamsTblIndex,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Parameters"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void             CreateControls     (lua_State          *State,
                                       int                 ParamsTblIndex);

  void             FillResultTable    (lua_State          *State);

  private          :

  std::vector<P3DPlugParamCtrl*>       Params;

  DECLARE_CLASS(P3DPlugParamDialog)
 };

IMPLEMENT_CLASS(P3DPlugParamDialog,wxDialog)

                   P3DPlugParamDialog::P3DPlugParamDialog
                                      ()
 {
  Init();
 }

                   P3DPlugParamDialog::P3DPlugParamDialog
                                      (wxWindow           *Parent,
                                       lua_State          *State,
                                       int                 ParamsTblIndex,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  Init();
  Create(Parent,State,ParamsTblIndex,Id,Caption,Pos,Size,Style);
 }

                   P3DPlugParamDialog::~P3DPlugParamDialog
                                      ()
 {
  for (unsigned int Index = 0; Index < Params.size(); Index++)
   {
    delete Params[Index];
   }
 }

void               P3DPlugParamDialog::Init
                                      ()
 {
 }

bool               P3DPlugParamDialog::Create
                                      (wxWindow           *Parent,
                                       lua_State          *State,
                                       int                 ParamsTblIndex,
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

  CreateControls(State,ParamsTblIndex);

  return(true);
 }

void               P3DPlugParamDialog::CreateControls
                                      (lua_State          *State,
                                       int                 ParamsTblIndex)
 {
  wxBoxSizer           *TopSizer = new wxBoxSizer(wxVERTICAL);
  unsigned int          ParamCount;
  P3DPlugLUAControl     Control(State);

  ParamCount = Control.GetTableSizeI(ParamsTblIndex);

  wxFlexGridSizer *ParamsGridSizer = new wxFlexGridSizer(ParamCount,2,3,3);

  ParamsGridSizer->AddGrowableCol(1);

  for (unsigned int ParamIndex = 1; ParamIndex <= ParamCount; ParamIndex++)
   {
    lua_rawgeti(State,ParamsTblIndex,ParamIndex);

    if (lua_istable(State,-1))
     {
      wxString Type  = GetTableString(State,lua_gettop(State),"type");
      wxString Label = GetTableString(State,lua_gettop(State),"label");
      wxString Name  = GetTableString(State,lua_gettop(State),"name");
      P3DPlugParamCtrl *Param = 0;

      if      (Type == wxT("number"))
       {
        Param = new P3DPlugParamNumberCtrl(Name);
       }
      else if (Type == wxT("choice"))
       {
        Param = new P3DPlugParamChoiceCtrl(Name);
       }
      else
       {
        ::wxMessageBox(wxString::Format(wxT("Parameter %s type (%s) is not supported"),Name.c_str(),Type.c_str()),wxT("Error"),wxICON_ERROR | wxOK);
       }

      if (Param != 0)
       {
        wxWindow  *Ctrl = Param->CreateControl(this,wxID_ANY,State);

        ParamsGridSizer->Add(new wxStaticText(this,wxID_ANY,Label),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

        Params.push_back(Param);

        if (ParamIndex == 1)
         {
          Ctrl->SetFocus();
         }

        ParamsGridSizer->Add(Ctrl,1,wxALL | wxALIGN_RIGHT,1);
       }
     }
    else
     {
      ::wxMessageBox(wxT("Parameter description must be a table"),wxT("Error"),wxICON_ERROR | wxOK);
     }
   }

  TopSizer->Add(ParamsGridSizer,1,wxGROW | wxALL,5);

  wxBoxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  wxButton *OkButton = new wxButton(this,wxID_OK,wxT("Ok"));

  ButtonSizer->Add(OkButton,0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")),0,wxALL,5);

  TopSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxALL,5);

  OkButton->SetDefault();

  SetSizer(TopSizer);
  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DPlugParamDialog::FillResultTable
                                      (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);

  Control.PushNewTable();

  for (unsigned int Index = 0; Index < Params.size(); Index++)
   {
    Params[Index]->RegisterValue(State);
   }
 }

static int         ShowFileOpenDialog (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Message;
  wxString                             FileName;

  Message = Control.GetArgString(1);

  Control.Commit();

  FileName = wxFileSelector(wxString(Message,wxConvUTF8),
                            wxEmptyString,
                            wxEmptyString,
                            wxEmptyString,
                            wxT("*.*"),
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (!FileName.empty())
   {
    Control.PushString(FileName.mb_str());
   }
  else
   {
    Control.PushNil();
   }

  Control.Commit();

  return(1);
 }

static int         ShowFileSaveDialog (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Message;
  wxString                             FileName;

  Message = Control.GetArgString(1);

  Control.Commit();

  FileName = wxFileSelector(wxString(Message,wxConvUTF8),
                            wxEmptyString,
                            wxEmptyString,
                            wxEmptyString,
                            wxT("*.*"),
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (!FileName.empty())
   {
    Control.PushString(FileName.mb_str());
   }
  else
   {
    Control.PushNil();
   }

  Control.Commit();

  return(1);
 }

static int         ShowMessageBox     (lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Message;
  const char                          *Caption;

  Message = Control.GetArgString(1);
  Caption = Control.GetArgString(2);

  Control.Commit();

  wxMessageBox(wxString(Message,wxConvUTF8),
               wxString(Caption,wxConvUTF8));

  return(0);
 }

static int         ShowYesNoMessageBox(lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  const char                          *Message;
  const char                          *Caption;

  Message = Control.GetArgString(1);
  Caption = Control.GetArgString(2);

  Control.Commit();

  if (wxMessageBox(wxString(Message,wxConvUTF8),
                   wxString(Caption,wxConvUTF8),
                   wxYES_NO) == wxYES)
   {
    Control.PushBool(true);
   }
  else
   {
    Control.PushBool(false);
   }

  return(1);
 }

static int         ShowParameterDialog(lua_State          *State)
 {
  P3DPlugLUAControl                    Control(State);
  unsigned int                         ParamCount;

  Control.CheckArgTable(1);
  Control.Commit();

  P3DPlugParamDialog *ParamDialog = new P3DPlugParamDialog(NULL,State,1);

  if (ParamDialog->ShowModal() == wxID_OK)
   {
    ParamDialog->FillResultTable(State);
   }
  else
   {
    Control.PushNil();
   }

  delete ParamDialog;

  Control.Commit();

  return(1);
 }

extern void        P3DPlugLuaRegisterUI
                                      (lua_State          *State)
 {
  lua_register(State,"ShowFileOpenDialog",ShowFileOpenDialog);
  lua_register(State,"ShowFileSaveDialog",ShowFileSaveDialog);
  lua_register(State,"ShowMessageBox",ShowMessageBox);
  lua_register(State,"ShowYesNoMessageBox",ShowYesNoMessageBox);
  lua_register(State,"ShowParameterDialog",ShowParameterDialog);
 }

