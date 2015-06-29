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

#include <ngpcore/p3dmath.h>
#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dbalgbase.h>

static bool        IsPointInsideCircle(float X, float Y, float R)
 {
  return X * X + Y * Y <= R * R;
 }

                   P3DBranchingAlgBase::P3DBranchingAlgBase
                                      ()
 {
  Shape           = SQUARE_SHAPE;
  Spread          = 0.0f;
  Density         = 1.0f;
  DensityV        = 0.0f;
  MinNumber       = 1;
  MaxLimitEnabled = true;
  MaxNumber       = 1;

  DeclFactor  = 0.0f;
  DeclFactorV = 0.0f;

  Rotation = 0.0f;
 }

P3DBranchingAlg   *P3DBranchingAlgBase::CreateCopy
                                      () const
 {
  P3DBranchingAlgBase                 *Result;

  Result = new P3DBranchingAlgBase();

  Result->Shape           = Shape;
  Result->Spread          = Spread;
  Result->Density         = Density;
  Result->DensityV        = DensityV;
  Result->MinNumber       = MinNumber;
  Result->MaxLimitEnabled = MaxLimitEnabled;
  Result->MaxNumber       = MaxNumber;

  Result->DeclFactor  = DeclFactor;
  Result->DeclFactorV = DeclFactorV;

  Result->Rotation = Rotation;

  return(Result);
 }

unsigned int       P3DBranchingAlgBase::GetShape
                                      () const
 {
  return Shape;
 }

void               P3DBranchingAlgBase::SetShape
                                      (unsigned int                  Shape)
 {
  this->Shape = Shape == CIRCLE_SHAPE ? CIRCLE_SHAPE : SQUARE_SHAPE;
 }

float              P3DBranchingAlgBase::GetSpread
                                      () const
 {
  return(Spread);
 }

void               P3DBranchingAlgBase::SetSpread
                                      (float                         Spread)
 {
  if (Spread < 0.0f)
   {
   }
  else
   {
    this->Spread = Spread;
   }
 }

float              P3DBranchingAlgBase::GetDensity
                                      () const
 {
  return(Density);
 }

void               P3DBranchingAlgBase::SetDensity
                                      (float                         Density)
 {
  if (Density < 0.0f)
   {
   }
  else
   {
    this->Density = Density;
   }
 }

float              P3DBranchingAlgBase::GetDensityV
                                      () const
 {
  return(DensityV);
 }

void               P3DBranchingAlgBase::SetDensityV
                                      (float                         DensityV)
 {
  this->DensityV = P3DMath::Clampf(0.0f,1.0f,DensityV);
 }

unsigned int       P3DBranchingAlgBase::GetMinNumber
                                      () const
 {
  return MinNumber;
 }

void               P3DBranchingAlgBase::SetMinNumber
                                      (unsigned int                  MinNumber)
 {
  this->MinNumber = MinNumber;

  if (MinNumber > MaxNumber)
   {
    MaxNumber = MinNumber;
   }
 }

bool               P3DBranchingAlgBase::IsMaxLimitEnabled
                                      () const
 {
  return MaxLimitEnabled;
 }

void               P3DBranchingAlgBase::SetMaxLimitEnabled
                                      (bool                          IsEnabled)
 {
  MaxLimitEnabled = IsEnabled;
 }

unsigned int       P3DBranchingAlgBase::GetMaxNumber
                                      () const
 {
  return MaxNumber;
 }

void               P3DBranchingAlgBase::SetMaxNumber
                                      (unsigned int                  MaxNumber)
 {
  this->MaxNumber = MaxNumber;

  if (MaxNumber < MinNumber)
   {
    MinNumber = MaxNumber;
   }
 }

float              P3DBranchingAlgBase::GetDeclFactor
                                      () const
 {
  return(DeclFactor);
 }

void               P3DBranchingAlgBase::SetDeclFactor
                                      (float                         DeclFactor)
 {
  this->DeclFactor = P3DMath::Clampf(-1.0f,1.0f,DeclFactor);
 }

float              P3DBranchingAlgBase::GetDeclFactorV
                                      () const
 {
  return(DeclFactorV);
 }

void               P3DBranchingAlgBase::SetDeclFactorV
                                      (float                         DeclFactorV)
 {
  this->DeclFactorV = P3DMath::Clampf(0.0f,1.0f,DeclFactorV);
 }

float              P3DBranchingAlgBase::GetRotationAngle
                                      () const
 {
  return(Rotation);
 }

