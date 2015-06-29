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
#include <p3dcmdqueue.h>
#include <p3duioptgeneral.h>
#include <p3duilicensedialog.h>

enum
 {
  wxID_SEED_CTRL = wxID_HIGHEST + 1,
  wxID_LOD_CTRL,
  wxID_RANDOMNESS_STATE_CTRL,
  wxID_AUTHOR_CTRL,
  wxID_AUTHOR_URL_CTRL,
  wxID_LICENSE_NAME_CTRL,
  wxID_LICENSE_WIZARD_BUTTON,
  wxID_LICENSE_URL_CTRL,
  wxID_PLANT_INFO_URL_CTRL
 };

BEGIN_EVENT_TABLE(P3DOptGeneralPanel,wxPanel)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_SEED_CTRL,P3DOptGeneralPanel::OnSeedChanged)
 EVT_SPINSLIDER_VALUE_CHANGED(wxID_LOD_CTRL,P3DOptGeneralPanel::OnLODChanged)
 EVT_CHECKBOX(wxID_RANDOMNESS_STATE_CTRL,P3DOptGeneralPanel::OnRandomnessStateChanged)
 EVT_TEXT(wxID_AUTHOR_CTRL,P3DOptGeneralPanel::OnAuthorChanged)
 EVT_TEXT(wxID_AUTHOR_URL_CTRL,P3DOptGeneralPanel::OnAuthorURLChanged)
 EVT_TEXT(wxID_LICENSE_NAME_CTRL,P3DOptGeneralPanel::OnLicenseNameChanged)
 EVT_BUTTON(wxID_LICENSE_WIZARD_BUTTON,P3DOptGeneralPanel::OnLicenseWizardClicked)
 EVT_TEXT(wxID_LICENSE_URL_CTRL,P3DOptGeneralPanel::OnLicenseURLChanged)
 EVT_TEXT(wxID_PLANT_INFO_URL_CTRL,P3DOptGeneralPanel::OnPlantInfoURLChanged)
