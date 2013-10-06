/***************************************************************************

 Copyright (c) 2013 Sergey Prokhorchuk.
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

#include <string.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3diostream.h>
#include <ngpcore/p3dmathspline.h>
#include <ngpcore/p3dsplineio.h>

#define P3D_SPLINE_FMT_VERSION_MAJOR (1)
#define P3D_SPLINE_FMT_VERSION_MINOR (0)

void               P3DSaveSplineCurve (P3DOutputStringFmtStream
                                                          *FmtStream,
                                       const P3DMathNaturalCubicSpline
                                                          *Spline)
 {
  unsigned int                         CPCount;
  unsigned int                         CPIndex;

  CPCount = Spline->GetCPCount();

  FmtStream->WriteString("su","CPCount",CPCount);

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    FmtStream->WriteString("sff","Point",Spline->GetCPX(CPIndex),Spline->GetCPY(CPIndex));
   }
 }

void               P3DLoadSplineCurve (P3DMathNaturalCubicSpline
                                                          *Spline,
                                       P3DInputStringFmtStream
                                                          *SourceStream,
                                       const char         *CurveName)
 {
  unsigned int                         CPCount;
  unsigned int                         CPIndex;
  float                                X,Y;

  if (strcmp(CurveName,"CubicSpline") != 0)
   {
    throw P3DExceptionGeneric("Unsupported curve type");
   }

  CPCount = Spline->GetCPCount();

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    Spline->DelCP(0);
   }

  SourceStream->ReadFmtStringTagged("CPCount","u",&CPCount);

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    SourceStream->ReadFmtStringTagged("Point","ff",&X,&Y);

    Spline->AddCP(X,Y);
   }
 }

extern void        P3DExportSplineCurve
                                      (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMathNaturalCubicSpline
                                                          *Spline)
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("suu","P3C",
                        (unsigned int)P3D_SPLINE_FMT_VERSION_MAJOR,
                        (unsigned int)P3D_SPLINE_FMT_VERSION_MINOR);

  FmtStream.WriteString("ss","Type","CubicSpline");

  P3DSaveSplineCurve(&FmtStream,Spline);
 }

extern void        P3DImportSplineCurve
                                      (P3DMathNaturalCubicSpline
                                                          *Spline,
                                       P3DInputStringStream
                                                          *SourceStream)
 {
  unsigned int                         VersionMajor;
  unsigned int                         VersionMinor;
  char                                 StrValue[255 + 1];
  P3DInputStringFmtStream              FmtStream(SourceStream);

  FmtStream.ReadFmtStringTagged("P3C","uu",&VersionMajor,&VersionMinor);

  if ((VersionMajor != P3D_SPLINE_FMT_VERSION_MAJOR) ||
      (VersionMinor  > P3D_SPLINE_FMT_VERSION_MAJOR))
   {
    throw P3DExceptionGeneric("unsupported file format version");
   }

  FmtStream.ReadFmtStringTagged("Type","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(Spline,&FmtStream,StrValue);
 }