void               P3DBranchingAlgBase::SetRotationAngle
                                      (float               Rotation)
 {
  this->Rotation = P3DMath::Clampf(0.0f,(float)P3DMATH_2PI,Rotation);
 }

unsigned int       P3DBranchingAlgBase::CalcBranchCount
                                      (P3DMathRNG         *RNG) const
 {
  unsigned int     BranchCount;

  // It is important to not use RNG if Spread is 0 for
  // backward compatibility
  if (P3DMATH_ALMOST_ZERO(Spread))
   {
    BranchCount = 0;
   }
  else
   {
    float Area;

    Area = Spread * Spread;

    if (RNG != 0)
     {
      BranchCount = (int)(Area * (Density + RNG->UniformFloat(-DensityV,DensityV) * Density));
     }
    else
     {
      BranchCount = (int)(Area * Density);
     }
   }

  if (BranchCount < MinNumber)
   {
    BranchCount = MinNumber;
   }

  if (MaxLimitEnabled && BranchCount > MaxNumber)
   {
    BranchCount = MaxNumber;
   }

  return BranchCount;
 }

void               P3DBranchingAlgBase::GenOffsetSquare
                                      (float              *X,
                                       float              *Y,
                                       P3DMathRNG         *RNG) const
 {
  float HalfSpread = Spread * 0.5f;

  *X = RNG->UniformFloat(-HalfSpread,HalfSpread);
  *Y = RNG->UniformFloat(-HalfSpread,HalfSpread);
 }

void               P3DBranchingAlgBase::GenOffsetCircle
                                      (float              *X,
                                       float              *Y,
                                       P3DMathRNG         *RNG) const
 {
  const unsigned int MAX_RETRIES  = 100;
  unsigned int       Counter      = 0;
  bool               InsideCircle = false;
  float              HalfSpread   = Spread * 0.5f;

  while (!InsideCircle && Counter < MAX_RETRIES)
   {
    GenOffsetSquare(X,Y,RNG);

    InsideCircle = IsPointInsideCircle(*X,*Y,HalfSpread);

    Counter++;
   }

  if (!InsideCircle)
   {
    *X = *Y = 0.0f;
   }
 }

void               P3DBranchingAlgBase::GenBranchOffset
                                      (P3DVector3f        *Offset,
                                       P3DMathRNG         *RNG) const
 {
  // It is important to not use RNG if Spread is 0 for
  // backward compatibility
  if (P3DMATH_ALMOST_ZERO(Spread))
   {
    Offset->Set(0.0f,0.0f,0.0f);
   }
  else if (RNG != 0)
   {
    float X;
    float Z;

    if (Shape == CIRCLE_SHAPE)
     {
      GenOffsetCircle(&X,&Z,RNG);
     }
    else
     {
      GenOffsetSquare(&X,&Z,RNG);
     }

    Offset->Set(X,0.0f,Z);
   }
  else
   {
    Offset->Set(0.0f,0.0f,0.0f);
   }
 }

void               P3DBranchingAlgBase::CalcBranchOrientationInCircleShape
                                      (P3DQuaternionf     *Orientation,
                                       float               X,
                                       float               Z,
                                       P3DMathRNG         *RNG) const
 {
  P3DQuaternionf   Rotation1Quat;
  P3DQuaternionf   Rotation2Quat;
  P3DQuaternionf   DeclinationQuat;
  P3DQuaternionf   TempQuat;
  float            DeclinationAngle;

  float XAxisVsPointAngle = P3DMath::ATan2(X,Z);

  Rotation1Quat.FromAxisAndAngle(0.0f,1.0f,0.0f,XAxisVsPointAngle);
  Rotation2Quat.FromAxisAndAngle(0.0f,1.0f,0.0f,Rotation);

  float SquaredRadius = Spread * Spread * 0.25f;
  float NormDistance;

  if (!P3DMATH_ALMOST_ZERO(SquaredRadius))
   {
    NormDistance = P3DMath::Sqrtf((X * X + Z * Z) / (Spread * Spread * 0.25f));
   }
  else
   {
    NormDistance = 0.0f;
   }

  if (RNG != 0)
   {
    NormDistance += RNG->UniformFloat(-DeclFactorV,DeclFactorV);
   }

  DeclinationAngle = P3DMath::Clampf
                      (-P3DMATH_PI * 0.5,P3DMATH_PI * 0.5,
                       NormDistance * DeclFactor * P3DMATH_PI * 0.5);

  DeclinationQuat.FromAxisAndAngle(1.0f,0.0f,0.0f,DeclinationAngle);

  P3DQuaternionf::CrossProduct(TempQuat.q,Rotation1Quat.q,DeclinationQuat.q);
  P3DQuaternionf::CrossProduct(Orientation->q,TempQuat.q,Rotation2Quat.q);
 }