END_EVENT_TABLE()

                   P3DOptGeneralPanel::P3DOptGeneralPanel
                                      (wxWindow           *parent)
                   : P3DUIParamPanel(parent)
 {
  wxBoxSizer *TopSizer = new wxBoxSizer(wxVERTICAL);

  TopSizer->Add(CreateRandomnessBox(),0,wxEXPAND | wxALL,1);
  TopSizer->Add(CreateLODBox(),0,wxEXPAND | wxALL,1);
  TopSizer->Add(CreateModelInfoBox(),0,wxEXPAND | wxALL,1);

  SetSizer(TopSizer);

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

wxSizer           *P3DOptGeneralPanel::CreateRandomnessBox
                                      ()
 {
  wxStaticBoxSizer *SeedTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Randomness")),wxVERTICAL);
  wxFlexGridSizer  *SeedGridSizer = new wxFlexGridSizer(2,2,3,3);

  SeedGridSizer->AddGrowableCol(1);

  SeedGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Seed")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_SEED_CTRL,wxSPINSLIDER_MODE_INTEGER,P3DApp::GetApp()->GetModel()->GetBaseSeed(),1,100000);
  spin_slider->SetStdStep(10);
  spin_slider->SetSmallStep(1);
  spin_slider->SetLargeMove(100);
  spin_slider->SetStdMove(10);
  spin_slider->SetSmallMove(1);

  SeedGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  SeedGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Disable randomness")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *RandomnessDisabledCheckBox = new wxCheckBox(this,wxID_RANDOMNESS_STATE_CTRL,wxT(""));
  RandomnessDisabledCheckBox->SetValue((P3DApp::GetApp()->GetModel()->GetFlags() & P3D_MODEL_FLAG_NO_RANDOMNESS) != 0);

  SeedGridSizer->Add(RandomnessDisabledCheckBox,1,wxALL | wxALIGN_RIGHT,1);

  SeedTopSizer->Add(SeedGridSizer,0,wxEXPAND,0);

  return SeedTopSizer;
 }

wxSizer           *P3DOptGeneralPanel::CreateLODBox
                                      ()
 {
  wxStaticBoxSizer *LODTopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("LOD")),wxVERTICAL);
  wxFlexGridSizer  *LODGridSizer = new wxFlexGridSizer(1,2,3,3);

  LODGridSizer->AddGrowableCol(1);

  LODGridSizer->Add(new wxStaticText(this,wxID_ANY,wxT("LOD level")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxSpinSliderCtrl *spin_slider = new wxSpinSliderCtrl(this,wxID_LOD_CTRL,wxSPINSLIDER_MODE_FLOAT,P3DApp::GetApp()->GetLODLevel(),0.0f,1.0f);

  spin_slider->SetStdStep(0.1f);
  spin_slider->SetSmallStep(0.01f);
  spin_slider->SetLargeMove(0.05f);
  spin_slider->SetStdMove(0.02f);
  spin_slider->SetSmallMove(0.01f);

  LODGridSizer->Add(spin_slider,1,wxALL | wxALIGN_RIGHT,1);

  LODTopSizer->Add(LODGridSizer,0,wxEXPAND,0);

  return LODTopSizer;
 }

wxSizer           *P3DOptGeneralPanel::CreateModelInfoBox
                                      ()
 {
  wxStaticBoxSizer *TopSizer  = new wxStaticBoxSizer(new wxStaticBox(this,wxID_STATIC,wxT("Model information")),wxVERTICAL);
  wxBoxSizer       *BoxSizer  = new wxBoxSizer(wxVERTICAL);

  const P3DModelMetaInfo *MetaInfo = P3DApp::GetApp()->GetModel()->GetMetaInfo();

  BoxSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Author:")),0,wxALL,1);
  BoxSizer->Add(CreateInfoTextCtrl(wxID_AUTHOR_CTRL,MetaInfo->GetAuthor()),0,wxALL | wxEXPAND,1);

  BoxSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Author URL:")),0,wxALL,1);
  BoxSizer->Add(CreateInfoTextCtrl(wxID_AUTHOR_URL_CTRL,MetaInfo->GetAuthorURL()),0,wxALL | wxEXPAND,1);

  BoxSizer->Add(new wxStaticText(this,wxID_ANY,wxT("License:")),0,wxALL,1);

  wxBoxSizer *LicenseSizer = new wxBoxSizer(wxHORIZONTAL);

  LicenseSizer->Add(CreateInfoTextCtrl(wxID_LICENSE_NAME_CTRL,MetaInfo->GetLicenseName()),1,wxALL | wxEXPAND,1);

  wxButton *LicenseWizardButton = new wxButton(this,wxID_LICENSE_WIZARD_BUTTON,wxT("..."));

  wxSize    ButtonSize = LicenseWizardButton->GetSize();

  ButtonSize.SetWidth(ButtonSize.GetHeight());

  LicenseWizardButton->SetMaxSize(ButtonSize);

  LicenseSizer->Add(LicenseWizardButton,0,wxALL,1);

  BoxSizer->Add(LicenseSizer,0,wxEXPAND | wxALL,1);

  BoxSizer->Add(new wxStaticText(this,wxID_ANY,wxT("License URL:")),0,wxALL,1);
  BoxSizer->Add(CreateInfoTextCtrl(wxID_LICENSE_URL_CTRL,MetaInfo->GetLicenseURL()),0,wxALL | wxEXPAND,1);
  BoxSizer->Add(new wxStaticText(this,wxID_ANY,wxT("Plant description URL:")),0,wxALL,1);
  BoxSizer->Add(CreateInfoTextCtrl(wxID_PLANT_INFO_URL_CTRL,MetaInfo->GetPlantInfoURL()),0,wxALL | wxEXPAND,1);

  TopSizer->Add(BoxSizer,0,wxEXPAND,0);

  return TopSizer;
 }

wxTextCtrl        *P3DOptGeneralPanel::CreateInfoTextCtrl
                                      (int                 id,
                                       const char         *value)
 {
  wxTextCtrl *Ctrl = new wxTextCtrl(this,id,InfoValueToText(value));

  Ctrl->SetMaxLength(P3DModelMetaInfo::ValueMaxLength);

  return Ctrl;
 }

wxString           P3DOptGeneralPanel::InfoValueToText
                                      (const char         *value)
 {
  if (value == 0)
   {
    return wxEmptyString;
   }
  else
   {
    return wxString(value,wxConvUTF8);
   }
 }

const char        *P3DOptGeneralPanel::TextToInfoValue
                                      (const wxString     &text)
 {
  wxString trimmedText = text;

  trimmedText.Trim(false); // remove leading white-spaces
  trimmedText.Trim(true);  // remove trailing white-spaces

  if (trimmedText.IsEmpty())
   {
    return 0;
   }
  else
   {
    return trimmedText.mb_str();
   }
 }

namespace {

class ChangeModelSeedCommand : public P3DEditCommand
 {
  public           :

                   ChangeModelSeedCommand
                                      (unsigned int        NewSeed)
   {
    OldSeed       = P3DApp::GetApp()->GetModel()->GetBaseSeed();
    this->NewSeed = NewSeed;
   }

  virtual void     Exec               ()
   {
    P3DApp::GetApp()->GetModel()->SetBaseSeed(NewSeed);
    P3DApp::GetApp()->InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    P3DApp::GetApp()->GetModel()->SetBaseSeed(OldSeed);
    P3DApp::GetApp()->InvalidatePlant();
   }

  private          :

  unsigned int     NewSeed;
  unsigned int     OldSeed;
 };

class SetModelInfoStrCmd : public P3DEditCommand
 {
  public           :

                   SetModelInfoStrCmd (void (P3DModelMetaInfo::*SetInfoMethod)(const char*),
                                       const wxString          &NewValue,
                                       const wxString          &OldValue)
   {
    this->SetInfoMethod = SetInfoMethod;
    this->NewValue      = NewValue;
    this->OldValue      = OldValue;
   }

  virtual void     Exec               ()
   {
    (P3DApp::GetApp()->GetModel()->GetMetaInfo()->*SetInfoMethod)(NewValue.mb_str());
    P3DApp::GetApp()->InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    (P3DApp::GetApp()->GetModel()->GetMetaInfo()->*SetInfoMethod)(OldValue.mb_str());
    P3DApp::GetApp()->InvalidatePlant();
   }

  private          :

  void             (P3DModelMetaInfo::*SetInfoMethod)(const char*);
  wxString         NewValue;
  wxString         OldValue;
 };
}

void               P3DOptGeneralPanel::OnSeedChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->ExecEditCmd(new ChangeModelSeedCommand(event.GetIntValue()));
 }

