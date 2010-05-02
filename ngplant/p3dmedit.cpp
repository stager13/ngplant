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
#include <wx/treectrl.h>
#include <wx/notebook.h>

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemquad.h>
#include <ngpcore/p3dmodelstemwings.h>
#include <ngpcore/p3dmodelstemgmesh.h>
#include <ngpcore/p3dbalgbase.h>
#include <ngpcore/p3dbalgstd.h>
#include <ngpcore/p3dbalgwings.h>

#include <p3dwxlistdlg.h>

#include <p3dapp.h>
#include <p3dmaterialstd.h>

#include <p3duimodelstemtube.h>
#include <p3duimodelstemquad.h>
#include <p3duimodelstemwings.h>
#include <p3duimodelstemempty.h>
#include <p3duibalgbase.h>
#include <p3duibalgstd.h>
#include <p3duibalgwings.h>
#include <p3duimaterialstd.h>
#include <p3duioptgeneral.h>

#include <p3dpluglua.h>

#include <p3dmedit.h>

enum
 {
  PLANT_TREE_CTRL_ID = wxID_HIGHEST + 120,
  PLANT_TREE_APPEND_BRANCH_ID,
  PLANT_TREE_APPEND_BRANCH_NEW_ID,
  PLANT_TREE_APPEND_BRANCH_COPY_ID,
  PLANT_TREE_REMOVE_STEM_ID,
  PLANT_TREE_RENAME_STEM_ID,
  PLANT_TREE_HIDESHOW_STEM_ID,

  P3D_SET_STEM_MODEL_ID,
  P3D_SET_STEM_MODEL_TUBE_ID,
  P3D_SET_STEM_MODEL_QUAD_ID,
  P3D_SET_STEM_MODEL_WINGS_ID,
  P3D_SET_STEM_MODEL_GMESH_ID,

  P3D_AUTO_UPDATE_ID,
  P3D_UPDATE_ID,

  P3D_BRANCH_PANEL_ID,

  wxID_GMESH_PLUGIN_FIRST = wxID_HIGHEST + 12000,
  wxID_GMESH_PLUGIN_LAST  = wxID_GMESH_PLUGIN_FIRST+ 250
 };

static void        ExpandTreeCtrlRecursive
                                      (wxTreeCtrl         *TreeCtrl,
                                       const wxTreeItemId &ItemId)
 {
  wxTreeItemId                         ChildId;
  #if wxCHECK_VERSION(2,8,0)
  wxTreeItemIdValue                    Cookie;
  #else
  long                                 Cookie;
  #endif

  TreeCtrl->Expand(ItemId);

  ChildId = TreeCtrl->GetFirstChild(ItemId,Cookie);

  while (ChildId.IsOk())
   {
    ExpandTreeCtrlRecursive(TreeCtrl,ChildId);

    ChildId = TreeCtrl->GetNextChild(ItemId,Cookie);
   }
 }


                    P3DBranchPanel::P3DBranchPanel
                                      (wxWindow           *parent,
                                       P3DBranchModel     *BranchModel)
                   : wxNotebook(parent,P3D_BRANCH_PANEL_ID)
 {
  this->BranchModel = BranchModel;

  #if wxCHECK_VERSION(2,8,0)
  #else
  wxNotebookSizer *TopSizer = new wxNotebookSizer(this);
  #endif

  CreateTabs();
 }

void               P3DBranchPanel::SwitchToBranch
                                      (P3DBranchModel     *BranchModel)
 {
  int                                  OldPageIndex;
  size_t                               OldPageCount;

  OldPageCount = GetPageCount();
  OldPageIndex = GetSelection();

  DeleteAllPages();

  this->BranchModel = BranchModel;

  CreateTabs();

  if (OldPageIndex != wxNOT_FOUND)
   {
    if (OldPageCount == 1) /* for base */
     {
      if (GetPageCount() == 1)
       {
        SetSelection(OldPageIndex);
       }
      else
       {
        SetSelection(0);
       }
     }
    else
     {
      if (GetPageCount() == 1)
       {
        SetSelection(0);
       }
      else
       {
        SetSelection(OldPageIndex);
       }
     }
   }

  #if defined(__WXMSW__)
   {
    Refresh();
   }
  #endif

 }

void               P3DBranchPanel::CreateTabs
                                      ()
 {
  if (BranchModel != 0)
   {
    wxWindow                          *StemModelWindow;

    StemModelWindow = 0;

    P3DStemModelTube *StemModelTube = dynamic_cast<P3DStemModelTube*>(BranchModel->GetStemModel());

    if (StemModelTube != 0)
     {
      StemModelWindow = new P3DStemTubePanel(this,StemModelTube,BranchModel->GetVisRangeState());
     }

    if (StemModelWindow == 0)
     {
      P3DStemModelQuad *StemModelQuad = dynamic_cast<P3DStemModelQuad*>(BranchModel->GetStemModel());

      if (StemModelQuad != 0)
       {
        StemModelWindow = new P3DStemQuadPanel(this,StemModelQuad,BranchModel->GetVisRangeState());
       }
     }

    if (StemModelWindow == 0)
     {
      P3DStemModelWings *StemModelWings = dynamic_cast<P3DStemModelWings*>(BranchModel->GetStemModel());

      if (StemModelWings != 0)
       {
        StemModelWindow = new P3DStemWingsPanel(this,StemModelWings,BranchModel->GetVisRangeState());
       }
     }

    if (StemModelWindow == 0)
     {
      P3DStemModelGMesh *StemModelGMesh = dynamic_cast<P3DStemModelGMesh*>(BranchModel->GetStemModel());

      if (StemModelGMesh != 0)
       {
        StemModelWindow = new P3DStemEmptyPanel(this);
       }
     }

    if (StemModelWindow != 0)
     {
      AddPage(StemModelWindow,wxT("Stem"));

      StemModelWindow->SetAutoLayout(true);
     }

    wxWindow                          *MaterialWindow;

    MaterialWindow = 0;

    P3DMaterialInstanceSimple         *MaterialSimple;

    MaterialSimple = dynamic_cast<P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

    if (MaterialSimple != 0)
     {
      MaterialWindow = new P3DMaterialStdPanel(this,MaterialSimple,BranchModel->GetStemModel());
     }

    if (MaterialWindow != 0)
     {
      AddPage(MaterialWindow,wxT("Material"));

      MaterialWindow->SetAutoLayout(true);
     }

    wxWindow                          *BranchingAlgWindow;

    BranchingAlgWindow = 0;

    P3DBranchingAlgStd *BranchingAlgStd = dynamic_cast<P3DBranchingAlgStd*>(BranchModel->GetBranchingAlg());

    if (BranchingAlgStd != 0)
     {
      BranchingAlgWindow = new P3DBranchingAlgStdPanel(this,BranchingAlgStd);
     }

    if (BranchingAlgWindow == 0)
     {
      P3DBranchingAlgBase *BranchingAlgBase = dynamic_cast<P3DBranchingAlgBase*>(BranchModel->GetBranchingAlg());

      if (BranchingAlgBase != 0)
       {
        BranchingAlgWindow = new P3DBranchingAlgBasePanel(this,BranchingAlgBase);
       }
     }

    if (BranchingAlgWindow == 0)
     {
      P3DBranchingAlgWings *BranchingAlgWings = dynamic_cast<P3DBranchingAlgWings*>(BranchModel->GetBranchingAlg());

      if (BranchingAlgWings != 0)
       {
        BranchingAlgWindow = new P3DBranchingAlgWingsPanel(this,BranchingAlgWings);
       }
     }

    if (BranchingAlgWindow != 0)
     {
      AddPage(BranchingAlgWindow,wxT("Branching"));

      BranchingAlgWindow->SetAutoLayout(true);
     }
   }

  P3DOptGeneralPanel *OptGeneralPanel = new P3DOptGeneralPanel(this);

  AddPage(OptGeneralPanel,wxT("General"));

  OptGeneralPanel->SetAutoLayout(true);

  Fit();
 }

