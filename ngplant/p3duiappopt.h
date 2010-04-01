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

#ifndef __P3DUIAPPOPT_H__
#define __P3DUIAPPOPT_H__

#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/notebook.h>

#include <ngplant/p3dapp.h>
#include <ngplant/p3dappprefs.h>

class P3DAppOptDialog : public wxDialog
 {
  public           :

                   P3DAppOptDialog    ();
                   P3DAppOptDialog    (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Preferences"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void             Init               ();

  bool             Create             (wxWindow           *Parent,
                                       wxWindowID          Id = wxID_ANY,
                                       const wxString     &Caption = wxT("Preferences"),
                                       const wxPoint      &Pos = wxDefaultPosition,
                                       const wxSize       &Size = wxDefaultSize,
                                       long                Style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

  void             CreateControls     ();
  void             CreateTexPathsPage (wxNotebook         *Notebook);
  void             CreateExportPage   (wxNotebook         *Notebook);
  void             CreateCameraControlPage
                                      (wxNotebook         *Notebook);
  void             CreateMiscPage     (wxNotebook         *Notebook);

  bool             TransferDataToWindow
                                      ();
  bool             TransferDataFromWindow
                                      ();

  /* event handlers */

  void             OnTexPathMoveUpUpdate   (wxUpdateUIEvent    &Event);
  void             OnTexPathMoveDownUpdate (wxUpdateUIEvent    &Event);
  void             OnTexPathRemoveUpdate   (wxUpdateUIEvent    &Event);
  void             OnTexPathAppendUpdate   (wxUpdateUIEvent    &Event);
  void             OnTexPathUpdateUpdate   (wxUpdateUIEvent    &Event);

  void             OnTexPathMoveUpClick    (wxCommandEvent     &Event);
  void             OnTexPathMoveDownClick  (wxCommandEvent     &Event);
  void             OnTexPathRemoveClick    (wxCommandEvent     &Event);
  void             OnTexPathAppendClick    (wxCommandEvent     &Event);
  void             OnTexPathUpdateClick    (wxCommandEvent     &Event);
  void             OnTexPathBrowseClick    (wxCommandEvent     &Event);

  void             OnGroundColorClick      (wxCommandEvent     &Event);
  void             OnBackgroundColorClick  (wxCommandEvent     &Event);
  void             OnPluginsPathBrowseClick(wxCommandEvent     &Event);

  /* data access methods */

  unsigned int     GetTexPathsCount   () const;
  const char      *GetTexPath         (unsigned int        Index) const;

  void             AddTexPath         (const char         *TexPath);

  void             GetGroundColor     (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const;

  void             SetGroundColor     (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B);

  void             GetBackgroundColor (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const;

  void             SetBackgroundColor (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B);

  bool             GetGroundVisible   () const;
  void             SetGroundVisible   (bool                Visible);

  void             SetExport3DPrefs   (const P3DExport3DPrefs
                                                          *Prefs);
  void             GetExport3DPrefs   (P3DExport3DPrefs   *Prefs);

  const P3DCameraControlPrefs
                  &GetCameraControlPrefs
                                      () const;
  void             SetCameraControlPrefs
                                      (const P3DCameraControlPrefs
                                                          &Prefs);

  const P3DRenderQuirksPrefs
                  &GetRenderQuirksPrefs
                                      () const;
  void             SetRenderQuirksPrefs
                                      (const P3DRenderQuirksPrefs
                                                          &Prefs);

  void             SetPluginsPath     (const wxString     &PluginsPath);
  const wxString  &GetPluginsPath     () const;

  void             SetCurveCtrlPrefs  (unsigned int        BestWidth,
                                       unsigned int        BestHeight);

  void             GetCurveCtrlPrefs  (unsigned int       *BestWidth,
                                       unsigned int       *BestHeight) const;

  private          :

  wxNotebook                          *OptsNotebook;

  std::vector<std::string>             TexPaths;

  P3DAppColor3b                        GroundColor;
  bool                                 GroundVisible;

  P3DAppColor3b                        BackgroundColor;

  P3DExport3DPrefs                     Export3DPrefs;
  P3DCameraControlPrefs                CameraControlPrefs;
  P3DRenderQuirksPrefs                 RenderQuirksPrefs;

  wxString                             PluginsPath;

  unsigned int                         CurveCtrlBestWidth;
  unsigned int                         CurveCtrlBestHeight;

  DECLARE_CLASS(P3DAppOptDialog)
  DECLARE_EVENT_TABLE()
 };

#endif