void               P3DOptGeneralPanel::OnLODChanged
                                      (wxSpinSliderEvent  &event)
 {
  P3DApp::GetApp()->SetLODLevel(event.GetFloatValue());

  P3DApp::GetApp()->InvalidatePlant();
 }

void               P3DOptGeneralPanel::OnRandomnessStateChanged
                                      (wxCommandEvent     &event)
 {
  P3DPlantModel* Model = P3DApp::GetApp()->GetModel();
  unsigned int   Flags = Model->GetFlags();

  if (event.IsChecked())
   {
    Flags |= P3D_MODEL_FLAG_NO_RANDOMNESS;
   }
  else
   {
    Flags &= ~P3D_MODEL_FLAG_NO_RANDOMNESS;
   }

  Model->SetFlags(Flags);
  P3DApp::GetApp()->InvalidatePlant();
 }

void               P3DOptGeneralPanel::OnAuthorChanged
                                      (wxCommandEvent     &event)
 {
  /* It seems that wxWidgets (at least gtk version) sends EVT_TEXT */
  /* events even during creation of wxTextCtrl. To workaround this */
  /* issue we ignore events if text wasn't changed.                */
  wxString NewValue = event.GetString();
  wxString OldValue = InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetAuthor());

  if (NewValue.Cmp(OldValue) != 0)
   {
    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetAuthor,NewValue,OldValue));
   }
 }

void               P3DOptGeneralPanel::OnAuthorURLChanged
                                      (wxCommandEvent     &event)
 {
  /* It seems that wxWidgets (at least gtk version) sends EVT_TEXT */
  /* events even during creation of wxTextCtrl. To workaround this */
  /* issue we ignore events if text wasn't changed.                */
  wxString NewValue = event.GetString();
  wxString OldValue = InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetAuthorURL());

  if (NewValue.Cmp(OldValue) != 0)
   {
    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetAuthorURL,NewValue,OldValue));
   }
 }

