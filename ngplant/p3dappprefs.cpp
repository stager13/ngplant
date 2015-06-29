/***************************************************************************

 Copyright (C) 2008  Sergey Prokhorchuk

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

#include <p3dwxcurvectrl.h>

#include <p3dappprefs.h>

static const wxChar    *ExportHiddenGroupsPath      = wxT("Export/ExportHiddenGroups");
static const wxChar    *ExportOutVisRangeGroupsPath = wxT("Export/ExportOutVisRangeGroups");

static bool        wxConfigBaseWriteIntWrapper
                                      (wxConfigBase       *Config,
                                       const wxString     &Key,
                                       int                 Value)
 {
  #ifdef __MINGW32__
  wxString Buffer;

  Buffer.Printf(wxT("%d"),(int)Value);

  return Config->Write(Key,Buffer);
  #else
  return Config->Write(Key,Value);
  #endif
 }

                   P3DExport3DPrefs::P3DExport3DPrefs
                                      ()
 {
  SetDefaults();
 }

unsigned int       P3DExport3DPrefs::IntToMode
                                      (int                 Value)
 {
  if ((Value == P3D_ALWAYS) || (Value == P3D_NEVER)  || (Value == P3D_ASK))
   {
    return(Value);
   }
  else
   {
    return(P3D_ASK);
   }
 }

void               P3DExport3DPrefs::Read
                                      (const wxConfigBase *Config)
 {
  int              CfgParamInt;

  SetDefaults();

  if (Config->Read(ExportHiddenGroupsPath,&CfgParamInt))
   {
    HiddenGroupsExportMode = IntToMode(CfgParamInt);
   }

  if (Config->Read(ExportOutVisRangeGroupsPath,&CfgParamInt))
   {
    OutVisRangeExportMode = IntToMode(CfgParamInt);
   }
 }

void               P3DExport3DPrefs::Save
                                      (wxConfigBase       *Config) const
 {
  wxConfigBaseWriteIntWrapper(Config,ExportHiddenGroupsPath,HiddenGroupsExportMode);
  wxConfigBaseWriteIntWrapper(Config,ExportOutVisRangeGroupsPath,OutVisRangeExportMode);
 }

void               P3DExport3DPrefs::SetDefaults
                                      ()
 {
  HiddenGroupsExportMode = P3D_ASK;
  OutVisRangeExportMode  = P3D_ASK;
 }

static const wxChar    *RotationModePath  = wxT("/3DView/RotationMode");
static const wxChar    *MouseRotXSensPath = wxT("/3DView/MouseRotXSens");
static const wxChar    *MouseRotYSensPath = wxT("/3DView/MouseRotYSens");
static const wxChar    *EmulateNumpadPath = wxT("/3DView/EmulateNumpad");

                   P3DCameraControlPrefs::P3DCameraControlPrefs
                                           ()
 {
  Init();
 }

void               P3DCameraControlPrefs::Init
                                      ()
 {
  MouseRotYCS   = false;
  MouseRotXSens = 0.02f;
  MouseRotYSens = 0.02f;
  EmulateNumpad = false;
 }

void               P3DCameraControlPrefs::Read
                                      (const wxConfigBase *Config)
 {
  int              ParamInt;
  double           ParamDouble;

  Init();

  if (Config->Read(RotationModePath,&ParamInt) &&
      (ParamInt >= 0) && (ParamInt < 2))
   {
    if (ParamInt == 0)
     {
      MouseRotYCS = false; /* turntable */
     }
    else
     {
      MouseRotYCS = true; /* trackball */
     }
   }

  if (Config->Read(MouseRotXSensPath,&ParamDouble))
   {
    MouseRotXSens = (float)ParamDouble;
   }

  if (Config->Read(MouseRotYSensPath,&ParamDouble))
   {
    MouseRotYSens = (float)ParamDouble;
   }

  if (Config->Read(EmulateNumpadPath,&ParamInt))
   {
    EmulateNumpad = (bool)ParamInt;
   }
 }

void               P3DCameraControlPrefs::Save
                                      (wxConfigBase       *Config) const
 {
  wxConfigBaseWriteIntWrapper(Config,RotationModePath,MouseRotYCS ? 1 : 0);
  Config->Write(MouseRotXSensPath,MouseRotXSens);
  Config->Write(MouseRotYSensPath,MouseRotYSens);
  wxConfigBaseWriteIntWrapper(Config,EmulateNumpadPath,EmulateNumpad ? 1 : 0);
 }

static const wxChar    *GroundVisiblePath   = wxT("/3DView/GroundVisible");
static const wxChar    *GroundColorPath     = wxT("/3DView/GroundColor");
static const wxChar    *BackgroundColorPath = wxT("/3DView/BackgroundColor");

                   P3D3DViewPrefs::P3D3DViewPrefs
                                      ()
 {
  SetDefaults();
 }

void               P3D3DViewPrefs::Read
                                      (const wxConfigBase *Config)
 {
  wxString         ColorStr;
  int              CfgParamInt;

  SetDefaults();

  if (Config->Read(GroundVisiblePath,&CfgParamInt))
   {
    GroundVisible = (bool)CfgParamInt;
   }

  if (Config->Read(GroundColorPath,&ColorStr))
   {
    ColorFromStr(&GroundColor,ColorStr);
   }

  if (Config->Read(BackgroundColorPath,&ColorStr))
   {
    ColorFromStr(&BackgroundColor,ColorStr);
   }
 }

