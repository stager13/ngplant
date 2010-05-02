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

#include <sys/stat.h>

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/cmdline.h>
#include <wx/dir.h>

#if !defined(__WXMSW__) && !defined(__WXPM__)
 #include "images/ngplant.xpm"
#endif

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemquad.h>
#include <ngpcore/p3dbalgbase.h>
#include <ngpcore/p3dbalgstd.h>
#include <ngpcore/p3diostream.h>

#include <ngput/p3dospath.h>

#include <images/p3dnotexxpm.h>
#include <images/p3dclosexpm.h>

#include <p3dmedit.h>
#include <p3dexpobj.h>
#include <p3dmaterialstd.h>
#include <p3dcanvas3d.h>
#include <p3duiappopt.h>
#include <p3dwxcurvectrl.h>
#include <p3dpluglua.h>

#include <p3dlog.h>

#include <p3dapp.h>

enum
 {
  wxID_EXPORT      = wxID_HIGHEST + 1200,
  wxID_EXPORT_OBJ,
  wxID_RUN_SCRIPT,
  wxID_EDIT_PREFERENCES,

  wxID_EXPORT_PLUGIN_FIRST = wxID_EDIT_PREFERENCES + 10000,
  wxID_EXPORT_PLUGIN_LAST  = wxID_EXPORT_PLUGIN_FIRST + 250
 };

BEGIN_EVENT_TABLE(P3DMainFrame,wxFrame)
 EVT_MENU(wxID_NEW,P3DMainFrame::OnNew)
 EVT_MENU(wxID_OPEN,P3DMainFrame::OnOpen)
 EVT_MENU(wxID_SAVE,P3DMainFrame::OnSave)
 EVT_MENU(wxID_SAVEAS,P3DMainFrame::OnSaveAs)
 EVT_MENU(wxID_EXPORT_OBJ,P3DMainFrame::OnExportObj)
 EVT_MENU_RANGE(wxID_EXPORT_PLUGIN_FIRST,wxID_EXPORT_PLUGIN_LAST,P3DMainFrame::OnExportObjPlugin)
 EVT_MENU(wxID_RUN_SCRIPT,P3DMainFrame::OnRunScript)
 EVT_MENU(wxID_EXIT,P3DMainFrame::OnQuit)
 EVT_MENU(wxID_ABOUT,P3DMainFrame::OnAbout)
 EVT_MENU(wxID_EDIT_PREFERENCES,P3DMainFrame::OnEditPreferences)
 EVT_MENU(wxID_UNDO,P3DMainFrame::OnUndo)
 EVT_MENU(wxID_REDO,P3DMainFrame::OnRedo)
END_EVENT_TABLE()