void               P3DBranchPanel::UpdateControls
                                      ()
 {
  for (unsigned int PageIndex = 0; PageIndex < GetPageCount(); PageIndex++)
   {
    P3DUIParamPanel *ParamPanel = dynamic_cast<P3DUIParamPanel*>(GetPage(PageIndex));

    if (ParamPanel != NULL)
     {
      ParamPanel->UpdateControls();
     }
   }
 }

class P3DPlantModelTreeCtrlItemData : public wxTreeItemData
 {
  public           :

                   P3DPlantModelTreeCtrlItemData
                                      (P3DBranchModel     *BranchModel)
   {
    this->BranchModel = BranchModel;
   }

  P3DBranchModel  *GetBranchModel     ()
   {
    return(BranchModel);
   }

  void             SetBranchModel     (P3DBranchModel     *BranchModel)
   {
    this->BranchModel = BranchModel;
   }

  private          :

  P3DBranchModel                      *BranchModel;
 };

BEGIN_EVENT_TABLE(P3DPlantModelTreeCtrl,wxTreeCtrl)
 EVT_TREE_SEL_CHANGED(PLANT_TREE_CTRL_ID,P3DPlantModelTreeCtrl::OnSelectionChanged)
 EVT_TREE_ITEM_RIGHT_CLICK(PLANT_TREE_CTRL_ID,P3DPlantModelTreeCtrl::OnItemRightClick)
 EVT_MENU(PLANT_TREE_APPEND_BRANCH_NEW_ID,P3DPlantModelTreeCtrl::OnAppendBranchNewClick)
 EVT_MENU(PLANT_TREE_APPEND_BRANCH_COPY_ID,P3DPlantModelTreeCtrl::OnAppendBranchCopyClick)
 EVT_MENU(PLANT_TREE_REMOVE_STEM_ID,P3DPlantModelTreeCtrl::OnRemoveStemClick)
 EVT_MENU(PLANT_TREE_RENAME_STEM_ID,P3DPlantModelTreeCtrl::OnRenameStemClick)
 EVT_MENU(PLANT_TREE_HIDESHOW_STEM_ID,P3DPlantModelTreeCtrl::OnHideShowStemClick)
 EVT_MENU(P3D_SET_STEM_MODEL_TUBE_ID,P3DPlantModelTreeCtrl::OnSetStemModelTubeClick)
 EVT_MENU(P3D_SET_STEM_MODEL_QUAD_ID,P3DPlantModelTreeCtrl::OnSetStemModelQuadClick)
 EVT_MENU(P3D_SET_STEM_MODEL_WINGS_ID,P3DPlantModelTreeCtrl::OnSetStemModelWingsClick)
 EVT_MENU_RANGE(wxID_GMESH_PLUGIN_FIRST,wxID_GMESH_PLUGIN_LAST,P3DPlantModelTreeCtrl::OnSetStemModelGMeshClick)
END_EVENT_TABLE()

                   P3DPlantModelTreeCtrl::P3DPlantModelTreeCtrl
                                      (wxWindow           *parent,
                                       P3DPlantModel      *PlantModel,
                                       P3DBranchPanel     *BranchPanel)
                   : wxTreeCtrl(parent,PLANT_TREE_CTRL_ID,wxDefaultPosition,
                                wxDefaultSize,wxTR_HAS_BUTTONS | wxRAISED_BORDER)
 {
  this->BranchPanel = BranchPanel;

  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

  P3DBranchModel  *PlantBase;

  PlantBase = PlantModel->GetPlantBase();

  P3DMathRNGSimple           RNG(PlantModel->GetBaseSeed());

  wxTreeItemId RootId = AddRoot(MakeTreeItemLabel(PlantBase->GetName(),PlantBase));

  SetItemData(RootId,new P3DPlantModelTreeCtrlItemData(PlantBase));

  AppendChildrenRecursive(PlantBase,RootId);

  #if defined(__WXMSW__)
   {
    SetItemBold(RootId,true);
   }
  #endif
 }

void               P3DPlantModelTreeCtrl::AppendChildrenRecursive
                                      (P3DBranchModel     *BranchModel,
                                       wxTreeItemId        BranchItemId)
 {
  unsigned int                         BranchIndex;
  P3DBranchModel                      *SubBranchModel;

  for (BranchIndex = 0;
       BranchIndex < BranchModel->GetSubBranchCount();
       BranchIndex++)
   {
    SubBranchModel = BranchModel->GetSubBranchModel(BranchIndex);

    wxTreeItemId ChildId = AppendItem(BranchItemId,MakeTreeItemLabel(SubBranchModel->GetName(),SubBranchModel));

    SetItemData(ChildId,new P3DPlantModelTreeCtrlItemData(SubBranchModel));

    AppendChildrenRecursive(SubBranchModel,ChildId);
   }
 }