void               P3DOptGeneralPanel::OnLicenseNameChanged
                                      (wxCommandEvent     &event)
 {
  /* It seems that wxWidgets (at least gtk version) sends EVT_TEXT */
  /* events even during creation of wxTextCtrl. To workaround this */
  /* issue we ignore events if text wasn't changed.                */
  wxString NewValue = event.GetString();
  wxString OldValue = InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetLicenseName());

  if (NewValue.Cmp(OldValue) != 0)
   {
    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetLicenseName,NewValue,OldValue));
   }
 }

void               P3DOptGeneralPanel::OnLicenseURLChanged
                                      (wxCommandEvent     &event)
 {
  /* It seems that wxWidgets (at least gtk version) sends EVT_TEXT */
  /* events even during creation of wxTextCtrl. To workaround this */
  /* issue we ignore events if text wasn't changed.                */
  wxString NewValue = event.GetString();
  wxString OldValue = InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetLicenseURL());

  if (NewValue.Cmp(OldValue) != 0)
   {
    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetLicenseURL,NewValue,OldValue));
   }
 }

void               P3DOptGeneralPanel::OnPlantInfoURLChanged
                                      (wxCommandEvent     &event)
 {
  /* It seems that wxWidgets (at least gtk version) sends EVT_TEXT */
  /* events even during creation of wxTextCtrl. To workaround this */
  /* issue we ignore events if text wasn't changed.                */
  wxString NewValue = event.GetString();
  wxString OldValue = InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetPlantInfoURL());

  if (NewValue.Cmp(OldValue) != 0)
   {
    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetPlantInfoURL,NewValue,OldValue));
   }
 }

void               P3DOptGeneralPanel::UpdateControls
                                      ()
 {
  const P3DPlantModel *Model = P3DApp::GetApp()->GetModel();

  wxSpinSliderCtrl *SpinSlider;

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(wxID_SEED_CTRL);

  if (SpinSlider != NULL)
   {
    SpinSlider->SetValue(Model->GetBaseSeed());
   }

  wxCheckBox *CheckBox;

  CheckBox = (wxCheckBox*)FindWindow(wxID_RANDOMNESS_STATE_CTRL);

  if (CheckBox != NULL)
   {
    CheckBox->SetValue
     ((Model->GetFlags() & P3D_MODEL_FLAG_NO_RANDOMNESS) != 0);
   }

  const P3DModelMetaInfo *MetaInfo = Model->GetMetaInfo();

  UpdateTextCtrl(wxID_AUTHOR_CTRL,MetaInfo->GetAuthor());
  UpdateTextCtrl(wxID_AUTHOR_URL_CTRL,MetaInfo->GetAuthorURL());
  UpdateTextCtrl(wxID_LICENSE_NAME_CTRL,MetaInfo->GetLicenseName());
  UpdateTextCtrl(wxID_LICENSE_URL_CTRL,MetaInfo->GetLicenseURL());
  UpdateTextCtrl(wxID_PLANT_INFO_URL_CTRL,MetaInfo->GetPlantInfoURL());
 }

void               P3DOptGeneralPanel::UpdateTextCtrl
                                      (int                 id,
                                       const char         *value)
 {
  wxTextCtrl *InfoTextCtrl;

  InfoTextCtrl = (wxTextCtrl*)FindWindow(id);

  InfoTextCtrl->ChangeValue(InfoValueToText(value));
 }

void               P3DOptGeneralPanel::OnLicenseWizardClicked
                                      (wxCommandEvent     &event)
 {
  P3DLicenseDialog  LicenseDialog(NULL,wxID_ANY);

  if (LicenseDialog.ShowModal() == wxID_OK)
   {
    UpdateTextCtrl(wxID_LICENSE_NAME_CTRL,LicenseDialog.GetLicenseName().mb_str());
    UpdateTextCtrl(wxID_LICENSE_URL_CTRL,LicenseDialog.GetLicenseURL().mb_str());

    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetLicenseName,
             LicenseDialog.GetLicenseName(),
             InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetLicenseName())));

    P3DApp::GetApp()->ExecEditCmd
     (new SetModelInfoStrCmd
           (&P3DModelMetaInfo::SetLicenseURL,
             LicenseDialog.GetLicenseURL(),
             InfoValueToText(P3DApp::GetApp()->GetModel()->GetMetaInfo()->GetLicenseURL())));
   }
 }

