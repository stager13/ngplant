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

#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/filename.h>
#include <wx/dirdlg.h>
#include <wx/colordlg.h>

#include <p3duiappopt.h>
#include <p3dwx.h>

#define TUBE_CROSS_SECT_RESOLUTION_MIN (3)
#define TUBE_CROSS_SECT_RESOLUTION_MAX (16)

enum
 {
  ID_TEXPATHS_LISTBOX = wxID_HIGHEST + 1300,
  ID_TEXPATH_TEXTCTRL,
  ID_TEXPATH_REMOVE,
  ID_TEXPATH_MOVE_UP,
  ID_TEXPATH_MOVE_DOWN,
  ID_TEXPATH_APPEND,
  ID_TEXPATH_UPDATE,
  ID_TEXPATH_BROWSE,
  ID_EXPORT_HIDDEN_MODE,
  ID_EXPORT_OUTVIS_MODE,
  ID_GROUND_COLOR,
  ID_GROUND_VISIBLE,
  ID_BACKGROUND_COLOR,
  ID_PLUGINS_PATH,
  ID_PLUGINS_PATH_BROWSE,
  ID_CAMERA_ROT_Y_SPACE,
  ID_CAMERA_ROT_X_SENS,
  ID_CAMERA_ROT_Y_SENS,

  ID_CURVE_CTRL_WIDTH,
  ID_CURVE_CTRL_HEIGHT,
  ID_USE_COLOR_ARRAY,

  ID_CROSSSECT_RES_LEVEL0,
  ID_CROSSSECT_RES_LEVEL1,
  ID_CROSSSECT_RES_LEVEL2
 };

IMPLEMENT_CLASS(P3DAppOptDialog,wxDialog)

BEGIN_EVENT_TABLE(P3DAppOptDialog,wxDialog)
 EVT_UPDATE_UI(ID_TEXPATH_MOVE_UP,P3DAppOptDialog::OnTexPathMoveUpUpdate)
 EVT_UPDATE_UI(ID_TEXPATH_MOVE_DOWN,P3DAppOptDialog::OnTexPathMoveDownUpdate)
 EVT_UPDATE_UI(ID_TEXPATH_REMOVE,P3DAppOptDialog::OnTexPathRemoveUpdate)
 EVT_UPDATE_UI(ID_TEXPATH_APPEND,P3DAppOptDialog::OnTexPathAppendUpdate)
 EVT_UPDATE_UI(ID_TEXPATH_UPDATE,P3DAppOptDialog::OnTexPathUpdateUpdate)
 EVT_BUTTON(ID_TEXPATH_MOVE_UP,P3DAppOptDialog::OnTexPathMoveUpClick)
 EVT_BUTTON(ID_TEXPATH_MOVE_DOWN,P3DAppOptDialog::OnTexPathMoveDownClick)
 EVT_BUTTON(ID_TEXPATH_REMOVE,P3DAppOptDialog::OnTexPathRemoveClick)
 EVT_BUTTON(ID_TEXPATH_APPEND,P3DAppOptDialog::OnTexPathAppendClick)
 EVT_BUTTON(ID_TEXPATH_UPDATE,P3DAppOptDialog::OnTexPathUpdateClick)
 EVT_BUTTON(ID_TEXPATH_BROWSE,P3DAppOptDialog::OnTexPathBrowseClick)
 EVT_BUTTON(ID_GROUND_COLOR,P3DAppOptDialog::OnGroundColorClick)
 EVT_BUTTON(ID_BACKGROUND_COLOR,P3DAppOptDialog::OnBackgroundColorClick)
 EVT_BUTTON(ID_PLUGINS_PATH_BROWSE,P3DAppOptDialog::OnPluginsPathBrowseClick)
END_EVENT_TABLE()


                   P3DAppOptDialog::P3DAppOptDialog
                                      ()
 {
  Init();
 }

                   P3DAppOptDialog::P3DAppOptDialog
                                      (wxWindow           *Parent,
                                       wxWindowID          Id,
                                       const wxString     &Caption,
                                       const wxPoint      &Pos,
                                       const wxSize       &Size,
                                       long                Style)
 {
  Init();
  Create(Parent,Id,Caption,Pos,Size,Style);
 }

void               P3DAppOptDialog::Init
                                      ()
 {
  GroundColor.R = 0x80;
  GroundColor.G = 0x80;
  GroundColor.B = 0x80;

  GroundVisible = true;

  BackgroundColor.R = 0x00;
  BackgroundColor.G = 0x00;
  BackgroundColor.B = 0x00;

  Export3DPrefs.HiddenGroupsExportMode = P3D_ASK;
  Export3DPrefs.OutVisRangeExportMode  = P3D_ASK;

  CurveCtrlBestWidth  = 140;
  CurveCtrlBestHeight = 40;
 }

bool               P3DAppOptDialog::Create
                                      (wxWindow           *Parent,
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

  CreateControls();

  return(true);
 }