void               P3DPlantModelTreeCtrl::OnSelectionChanged
                                      (wxTreeEvent        &event)
 {
  P3DBranchModel                      *BranchModel;

  #if defined(__WXMSW__)
   {
    SetItemBold(event.GetOldItem(),false);
    SetItemBold(event.GetItem(),true);
   }
  #endif

  if (!event.GetItem().IsOk())
   {
    return;
   }

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(event.GetItem())))->GetBranchModel();

  BranchPanel->SwitchToBranch(BranchModel);
 }

namespace {

class AppendBranchCommand : public P3DEditCommand
 {
  public           :

                   AppendBranchCommand(P3DBranchModel          *ParentBranchModel,
                                       P3DBranchModel          *ChildBranchModel,
                                       P3DPlantModelTreeCtrl   *ModelTreeCtrl)
   {
    this->ParentBranchModel = ParentBranchModel;
    this->ChildBranchModel  = ChildBranchModel;
    this->ModelTreeCtrl     = ModelTreeCtrl;
   }

  virtual         ~AppendBranchCommand()
   {
    delete ChildBranchModel;
   }

  virtual void     Exec               ()
   {
    ParentBranchModel->AppendSubBranch(ChildBranchModel);

    wxTreeItemId   ParentItemId;

    if (ModelTreeCtrl->FindItemByModel(&ParentItemId,
                                        ModelTreeCtrl->GetRootItem(),
                                        ParentBranchModel))
     {
      wxTreeItemId ChildId = ModelTreeCtrl->AppendItem
                              (ParentItemId,
                               ModelTreeCtrl->MakeTreeItemLabel
                                (ChildBranchModel->GetName(),ChildBranchModel));

      ModelTreeCtrl->SetItemData(ChildId,new P3DPlantModelTreeCtrlItemData(ChildBranchModel));

      ModelTreeCtrl->SelectItem(ChildId);

      wxGetApp().InvalidatePlant();
     }

    ChildBranchModel = NULL;
   }

  virtual void     Undo               ()
   {
    ChildBranchModel = ParentBranchModel->DetachSubBranch
                        (ParentBranchModel->GetSubBranchCount() - 1);


    wxTreeItemId   ItemId;

    if (ModelTreeCtrl->FindItemByModel(&ItemId,
                                        ModelTreeCtrl->GetRootItem(),
                                        ChildBranchModel))
     {
      if (ModelTreeCtrl->GetSelection() == ItemId)
       {
        ModelTreeCtrl->SelectItem(ModelTreeCtrl->GetItemParent(ItemId));
       }

      ModelTreeCtrl->Delete(ItemId);
     }

    wxGetApp().InvalidatePlant();
   }

  P3DBranchModel                      *ParentBranchModel;
  P3DBranchModel                      *ChildBranchModel;
  P3DPlantModelTreeCtrl               *ModelTreeCtrl;
 };
}

void               P3DPlantModelTreeCtrl::OnAppendBranchNewClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel                      *ParentBranchModel;
  P3DBranchModel                      *ChildBranchModel;

  ParentBranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

  ChildBranchModel = new P3DBranchModel();

  P3DPlantModel::BranchModelSetUniqueName(wxGetApp().GetModel(),ChildBranchModel);

  ChildBranchModel->SetStemModel(wxGetApp().CreateStemModelStd());

  if (GetSelection() == GetRootItem())
   {
    ChildBranchModel->SetBranchingAlg(new P3DBranchingAlgBase());
   }
  else
   {
    ChildBranchModel->SetBranchingAlg(wxGetApp().CreateBranchingAlgStd());
   }

  ChildBranchModel->SetMaterialInstance(wxGetApp().CreateMatInstanceStd());

  wxGetApp().ExecEditCmd
   (new AppendBranchCommand(ParentBranchModel,ChildBranchModel,this));
 }

void               P3DPlantModelTreeCtrl::OnAppendBranchCopyClick
                                      (wxCommandEvent     &event)
 {
  P3DListDialog                        SourceSelectionDialog(NULL,wxID_ANY,wxT("Select group to copy"));
  bool                                 Done;
  P3DPlantModel                       *PlantModel;
  P3DBranchModel                      *BranchModel;
  unsigned int                         BranchIndex;

  PlantModel = wxGetApp().GetModel();

  BranchIndex = 0;
  BranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,BranchIndex);

  while (BranchModel != 0)
   {
    SourceSelectionDialog.AddChoice(BranchModel->GetName());

    BranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,++BranchIndex);
   }

  SourceSelectionDialog.SetSelection(0);

  if (SourceSelectionDialog.ShowModal() == wxID_OK)
   {
    int            SourceBranchIndex;

    SourceBranchIndex = SourceSelectionDialog.GetSelection();

    if (SourceBranchIndex >= 0)
     {
      const P3DBranchModel            *SourceBranchModel;
      P3DBranchModel                  *ParentBranchModel;
      P3DBranchModel                  *ChildBranchModel;
      P3DStemModelWings               *WingsStemModel;
      const P3DBranchingAlg           *SourceBranchingAlg;
      P3DStemModel                    *NewStemModel;
      P3DBranchingAlg                 *NewBranchingAlg;
      P3DVisRangeState                *NewVisRange;
      float                            MinRange,MaxRange;

      SourceBranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,SourceBranchIndex);
      ParentBranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

      if (dynamic_cast<const P3DStemModelWings*>(SourceBranchModel->GetStemModel()) != 0)
       {
        if (ParentBranchModel->GetStemModel() == 0) /* trunk */
         {
          ::wxMessageBox(wxT("\"Wings\" stem can be added to \"Tube\" stems only"),
                         wxT("Error"),
                         wxICON_ERROR | wxOK);

          return;
         }
       }

      ChildBranchModel = new P3DBranchModel();

      P3DPlantModel::BranchModelSetUniqueName(PlantModel,ChildBranchModel);

      NewStemModel = SourceBranchModel->GetStemModel()->CreateCopy();

      WingsStemModel = dynamic_cast<P3DStemModelWings*>(NewStemModel);

      if (WingsStemModel != 0)
       {
        WingsStemModel->SetParent((P3DStemModelTube*)ParentBranchModel->GetStemModel());
       }

      ChildBranchModel->SetStemModel(NewStemModel);

      if ((dynamic_cast<const P3DBranchingAlgBase*>(SourceBranchModel->GetBranchingAlg())) != 0)
       {
        if (ParentBranchModel->GetStemModel() == 0)
         {
          NewBranchingAlg = SourceBranchModel->GetBranchingAlg()->CreateCopy();
         }
        else
         {
          NewBranchingAlg = wxGetApp().CreateBranchingAlgStd();
         }
       }
      else
       {
        if (ParentBranchModel->GetStemModel() == 0)
         {
          NewBranchingAlg = new P3DBranchingAlgBase();
         }
        else
         {
          NewBranchingAlg = SourceBranchModel->GetBranchingAlg()->CreateCopy();
         }
       }

      ChildBranchModel->SetBranchingAlg(NewBranchingAlg);

      ChildBranchModel->SetMaterialInstance(SourceBranchModel->GetMaterialInstance()->CreateCopy());

      NewVisRange = ChildBranchModel->GetVisRangeState();

      NewVisRange->SetState(SourceBranchModel->GetVisRangeState()->IsEnabled());

      SourceBranchModel->GetVisRangeState()->GetRange(&MinRange,&MaxRange);

      NewVisRange->SetRange(MinRange,MaxRange);

      wxGetApp().ExecEditCmd
       (new AppendBranchCommand(ParentBranchModel,ChildBranchModel,this));
     }
   }
 }