void               P3D3DViewPrefs::Save
                                      (wxConfigBase       *Config) const
 {
  wxConfigBaseWriteIntWrapper(Config,GroundVisiblePath,GroundVisible);
  Config->Write(GroundColorPath,ColorToStr(&GroundColor));
  Config->Write(BackgroundColorPath,ColorToStr(&BackgroundColor));
 }

void               P3D3DViewPrefs::SetDefaults
                                      ()
 {
  GroundVisible   = true;
  GroundColor     = P3DAppColor3b(0x80,0x80,0x80);
  BackgroundColor = P3DAppColor3b(0x00,0x00,0x00);
 }

bool               P3D3DViewPrefs::ColorFromStr
                                      (P3DAppColor3b      *Color,
                                       const wxString     &Str)
 {
  long R,G,B;

  if (Str.Length() != 6)
   {
    return(false);
   }

  if ((Str.Mid(0,2).ToLong(&R,16)) &&
      (Str.Mid(2,2).ToLong(&G,16)) &&
      (Str.Mid(4,2).ToLong(&B,16)))
   {
    Color->R = (unsigned char)R;
    Color->G = (unsigned char)G;
    Color->B = (unsigned char)B;

    return(true);
   }
  else
   {
    return(false);
   }
 }

wxString           P3D3DViewPrefs::ColorToStr
                                      (const P3DAppColor3b*Color)
 {
  wxString         Result;

  Result.Printf(wxT("%02X%02X%02X"),
                (unsigned int)Color->R,
                (unsigned int)Color->G,
                (unsigned int)Color->B);

  return(Result);
 }


static const wxChar    *UICurveCtrlWidthPath  = wxT("/UI/CurveCtrl/Width");
static const wxChar    *UICurveCtrlHeightPath = wxT("/UI/CurveCtrl/Height");

void               P3DUIControlsPrefs::Read
                                      (const wxConfigBase *Config)
 {
  int              ParamInt;

  if (Config->Read(UICurveCtrlWidthPath,&ParamInt) &&
      (ParamInt >= P3DCurveCtrlMinWidth))
   {
    P3DCurveCtrl::BestWidth = (unsigned int)ParamInt;
   }

  if (Config->Read(UICurveCtrlHeightPath,&ParamInt) &&
      (ParamInt >= P3DCurveCtrlMinHeight))
   {
    P3DCurveCtrl::BestHeight = (unsigned int)ParamInt;
   }
 }

void               P3DUIControlsPrefs::Save
                                      (wxConfigBase       *Config)
 {
  wxConfigBaseWriteIntWrapper(Config,UICurveCtrlWidthPath,P3DCurveCtrl::BestWidth);
  wxConfigBaseWriteIntWrapper(Config,UICurveCtrlHeightPath,P3DCurveCtrl::BestHeight);
 }

static const wxChar    *RenderQuirksUseColorArrayPath  = wxT("/RenderQuirks/UseColorArray");

                   P3DRenderQuirksPrefs::P3DRenderQuirksPrefs
                                      ()
 {
  SetDefaults();
 }

void               P3DRenderQuirksPrefs::Read
                                      (const wxConfigBase *Config)
 {
  int              ParamInt;

  SetDefaults();

  if (Config->Read(RenderQuirksUseColorArrayPath,&ParamInt))
   {
    UseColorArray = ParamInt;
   }
 }

void               P3DRenderQuirksPrefs::Save
                                      (wxConfigBase       *Config) const
 {
  wxConfigBaseWriteIntWrapper(Config,RenderQuirksUseColorArrayPath,UseColorArray ? 1 : 0);
 }

void               P3DRenderQuirksPrefs::SetDefaults
                                      ()
 {
  UseColorArray = false;
 }

static const wxChar    *TubeCrossSectResolution0Path  = wxT("/Model/Tube/CrossResolution0");
static const wxChar    *TubeCrossSectResolution1Path  = wxT("/Model/Tube/CrossResolution1");
static const wxChar    *TubeCrossSectResolution2Path  = wxT("/Model/Tube/CrossResolution2");

                   P3DModelPrefs::P3DModelPrefs
                                      ()
 {
  SetDefaults();
 }

void               P3DModelPrefs::Read(const wxConfigBase *Config)
 {
  int              CfgParamInt;

  SetDefaults();

  if (Config->Read(TubeCrossSectResolution0Path,&CfgParamInt))
   {
    TubeCrossSectResolution[0] = CfgParamInt < 3 ? 3 : CfgParamInt;
   }

  if (Config->Read(TubeCrossSectResolution1Path,&CfgParamInt))
   {
    TubeCrossSectResolution[1] = CfgParamInt < 3 ? 3 : CfgParamInt;
   }

  if (Config->Read(TubeCrossSectResolution2Path,&CfgParamInt))
   {
    TubeCrossSectResolution[2] = CfgParamInt < 3 ? 3 : CfgParamInt;
   }
 }

void               P3DModelPrefs::Save(wxConfigBase       *Config) const
 {
  wxConfigBaseWriteIntWrapper(Config,TubeCrossSectResolution0Path,TubeCrossSectResolution[0]);
  wxConfigBaseWriteIntWrapper(Config,TubeCrossSectResolution1Path,TubeCrossSectResolution[1]);
  wxConfigBaseWriteIntWrapper(Config,TubeCrossSectResolution2Path,TubeCrossSectResolution[2]);
 }

void               P3DModelPrefs::SetDefaults
                                      ()
 {
  TubeCrossSectResolution[0] = 8;
  TubeCrossSectResolution[1] = 6;
  TubeCrossSectResolution[2] = 4;
 }