class P3DUndoRedoMenuStateUpdater
 {
  public           :

                   P3DUndoRedoMenuStateUpdater
                                      (wxMenuBar          *MenuBar,
                                       const P3DEditCommandQueue
                                                          &CommandQueue)
                   : Queue(CommandQueue)
   {
    this->MenuBar = MenuBar;

    UndoEmpty = Queue.UndoQueueEmpty();
    RedoEmpty = Queue.RedoQueueEmpty();
   }

                  ~P3DUndoRedoMenuStateUpdater
                                      ()
   {
    wxMenuItem                        *Item;

    if (UndoEmpty != Queue.UndoQueueEmpty())
     {
      Item = MenuBar->FindItem(wxID_UNDO);

      if (Item != NULL)
       {
        Item->Enable(UndoEmpty);
       }
     }

    if (RedoEmpty != Queue.RedoQueueEmpty())
     {
      Item = MenuBar->FindItem(wxID_REDO);

      if (Item != NULL)
       {
        Item->Enable(RedoEmpty);
       }
     }
   }

  private          :

  const P3DEditCommandQueue           &Queue;
  wxMenuBar                           *MenuBar;
  bool                                 UndoEmpty;
  bool                                 RedoEmpty;
 };

                   P3DMainFrame::P3DMainFrame  (const wxChar   *title)
                    : wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxSize(640,400))
 {
  wxMenu          *FileMenu = new wxMenu();
  wxMenu          *EditMenu = new wxMenu();
  wxMenu          *HelpMenu = new wxMenu();
  wxMenu          *ExportMenu = new wxMenu();
  const P3DPluginInfoVector
                  &ExportPlugins = wxGetApp().GetExportPlugins();
  int              MenuItemId = wxID_EXPORT_PLUGIN_FIRST;

  SetIcon(wxICON(ngplant));

  ExportMenu->Append(wxID_EXPORT_OBJ,wxT("Alias Wavefront .OBJ"));

  for (unsigned int Index = 0; Index < ExportPlugins.size(); Index++)
   {
    ExportMenu->Append(MenuItemId++,wxString(ExportPlugins[Index].GetMenuName(),wxConvUTF8));
   }

  ExportMenu->Append(wxID_RUN_SCRIPT,wxT("Run export script..."));

  FileMenu->Append(wxID_NEW,wxT("&New\tCtrl-N"));
  FileMenu->Append(wxID_OPEN,wxT("&Open...\tCtrl-O"));
  FileMenu->Append(wxID_SAVE,wxT("&Save\tCtrl-S"));
  FileMenu->Append(wxID_SAVEAS,wxT("Save as..."));
  FileMenu->Append(wxID_EXPORT,wxT("Export to"),ExportMenu);
  FileMenu->Append(wxID_EDIT_PREFERENCES,wxT("Preferences..."));
  FileMenu->Append(wxID_EXIT,wxT("E&xit\tAlt-X"));

  wxMenuItem *Item;

  Item = EditMenu->Append(wxID_UNDO,wxT("Undo\tCtrl-Z"));
  Item->Enable(false);

  Item = EditMenu->Append(wxID_REDO,wxT("&Redo\tCtrl-R"));
  Item->Enable(false);

  HelpMenu->Append(wxID_ABOUT,wxT("About..."));

  wxMenuBar       *MenuBar = new wxMenuBar();
  MenuBar->Append(FileMenu,wxT("&File"));
  MenuBar->Append(EditMenu,wxT("&Edit"));
  MenuBar->Append(HelpMenu,wxT("&Help"));

  Canvas3D = new P3DCanvas3D(this);

  EditPanel = new P3DModelEditPanel(this);

  wxBoxSizer *main_sizer = new wxBoxSizer(wxHORIZONTAL);

  main_sizer->Add(Canvas3D,1,wxEXPAND | wxALL,2);
  main_sizer->Add(EditPanel,0,wxEXPAND | wxALL,2);

  SetMenuBar(MenuBar);

  SetSizer(main_sizer);

  main_sizer->Fit(this);
  main_sizer->SetSizeHints(this);
 }

void               P3DMainFrame::OnQuit   (wxCommandEvent     &event)
 {
  Close();
 }

void               P3DMainFrame::OnAbout  (wxCommandEvent     &event)
 {
  ::wxMessageBox(wxT("ngPlant 0.9.7\n\nCopyright (c) 2006-2008 Sergey Prokhorchuk\nProject page: ngplant.sourceforge.net\n\nReleased under the GNU General Public License"),
                 wxT("About - ngPlant"),
                 wxOK,
                 this);
 }

void               P3DMainFrame::OnSave   (wxCommandEvent     &event)
 {
  if (wxGetApp().GetFileName().empty())
   {
    OnSaveAs(event);
   }
  else
   {
    Save(wxGetApp().GetFileName().mb_str());
   }
 }

void               P3DMainFrame::OnSaveAs (wxCommandEvent     &event)
 {
  wxString                                 FileName;

  FileName = ::wxFileSelector(wxT("File name"),wxT(""),wxT(""),wxT(".ngp"),wxT("*.ngp"),wxSAVE | wxOVERWRITE_PROMPT);

  if (!FileName.empty())
   {
    Save(FileName.mb_str());
   }
 }