class P3DRemoveStemCommand : public P3DEditCommand
 {
  public           :

                   P3DRemoveStemCommand
                                      (P3DBranchModel     *ParentBranchModel,
                                       unsigned int        SubBranchIndex,
                                       P3DPlantModelTreeCtrl
                                                          *ModelTreeCtrl)
   {
    this->ParentBranchModel = ParentBranchModel;
    this->SubBranchIndex    = SubBranchIndex;
    this->ModelTreeCtrl     = ModelTreeCtrl;

    BranchModel = 0;
   }

  virtual         ~P3DRemoveStemCommand
                                      ()
   {
    delete BranchModel;
   }

  virtual void     Exec               ()
   {
    BranchModel = ParentBranchModel->DetachSubBranch(SubBranchIndex);

    wxTreeItemId   ItemId;

    if (ModelTreeCtrl->FindItemByModel(&ItemId,ModelTreeCtrl->GetRootItem(),BranchModel))
     {
      ModelTreeCtrl->SelectItem(ModelTreeCtrl->GetItemParent(ItemId));
      ModelTreeCtrl->Delete(ItemId);
     }

    wxGetApp().InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    ParentBranchModel->InsertSubBranch(BranchModel,SubBranchIndex);

    wxTreeItemId   ParentId;

    if (ModelTreeCtrl->FindItemByModel
         (&ParentId,ModelTreeCtrl->GetRootItem(),ParentBranchModel))
     {
      wxTreeItemId ItemId;

      ItemId = ModelTreeCtrl->InsertItem
                (ParentId,
                 SubBranchIndex,
                 ModelTreeCtrl->MakeTreeItemLabel(BranchModel->GetName(),BranchModel));

      ModelTreeCtrl->SetItemData(ItemId,new P3DPlantModelTreeCtrlItemData(BranchModel));

      ModelTreeCtrl->AppendChildrenRecursive(BranchModel,ItemId);

      ExpandTreeCtrlRecursive(ModelTreeCtrl,ItemId);

      ModelTreeCtrl->SelectItem(ItemId);
     }

    BranchModel = 0;

    wxGetApp().InvalidatePlant();
   }

  private          :

  P3DBranchModel                      *ParentBranchModel;
  unsigned int                         SubBranchIndex;

  P3DBranchModel                      *BranchModel;

  P3DPlantModelTreeCtrl               *ModelTreeCtrl;
 };

bool               P3DPlantModelTreeCtrl::FindItemByModel
                                      (wxTreeItemId       *FoundItemId,
                                       wxTreeItemId        RootId,
                                       const P3DBranchModel
                                                          *BranchModel) const
 {
  wxTreeItemId                         ChildId;
  #if wxCHECK_VERSION(2,8,0)
  wxTreeItemIdValue                    Cookie;
  #else
  long                                 Cookie;
  #endif

  if (((P3DPlantModelTreeCtrlItemData*)(GetItemData(RootId)))->GetBranchModel()
       == BranchModel)
   {
    *FoundItemId = RootId;

    return(true);
   }

  ChildId = GetFirstChild(RootId,Cookie);

  while (ChildId.IsOk())
   {
    if (FindItemByModel(FoundItemId,ChildId,BranchModel))
     {
      return(true);
     }

    ChildId = GetNextChild(RootId,Cookie);
   }

  return(false);
 }

void               P3DPlantModelTreeCtrl::OnRemoveStemClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel  *BranchModel;
  P3DBranchModel  *ParentBranchModel;
  wxTreeItemId     ParentId;
  wxTreeItemId     ItemId;
  wxTreeItemId     CurrItemId;
  unsigned int     SubBranchIndex;

  ItemId   = GetSelection();
  ParentId = GetItemParent(ItemId);

  if (!ParentId.IsOk())
   {
    return;
   }

  SubBranchIndex = 0;

  CurrItemId = ItemId;

  while (CurrItemId.IsOk())
   {
    SubBranchIndex++;
    CurrItemId = GetPrevSibling(CurrItemId);
   }

  ParentBranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(ParentId)))->GetBranchModel();

  wxGetApp().ExecEditCmd
   (new P3DRemoveStemCommand(ParentBranchModel,SubBranchIndex - 1,this));
 }

namespace {
class RenameStemCommand : public P3DEditCommand
 {
  public           :

                   RenameStemCommand  (P3DBranchModel     *BranchModel,
                                       const wxString     &NewName,
                                       P3DPlantModelTreeCtrl
                                                          *ModelTreeCtrl)
   {
    this->BranchModel   = BranchModel;
    this->ModelTreeCtrl = ModelTreeCtrl;

    StoredName = NewName;
   }

  virtual void     Exec               ()
   {
    ChangeName();
   }

  virtual void     Undo               ()
   {
    ChangeName();
   }

  private          :

  void             ChangeName         ()
   {
    wxString       TempName = wxString(BranchModel->GetName(),wxConvUTF8);

    BranchModel->SetName(StoredName.mb_str(wxConvUTF8));

    StoredName = TempName;

    wxTreeItemId ItemId;

    if (ModelTreeCtrl->FindItemByModel
         (&ItemId,ModelTreeCtrl->GetRootItem(),BranchModel))
     {
      ModelTreeCtrl->UpdateLabel(ItemId);
     }
   }

  P3DPlantModelTreeCtrl               *ModelTreeCtrl;
  P3DBranchModel                      *BranchModel;
  wxString                             StoredName;
 };
}

