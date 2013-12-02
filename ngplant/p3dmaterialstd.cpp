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

#include <string.h>

#include <string>

#include <wx/wx.h>

#include <ngput/p3dglext.h>

#include <ngpcore/p3dmodel.h>

#include <p3dapp.h>
#include <p3dtexture.h>

#include <p3dmaterialstd.h>

                   P3DIDEMaterialFactory::P3DIDEMaterialFactory
                                      (P3DTexManagerGL    *TexManager,
                                       P3DShaderManager   *ShaderManager)
 {
  this->TexManager    = TexManager;
  this->ShaderManager = ShaderManager;
 }

P3DMaterialInstance
                  *P3DIDEMaterialFactory::CreateMaterial
                                      (const P3DMaterialDef
                                                          &MaterialDef) const
 {
  return(new P3DMaterialInstanceSimple(TexManager,ShaderManager,MaterialDef));
 }

                   P3DMaterialInstanceSimple::P3DMaterialInstanceSimple
                                      (P3DTexManagerGL    *TexManager,
                                       P3DShaderManager   *ShaderManager,
                                       const P3DMaterialDef
                                                          &MaterialDef)
                   : MatDef(MaterialDef)
 {
  unsigned int     TexLayer;

  this->TexManager    = TexManager;
  this->ShaderManager = ShaderManager;

  Hidden   = false;
  Selected = false;

  AlphaTestValue = 0.5f;

  for (TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    TexHandles[TexLayer] = P3DTexHandleNULL;
   }

  for (TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    if (MatDef.GetTexName(TexLayer) != NULL)
     {
      std::string SystemName = P3DApp::GetApp()->GetTexFS()->Generic2System(MatDef.GetTexName(TexLayer));

      if (SystemName.length() > 0)
       {
        wxString                       ErrorMessage;

        TexHandles[TexLayer] = TexManager->LoadFromFile(SystemName.c_str(),ErrorMessage);
       }

      if (TexHandles[TexLayer] == P3DTexHandleNULL)
       {
        MatDef.SetTexName(TexLayer,NULL);
       }
     }
   }

  ShaderHandle = ShaderManager->GenShader
                  (TexHandles[P3D_TEX_DIFFUSE] != P3DTexHandleNULL,
                   TexHandles[P3D_TEX_NORMAL_MAP] != P3DTexHandleNULL,
                   MatDef.IsDoubleSided());
 }

                   P3DMaterialInstanceSimple::~P3DMaterialInstanceSimple
                                      ()
 {
  unsigned int     TexLayer;

  for (TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
   {
    if (TexHandles[TexLayer] != P3DTexHandleNULL)
     {
      TexManager->FreeTexture(TexHandles[TexLayer]);
     }
   }

  if (ShaderHandle != P3DShaderHandleNULL)
   {
    ShaderManager->FreeShader(ShaderHandle);
   }
 }

P3DMaterialInstance
                  *P3DMaterialInstanceSimple::CreateCopy
                                      () const
 {
  return(new P3DMaterialInstanceSimple(TexManager,ShaderManager,MatDef));
 }

void               P3DMaterialInstanceSimple::GetColor
                                      (float              *R,
                                       float              *G,
                                       float              *B) const
 {
  MatDef.GetColor(R,G,B);
 }

void               P3DMaterialInstanceSimple::SetColor
                                      (float               R,
                                       float               G,
                                       float               B)
 {
  MatDef.SetColor(R,G,B);
 }

P3DTexHandle       P3DMaterialInstanceSimple::GetTexHandle
                                      (unsigned int        Layer) const
 {
  return(TexHandles[Layer]);
 }

void               P3DMaterialInstanceSimple::SetTexHandle
                                      (unsigned int        Layer,
                                       P3DTexHandle        TexHandle)
 {
  bool                                 UpdateShader;

  UpdateShader = false;

  if (TexHandles[Layer] != TexHandle)
   {
    if (TexHandles[Layer] != P3DTexHandleNULL)
     {
      TexManager->FreeTexture(TexHandles[Layer]);
     }

    if (TexHandle == P3DTexHandleNULL)
     {
      MatDef.SetTexName(Layer,NULL);
     }
    else
     {
      MatDef.SetTexName(Layer,TexManager->GetGenericName(TexHandle));
     }

    if ((Layer == P3D_TEX_DIFFUSE) || (Layer == P3D_TEX_NORMAL_MAP))
     {
      if ((TexHandles[Layer] == P3DTexHandleNULL) ||
          (TexHandle         == P3DTexHandleNULL))
       {
        UpdateShader = true;
       }
     }

    TexHandles[Layer] = TexHandle;
   }

  if (UpdateShader)
   {
    if (ShaderHandle != P3DShaderHandleNULL)
     {
      ShaderManager->FreeShader(ShaderHandle);
     }

    ShaderHandle = ShaderManager->GenShader
                    (TexHandles[P3D_TEX_DIFFUSE] != P3DTexHandleNULL,
                     TexHandles[P3D_TEX_NORMAL_MAP] != P3DTexHandleNULL,
                     MatDef.IsDoubleSided());
   }
 }

const char        *P3DMaterialInstanceSimple::GetTexFileName
                                      (unsigned int        Layer) const
 {
  if (TexHandles[Layer] == P3DTexHandleNULL)
   {
    return(0);
   }
  else
   {
    return(TexManager->GetTexFileName(TexHandles[Layer]));
   }
 }

bool               P3DMaterialInstanceSimple::IsDoubleSided
                                      () const
 {
  return(MatDef.IsDoubleSided());
 }

void               P3DMaterialInstanceSimple::SetDoubleSided
                                      (bool                DoubleSided)
 {
  if (DoubleSided != MatDef.IsDoubleSided())
   {
    MatDef.SetDoubleSided(DoubleSided);

    if (ShaderHandle != P3DShaderHandleNULL)
     {
      ShaderManager->FreeShader(ShaderHandle);
     }

    ShaderHandle = ShaderManager->GenShader
                    (TexHandles[P3D_TEX_DIFFUSE] != P3DTexHandleNULL,
                     TexHandles[P3D_TEX_NORMAL_MAP] != P3DTexHandleNULL,
                     DoubleSided);
   }
 }

bool               P3DMaterialInstanceSimple::IsTransparent
                                      () const
 {
  return(MatDef.IsTransparent());
 }

void               P3DMaterialInstanceSimple::SetTransparent
                                      (bool                Transparent)
 {
  MatDef.SetTransparent(Transparent);
 }

bool               P3DMaterialInstanceSimple::IsBillboard
                                      () const
 {
  return(MatDef.IsBillboard());
 }

unsigned int       P3DMaterialInstanceSimple::GetBillboardMode
                                      () const
 {
  return(MatDef.GetBillboardMode());
 }

void               P3DMaterialInstanceSimple::SetBillboardMode
                                      (unsigned int        Mode)
 {
  MatDef.SetBillboardMode(Mode);
 }

bool               P3DMaterialInstanceSimple::IsAlphaCtrlEnabled
                                      () const
 {
  return(MatDef.IsAlphaCtrlEnabled());
 }

void               P3DMaterialInstanceSimple::SetAlphaCtrlState
                                      (bool                Enable)
 {
  MatDef.SetAlphaCtrlState(Enable);
 }

float              P3DMaterialInstanceSimple::GetAlphaFadeIn
                                      () const
 {
  return(MatDef.GetAlphaFadeIn());
 }

float              P3DMaterialInstanceSimple::GetAlphaFadeOut
                                      () const
 {
  return(MatDef.GetAlphaFadeOut());
 }

void               P3DMaterialInstanceSimple::SetAlphaFadeIn
                                      (float               FadeIn)
 {
  MatDef.SetAlphaFadeIn(FadeIn);
 }

void               P3DMaterialInstanceSimple::SetAlphaFadeOut
                                      (float               FadeOut)
 {
  MatDef.SetAlphaFadeOut(FadeOut);
 }

void               P3DMaterialInstanceSimple::SetAlphaTestValue
                                      (float               Value)
 {
  AlphaTestValue = P3DMath::Clampf(0.0f,1.0f,Value);
 }

bool               P3DMaterialInstanceSimple::IsHidden
                                      () const
 {
  return(Hidden);
 }

void               P3DMaterialInstanceSimple::SetHidden
                                      (bool                Hidden)
 {
  this->Hidden = Hidden;
 }

bool               P3DMaterialInstanceSimple::IsSelected
                                      () const
 {
  return(Selected);
 }

void               P3DMaterialInstanceSimple::SetSelected
                                      (bool                Selected)
 {
  this->Selected = Selected;
 }

void               P3DIDEMaterialSaver::Save
                                      (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMaterialInstance
                                                          *Material) const
 {
  ((P3DMaterialInstanceSimple*)Material)->GetMaterialDef()->Save(TargetStream);
 }