void               P3DMainFrame::OnEditPreferences
                                          (wxCommandEvent     &event)
 {
  P3DAppOptDialog  PreferencesDialog(NULL,wxID_ANY);

  wxConfigBase                        *Cfg;

  Cfg = wxConfigBase::Get();

  if (Cfg->HasGroup(wxT("/TexLocations")))
   {
    Cfg->SetPath(wxT("/TexLocations"));

    bool     Cont;
    wxString EntryName;
    wxString PathStr;
    long     Cookie;

    Cont = Cfg->GetFirstEntry(EntryName,Cookie);

    while (Cont)
     {
      if (Cfg->Read(EntryName,&PathStr))
       {
        PreferencesDialog.AddTexPath(PathStr.mb_str(wxConvUTF8));
       }

      Cont = Cfg->GetNextEntry(EntryName,Cookie);
     }

    Cfg->SetPath(wxT("/"));
   }

  unsigned char R,G,B;

  wxGetApp().GetGroundColor(&R,&G,&B);

  PreferencesDialog.SetGroundColor(R,G,B);
  PreferencesDialog.SetGroundVisible(wxGetApp().IsGroundVisible());

  wxGetApp().GetBackgroundColor(&R,&G,&B);
  PreferencesDialog.SetBackgroundColor(R,G,B);

  PreferencesDialog.SetExport3DPrefs(wxGetApp().GetExport3DPrefs());
  PreferencesDialog.SetCameraControlPrefs(*wxGetApp().GetCameraControlPrefs());
  PreferencesDialog.SetRenderQuirksPrefs(wxGetApp().GetRenderQuirksPrefs());

  PreferencesDialog.SetPluginsPath(wxGetApp().GetPluginsPath());

  PreferencesDialog.SetCurveCtrlPrefs
   (P3DCurveCtrl::BestWidth,P3DCurveCtrl::BestHeight);

  if (PreferencesDialog.ShowModal() == wxID_OK)
   {
    #if wxCHECK_VERSION(2,8,0)
     {
      Cfg->DeleteGroup(wxT("/TexLocations"));
     }
    #else /* It look's like a bug in wxWidgets 2.4.X (wxConfigFile) */
     {
      if (Cfg->HasGroup(wxT("/TexLocations")))
       {
        Cfg->SetPath(wxT("/TexLocations"));

        bool     Cont;
        wxString EntryName;
        wxString PrevName;
        long     Cookie;

        Cont = Cfg->GetFirstEntry(EntryName,Cookie);

        while (Cont)
         {
          PrevName = EntryName;

          Cont = Cfg->GetNextEntry(EntryName,Cookie);

          Cfg->DeleteEntry(PrevName,FALSE);
         }

        Cfg->SetPath(wxT("/"));
       }
     }
    #endif

    Cfg->SetPath(wxT("/TexLocations"));

    for (unsigned int Index = 0;
         Index < PreferencesDialog.GetTexPathsCount();
         Index++)
     {
      wxString EntryName = wxString::Format(wxT("Location%u"),Index);

      Cfg->Write(EntryName,wxString(PreferencesDialog.GetTexPath(Index),wxConvUTF8));
     }

    Cfg->SetPath(wxT("/"));

    P3D3DViewPrefs View3DPrefs;

    PreferencesDialog.GetGroundColor(&View3DPrefs.GroundColor.R,
                                     &View3DPrefs.GroundColor.G,
                                     &View3DPrefs.GroundColor.B);
    wxGetApp().SetGroundColor(View3DPrefs.GroundColor.R,
                              View3DPrefs.GroundColor.G,
                              View3DPrefs.GroundColor.B);

    View3DPrefs.GroundVisible = PreferencesDialog.GetGroundVisible();

    wxGetApp().SetGroundVisibility(View3DPrefs.GroundVisible);

    PreferencesDialog.GetBackgroundColor(&View3DPrefs.BackgroundColor.R,
                                         &View3DPrefs.BackgroundColor.G,
                                         &View3DPrefs.BackgroundColor.B);
    wxGetApp().SetBackgroundColor(View3DPrefs.BackgroundColor.R,
                                  View3DPrefs.BackgroundColor.G,
                                  View3DPrefs.BackgroundColor.B);

    View3DPrefs.Save(Cfg);

    wxGetApp().SetCameraControlPrefs(&PreferencesDialog.GetCameraControlPrefs());

    wxGetApp().GetCameraControlPrefs()->Save(Cfg);

    PreferencesDialog.GetExport3DPrefs(wxGetApp().GetExport3DPrefs());
    wxGetApp().GetExport3DPrefs()->Save(Cfg);

    wxGetApp().SetPluginsPath(PreferencesDialog.GetPluginsPath());

    Cfg->Write(wxT("/Paths/Plugins"),wxGetApp().GetPluginsPath());

    unsigned int   NewBestWidth;
    unsigned int   NewBestHeight;

    PreferencesDialog.GetCurveCtrlPrefs(&NewBestWidth,&NewBestHeight);

    if ((NewBestWidth  != P3DCurveCtrl::BestWidth) ||
        (NewBestHeight != P3DCurveCtrl::BestHeight))
     {
      P3DCurveCtrl::BestWidth  = NewBestWidth;
      P3DCurveCtrl::BestHeight = NewBestHeight;

      EditPanel->UpdatePanelMinSize();
     }

    P3DUIControlsPrefs::Save(Cfg);

    wxGetApp().SetRenderQuirksPrefs(PreferencesDialog.GetRenderQuirksPrefs());
    wxGetApp().GetRenderQuirksPrefs().Save(Cfg);

    InvalidatePlant();
   }
 }