void               P3DPlantModelTreeCtrl::OnRenameStemClick
                                      (wxCommandEvent     &event)
 {
  wxTreeItemId     ItemId;
  P3DBranchModel  *BranchModel;
  wxString         NewName;
  bool             Done;

  ItemId = GetSelection();

  if (!ItemId.IsOk())
   {
    return;
   }

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(ItemId)))->GetBranchModel();

  Done = false;

  while (!Done)
   {
    Done    = true;
    NewName = ::wxGetTextFromUser(wxT("Enter new branch group name:"),
                                  wxT("Branch group name"),
                                  wxString(BranchModel->GetName(),wxConvUTF8));

    if (!NewName.IsEmpty())
     {
      P3DBranchModel  *TempBranchModel;

      TempBranchModel = P3DPlantModel::GetBranchModelByName(wxGetApp().GetModel(),NewName.mb_str(wxConvUTF8));

      if (TempBranchModel == 0)
       {
        wxGetApp().ExecEditCmd
         (new RenameStemCommand(BranchModel,NewName,this));
       }
      else
       {
        if (TempBranchModel != BranchModel)
         {
          ::wxMessageBox(wxT("Branch with this name already exists"),
                         wxT("Error"),
                         wxICON_ERROR | wxOK);

          Done = false;
         }
       }
     }
   }
 }

namespace {

class ChangeStemModelCommand : public P3DEditCommand
  {
   public           :

                   ChangeStemModelCommand
                                      (P3DBranchModel     *ParentBranchModel,
                                       unsigned int        SubBranchIndex,
                                       P3DBranchModel     *OldBranchModel,
                                       P3DStemModel       *NewStemModel,
                                       P3DBranchingAlg    *NewBranchingAlg,
                                       P3DPlantModelTreeCtrl
                                                          *ModelTreeCtrl,
                                       P3DBranchPanel     *BranchPanel)
   {
    P3DVisRangeState                  *NewVisRange;
    float                              MinRange,MaxRange;
    P3DMaterialInstanceSimple         *MaterialSimple;

    this->ParentBranchModel = ParentBranchModel;
    this->SubBranchIndex    = SubBranchIndex;
    this->ModelTreeCtrl     = ModelTreeCtrl;
    this->BranchPanel       = BranchPanel;

    NextBranchModel = new P3DBranchModel();

    NextBranchModel->SetName(OldBranchModel->GetName());

    NextBranchModel->SetStemModel(NewStemModel);
    NextBranchModel->SetBranchingAlg(NewBranchingAlg);

    MaterialSimple = dynamic_cast<P3DMaterialInstanceSimple*>
                      (OldBranchModel->GetMaterialInstance()->CreateCopy());

    MaterialSimple->SetBillboardMode(P3D_BILLBOARD_MODE_NONE);
    NextBranchModel->SetMaterialInstance(MaterialSimple);

    NewVisRange = NextBranchModel->GetVisRangeState();
    NewVisRange->SetState(OldBranchModel->GetVisRangeState()->IsEnabled());
    OldBranchModel->GetVisRangeState()->GetRange(&MinRange,&MaxRange);
    NewVisRange->SetRange(MinRange,MaxRange);
   }

                   ~ChangeStemModelCommand
                                      ()
   {
    delete NextBranchModel;
   }

  virtual void     Exec               ()
   {
    ChangeBranchModel();
   }

  virtual void     Undo               ()
   {
    ChangeBranchModel();
   }

  private          :

  void             ChangeBranchModel  ()
   {
    P3DBranchModel                    *TempBranchModel;
    bool                               RefreshPanel;

    RefreshPanel = false;

    TempBranchModel = ParentBranchModel->DetachSubBranch(SubBranchIndex);

    if (ModelTreeCtrl->GetSelection().IsOk())
     {
      P3DBranchModel                  *SelectedBranchModel;

      SelectedBranchModel = ModelTreeCtrl->GetBranchModelByItemId
                             (ModelTreeCtrl->GetSelection());

      if (SelectedBranchModel == TempBranchModel)
       {
        RefreshPanel = true;
       }
     }

    ParentBranchModel->InsertSubBranch(NextBranchModel,SubBranchIndex);

    wxTreeItemId   ItemId;

    if (ModelTreeCtrl->FindItemByModel
        (&ItemId,ModelTreeCtrl->GetRootItem(),TempBranchModel))
     {
      ((P3DPlantModelTreeCtrlItemData*)
        (ModelTreeCtrl->GetItemData(ItemId)))->SetBranchModel(NextBranchModel);
     }

    wxGetApp().InvalidatePlant();

    if (RefreshPanel)
     {
      BranchPanel->SwitchToBranch(NextBranchModel);
     }

    NextBranchModel = TempBranchModel;
   }

  P3DBranchModel                      *ParentBranchModel;
  P3DBranchModel                      *NextBranchModel;
  unsigned int                         SubBranchIndex;
  P3DPlantModelTreeCtrl               *ModelTreeCtrl;
  P3DBranchPanel                      *BranchPanel;
 };
}

void               P3DPlantModelTreeCtrl::OnSetStemModelTubeClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel                      *BranchModel;
  P3DStemModel                        *StemModel;

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

  StemModel = wxGetApp().CreateStemModelTube();

  unsigned int SubBranchIndex;
  wxTreeItemId ItemId;
  wxTreeItemId ParentItemId;

  SubBranchIndex = 0;
  ItemId         = GetSelection();
  ParentItemId   = GetItemParent(ItemId);

  while ((ItemId = GetPrevSibling(ItemId)).IsOk())
   {
    SubBranchIndex++;
   }

  wxGetApp().ExecEditCmd
   (new ChangeStemModelCommand
         (GetBranchModelByItemId(ParentItemId),
          SubBranchIndex,
          BranchModel,
          StemModel,
          (ParentItemId == GetRootItem()) ?
           (P3DBranchingAlg*)new P3DBranchingAlgBase() :
           (P3DBranchingAlg*)new P3DBranchingAlgStd(),
          this,
          BranchPanel));
 }