void               P3DBranchingAlgBase::CreateBranches
                                      (P3DBranchingFactory          *Factory,
                                       const P3DStemModelInstance   *Parent P3D_UNUSED_ATTR,
                                       P3DMathRNG                   *RNG)
 {
  unsigned int     BranchCount;
  P3DVector3f      BranchOffset;
  P3DQuaternionf   Orientation;

  if (Shape == SQUARE_SHAPE)
   {
    Orientation.FromAxisAndAngle(0.0f,1.0f,0.0f,Rotation);
   }

  BranchCount = CalcBranchCount(RNG);

  for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
   {
    GenBranchOffset(&BranchOffset,RNG);

    if (Shape == CIRCLE_SHAPE)
     {
      CalcBranchOrientationInCircleShape(&Orientation,BranchOffset.X(),BranchOffset.Z(),RNG);
     }

    Factory->GenerateBranch(&BranchOffset,&Orientation);
   }
 }

void               P3DBranchingAlgBase::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","BranchingAlg","Base");

  FmtStream.WriteString("su","Shape",Shape);
  FmtStream.WriteString("sf","Spread",Spread);

  FmtStream.WriteString("sf","Density",Density);
  FmtStream.WriteString("sf","DensityV",DensityV);

  FmtStream.WriteString("su","MinNumber",MinNumber);
  FmtStream.WriteString("sb","MaxLimitEnabled",MaxLimitEnabled);
  FmtStream.WriteString("su","MaxNumber",MaxNumber);

  FmtStream.WriteString("sf","DeclFactor",DeclFactor);
  FmtStream.WriteString("sf","DeclFactorV",DeclFactorV);

  FmtStream.WriteString("sf","RotAngle",Rotation);
 }

void               P3DBranchingAlgBase::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version)
 {
  bool                                 BoolValue;
  unsigned int                         UintValue;
  float                                FloatValue;

  if (Version->Minor > 11)
   {
    SourceStream->ReadFmtStringTagged("Shape","u",&UintValue);
    SetShape(UintValue);

    SourceStream->ReadFmtStringTagged("Spread","f",&FloatValue);
    SetSpread(FloatValue);

    SourceStream->ReadFmtStringTagged("Density","f",&FloatValue);
    SetDensity(FloatValue);
    SourceStream->ReadFmtStringTagged("DensityV","f",&FloatValue);
    SetDensityV(FloatValue);

    SourceStream->ReadFmtStringTagged("MinNumber","u",&UintValue);
    SetMinNumber(UintValue);
    SourceStream->ReadFmtStringTagged("MaxLimitEnabled","b",&BoolValue);
    SetMaxLimitEnabled(BoolValue);
    SourceStream->ReadFmtStringTagged("MaxNumber","u",&UintValue);
    SetMaxNumber(UintValue);

    SourceStream->ReadFmtStringTagged("DeclFactor","f",&FloatValue);
    SetDeclFactor(FloatValue);
    SourceStream->ReadFmtStringTagged("DeclFactorV","f",&FloatValue);
    SetDeclFactorV(FloatValue);
   }

  SourceStream->ReadFmtStringTagged("RotAngle","f",&FloatValue);
  SetRotationAngle(FloatValue);
 }

void               P3DBranchingAlgBase::MakeBranchWorldMatrix
                                      (float                        *WorldTransform,
                                       const P3DVector3f            *Offset,
                                       const P3DQuaternionf         *Orientation)
 {
  if (Orientation != 0)
   {
    if (Offset != 0)
     {
      P3DMatrix4x4f OrientationTransform;
      P3DMatrix4x4f OffsetTransform;

      Orientation->ToMatrix(OrientationTransform.m);
      P3DMatrix4x4f::MakeTranslation(OffsetTransform.m,Offset->X(),Offset->Y(),Offset->Z());

      P3DMatrix4x4f::MultMatrix(WorldTransform,OffsetTransform.m,OrientationTransform.m);
     }
    else
     {
      Orientation->ToMatrix(WorldTransform);
     }
   }
  else
   {
    if (Offset != 0)
     {
      P3DMatrix4x4f::MakeTranslation(WorldTransform,Offset->X(),Offset->Y(),Offset->Z());
     }
    else
     {
      P3DMatrix4x4f::MakeIdentity(WorldTransform);
     }
   }
 }