void               P3DMainFrame::OnExportObj
                                          (wxCommandEvent     &event)
 {
  wxString                                 FileName;

  FileName = ::wxFileSelector(wxT("File name"),wxT(""),wxT(""),wxT(".obj"),wxT("*.obj"),wxSAVE | wxOVERWRITE_PROMPT);

  if (!FileName.empty())
   {
    wxFileName                              MTLFileName(FileName);

    MTLFileName.SetExt(wxT("mtl"));

    bool ExportHiddenGroups;
    bool ExportOutVisRangeGroups;

    if      (wxGetApp().GetExport3DPrefs()->HiddenGroupsExportMode == P3D_ALWAYS)
     {
      ExportHiddenGroups = true;
     }
    else if (wxGetApp().GetExport3DPrefs()->HiddenGroupsExportMode == P3D_NEVER)
     {
      ExportHiddenGroups = false;
     }
    else
     {
      if (wxMessageBox(wxT("Export hidden branch groups?"),wxT("Export mode"),wxYES_NO) == wxYES)
       {
        ExportHiddenGroups = true;
       }
      else
       {
        ExportHiddenGroups = false;
       }
     }

    if      (wxGetApp().GetExport3DPrefs()->OutVisRangeExportMode == P3D_ALWAYS)
     {
      ExportOutVisRangeGroups = true;
     }
    else if (wxGetApp().GetExport3DPrefs()->OutVisRangeExportMode == P3D_NEVER)
     {
      ExportOutVisRangeGroups = false;
     }
    else
     {
      if (wxMessageBox(wxT("Export branch groups which are outside visibility range?"),wxT("Export mode"),wxYES_NO) == wxYES)
       {
        ExportOutVisRangeGroups = true;
       }
      else
       {
        ExportOutVisRangeGroups = false;
       }
     }


    if (!P3DModelExportOBJ(FileName.mb_str(),
                           MTLFileName.GetFullPath().mb_str(),
                           wxGetApp().GetModel(),
                           ExportHiddenGroups,
                           ExportOutVisRangeGroups,
                           wxGetApp().GetLODLevel()))
     {
      ::wxMessageBox(wxT("Error while exporting model"),wxT("Error"),wxOK | wxICON_ERROR);
     }
   }
 }

void               P3DMainFrame::OnExportObjPlugin
                                          (wxCommandEvent     &event)
 {
  int              PluginIndex;
  const P3DPluginInfoVector
                  &ExportPlugins = wxGetApp().GetExportPlugins();

  PluginIndex = event.GetId() - wxID_EXPORT_PLUGIN_FIRST;

  if ((PluginIndex < 0) || (PluginIndex >= ExportPlugins.size()))
   {
    return;
   }

  P3DPlugLuaRunScript(ExportPlugins[PluginIndex].GetFileName(),
                      wxGetApp().GetModel());
 }

void               P3DMainFrame::OnRunScript
                                          (wxCommandEvent     &event)
 {
  wxString                                 FileName;

  FileName = ::wxFileSelector(wxT("Select script to run"),wxT(""),wxT(""),wxT(".obj"),wxT("*.lua"),wxOPEN | wxFILE_MUST_EXIST);

  if (!FileName.empty())
   {
    P3DPlugLuaRunScript(FileName.mb_str(),wxGetApp().GetModel());
   }
 }

void               P3DMainFrame::Save  (const char         *FileName)
 {
  try
   {
    P3DOutputStringStreamFile           TargetStream;
    P3DIDEMaterialSaver                 MaterialSaver;

    TargetStream.Open(FileName);

    wxGetApp().GetModel()->Save(&TargetStream,&MaterialSaver);

    TargetStream.Close();

    wxGetApp().SetFileName(FileName);
   }
  catch (...)
   {
    ::wxMessageBox(wxT("Error while saving model"),wxT("Error"),wxOK | wxICON_ERROR);
   }
 }

void               P3DMainFrame::OnNew(wxCommandEvent     &event)
 {
  if (::wxMessageBox(wxT("Your current model will be discarded. Are you sure?"),
                     wxT("Confirmation"),
                     wxOK | wxCANCEL) == wxOK)
   {
    P3DPlantModel                     *NewModel;

    NewModel = wxGetApp().CreateNewPlantModel();

    EditPanel->HideAll();

    wxGetApp().SetModel(NewModel);

    wxGetApp().SetFileName("");

    EditPanel->RestoreAll();
   }
 }