void               P3DPlantModelTreeCtrl::OnSetStemModelQuadClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel                      *BranchModel;
  P3DStemModel                        *StemModel;

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

  StemModel = wxGetApp().CreateStemModelQuad();

  unsigned int SubBranchIndex;
  wxTreeItemId ItemId;
  wxTreeItemId ParentItemId;

  SubBranchIndex = 0;
  ItemId         = GetSelection();
  ParentItemId   = GetItemParent(ItemId);

  while ((ItemId = GetPrevSibling(ItemId)).IsOk())
   {
    SubBranchIndex++;
   }

  wxGetApp().ExecEditCmd
   (new ChangeStemModelCommand
         (GetBranchModelByItemId(ParentItemId),
          SubBranchIndex,
          BranchModel,
          StemModel,
          (ParentItemId == GetRootItem()) ?
           (P3DBranchingAlg*)new P3DBranchingAlgBase() :
           (P3DBranchingAlg*)new P3DBranchingAlgStd(),
          this,
          BranchPanel));
 }

void               P3DPlantModelTreeCtrl::OnSetStemModelWingsClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel                      *BranchModel;
  P3DBranchModel                      *ParentBranchModel;
  const P3DStemModelTube              *ParentStemModel;
  P3DStemModel                        *StemModel;
  P3DBranchingAlg                     *BranchingAlg;
  wxTreeItemId                         ParentId;

  ParentId = GetItemParent(GetSelection());

  if (!ParentId.IsOk())
   {
    return;
   }

  ParentBranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(ParentId)))->GetBranchModel();
  ParentStemModel   = dynamic_cast<const P3DStemModelTube*>(ParentBranchModel->GetStemModel());

  if (ParentStemModel == 0)
   {
    return;
   }

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

  StemModel    = new P3DStemModelWings(ParentStemModel);
  BranchingAlg = new P3DBranchingAlgWings();

  unsigned int SubBranchIndex;
  wxTreeItemId ItemId;
  wxTreeItemId ParentItemId;

  SubBranchIndex = 0;
  ItemId         = GetSelection();
  ParentItemId   = GetItemParent(ItemId);

  while ((ItemId = GetPrevSibling(ItemId)).IsOk())
   {
    SubBranchIndex++;
   }

  wxGetApp().ExecEditCmd
   (new ChangeStemModelCommand
         (ParentBranchModel,
          SubBranchIndex,
          BranchModel,
          StemModel,
          BranchingAlg,
          this,
          BranchPanel));
 }

void               P3DPlantModelTreeCtrl::OnSetStemModelGMeshClick
                                      (wxCommandEvent     &event)
 {
  wxTreeItemId                         ItemId;
  wxTreeItemId                         ParentItemId;
  P3DBranchModel                      *BranchModel;
  P3DBranchModel                      *NewBranchModel;
  P3DStemModelGMesh                   *StemModel;
  int                                  PluginIndex;
  const P3DPluginInfoVector           &GMeshPlugins = wxGetApp().GetGMeshPlugins();
  unsigned int                         SubBranchIndex;

  PluginIndex = event.GetId() - wxID_GMESH_PLUGIN_FIRST;

  if ((PluginIndex < 0) || (PluginIndex >= GMeshPlugins.size()))
   {
    return;
   }

  ItemId       = GetSelection();
  ParentItemId = GetItemParent(ItemId);

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(ItemId)))->GetBranchModel();

  P3DGMeshData *MeshData = P3DPlugLuaRunGMeshGenerator(GMeshPlugins[PluginIndex].GetFileName());

  if (MeshData == 0)
   {
    return;
   }

  StemModel = new P3DStemModelGMesh();
  StemModel->SetMeshData(MeshData);

  SubBranchIndex = 0;

  while ((ItemId = GetPrevSibling(ItemId)).IsOk())
   {
    SubBranchIndex++;
   }

  wxGetApp().ExecEditCmd
   (new ChangeStemModelCommand
         (GetBranchModelByItemId(ParentItemId),
          SubBranchIndex,
          BranchModel,
          StemModel,
          (ParentItemId == GetRootItem()) ?
           (P3DBranchingAlg*)new P3DBranchingAlgBase() :
           (P3DBranchingAlg*)new P3DBranchingAlgStd(),
          this,
          BranchPanel));
 }

namespace {

class HideShowEditCommand : public P3DEditCommand
 {
  public           :

                   HideShowEditCommand(P3DMaterialInstanceSimple
                                                          *Material)
   {
    this->Material = Material;
   }

  virtual void     Exec               ()
   {
    Material->SetHidden(!Material->IsHidden());

    wxGetApp().InvalidatePlant();
   }

  virtual void     Undo               ()
   {
    Exec();
   }

  private          :

  P3DMaterialInstanceSimple           *Material;
 };
}

void               P3DPlantModelTreeCtrl::OnHideShowStemClick
                                      (wxCommandEvent     &event)
 {
  P3DBranchModel                      *BranchModel;
  P3DMaterialInstanceSimple           *MaterialSimple;

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();
  MaterialSimple = dynamic_cast<P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

  wxGetApp().ExecEditCmd(new HideShowEditCommand(MaterialSimple));
 }

