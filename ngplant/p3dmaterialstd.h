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

#ifndef __P3DMATERIALSTD_H__
#define __P3DMATERIALSTD_H__

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3diostream.h>

#include <p3dtexture.h>
#include <p3dshaders.h>

class P3DIDEMaterialFactory : public P3DMaterialFactory
 {
  public           :

                   P3DIDEMaterialFactory
                                      (P3DTexManagerGL    *TexManager,
                                       P3DShaderManager   *ShaderManager);

  virtual P3DMaterialInstance
                  *CreateMaterial     (const P3DMaterialDef
                                                          &MaterialDef) const;

  private          :

  P3DTexManagerGL *TexManager;
  P3DShaderManager*ShaderManager;
 };

class P3DMaterialInstanceSimple : public P3DMaterialInstance
 {
  public           :

                   P3DMaterialInstanceSimple
                                      (P3DTexManagerGL    *TexManager,
                                       P3DShaderManager   *ShaderManager,
                                       const P3DMaterialDef
                                                          &MaterialDef);

  virtual         ~P3DMaterialInstanceSimple
                                      ();

  void             GetColor           (float              *R,
                                       float              *G,
                                       float              *B) const;

  void             SetColor           (float               R,
                                       float               G,
                                       float               B);

  P3DTexHandle     GetTexHandle       (unsigned int        Layer) const;
  void             SetTexHandle       (unsigned int        Layer,
                                       P3DTexHandle        TexHandle);

  const char      *GetTexFileName     (unsigned int        Layer) const;

  bool             IsDoubleSided      () const;
  void             SetDoubleSided     (bool                DoubleSided);

  bool             IsTransparent      () const;
  void             SetTransparent     (bool                Transparent);

  bool             IsBillboard        () const;
  unsigned int     GetBillboardMode   () const;
  void             SetBillboardMode   (unsigned int        Mode);

  bool             IsAlphaCtrlEnabled () const;
  void             SetAlphaCtrlState  (bool                Enable);

  float            GetAlphaFadeIn     () const;
  float            GetAlphaFadeOut    () const;

  void             SetAlphaFadeIn     (float               FadeIn);
  void             SetAlphaFadeOut    (float               FadeOut);

  void             SetAlphaTestValue  (float               Value);

  virtual
  const
  P3DMaterialDef  *GetMaterialDef     () const
   {
    return(&MatDef);
   }

  virtual
  P3DMaterialInstance
                  *CreateCopy         () const;

  bool             IsHidden           () const;
  void             SetHidden          (bool                Hidden);

  bool             IsSelected         () const;
  void             SetSelected        (bool                Selected);

  private          :

  P3DTexHandle     TexHandles[P3D_MAX_TEX_LAYERS];
  P3DShaderHandle  ShaderHandle;
  P3DMaterialDef   MatDef;
  P3DTexManagerGL *TexManager;
  P3DShaderManager*ShaderManager;

  float            AlphaTestValue;

  /* next fields must not be here, but material is the only thing which is   */
  /* created outside ngpcore. These fields are used by designer application. */

  bool             Hidden;
  bool             Selected;
 };

class P3DIDEMaterialSaver : public P3DMaterialSaver
 {
  public           :

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMaterialInstance
                                                          *Material) const;
 };

#endif