void               P3DMainFrame::OnOpen
                                      (wxCommandEvent     &event)
 {
  wxString                             FileName;
  P3DPlantModel                       *NewModel;

  FileName = ::wxFileSelector(wxT("File name"),wxT(""),wxT(""),wxT(".ngp"),wxT("*.ngp"),wxOPEN | wxFILE_MUST_EXIST);

  if (!FileName.empty())
   {
    NewModel = 0;

    try
     {
      P3DInputStringStreamFile         SourceStream;
      P3DIDEMaterialFactory            MaterialFactory
                                        (wxGetApp().GetTexManager(),
                                         wxGetApp().GetShaderManager());

      SourceStream.Open(FileName.mb_str());

      NewModel = new P3DPlantModel();

      NewModel->Load(&SourceStream,&MaterialFactory);

      SourceStream.Close();

      wxGetApp().SetFileName(FileName.mb_str());

      EditPanel->HideAll();

      wxGetApp().SetModel(NewModel);

      NewModel = 0;

      EditPanel->RestoreAll();
     }
    catch (...)
     {
      ::wxMessageBox(wxT("Error while loading model"),wxT("Error"),wxOK | wxICON_ERROR);
     }

    delete NewModel;
   }
 }

void               P3DMainFrame::OnUndo
                                      (wxCommandEvent     &event)
 {
  wxGetApp().Undo();
 }

void               P3DMainFrame::OnRedo
                                      (wxCommandEvent     &event)
 {
  wxGetApp().Redo();
 }

void               P3DMainFrame::UpdateControls
                                      ()
 {
  EditPanel->UpdateControls();
 }

void               P3DMainFrame::InvalidatePlant
                                      ()
 {
  Canvas3D->Refresh();
  EditPanel->PlantInvalidated();
 }

IMPLEMENT_APP(P3DApp)

                   P3DApp::~P3DApp    ()
 {
  delete PlantModel;
 }

P3DPlantModel     *P3DApp::GetModel   ()
 {
  return(PlantModel);
 }

const P3DPlantObject
                  *P3DApp::GetPlantObject
                                      () const
 {
  if  ((PlantObjectDirty) && (PlantObjectAutoUpdate) &&
       (MainFrame->IsGLExtInited()))
   {
    bool InitMode;

    if (PlantObject == 0)
     {
      InitMode = true;
     }
    else
     {
      InitMode = false;

      delete PlantObject;
     }

    try
     {
      PlantObject      = new P3DPlantObject(PlantModel,RenderQuirks.UseColorArray);
      PlantObjectDirty = false;
     }
    catch (...)
     {
      PlantObject = 0;
     }

    if ((InitMode) && (PlantObject != 0))
     {
      MainFrame->EditPanel->PlantInvalidated();
     }
   }

  return(PlantObject);
 }

void               P3DApp::SetModel   (P3DPlantModel      *Model)
 {
  P3DUndoRedoMenuStateUpdater           Updater(MainFrame->GetMenuBar(),CommandQueue);

  CommandQueue.Clear();

  if (PlantModel != Model)
   {
    delete PlantModel;

    PlantModel = Model;

    InvalidatePlant();
   }
 }

P3DTexManagerGL   *P3DApp::GetTexManager
                                      ()
 {
  return(&TexManager);
 }

P3DShaderManager  *P3DApp::GetShaderManager
                                      ()
 {
  return(&ShaderManager);
 }

P3DIDEVFS         *P3DApp::GetTexFS   ()
 {
  return(&TexFS);
 }

const wxBitmap    &P3DApp::GetBitmap  (unsigned int        Bitmap)
 {
  return(Bitmaps[Bitmap]);
 }

P3DStemModel      *P3DApp::CreateStemModelStd
                                      () const
 {
  return(CreateStemModelTube());
 }

P3DStemModel      *P3DApp::CreateStemModelTube
                                      () const
 {
  P3DStemModelTube *StdStemModel = new P3DStemModelTube();

  StdStemModel->SetLength(0.5f);

  P3DMathNaturalCubicSpline TempCurve;

  TempCurve.SetLinear(0.0f,1.0f,1.0f,0.0f);

  StdStemModel->SetProfileScaleCurve(&TempCurve);

  return(StdStemModel);
 }

