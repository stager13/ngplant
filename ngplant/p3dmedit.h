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

#ifndef __P3DMEDIT_H__
#define __P3DMEDIT_H__

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>

#include <ngpcore/p3dmodel.h>

class P3DBranchPanel : public wxNotebook
 {
  public           :

                   P3DBranchPanel     (wxWindow           *parent,
                                       P3DBranchModel     *BranchModel = 0);

  void             SwitchToBranch     (P3DBranchModel     *BranchModel);

  private          :

  void             CreateTabs         ();

  P3DBranchModel                      *BranchModel;
 };

class P3DPlantModelTreeCtrl : public wxTreeCtrl
 {
  public           :

                   P3DPlantModelTreeCtrl
                                      (wxWindow           *parent,
                                       P3DPlantModel      *PlantModel,
                                       P3DBranchPanel     *BranchPanel);

  void             OnSelectionChanged (wxTreeEvent        &event);
  void             OnItemRightClick   (wxTreeEvent        &event);

  void             OnAppendBranchNewClick
                                      (wxCommandEvent     &event);
  void             OnAppendBranchCopyClick
                                      (wxCommandEvent     &event);
  void             OnRemoveStemClick  (wxCommandEvent     &event);
  void             OnRenameStemClick  (wxCommandEvent     &event);
  void             OnHideShowStemClick(wxCommandEvent     &event);

  void             OnSetStemModelTubeClick
                                      (wxCommandEvent     &event);

  void             OnSetStemModelQuadClick
                                      (wxCommandEvent     &event);

  void             OnSetStemModelWingsClick
                                      (wxCommandEvent     &event);

  void             OnSetStemModelGMeshClick
                                      (wxCommandEvent     &event);

  void             PlantInvalidated   ();

  private          :

  void             AppendChildrenRecursive
                                      (P3DBranchModel     *BranchModel,
                                       wxTreeItemId        BranchItemId);

  wxString         MakeTreeItemLabel  (const char         *Prefix,
                                       P3DBranchModel     *BranchModel);

  void             UpdateLabel        (wxTreeItemId        ItemId);

  P3DBranchPanel  *BranchPanel;

  DECLARE_EVENT_TABLE();
 };

class P3DModelEditPanel : public wxPanel
 {
  public           :

                   P3DModelEditPanel  (wxWindow           *Parent);

  void             HideAll            ();
  void             RestoreAll         ();

  void             UpdatePanelMinSize ();

  void             PlantInvalidated   ();

  void             OnAutoUpdateChanged(wxCommandEvent     &Event);
  void             OnUpdateClicked    (wxCommandEvent     &Event);

  P3DPlantModelTreeCtrl               *PlantModelTreeCtrl;

  private          :

  void             CreateControls     ();

  DECLARE_EVENT_TABLE();
 };

#endif