void               P3DPlantModelTreeCtrl::OnItemRightClick
                                      (wxTreeEvent        &event)
 {
  P3DBranchModel                      *BranchModel;
  P3DMaterialInstanceSimple           *MaterialSimple;

  if (event.GetItem() != GetSelection())
   {
    SelectItem(event.GetItem());
   }

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(GetSelection())))->GetBranchModel();

  wxMenu                               PopupMenu;
  wxMenu                              *StemModelMenu;
  wxMenu                              *AppendBranchMenu;

  P3DStemModel *StemModel = BranchModel->GetStemModel();

  if (StemModel == 0)
   {
   }
  else
   {
    StemModelMenu = new wxMenu();
    StemModelMenu->Append(P3D_SET_STEM_MODEL_TUBE_ID,wxT("Tube"));
    StemModelMenu->Append(P3D_SET_STEM_MODEL_QUAD_ID,wxT("Quad"));
    StemModelMenu->Append(P3D_SET_STEM_MODEL_WINGS_ID,wxT("Wings"));

    if      (dynamic_cast<P3DStemModelTube*>(StemModel) != 0)
     {
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_TUBE_ID,false);
     }
    else if (dynamic_cast<P3DStemModelQuad*>(StemModel) != 0)
     {
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_QUAD_ID,false);
     }
    else if (dynamic_cast<P3DStemModelWings*>(StemModel) != 0)
     {
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_WINGS_ID,false);
     }

    const P3DPluginInfoVector &GMeshPlugins = wxGetApp().GetGMeshPlugins();

    if (GMeshPlugins.size() > 0)
     {
      wxMenu      *GMeshPluginsMenu = new wxMenu();
      int          MenuItemId = wxID_GMESH_PLUGIN_FIRST;

      for (unsigned int Index = 0; Index < GMeshPlugins.size(); Index++)
       {
        GMeshPluginsMenu->Append(MenuItemId++,wxString(GMeshPlugins[Index].GetMenuName(),wxConvUTF8));
       }

      StemModelMenu->Append(P3D_SET_STEM_MODEL_GMESH_ID,wxT("G-Mesh"),GMeshPluginsMenu);
     }
    else
     {
      StemModelMenu->Append(P3D_SET_STEM_MODEL_GMESH_ID,wxT("G-Mesh"));
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_GMESH_ID,false);
     }

    if (BranchModel->GetSubBranchCount() > 0)
     {
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_QUAD_ID,false);
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_WINGS_ID,false);
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_GMESH_ID,false);
     }

    if (GetItemParent(GetSelection()) == GetRootItem())
     {
      StemModelMenu->Enable(P3D_SET_STEM_MODEL_WINGS_ID,false);
     }
   }

  MaterialSimple = dynamic_cast<P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

  AppendBranchMenu = new wxMenu();
  AppendBranchMenu->Append(PLANT_TREE_APPEND_BRANCH_NEW_ID,wxT("New branch"));
  AppendBranchMenu->Append(PLANT_TREE_APPEND_BRANCH_COPY_ID,wxT("Copy branch..."));

  PopupMenu.Append(PLANT_TREE_APPEND_BRANCH_ID,wxT("Append branch"),AppendBranchMenu);
  PopupMenu.Append(PLANT_TREE_REMOVE_STEM_ID,wxT("Delete stem"));

  if (GetItemParent(GetSelection()).IsOk())
   {
    PopupMenu.AppendSeparator();
    PopupMenu.Append(P3D_SET_STEM_MODEL_ID,wxT("Stem model"),StemModelMenu);

    PopupMenu.AppendSeparator();

    if (MaterialSimple->IsHidden())
     {
      PopupMenu.Append(PLANT_TREE_HIDESHOW_STEM_ID,wxT("Show"));
     }
    else
     {
      PopupMenu.Append(PLANT_TREE_HIDESHOW_STEM_ID,wxT("Hide"));
     }
   }

  PopupMenu.Append(PLANT_TREE_RENAME_STEM_ID,wxT("Rename..."));

  if (GetRootItem() == GetSelection())
   {
    PopupMenu.Enable(PLANT_TREE_REMOVE_STEM_ID,false);
   }

  if (BranchModel->GetSubBranchCount() < P3DBranchModelSubBranchMaxCount)
   {
   }
  else
   {
    PopupMenu.Enable(PLANT_TREE_APPEND_BRANCH_ID,false);
   }

  if      (dynamic_cast<P3DStemModelQuad*>(StemModel) != 0)
   {
    PopupMenu.Enable(PLANT_TREE_APPEND_BRANCH_ID,false);
   }
  else if (dynamic_cast<P3DStemModelWings*>(StemModel) != 0)
   {
    PopupMenu.Enable(PLANT_TREE_APPEND_BRANCH_ID,false);
   }

  this->PopupMenu(&PopupMenu,event.GetPoint());
 }

void               P3DPlantModelTreeCtrl::UpdateLabel
                                      (wxTreeItemId        ItemId)
 {
  P3DBranchModel                      *BranchModel;
  wxTreeItemIdValue                    Cookie;

  BranchModel = ((P3DPlantModelTreeCtrlItemData*)(GetItemData(ItemId)))->GetBranchModel();

  SetItemText(ItemId,MakeTreeItemLabel(BranchModel->GetName(),BranchModel));

  wxTreeItemId ChildId = GetFirstChild(ItemId,Cookie);

  while (ChildId.IsOk())
   {
    UpdateLabel(ChildId);

    ChildId = GetNextChild(ItemId,Cookie);
   }
 }

void               P3DPlantModelTreeCtrl::PlantInvalidated
                                      ()
 {
  P3DMathRNGSimple           RNG(wxGetApp().GetModel()->GetBaseSeed());

  UpdateLabel(GetRootItem());
 }

wxString           P3DPlantModelTreeCtrl::MakeTreeItemLabel
                                      (const char         *Prefix,
                                       P3DBranchModel     *BranchModel)
 {
  wxString         Result;

  if ((BranchModel == 0) || (wxGetApp().IsPlantObjectDirty()))
   {
    Result = wxString(Prefix,wxConvUTF8) + wxT(" [?/?]");
   }
  else
   {
    const P3DPlantObject    *PlantObject;

    PlantObject = wxGetApp().GetPlantObject();

    if (BranchModel->GetStemModel() == 0)
     {
      Result = wxString(Prefix,wxConvUTF8) +
                wxString::Format(wxT(" [%d/%d]"),
                                 PlantObject->GetTotalVertexCount(),
                                 PlantObject->GetTotalTriangleCount());
     }
    else
     {
      const P3DPlantModel             *Model;
      const P3DBranchModel            *TempBranchModel;
      unsigned int                     GroupIndex;
      P3DMaterialInstanceSimple       *SimpleMaterial;
      wxString                         StatStr;

      Model = wxGetApp().GetModel();

      GroupIndex      = 0;
      TempBranchModel = P3DPlantModel::GetBranchModelByIndex(Model,GroupIndex);

      while (TempBranchModel != 0)
       {
        if (TempBranchModel == BranchModel) break;

        GroupIndex++;

        TempBranchModel = P3DPlantModel::GetBranchModelByIndex(Model,GroupIndex);
       }

      if (TempBranchModel != 0)
       {
        StatStr = wxString::Format(wxT("[%d/%d]"),
                                   PlantObject->GetGroupVertexCount(GroupIndex),
                                   PlantObject->GetGroupTriangleCount(GroupIndex));
       }
      else
       {
        StatStr = wxT("[?/?]");
       }

      SimpleMaterial = dynamic_cast<P3DMaterialInstanceSimple*>(BranchModel->GetMaterialInstance());

      if (SimpleMaterial->IsHidden())
       {
        StatStr = wxT("[H]") + StatStr;
       }

      Result = wxString(Prefix,wxConvUTF8) + wxT(" ") + StatStr;
     }
   }

  return(Result);
 }


P3DBranchModel    *P3DPlantModelTreeCtrl::GetBranchModelByItemId
                                      (wxTreeItemId        ItemId)
 {
  if (ItemId.IsOk())
   {
    return(((P3DPlantModelTreeCtrlItemData*)(GetItemData(ItemId)))->GetBranchModel());
   }
  else
   {
    return(0);
   }
 }