P3DStemModel      *P3DApp::CreateStemModelQuad
                                      () const
 {
  P3DStemModelQuad *StdStemModel = new P3DStemModelQuad();

  StdStemModel->SetLength(0.5f);
  StdStemModel->SetWidth(0.5f);

  return(StdStemModel);
 }

P3DBranchingAlg   *P3DApp::CreateBranchingAlgStd
                                      () const
 {
  return(new P3DBranchingAlgStd());
 }

P3DMaterialInstanceSimple
                  *P3DApp::CreateMatInstanceStd
                                      () const
 {
  P3DMaterialInstanceSimple           *Result;
  P3DMaterialDef                       StdMaterialDef;

  StdMaterialDef.SetColor(0.7f,0.7f,0.7f);

  Result = new P3DMaterialInstanceSimple(const_cast<P3DTexManagerGL*>(&TexManager),
                                         const_cast<P3DShaderManager*>(&ShaderManager),
                                         StdMaterialDef);

  return(Result);
 }

void               P3DApp::InvalidatePlant
                                      ()
 {
  if (PlantObjectAutoUpdate)
   {
    ForceUpdate();
   }
  else
   {
    PlantObjectDirty = true;
    MainFrame->InvalidatePlant();
   }
 }

void               P3DApp::InvalidateCamera
                                      ()
 {
  if (PlantObject != 0)
   {
    PlantObject->InvalidateCamera();
   }
 }

void               P3DApp::ForceUpdate()
 {
  if (MainFrame->IsGLExtInited())
   {
    delete PlantObject;

    try
     {
      PlantObject      = new P3DPlantObject(PlantModel,RenderQuirks.UseColorArray);
      PlantObjectDirty = false;
     }
    catch (...)
     {
      PlantObject = 0;
     }

    MainFrame->InvalidatePlant();
   }
 }

bool               P3DApp::IsPlantObjectDirty
                                      () const
 {
  return(PlantObjectDirty);
 }

wxString           P3DApp::GetFileName() const
 {
  return(PlantFileName);
 }

void               P3DApp::SetFileName(const char         *FileName)
 {
  PlantFileName = wxString(FileName,wxConvUTF8);

  if (PlantFileName.empty())
   {
    MainFrame->SetTitle(wxT("ngPlant designer"));
   }
  else
   {
    wxFileName       TempFileName(PlantFileName);

    wxString         ShortName;

    ShortName = TempFileName.GetFullName();

    MainFrame->SetTitle(wxString(wxT("ngPlant designer [")) + ShortName + wxT("]"));
   }
 }

void               P3DApp::InitTexFS  ()
 {
  wxConfigBase                        *Cfg;

  Cfg = wxConfigBase::Get();

  if (Cfg->HasGroup(wxT("/TexLocations")))
   {
    Cfg->SetPath(wxT("/TexLocations"));

    bool     Cont;
    wxString EntryName;
    wxString PathStr;
    long     Cookie;

    Cont = Cfg->GetFirstEntry(EntryName,Cookie);

    while (Cont)
     {
      if (Cfg->Read(EntryName,&PathStr))
       {
        TexFS.AppendEntry(PathStr.mb_str(wxConvUTF8));
       }

      Cont = Cfg->GetNextEntry(EntryName,Cookie);
     }

    Cfg->SetPath(wxT("/"));
   }
 }

P3DPlantModel     *P3DApp::CreateNewPlantModel
                                      () const
 {
  P3DPlantModel                       *NewPlantModel;
  P3DBranchModel                      *TrunkModel;
  P3DStemModelTube                    *TrunkStemModel;
  P3DBranchingAlgBase                 *BranchingAlg;

  NewPlantModel = new P3DPlantModel();

  NewPlantModel->SetBaseSeed(123);
  NewPlantModel->GetPlantBase()->SetName("Plant");

  TrunkModel     = new P3DBranchModel();
  TrunkStemModel = new P3DStemModelTube();
  BranchingAlg   = new P3DBranchingAlgBase();

  TrunkStemModel->SetLength(15.0f);

  P3DMathNaturalCubicSpline TempCurve;

  TempCurve.SetLinear(0.0f,1.0f,1.0f,0.0f);

  TrunkStemModel->SetProfileScaleCurve(&TempCurve);

  TrunkModel->SetName("Branch1");
  TrunkModel->SetStemModel(TrunkStemModel);
  TrunkModel->SetMaterialInstance(CreateMatInstanceStd());
  TrunkModel->SetBranchingAlg(BranchingAlg);

  NewPlantModel->GetPlantBase()->AppendSubBranch(TrunkModel);

  return(NewPlantModel);
 }