void               P3DAppOptDialog::CreateControls
                                      ()
 {
  wxBoxSizer  *TopSizer = new wxBoxSizer(wxVERTICAL);

  OptsNotebook = new wxNotebook(this,wxID_ANY);

  #if wxCHECK_VERSION(2,8,0)
  #else
  wxNotebookSizer *NotebookSizer = new wxNotebookSizer(OptsNotebook);
  #endif

  CreateTexPathsPage(OptsNotebook);
  CreateExportPage(OptsNotebook);
  CreateCameraControlPage(OptsNotebook);
  CreateModelPage(OptsNotebook);
  CreateMiscPage(OptsNotebook);

  OptsNotebook->Fit();

  TopSizer->Add(OptsNotebook,1,wxGROW | wxALL,5);

  wxBoxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  ButtonSizer->Add(new wxButton(this,wxID_OK,wxT("Ok")),0,wxALL,5);
  ButtonSizer->Add(new wxButton(this,wxID_CANCEL,wxT("Cancel")),0,wxALL,5);

  TopSizer->Add(ButtonSizer,0,wxALIGN_RIGHT | wxALL,5);

  SetSizer(TopSizer);
  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DAppOptDialog::CreateTexPathsPage
                                      (wxNotebook         *Notebook)
 {
  wxBoxSizer  *TopSizer = new wxBoxSizer(wxVERTICAL);
  wxPanel     *TexPathsPanel = new wxPanel(Notebook,wxID_ANY);

  wxListBox   *TexPathsListBox =
   new wxListBox(TexPathsPanel,ID_TEXPATHS_LISTBOX,wxDefaultPosition,wxDLG_UNIT(TexPathsPanel,wxSize(256,10 * 8)));

  TopSizer->Add(TexPathsListBox,1,wxGROW | wxALL,5);

  wxBoxSizer *ListBoxButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

  ListBoxButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_REMOVE,wxT("Remove")),
                           1,
                           wxALL,
                           1);

  ListBoxButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_MOVE_UP,wxT("Move up")),
                           1,
                           wxALL,
                           1);

  ListBoxButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_MOVE_DOWN,wxT("Move down")),
                           1,
                           wxALL,
                           1);

  TopSizer->Add(ListBoxButtonsSizer,0,wxGROW | wxALL,5);

  wxTextCtrl *TexPathTextCtrl = new wxTextCtrl(TexPathsPanel,ID_TEXPATH_TEXTCTRL);

  TopSizer->Add(TexPathTextCtrl,0,wxGROW | wxALL,5);

  wxBoxSizer *TexPathButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

  TexPathButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_APPEND,wxT("Append")),
                           1,
                           wxALL,
                           1);

  TexPathButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_UPDATE,wxT("Update")),
                           1,
                           wxALL,
                           1);

  TexPathButtonsSizer->Add(new wxButton(TexPathsPanel,ID_TEXPATH_BROWSE,wxT("Browse...")),
                           1,
                           wxALL,
                           1);

  TopSizer->Add(TexPathButtonsSizer,0,wxGROW | wxALL,5);

  TexPathsPanel->SetSizer(TopSizer);
  TopSizer->Fit(TexPathsPanel);
  TopSizer->SetSizeHints(TexPathsPanel);

  Notebook->AddPage(TexPathsPanel,wxT("Tex. locations"));

  TexPathsPanel->SetAutoLayout(true);
 }

