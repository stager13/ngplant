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

static wxChar     *RotationModePath  = wxT("/3DView/RotationMode");
static wxChar     *MouseRotXSensPath = wxT("/3DView/MouseRotXSens");
static wxChar     *MouseRotYSensPath = wxT("/3DView/MouseRotYSens");

                   P3DCameraControlPrefs::P3DCameraControlPrefs
                                           ()
 {
  Init();
 }

void               P3DCameraControlPrefs::Init
                                      ()
 {
  MouseRotYCS   = false;
  MouseRotXSens = 0.02;
  MouseRotYSens = 0.02;
 }

void               P3DCameraControlPrefs::Read
                                      (const wxConfigBase *Config)
 {
  long             ParamLong;
  double           ParamDouble;

  Init();

  if (Config->Read(RotationModePath,&ParamLong) &&
      (ParamLong >= 0) && (ParamLong < 2))
   {
    if (ParamLong == 0)
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
 }

void               P3DCameraControlPrefs::Save
                                      (wxConfigBase       *Config) const
 {
  Config->Write(RotationModePath,(long)(MouseRotYCS ? 1 : 0));
  Config->Write(MouseRotXSensPath,MouseRotXSens);
  Config->Write(MouseRotYSensPath,MouseRotYSens);
 }

static wxChar     *UICurveCtrlWidthPath  = wxT("/UI/CurveCtrl/Width");
static wxChar     *UICurveCtrlHeightPath = wxT("/UI/CurveCtrl/Height");

void               P3DUIControlsPrefs::Read
                                      (const wxConfigBase *Config)
 {
  long             ParamLong;

  if (Config->Read(UICurveCtrlWidthPath,&ParamLong) &&
      (ParamLong >= P3DCurveCtrlMinWidth))
   {
    P3DCurveCtrl::BestWidth = (unsigned int)ParamLong;
   }

  if (Config->Read(UICurveCtrlHeightPath,&ParamLong) &&
      (ParamLong >= P3DCurveCtrlMinHeight))
   {
    P3DCurveCtrl::BestHeight = (unsigned int)ParamLong;
   }
 }

void               P3DUIControlsPrefs::Save
                                      (wxConfigBase       *Config)
 {
  Config->Write(UICurveCtrlWidthPath,(long)P3DCurveCtrl::BestWidth);
  Config->Write(UICurveCtrlHeightPath,(long)P3DCurveCtrl::BestHeight);
 }