void               P3DApp::ExecEditCmd(P3DEditCommand     *Cmd)
 {
  P3DUndoRedoMenuStateUpdater          Updater(MainFrame->GetMenuBar(),CommandQueue);

  CommandQueue.PushAndExec(Cmd);
 }

void               P3DApp::Undo       ()
 {
  P3DUndoRedoMenuStateUpdater          Updater(MainFrame->GetMenuBar(),CommandQueue);

  CommandQueue.Undo();

  UpdateControls();
 }

void               P3DApp::Redo       ()
 {
  P3DUndoRedoMenuStateUpdater          Updater(MainFrame->GetMenuBar(),CommandQueue);

  CommandQueue.Redo();

  UpdateControls();
 }

void               P3DApp::UpdateControls
                                      ()
 {
  MainFrame->UpdateControls();
 }

void               P3DApp::OnInitCmdLine
                                      (wxCmdLineParser    &Parser)
 {
  Parser.AddSwitch(wxT("ns"),wxT("no-shaders"),wxT("disable shaders"));
  Parser.AddSwitch(wxT("es"),wxT("enable-stderr"),wxT("route log messages to stderr"));

  wxApp::OnInitCmdLine(Parser);
 }

bool               P3DApp::OnCmdLineParsed
                                      (wxCmdLineParser    &Parser)
 {
  if (wxApp::OnCmdLineParsed(Parser))
   {
    if (Parser.Found(wxT("ns")))
     {
      DisableShaders();
     }

    if (Parser.Found(wxT("verbose")))
     {
      P3DLogEnableVerbose();
     }

    if (Parser.Found(wxT("es")))
     {
      P3DLogEnableStdErr();
     }

    return(true);
   }
  else
   {
    return(false);
   }
 }

void               P3DApp::ScanPlugins()
 {
  wxDir            Dir(PluginsPath);
  wxString         FileName;
  P3DPluginInfo    PluginInfo;

  ExportPlugins.clear();
  GMeshPlugins.clear();

  if (!Dir.IsOpened())
   {
    return;
   }

  if (Dir.GetFirst(&FileName,wxEmptyString,wxDIR_FILES)) do
   {
    try
     {
      P3DInputStringStreamFile         SourceStream;
      wxFileName                       FullFileName(PluginsPath,FileName);

      SourceStream.Open(FullFileName.GetFullPath().mb_str());

      if (PluginInfo.LoadInfo(&SourceStream,FullFileName.GetFullPath().mb_str()))
       {
        if      (PluginInfo.GetType() == P3DPluginTypeModelExport)
         {
          ExportPlugins.push_back(PluginInfo);
         }
        else if (PluginInfo.GetType() == P3DPluginTypeGMeshGenerator)
         {
          GMeshPlugins.push_back(PluginInfo);
         }
       }

      SourceStream.Close();
     }
    catch (...)
     {
     }
   }
  while (Dir.GetNext(&FileName));
 }

bool               P3DApp::OnInit     ()
 {
  UseShaders = true;

  if (!wxApp::OnInit())
   {
    return(false);
   }

  ::wxInitAllImageHandlers();

  wxFileName                           CfgFileName;

  CfgFileName.AssignHomeDir();
  CfgFileName.AppendDir(wxT(".ngplant"));

  struct stat  CfgDirStat;

  if (stat(CfgFileName.GetFullPath().mb_str(wxConvUTF8),&CfgDirStat) != 0)
   {
    #ifdef _WIN32
    mkdir(CfgFileName.GetFullPath().mb_str(wxConvUTF8));
    #else
    mkdir(CfgFileName.GetFullPath().mb_str(wxConvUTF8),0775);
    #endif
   }

  CfgFileName.SetFullName(wxT(".ngplant"));

  wxConfigBase                        *Cfg;

  Cfg = new wxFileConfig(wxT("ngplant"),
                         wxEmptyString,
                         CfgFileName.GetFullPath(),
                         wxEmptyString,
                         wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);

  wxConfigBase::Set(Cfg);

  InitTexFS();

  View3DPrefs.Read(Cfg);
  CameraControlPrefs.Read(Cfg);
  P3DUIControlsPrefs::Read(Cfg);
  Export3DPrefs.Read(Cfg);
  RenderQuirks.Read(Cfg);

  if (!Cfg->Read(wxT("Paths/Plugins"),&PluginsPath))
   {
    #if defined(PLUGINS_DIR)
    PluginsPath = wxT(PLUGINS_DIR);
    #else
    PluginsPath = wxT("plugins");
    #endif
   }

  ScanPlugins();

  Bitmaps[P3D_BITMAP_NO_TEXTURE]     = wxBitmap(P3DNoTextureXpm);
  Bitmaps[P3D_BITMAP_REMOVE_TEXTURE] = wxBitmap(P3DCloseXPM);

  LODLevel = 1.0f;

  PlantModel  = CreateNewPlantModel();
  PlantObject = 0;
  PlantObjectDirty = true;
  PlantObjectAutoUpdate = true;

  MainFrame = new P3DMainFrame(wxT("ngPlant designer"));

  MainFrame->Show(TRUE);

  ForceUpdate();

  return(TRUE);
 }