static void        UpdatePanelMinSize (int                *Width,
                                       int                *Height,
                                       P3DBranchPanel     *Panel,
                                       P3DBranchModel     *BranchModel)
 {
  int                                  TempWidth;
  int                                  TempHeight;

  Panel->SwitchToBranch(BranchModel);

  Panel->GetSize(&TempWidth,&TempHeight);

  if (TempWidth > (*Width))
   {
    *Width = TempWidth;
   }

  if (TempHeight > (*Height))
   {
    *Height = TempHeight;
   }

  for (unsigned int Index = 0; Index < BranchModel->GetSubBranchCount(); Index++)
   {
    UpdatePanelMinSize(Width,Height,Panel,BranchModel->GetSubBranchModel(Index));
   }
 }

static void        CalcPanelMinSize   (int                *Width,
                                       int                *Height,
                                       wxWindow           *Parent,
                                       P3DBranchModel     *PlantBase)
 {
  P3DBranchPanel                      *BranchPanel;

  BranchPanel = new P3DBranchPanel(Parent,PlantBase);

  *Width  = 0;
  *Height = 0;

  UpdatePanelMinSize(Width,Height,BranchPanel,PlantBase);

  delete BranchPanel;
 }

BEGIN_EVENT_TABLE(P3DModelEditPanel,wxPanel)
 EVT_CHECKBOX(P3D_AUTO_UPDATE_ID,P3DModelEditPanel::OnAutoUpdateChanged)
 EVT_BUTTON(P3D_UPDATE_ID,P3DModelEditPanel::OnUpdateClicked)
END_EVENT_TABLE()

void               P3DModelEditPanel::CreateControls
                                      ()
 {
  wxBoxSizer      *TopSizer;
  P3DBranchPanel  *BranchPanel;
  int              MinWidth;
  int              MinHeight;
  wxBoxSizer      *UpdateControlSizer;

  TopSizer = (wxBoxSizer*)GetSizer();

  UpdateControlSizer = new wxBoxSizer(wxHORIZONTAL);

  wxCheckBox *AutoUpdateCheckBox;
  wxButton   *UpdateButton;

  AutoUpdateCheckBox = new wxCheckBox(this,P3D_AUTO_UPDATE_ID,wxT("Auto-update"));
  UpdateButton       = new wxButton(this,P3D_UPDATE_ID,wxT("Update"));

  if (wxGetApp().IsAutoUpdateMode())
   {
    AutoUpdateCheckBox->SetValue(true);
    UpdateButton->Disable();
   }
  else
   {
    AutoUpdateCheckBox->SetValue(false);
   }

  UpdateControlSizer->Add(AutoUpdateCheckBox,1,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
  UpdateControlSizer->Add(UpdateButton,0,wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,0);

  TopSizer->Add(UpdateControlSizer,0,wxGROW | wxALL,2);

  BranchPanel = new P3DBranchPanel(this,wxGetApp().GetModel()->GetPlantBase());

  TopSizer->Add(BranchPanel,0,wxGROW,0);

  CalcPanelMinSize(&MinWidth,&MinHeight,BranchPanel,wxGetApp().GetModel()->GetPlantBase());

  TopSizer->SetItemMinSize(BranchPanel,MinWidth,MinHeight);

  BranchPanel->SetSizeHints(MinWidth,MinHeight);

  PlantModelTreeCtrl = new P3DPlantModelTreeCtrl(this,wxGetApp().GetModel(),BranchPanel);

  ExpandTreeCtrlRecursive(PlantModelTreeCtrl,PlantModelTreeCtrl->GetRootItem());

  PlantModelTreeCtrl->SelectItem(PlantModelTreeCtrl->GetRootItem());

  #if defined(__WXMSW__)
   {
    PlantModelTreeCtrl->Refresh();
   }
  #endif

  TopSizer->Add(PlantModelTreeCtrl,1,wxGROW | wxTOP,1);

  TopSizer->SetItemMinSize(PlantModelTreeCtrl,MinWidth,60);
 }

                   P3DModelEditPanel::P3DModelEditPanel
                                      (wxWindow           *parent)
                   : wxPanel(parent,wxID_ANY,wxDefaultPosition)
 {
  wxBoxSizer       *TopSizer = new wxBoxSizer(wxVERTICAL);

  SetSizer(TopSizer);

  CreateControls();

  TopSizer->Fit(this);
  TopSizer->SetSizeHints(this);
 }

void               P3DModelEditPanel::HideAll
                                      ()
 {
  wxBoxSizer      *TopSizer;

  TopSizer = (wxBoxSizer*)GetSizer();

  TopSizer->Remove(0);
  TopSizer->Remove(1);

  DestroyChildren();

  PlantModelTreeCtrl = NULL;
 }

void               P3DModelEditPanel::RestoreAll
                                      ()
 {
  wxBoxSizer      *TopSizer;

  TopSizer = (wxBoxSizer*)GetSizer();

  CreateControls();

  TopSizer->Layout();
 }

void               P3DModelEditPanel::UpdatePanelMinSize
                                      ()
 {
  HideAll();
  RestoreAll();
 }

void               P3DModelEditPanel::PlantInvalidated
                                      ()
 {
  if (PlantModelTreeCtrl != NULL)
   {
    PlantModelTreeCtrl->PlantInvalidated();
   }
 }

void               P3DModelEditPanel::UpdateControls
                                      ()
 {
  P3DBranchPanel  *BranchPanel;

  BranchPanel = (P3DBranchPanel*)FindWindow(P3D_BRANCH_PANEL_ID);

  if (BranchPanel != NULL)
   {
    BranchPanel->UpdateControls();
   }
 }

void               P3DModelEditPanel::OnAutoUpdateChanged
                                      (wxCommandEvent     &Event)
 {
  wxGetApp().SetAutoUpdateMode(Event.IsChecked());

  wxButton   *UpdateButton = (wxButton*)FindWindow(P3D_UPDATE_ID);

  if (UpdateButton != 0)
   {
    UpdateButton->Enable(!Event.IsChecked());
   }

  if (Event.IsChecked())
   {
    wxGetApp().ForceUpdate();
   }
 }

void               P3DModelEditPanel::OnUpdateClicked
                                      (wxCommandEvent     &Event)
 {
  wxGetApp().ForceUpdate();
 }

