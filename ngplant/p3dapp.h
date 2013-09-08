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

#ifndef __P3DAPP_H__
#define __P3DAPP_H__

#include <vector>

#include <wx/wx.h>

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemquad.h>

#include <p3dtexture.h>
#include <p3dshaders.h>
#include <p3dmaterialstd.h>
#include <p3didevfs.h>
#include <p3dmedit.h>
#include <p3dpobject.h>
#include <p3dcanvas3d.h>
#include <p3dpluginfo.h>
#include <p3dappprefs.h>
#include <p3dcmdqueue.h>

class P3DMainFrame : public wxFrame
 {
  public           :

                   P3DMainFrame       (const wxChar       *title);

  void             OnNew              (wxCommandEvent     &event);
  void             OnOpen             (wxCommandEvent     &event);
  void             OnSave             (wxCommandEvent     &event);
  void             OnSaveAs           (wxCommandEvent     &event);
  void             OnExportObj        (wxCommandEvent     &event);
  void             OnExportObjPlugin  (wxCommandEvent     &event);
  void             OnRunScript        (wxCommandEvent     &event);
  void             OnQuit             (wxCommandEvent     &event);
  void             OnFrameClose       (wxCloseEvent       &event);
  void             OnAbout            (wxCommandEvent     &event);
  void             OnEditPreferences  (wxCommandEvent     &event);

  void             OnUndo             (wxCommandEvent     &event);
  void             OnRedo             (wxCommandEvent     &event);
  void             UpdateControls     ();
  bool             ApproveDataLoss    ();

  void             InvalidatePlant    ();

  bool             IsGLExtInited      () const
   {
    return(Canvas3D->IsGLExtInited());
   }

  P3DModelEditPanel                   *EditPanel;

  private          :

  P3DCanvas3D                         *Canvas3D;

  DECLARE_EVENT_TABLE()
 };

enum
 {
  P3D_BITMAP_NO_TEXTURE = 0,
  P3D_BITMAP_REMOVE_TEXTURE,

  P3D_TOTAL_BITMAPS
 };

class P3DApp : public wxApp
 {
  public           :

  virtual         ~P3DApp             ();

  virtual bool     OnInit             ();

  P3DPlantModel   *GetModel           ();
  void             SetModel           (P3DPlantModel      *Model);
  void             SaveModel          (const char         *FileName);

  const P3DPlantObject
                  *GetPlantObject     () const;
  P3DTexManagerGL *GetTexManager      ();
  P3DShaderManager*GetShaderManager   ();

  P3DBranchingAlg *CreateBranchingAlgStd
                                      () const;

  P3DStemModelTube*CreateStemModelTube() const;
  P3DStemModelQuad*CreateStemModelQuad() const;

  P3DMaterialInstanceSimple
                  *CreateMatInstanceStd
                                      () const;

  void             InvalidatePlant    ();
  void             InvalidateCamera   ();
  void             ForceUpdate        ();
  bool             IsPlantObjectDirty () const;
  bool             HasUnsavedChanges  () const;

  wxString         GetFileName        () const;
  void             SetFileName        (const char         *FileName);

  P3DIDEVFS       *GetTexFS           ();

  const wxBitmap  &GetBitmap          (unsigned int        Bitmap);

  P3DPlantModel   *CreateNewPlantModel() const;

  void             ExecEditCmd        (P3DEditCommand     *Cmd);
  void             Undo               ();
  void             Redo               ();
  void             UpdateControls     ();

  /* Some preferences stuff */

  void             GetGroundColor     (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const;

  void             SetGroundColor     (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B);

  bool             IsGroundVisible    () const;
  void             SetGroundVisibility(bool                Visible);

  void             GetBackgroundColor (unsigned char      *R,
                                       unsigned char      *G,
                                       unsigned char      *B) const;

  void             SetBackgroundColor (unsigned char       R,
                                       unsigned char       G,
                                       unsigned char       B);

  P3DExport3DPrefs*GetExport3DPrefs   ();

  const P3DCameraControlPrefs
                  *GetCameraControlPrefs
                                      () const;
  void             SetCameraControlPrefs
                                      (const P3DCameraControlPrefs
                                                          *Prefs);
  const P3DRenderQuirksPrefs
                  &GetRenderQuirksPrefs
                                      () const;
  void             SetRenderQuirksPrefs
                                      (const P3DRenderQuirksPrefs
                                                          &Prefs);

  const P3DModelPrefs
                  &GetModelPrefs      () const;
  void             SetModelPrefs      (const P3DModelPrefs&Prefs);

  void             SetPluginsPath     (const wxString     &PluginsPath);
  const wxString  &GetPluginsPath     () const;
  void             ScanPlugins        ();
  const P3DPluginInfoVector
                  &GetExportPlugins   () const;
  const P3DPluginInfoVector
                  &GetGMeshPlugins    () const;

  float            GetLODLevel        () const;
  void             SetLODLevel        (float               Level);

  bool             IsAutoUpdateMode   () const;
  void             SetAutoUpdateMode  (bool                Enable);

  bool             IsShadersEnabled   () const;
  void             DisableShaders     ();

  static P3DApp   *GetApp             ();

  private          :

  void             InitTexFS          ();

  virtual void     OnInitCmdLine      (wxCmdLineParser    &Parser);
  virtual bool     OnCmdLineParsed    (wxCmdLineParser    &Parser);

  P3DPlantModel   *PlantModel;
  mutable
  P3DPlantObject  *PlantObject;
  mutable bool     PlantObjectDirty;
  bool             PlantObjectAutoUpdate;
  bool             UnsavedChanges;

  P3DTexManagerGL  TexManager;
  P3DShaderManager ShaderManager;

  wxBitmap         Bitmaps[P3D_TOTAL_BITMAPS];

  P3DIDEVFS        TexFS;
  P3DMainFrame    *MainFrame;
  wxString         PlantFileName;

  P3D3DViewPrefs   View3DPrefs;

  P3DExport3DPrefs           Export3DPrefs;
  P3DCameraControlPrefs      CameraControlPrefs;
  P3DRenderQuirksPrefs       RenderQuirks;
  P3DModelPrefs              ModelPrefs;

  wxString         PluginsPath;
  P3DPluginInfoVector                  ExportPlugins;
  P3DPluginInfoVector                  GMeshPlugins;

  float            LODLevel;

  bool             UseShaders;

  P3DEditCommandQueue                 *CommandQueue;

  static P3DApp   *SelfPtr;
 };

DECLARE_APP(P3DApp)

#endif