void               P3DAppOptDialog::CreateExportPage
                                      (wxNotebook         *Notebook)
 {
  wxBoxSizer      *TopSizer  = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(2,2,2,2);
  wxPanel         *ExportPanel = new wxPanel(Notebook,wxID_ANY);

  GridSizer->AddGrowableCol(1);

  GridSizer->Add(new wxStaticText(ExportPanel,wxID_ANY,wxT("Export hidden branches")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxChoice *ExportHiddenModeCtrl = new wxChoice(ExportPanel,ID_EXPORT_HIDDEN_MODE,wxDefaultPosition,wxDefaultSize,(int)0,(const wxString*)NULL);
  ExportHiddenModeCtrl->Append(wxT("Always"));
  ExportHiddenModeCtrl->Append(wxT("Never"));
  ExportHiddenModeCtrl->Append(wxT("Ask"));

  GridSizer->Add(ExportHiddenModeCtrl,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(ExportPanel,wxID_ANY,wxT("Export branches outside vis. range")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxChoice *ExportOutVisModeCtrl = new wxChoice(ExportPanel,ID_EXPORT_OUTVIS_MODE,wxDefaultPosition,wxDefaultSize,(int)0,(const wxString*)NULL);
  ExportOutVisModeCtrl->Append(wxT("Always"));
  ExportOutVisModeCtrl->Append(wxT("Never"));
  ExportOutVisModeCtrl->Append(wxT("Ask"));

  GridSizer->Add(ExportOutVisModeCtrl,0,wxALL | wxALIGN_LEFT,1);

  TopSizer->Add(GridSizer,1,wxGROW | wxALL,5);

  ExportPanel->SetSizer(TopSizer);
  TopSizer->Fit(ExportPanel);
  TopSizer->SetSizeHints(ExportPanel);

  Notebook->AddPage(ExportPanel,wxT("Export"));

  ExportPanel->SetAutoLayout(true);
 }

void               P3DAppOptDialog::CreateModelPage
                                      (wxNotebook         *Notebook)
 {
  wxBoxSizer       *TopSizer          = new wxBoxSizer(wxVERTICAL);
  wxPanel          *ModelPanel        = new wxPanel(Notebook,wxID_ANY);
  wxStaticBoxSizer *TubeParamsSizer   = new wxStaticBoxSizer(wxVERTICAL,ModelPanel,wxT("Tube stem cross-section resolution"));
  wxFlexGridSizer  *CrossSectResSizer = new wxFlexGridSizer(3,2,2,2);
  wxSpinSliderCtrl *SpinSlider;

  CrossSectResSizer->AddGrowableCol(1);

  CrossSectResSizer->Add(new wxStaticText(ModelPanel,wxID_ANY,wxT("Level 0 (trunk)")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl
                    (ModelPanel,ID_CROSSSECT_RES_LEVEL0, wxSPINSLIDER_MODE_INTEGER,
                     ModelPrefs.TubeCrossSectResolution[0],TUBE_CROSS_SECT_RESOLUTION_MIN,TUBE_CROSS_SECT_RESOLUTION_MAX);
  SpinSlider->SetSensitivity(1,1,1,1,1);

  CrossSectResSizer->Add(SpinSlider,0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  CrossSectResSizer->Add(new wxStaticText(ModelPanel,wxID_ANY,wxT("Level 1")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl
                    (ModelPanel,ID_CROSSSECT_RES_LEVEL1, wxSPINSLIDER_MODE_INTEGER,
                     ModelPrefs.TubeCrossSectResolution[1],TUBE_CROSS_SECT_RESOLUTION_MIN,TUBE_CROSS_SECT_RESOLUTION_MAX);
  SpinSlider->SetSensitivity(1,1,1,1,1);

  CrossSectResSizer->Add(SpinSlider,0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  CrossSectResSizer->Add(new wxStaticText(ModelPanel,wxID_ANY,wxT("Level 2 and above")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  SpinSlider = new wxSpinSliderCtrl
                    (ModelPanel,ID_CROSSSECT_RES_LEVEL2, wxSPINSLIDER_MODE_INTEGER,
                     ModelPrefs.TubeCrossSectResolution[2],TUBE_CROSS_SECT_RESOLUTION_MIN,TUBE_CROSS_SECT_RESOLUTION_MAX);
  SpinSlider->SetSensitivity(1,1,1,1,1);

  CrossSectResSizer->Add(SpinSlider,0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  TubeParamsSizer->Add(CrossSectResSizer,1,wxGROW,0);
  TopSizer->Add(TubeParamsSizer,0,wxALL,5);

  ModelPanel->SetSizer(TopSizer);
  TopSizer->Fit(ModelPanel);
  TopSizer->SetSizeHints(ModelPanel);

  Notebook->AddPage(ModelPanel,wxT("Model"));

  ModelPanel->SetAutoLayout(true);
 }

void               P3DAppOptDialog::CreateCameraControlPage
                                      (wxNotebook         *Notebook)
 {
  wxBoxSizer      *TopSizer           = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *GridSizer          = new wxFlexGridSizer(3,2,2,2);
  wxPanel         *CameraControlPanel = new wxPanel(Notebook,wxID_ANY);

  GridSizer->AddGrowableCol(1);

  GridSizer->Add(new wxStaticText(CameraControlPanel,wxID_ANY,wxT("View rotation mode")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxChoice *RotationYAxisSpaceCtrl = new wxChoice(CameraControlPanel,ID_CAMERA_ROT_Y_SPACE,wxDefaultPosition,wxDefaultSize,(int)0,(const wxString*)NULL);
  RotationYAxisSpaceCtrl->Append(wxT("Trackball"));
  RotationYAxisSpaceCtrl->Append(wxT("Turntable"));

  GridSizer->Add(RotationYAxisSpaceCtrl,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(CameraControlPanel,wxID_ANY,wxT("Rotation mousensitivity (X)")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);
  wxTextCtrl *Sens = new wxTextCtrl(CameraControlPanel,ID_CAMERA_ROT_X_SENS);
  GridSizer->Add(Sens,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(CameraControlPanel,wxID_ANY,wxT("Rotation mousensitivity (Y)")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);
  Sens = new wxTextCtrl(CameraControlPanel,ID_CAMERA_ROT_Y_SENS);
  GridSizer->Add(Sens,0,wxALL | wxALIGN_LEFT,1);

  TopSizer->Add(GridSizer,1,wxGROW | wxALL,5);

  CameraControlPanel->SetSizer(TopSizer);
  TopSizer->Fit(CameraControlPanel);
  TopSizer->SetSizeHints(CameraControlPanel);

  Notebook->AddPage(CameraControlPanel,wxT("Control"));

  CameraControlPanel->SetAutoLayout(true);
 }

void               P3DAppOptDialog::CreateMiscPage
                                      (wxNotebook         *Notebook)
 {
  wxBoxSizer      *TopSizer  = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer *GridSizer = new wxFlexGridSizer(7,2,2,2);
  wxPanel         *MiscPanel = new wxPanel(Notebook,wxID_ANY);

  GridSizer->AddGrowableCol(1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Ground visible")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *GroundVisibleCheckBox = new wxCheckBox(MiscPanel,ID_GROUND_VISIBLE,wxT(""));
  GroundVisibleCheckBox->SetValue(GroundVisible);

  GridSizer->Add(GroundVisibleCheckBox,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Ground color")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxButton *ColorButton = new wxButton(MiscPanel,ID_GROUND_COLOR,wxEmptyString);

  GridSizer->Add(ColorButton,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Background color")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  ColorButton = new wxButton(MiscPanel,ID_BACKGROUND_COLOR,wxEmptyString);

  GridSizer->Add(ColorButton,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Plug-ins directory")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxBoxSizer *PluginsPathCtrlSizer = new wxBoxSizer(wxHORIZONTAL);

  wxTextCtrl *PluginsPathCtrl = new wxTextCtrl(MiscPanel,ID_PLUGINS_PATH);

  PluginsPathCtrlSizer->Add(PluginsPathCtrl,1,wxALIGN_CENTER_VERTICAL,0);
  PluginsPathCtrlSizer->Add(new wxButton(MiscPanel,ID_PLUGINS_PATH_BROWSE,wxT("Browse...")),0,wxALIGN_CENTER_VERTICAL,0);

  GridSizer->Add(PluginsPathCtrlSizer,0,wxALL | wxGROW,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Curve control width")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);
  wxTextCtrl *TextCtrl = new wxTextCtrl(MiscPanel,ID_CURVE_CTRL_WIDTH);
  GridSizer->Add(TextCtrl,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Curve control height")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);
  TextCtrl = new wxTextCtrl(MiscPanel,ID_CURVE_CTRL_HEIGHT);
  GridSizer->Add(TextCtrl,0,wxALL | wxALIGN_LEFT,1);

  GridSizer->Add(new wxStaticText(MiscPanel,wxID_ANY,wxT("Use color array")),0,wxALL | wxALIGN_CENTER_VERTICAL,1);

  wxCheckBox *UseColorArrayCheckBox = new wxCheckBox(MiscPanel,ID_USE_COLOR_ARRAY,wxT(""));
  UseColorArrayCheckBox->SetValue(RenderQuirksPrefs.UseColorArray);

  GridSizer->Add(UseColorArrayCheckBox,0,wxALL | wxALIGN_LEFT,1);

  TopSizer->Add(GridSizer,1,wxGROW | wxALL,5);

  MiscPanel->SetSizer(TopSizer);
  TopSizer->Fit(MiscPanel);
  TopSizer->SetSizeHints(MiscPanel);

  Notebook->AddPage(MiscPanel,wxT("Misc."));

  MiscPanel->SetAutoLayout(true);
 }

static int         ExportModeToSelection
                                      (unsigned int        Mode)
 {
  if (Mode == P3D_ALWAYS)
   {
    return(0);
   }
  else if (Mode == P3D_NEVER)
   {
    return(1);
   }
  else
   {
    return(2);
   }
 }

static
unsigned int       SelectionToExportMode
                                      (int                 Selection)
 {
  if (Selection == 0)
   {
    return(P3D_ALWAYS);
   }
  else if (Selection == 1)
   {
    return(P3D_NEVER);
   }
  else
   {
    return(P3D_ASK);
   }
 }

bool               P3DAppOptDialog::TransferDataToWindow
                                      ()
 {
  wxListBox   *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    for (unsigned int Index = 0; Index < TexPaths.size(); Index++)
     {
      TexPathsListBox->Append(wxString(TexPaths[Index].c_str(),wxConvUTF8));
     }
   }

  wxButton *ColorButton = (wxButton*)FindWindow(ID_GROUND_COLOR);

  if (ColorButton != 0)
   {
    ColorButton->SetBackgroundColour
     (wxColour(GroundColor.R,GroundColor.G,GroundColor.B));
   }

  ColorButton = (wxButton*)FindWindow(ID_BACKGROUND_COLOR);

  if (ColorButton != 0)
   {
    ColorButton->SetBackgroundColour
     (wxColour(BackgroundColor.R,BackgroundColor.G,BackgroundColor.B));
   }

  wxCheckBox *GroundVisibleCheckBox = (wxCheckBox*)FindWindow(ID_GROUND_VISIBLE);

  if (GroundVisibleCheckBox != 0)
   {
    GroundVisibleCheckBox->SetValue(GroundVisible);
   }

  wxChoice   *ExportHiddenModeCtrl = (wxChoice*)FindWindow(ID_EXPORT_HIDDEN_MODE);

  if (ExportHiddenModeCtrl != 0)
   {
    ExportHiddenModeCtrl->SetSelection
     (ExportModeToSelection(Export3DPrefs.HiddenGroupsExportMode));
   }

  wxChoice   *ExportOutVisModeCtrl = (wxChoice*)FindWindow(ID_EXPORT_OUTVIS_MODE);

  if (ExportOutVisModeCtrl != 0)
   {
    ExportOutVisModeCtrl->SetSelection
     (ExportModeToSelection(Export3DPrefs.OutVisRangeExportMode));
   }

  wxChoice   *RotationYAxisSpaceCtrl = (wxChoice*)FindWindow(ID_CAMERA_ROT_Y_SPACE);

  if (RotationYAxisSpaceCtrl != 0)
   {
    if (CameraControlPrefs.MouseRotYCS)
     {
      RotationYAxisSpaceCtrl->SetSelection(0);
     }
    else
     {
      RotationYAxisSpaceCtrl->SetSelection(1);
     }
   }

  wxTextCtrl *Sens = (wxTextCtrl*)FindWindow(ID_CAMERA_ROT_X_SENS);
  wxString    SensStr;

  if (Sens != 0)
   {
    SensStr.Printf(wxT("%.04f"),CameraControlPrefs.MouseRotXSens);
    Sens->SetValue(SensStr);
   }

  Sens = (wxTextCtrl*)FindWindow(ID_CAMERA_ROT_Y_SENS);

  if (Sens != 0)
   {
    SensStr.Printf(wxT("%.04f"),CameraControlPrefs.MouseRotYSens);
    Sens->SetValue(SensStr);
   }

  wxTextCtrl *PluginsPathCtrl = (wxTextCtrl*)FindWindow(ID_PLUGINS_PATH);

  if (PluginsPathCtrl != 0)
   {
    PluginsPathCtrl->SetValue(PluginsPath);
   }

  wxString    Str;
  wxTextCtrl *TextCtrl = (wxTextCtrl*)FindWindow(ID_CURVE_CTRL_WIDTH);

  if (TextCtrl != 0)
   {
    Str.Printf(wxT("%u"),CurveCtrlBestWidth);
    TextCtrl->SetValue(Str);
   }

  TextCtrl = (wxTextCtrl*)FindWindow(ID_CURVE_CTRL_HEIGHT);

  if (TextCtrl != 0)
   {
    Str.Printf(wxT("%u"),CurveCtrlBestHeight);
    TextCtrl->SetValue(Str);
   }

  wxCheckBox *UseColorArrayCheckBox = (wxCheckBox*)FindWindow(ID_USE_COLOR_ARRAY);

  if (UseColorArrayCheckBox != 0)
   {
    UseColorArrayCheckBox->SetValue(RenderQuirksPrefs.UseColorArray);
   }

  wxSpinSliderCtrl* SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL0);

  if (SpinSlider != 0)
   {
    SpinSlider->SetValue(ModelPrefs.TubeCrossSectResolution[0]);
   }

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL1);

  if (SpinSlider != 0)
   {
    SpinSlider->SetValue(ModelPrefs.TubeCrossSectResolution[1]);
   }

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL2);

  if (SpinSlider != 0)
   {
    SpinSlider->SetValue(ModelPrefs.TubeCrossSectResolution[2]);
   }

  return(true);
 }

bool               P3DAppOptDialog::TransferDataFromWindow
                                      ()
 {
  TexPaths.clear();

  wxListBox   *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    int                                Count;

    Count = TexPathsListBox->GetCount();

    for (int Index = 0; Index < Count; Index++)
     {
      TexPaths.push_back((const char*)(TexPathsListBox->GetString(Index).mb_str(wxConvUTF8)));
     }
   }

  wxButton *ColorButton = (wxButton*)FindWindow(ID_GROUND_COLOR);

  if (ColorButton != 0)
   {
    wxColour Color;

    Color = ColorButton->GetBackgroundColour();

    GroundColor.R = Color.Red();
    GroundColor.G = Color.Green();
    GroundColor.B = Color.Blue();
   }

  ColorButton = (wxButton*)FindWindow(ID_BACKGROUND_COLOR);

  if (ColorButton != 0)
   {
    wxColour Color;

    Color = ColorButton->GetBackgroundColour();

    BackgroundColor.R = Color.Red();
    BackgroundColor.G = Color.Green();
    BackgroundColor.B = Color.Blue();
   }

  wxCheckBox *GroundVisibleCheckBox = (wxCheckBox*)FindWindow(ID_GROUND_VISIBLE);

  if (GroundVisibleCheckBox != 0)
   {
    GroundVisible = GroundVisibleCheckBox->GetValue();
   }

  wxChoice   *ExportHiddenModeCtrl = (wxChoice*)FindWindow(ID_EXPORT_HIDDEN_MODE);

  if (ExportHiddenModeCtrl != 0)
   {
    Export3DPrefs.HiddenGroupsExportMode =
     SelectionToExportMode(ExportHiddenModeCtrl->GetSelection());
   }

  wxChoice   *ExportOutVisModeCtrl = (wxChoice*)FindWindow(ID_EXPORT_OUTVIS_MODE);

  if (ExportOutVisModeCtrl != 0)
   {
    Export3DPrefs.OutVisRangeExportMode =
     SelectionToExportMode(ExportOutVisModeCtrl->GetSelection());
   }

  wxChoice   *RotationYAxisSpaceCtrl = (wxChoice*)FindWindow(ID_CAMERA_ROT_Y_SPACE);

  if (RotationYAxisSpaceCtrl != 0)
   {
    if (RotationYAxisSpaceCtrl->GetSelection() == 0)
     {
      CameraControlPrefs.MouseRotYCS = true;
     }
    else
     {
      CameraControlPrefs.MouseRotYCS = false;
     }
   }

  wxTextCtrl *Sens = (wxTextCtrl*)FindWindow(ID_CAMERA_ROT_X_SENS);
  double      SensValue;

  if (Sens != 0)
   {
    if (Sens->GetValue().ToDouble(&SensValue))
     {
      CameraControlPrefs.MouseRotXSens = SensValue;
     }
    else
     {
      OptsNotebook->SetSelection(2);
      Sens->SetFocus();

      wxMessageBox(wxT("Invalid mouse sensitivity value"),wxT("Error"),wxOK | wxICON_ERROR);

      return false;
     }
   }

  Sens = (wxTextCtrl*)FindWindow(ID_CAMERA_ROT_Y_SENS);

  if (Sens != 0)
   {
    if (Sens->GetValue().ToDouble(&SensValue))
     {
      CameraControlPrefs.MouseRotYSens = SensValue;
     }
    else
     {
      OptsNotebook->SetSelection(2);
      Sens->SetFocus();

      wxMessageBox(wxT("Invalid mouse sensitivity value"),wxT("Error"),wxOK | wxICON_ERROR);

      return false;
     }
   }

  wxTextCtrl *PluginsPathCtrl = (wxTextCtrl*)FindWindow(ID_PLUGINS_PATH);

  if (PluginsPathCtrl != 0)
   {
    PluginsPath = PluginsPathCtrl->GetValue();
   }

  wxTextCtrl *TextCtrl;
  long        TempLong;
  bool        Ok;

  TextCtrl = (wxTextCtrl*)FindWindow(ID_CURVE_CTRL_WIDTH);

  if (TextCtrl != 0)
   {
    Ok = false;

    if (TextCtrl->GetValue().ToLong(&TempLong))
     {
      if (TempLong >= 140)
       {
        Ok = true;
       }
     }

    if (Ok)
     {
      CurveCtrlBestWidth = (unsigned int)TempLong;
     }
    else
     {
      TextCtrl->SetFocus();

      wxMessageBox(wxT("Invalid curve control width"),wxT("Error"),wxOK | wxICON_ERROR);

      return false;
     }
   }

  TextCtrl = (wxTextCtrl*)FindWindow(ID_CURVE_CTRL_HEIGHT);

  if (TextCtrl != 0)
   {
    Ok = false;

    if (TextCtrl->GetValue().ToLong(&TempLong))
     {
      if (TempLong >= 40)
       {
        Ok = true;
       }
     }

    if (Ok)
     {
      CurveCtrlBestHeight = (unsigned int)TempLong;
     }
    else
     {
      TextCtrl->SetFocus();

      wxMessageBox(wxT("Invalid curve control height"),wxT("Error"),wxOK | wxICON_ERROR);

      return false;
     }
   }

  wxCheckBox *UseColorArrayCheckBox = (wxCheckBox*)FindWindow(ID_USE_COLOR_ARRAY);

  if (UseColorArrayCheckBox != 0)
   {
    RenderQuirksPrefs.UseColorArray = UseColorArrayCheckBox->GetValue();
   }

  wxSpinSliderCtrl* SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL0);

  if (SpinSlider != 0)
   {
    ModelPrefs.TubeCrossSectResolution[0] = (unsigned int)SpinSlider->GetValue();
   }

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL1);

  if (SpinSlider != 0)
   {
    ModelPrefs.TubeCrossSectResolution[1] = (unsigned int)SpinSlider->GetValue();
   }

  SpinSlider = (wxSpinSliderCtrl*)FindWindow(ID_CROSSSECT_RES_LEVEL2);

  if (SpinSlider != 0)
   {
    ModelPrefs.TubeCrossSectResolution[2] = (unsigned int)SpinSlider->GetValue();
   }

  return(true);
 }

void               P3DAppOptDialog::OnTexPathMoveUpUpdate
                                      (wxUpdateUIEvent    &Event)
 {
  wxListBox   *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    if (TexPathsListBox->GetSelection() > 0)
     {
      Event.Enable(true);
     }
    else
     {
      Event.Enable(false);
     }
   }
  else
   {
    Event.Enable(false);
   }
 }

void               P3DAppOptDialog::OnTexPathMoveUpClick
                                      (wxCommandEvent     &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    int Selection = TexPathsListBox->GetSelection();

    if (Selection > 0)
     {
      wxString Value = TexPathsListBox->GetString(Selection);

      TexPathsListBox->Delete(Selection);
      TexPathsListBox->InsertItems(1,&Value,Selection - 1);

      TexPathsListBox->SetSelection(Selection - 1);
     }
   }
 }

void               P3DAppOptDialog::OnTexPathMoveDownUpdate
                                      (wxUpdateUIEvent    &Event)
 {
  wxListBox   *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    int Selection = TexPathsListBox->GetSelection();

    if ((Selection >= 0) && (Selection < (TexPathsListBox->GetCount() - 1)))
     {
      Event.Enable(true);
     }
    else
     {
      Event.Enable(false);
     }
   }
  else
   {
    Event.Enable(false);
   }
 }

void               P3DAppOptDialog::OnTexPathMoveDownClick
                                      (wxCommandEvent     &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    int Selection = TexPathsListBox->GetSelection();
    int Count     = TexPathsListBox->GetCount();

    if ((Selection >= 0) && (Selection < (Count - 1)))
     {
      wxString Value = TexPathsListBox->GetString(Selection);

      TexPathsListBox->Delete(Selection);

      if (Selection < (Count - 2))
       {
        TexPathsListBox->InsertItems(1,&Value,Selection + 1);
       }
      else
       {
        TexPathsListBox->Append(Value);
       }

      TexPathsListBox->SetSelection(Selection + 1);
     }
   }
 }

void               P3DAppOptDialog::OnTexPathRemoveUpdate
                                      (wxUpdateUIEvent    &Event)
 {
  wxListBox   *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    if (TexPathsListBox->GetSelection() >= 0)
     {
      Event.Enable(true);
     }
    else
     {
      Event.Enable(false);
     }
   }
  else
   {
    Event.Enable(false);
   }
 }

void               P3DAppOptDialog::OnTexPathRemoveClick
                                      (wxCommandEvent     &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);

  if (TexPathsListBox != 0)
   {
    if (TexPathsListBox->GetSelection() >= 0)
     {
      TexPathsListBox->Delete(TexPathsListBox->GetSelection());
     }
   }
 }

void               P3DAppOptDialog::OnTexPathAppendUpdate
                                      (wxUpdateUIEvent    &Event)
 {
  wxTextCtrl *TexPathTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXPATH_TEXTCTRL);

  if (TexPathTextCtrl != 0)
   {
    wxString Value = TexPathTextCtrl->GetValue();

    Value.Trim(true);
    Value.Trim(false);

    if (Value.IsEmpty())
     {
      Event.Enable(false);
     }
    else
     {
      Event.Enable(true);
     }
   }
  else
   {
    Event.Enable(false);
   }
 }

void               P3DAppOptDialog::OnTexPathAppendClick
                                      (wxCommandEvent     &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);
  wxTextCtrl *TexPathTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXPATH_TEXTCTRL);

  if ((TexPathTextCtrl != 0) && (TexPathsListBox != 0))
   {
    wxString Value = TexPathTextCtrl->GetValue();

    Value.Trim(true);
    Value.Trim(false);

    if (!Value.IsEmpty())
     {
      TexPathsListBox->Append(Value);
      TexPathsListBox->SetSelection(TexPathsListBox->GetCount() - 1);
      TexPathTextCtrl->SetValue(wxEmptyString);
     }
   }
 }

void               P3DAppOptDialog::OnTexPathUpdateUpdate
                                      (wxUpdateUIEvent    &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);
  wxTextCtrl *TexPathTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXPATH_TEXTCTRL);

  if ((TexPathTextCtrl != 0) && (TexPathsListBox != 0))
   {
    if (TexPathsListBox->GetSelection() >= 0)
     {
      wxString Value = TexPathTextCtrl->GetValue();

      Value.Trim(true);
      Value.Trim(false);

      if (Value.IsEmpty())
       {
        Event.Enable(false);
       }
      else
       {
        Event.Enable(true);
       }
     }
    else
     {
      Event.Enable(false);
     }
   }
  else
   {
    Event.Enable(false);
   }
 }

void               P3DAppOptDialog::OnTexPathUpdateClick
                                      (wxCommandEvent     &Event)
 {
  wxListBox  *TexPathsListBox = (wxListBox*)FindWindow(ID_TEXPATHS_LISTBOX);
  wxTextCtrl *TexPathTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXPATH_TEXTCTRL);

  if ((TexPathTextCtrl != 0) && (TexPathsListBox != 0))
   {
    if (TexPathsListBox->GetSelection() >= 0)
     {
      wxString Value = TexPathTextCtrl->GetValue();

      Value.Trim(true);
      Value.Trim(false);

      if (!Value.IsEmpty())
       {
        TexPathsListBox->SetString(TexPathsListBox->GetSelection(),Value);
        TexPathTextCtrl->SetValue(wxEmptyString);
       }
     }
   }
 }

void               P3DAppOptDialog::OnTexPathBrowseClick
                                      (wxCommandEvent     &Event)
 {
  wxDirDialog     Dialog(this,wxT("Select texture location"),wxFileName::GetHomeDir());

  if (Dialog.ShowModal() == wxID_OK)
   {
    wxTextCtrl *TexPathTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXPATH_TEXTCTRL);

    if (TexPathTextCtrl != 0)
     {
      TexPathTextCtrl->SetValue(Dialog.GetPath());
     }
   }
 }

void               P3DAppOptDialog::OnGroundColorClick
                                      (wxCommandEvent     &Event)
 {
  wxColourData                         ColorData;

  wxButton *ColorButton = (wxButton*)FindWindow(ID_GROUND_COLOR);

  if (ColorButton != 0)
   {
    ColorData.SetColour(ColorButton->GetBackgroundColour());
   }

  wxColourDialog                       Dialog(this,&ColorData);

  if (Dialog.ShowModal() == wxID_OK)
   {
    if (ColorButton != 0)
     {
      wxColourData UserColorData = Dialog.GetColourData();

      ColorButton->SetBackgroundColour(UserColorData.GetColour());
     }
   }
 }

void               P3DAppOptDialog::OnBackgroundColorClick
                                      (wxCommandEvent     &Event)
 {
  wxColourData                         ColorData;

  wxButton *ColorButton = (wxButton*)FindWindow(ID_BACKGROUND_COLOR);

  if (ColorButton != 0)
   {
    ColorData.SetColour(ColorButton->GetBackgroundColour());
   }

  wxColourDialog                       Dialog(this,&ColorData);

  if (Dialog.ShowModal() == wxID_OK)
   {
    if (ColorButton != 0)
     {
      wxColourData UserColorData = Dialog.GetColourData();

      ColorButton->SetBackgroundColour(UserColorData.GetColour());
     }
   }
 }

void               P3DAppOptDialog::OnPluginsPathBrowseClick
                                      (wxCommandEvent     &Event)
 {
  wxDirDialog     Dialog(this,wxT("Select plug-ins location"),wxFileName::GetHomeDir());

  if (Dialog.ShowModal() == wxID_OK)
   {
    wxTextCtrl *PluginsPathCtrl = (wxTextCtrl*)FindWindow(ID_PLUGINS_PATH);

    if (PluginsPathCtrl != 0)
     {
      PluginsPathCtrl->SetValue(Dialog.GetPath());
     }
   }
 }

unsigned int       P3DAppOptDialog::GetTexPathsCount
                                      () const
 {
  return(TexPaths.size());
 }

const char        *P3DAppOptDialog::GetTexPath
                                      (unsigned int        Index) const
 {
  return(TexPaths[Index].c_str());
 }

void               P3DAppOptDialog::AddTexPath
                                      (const char         *TexPath)
 {
  TexPaths.push_back(TexPath);
 }

void               P3DAppOptDialog::GetGroundColor
                                      (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const
 {
  *R = GroundColor.R;
  *G = GroundColor.G;
  *B = GroundColor.B;
 }

void               P3DAppOptDialog::SetGroundColor
                                      (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B)
 {
  GroundColor.R = R;
  GroundColor.G = G;
  GroundColor.B = B;
 }

bool               P3DAppOptDialog::GetGroundVisible
                                      () const
 {
  return(GroundVisible);
 }

void               P3DAppOptDialog::SetGroundVisible
                                      (bool                Visible)
 {
  GroundVisible = Visible;
 }

void               P3DAppOptDialog::GetBackgroundColor
                                      (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const
 {
  *R = BackgroundColor.R;
  *G = BackgroundColor.G;
  *B = BackgroundColor.B;
 }

void               P3DAppOptDialog::SetBackgroundColor
                                      (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B)
 {
  BackgroundColor.R = R;
  BackgroundColor.G = G;
  BackgroundColor.B = B;
 }


void               P3DAppOptDialog::SetExport3DPrefs
                                      (const P3DExport3DPrefs
                                                          *Prefs)
 {
  Export3DPrefs = *Prefs;
 }

void               P3DAppOptDialog::GetExport3DPrefs
                                      (P3DExport3DPrefs   *Prefs)
 {
  *Prefs = Export3DPrefs;
 }

const P3DCameraControlPrefs
                  &P3DAppOptDialog::GetCameraControlPrefs
                                      () const
 {
  return(CameraControlPrefs);
 }

void               P3DAppOptDialog::SetCameraControlPrefs
                                      (const P3DCameraControlPrefs
                                                          &Prefs)
 {
  CameraControlPrefs = Prefs;
 }

const P3DRenderQuirksPrefs
                  &P3DAppOptDialog::GetRenderQuirksPrefs
                                      () const
 {
  return(RenderQuirksPrefs);
 }

void               P3DAppOptDialog::SetRenderQuirksPrefs
                                      (const P3DRenderQuirksPrefs
                                                          &Prefs)
 {
  RenderQuirksPrefs = Prefs;
 }

void               P3DAppOptDialog::SetPluginsPath
                                      (const wxString     &PluginsPath)
 {
  this->PluginsPath = PluginsPath;
 }

const wxString    &P3DAppOptDialog::GetPluginsPath
                                      () const
 {
  return(PluginsPath);
 }

void               P3DAppOptDialog::SetCurveCtrlPrefs
                                      (unsigned int        BestWidth,
                                       unsigned int        BestHeight)
 {
  CurveCtrlBestWidth  = BestWidth;
  CurveCtrlBestHeight = BestHeight;
 }

void               P3DAppOptDialog::GetCurveCtrlPrefs
                                      (unsigned int       *BestWidth,
                                       unsigned int       *BestHeight) const
 {
  *BestWidth  = CurveCtrlBestWidth;
  *BestHeight = CurveCtrlBestHeight;
 }

const P3DModelPrefs
                  &P3DAppOptDialog::GetModelPrefs
                                      () const
 {
  return ModelPrefs;
 }

void               P3DAppOptDialog::SetModelPrefs
                                      (const P3DModelPrefs&Prefs)
 {
  ModelPrefs = Prefs;
 }

