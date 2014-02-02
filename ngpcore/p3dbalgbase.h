/***************************************************************************

 Copyright (c) 2007 Sergey Prokhorchuk.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the author nor the names of contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

***************************************************************************/

#ifndef __P3DBALGBASE_H__
#define __P3DBALGBASE_H__

#include <ngpcore/p3dmodel.h>

class P3DBranchingAlgBase : public P3DBranchingAlg
 {
  public           :

  enum { SQUARE_SHAPE, CIRCLE_SHAPE };

                   P3DBranchingAlgBase();

  virtual P3DBranchingAlg
                  *CreateCopy         () const;

  unsigned int     GetShape           () const;
  void             SetShape           (unsigned int        Shape);

  float            GetSpread          () const;
  void             SetSpread          (float               Spread);

  float            GetDensity         () const;
  void             SetDensity         (float               Density);

  float            GetDensityV        () const;
  void             SetDensityV        (float               DensityV);

  unsigned int     GetMinNumber       () const;
  void             SetMinNumber       (unsigned int        MinNumber);

  bool             IsMaxLimitEnabled  () const;
  void             SetMaxLimitEnabled (bool                IsEnabled);
  unsigned int     GetMaxNumber       () const;
  void             SetMaxNumber       (unsigned int        MaxNumber);

  float            GetDeclFactor     () const;
  void             SetDeclFactor     (float                DeclFactor);

  float            GetDeclFactorV    () const;
  void             SetDeclFactorV    (float                DeclFactorV);

  float            GetRotationAngle   () const;
  void             SetRotationAngle   (float               RotAngle);

  virtual void     CreateBranches     (P3DBranchingFactory          *Factory,
                                       const P3DStemModelInstance   *Parent,
                                       P3DMathRNG                   *RNG);

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream) const;

  virtual void     Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version);

  static void      MakeBranchWorldMatrix
                                      (float                        *WorldTransform,
                                       const P3DVector3f            *Offset,
                                       const P3DQuaternionf         *Orientation);

  private          :

  unsigned int     CalcBranchCount    (P3DMathRNG         *RNG) const;
  void             GenOffsetSquare    (float              *X,
                                       float              *Y,
                                       P3DMathRNG         *RNG) const;
  void             GenOffsetCircle    (float              *X,
                                       float              *Y,
                                       P3DMathRNG         *RNG) const;
  void             GenBranchOffset    (P3DVector3f        *Offset,
                                       P3DMathRNG         *RNG) const;

  void             CalcBranchOrientationInCircleShape
                                      (P3DQuaternionf     *Orientation,
                                       float               X,
                                       float               Z,
                                       P3DMathRNG         *RNG) const;

  unsigned int                         Shape;
  float                                Spread;

  float                                Density; /* branches per square meter */
  float                                DensityV;
  unsigned int                         MinNumber; /* min. number of branches */
  bool                                 MaxLimitEnabled; /* if false, MaxNumber is not used */
  unsigned int                         MaxNumber; /* max. number of branches */

  float                                DeclFactor;
  float                                DeclFactorV;

  float                                Rotation;
 };

#endif