void               P3DApp::GetGroundColor
                                      (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const
 {
  *R = View3DPrefs.GroundColor.R;
  *G = View3DPrefs.GroundColor.G;
  *B = View3DPrefs.GroundColor.B;
 }

void               P3DApp::SetGroundColor
                                      (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B)
 {
  View3DPrefs.GroundColor.R = R;
  View3DPrefs.GroundColor.G = G;
  View3DPrefs.GroundColor.B = B;

  InvalidatePlant();
 }

bool               P3DApp::IsGroundVisible
                                      () const
 {
  return(View3DPrefs.GroundVisible);
 }

void               P3DApp::SetGroundVisibility
                                      (bool                Visible)
 {
  if (View3DPrefs.GroundVisible != Visible)
   {
    View3DPrefs.GroundVisible = Visible;

    InvalidatePlant();
   }
 }

void               P3DApp::GetBackgroundColor
                                      (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const
 {
  *R = View3DPrefs.BackgroundColor.R;
  *G = View3DPrefs.BackgroundColor.G;
  *B = View3DPrefs.BackgroundColor.B;
 }

void               P3DApp::SetBackgroundColor
                                      (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B)
 {
  View3DPrefs.BackgroundColor.R = R;
  View3DPrefs.BackgroundColor.G = G;
  View3DPrefs.BackgroundColor.B = B;

  InvalidatePlant();
 }

P3DExport3DPrefs  *P3DApp::GetExport3DPrefs
                                      ()
 {
  return(&Export3DPrefs);
 }

const P3DCameraControlPrefs
                  *P3DApp::GetCameraControlPrefs
                                      () const
 {
  return(&CameraControlPrefs);
 }

void               P3DApp::SetCameraControlPrefs
                                      (const P3DCameraControlPrefs
                                                          *Prefs)
 {
  this->CameraControlPrefs = *Prefs;
 }

void               P3DApp::SetPluginsPath
                                      (const wxString     &PluginsPath)
 {
  this->PluginsPath = PluginsPath;
 }

const wxString    &P3DApp::GetPluginsPath
                                      () const
 {
  return(PluginsPath);
 }

const P3DPluginInfoVector
                  &P3DApp::GetExportPlugins
                                      () const
 {
  return(ExportPlugins);
 }

const P3DPluginInfoVector
                  &P3DApp::GetGMeshPlugins
                                      () const
 {
  return(GMeshPlugins);
 }

float              P3DApp::GetLODLevel() const
 {
  return(LODLevel);
 }

void               P3DApp::SetLODLevel(float               Level)
 {
  LODLevel = P3DMath::Clampf(0.0f,1.0f,Level);
 }

bool               P3DApp::IsAutoUpdateMode
                                      () const
 {
  return(PlantObjectAutoUpdate);
 }

void               P3DApp::SetAutoUpdateMode
                                      (bool                Enable)
 {
  PlantObjectAutoUpdate = Enable;
 }

bool               P3DApp::IsShadersEnabled
                                      () const
 {
  return(UseShaders);
 }

void               P3DApp::DisableShaders
                                      ()
 {
  UseShaders = false;

  ShaderManager.DisableShaders();
 }

const P3DRenderQuirksPrefs
                  &P3DApp::GetRenderQuirksPrefs
                                      () const
 {
  return(RenderQuirks);
 }

void               P3DApp::SetRenderQuirksPrefs
                                      (const P3DRenderQuirksPrefs
                                                          &Prefs)
 {
  RenderQuirks = Prefs;
 }

