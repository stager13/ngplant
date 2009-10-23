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

#include <stdlib.h>
#include <cstring>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>
#include <ngput/p3dimage.h>

                   P3DImageData::P3DImageData
                                      ()
 {
  Width        = 0;
  Height       = 0;
  ChannelCount = 0;
  ChannelType  = P3D_BYTE;
  Data         = 0;
 }

                   P3DImageData::~P3DImageData
                                      ()
 {
  free(Data);
 }

bool               P3DImageData::Create
                                      (unsigned int        Width,
                                       unsigned int        Height,
                                       unsigned int        ChannelCount,
                                       unsigned int        ChannelType)
 {
  P3DByte                             *NewData;

  if ((Width == 0) || (Height == 0))
   {
    return(false);
   }

  if ((ChannelCount != 1) && (ChannelCount != 3) && (ChannelCount != 4))
   {
    return(false);
   }

  if (ChannelType != P3D_BYTE)
   {
    return(false);
   }

  NewData = (P3DByte*)malloc(Width * Height * ChannelCount * sizeof(P3DByte));

  if (NewData == 0)
   {
    return(false);
   }

  this->Width        = Width;
  this->Height       = Height;
  this->ChannelCount = ChannelCount;
  this->ChannelType  = ChannelType;

  if (Data != 0)
   {
    free(Data);
   }

  Data = NewData;

  return(true);
 }

bool               P3DImageData::IsEmpty
                                      () const
 {
  if (Data == 0)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

unsigned int       P3DImageData::GetWidth
                                      () const
 {
  return(Width);
 }

unsigned int       P3DImageData::GetHeight
                                      () const
 {
  return(Height);
 }

unsigned int       P3DImageData::GetChannelCount
                                      () const
 {
  return(ChannelCount);
 }

unsigned int       P3DImageData::GetChannelType
                                      () const
 {
  return(ChannelType);
 }

void              *P3DImageData::GetData
                                      ()
 {
  return(Data);
 }

const void        *P3DImageData::GetConstData
                                      () const
 {
  return(Data);
 }

void               P3DImageData::PutPixel
                                      (unsigned int        X,
                                       unsigned int        Y,
                                       const unsigned char*Color)
 {
  P3DByte                             *Ptr;

  if ((X >= Width) || (Y >= Height))
   {
    return;
   }

  Ptr = ((P3DByte*)Data) + Y * Width * ChannelCount * sizeof(P3DByte) +
        X * ChannelCount * sizeof(P3DByte);

  for (unsigned int ChannelIndex = 0; ChannelIndex < ChannelCount; ChannelIndex++)
   {
    Ptr[ChannelIndex] = Color[ChannelIndex];
   }
 }

void               P3DImageData::GetPixel
                                      (unsigned int        X,
                                       unsigned int        Y,
                                       unsigned char      *Color) const
 {
  P3DByte                             *Ptr;

  if ((X >= Width) || (Y >= Height))
   {
    for (unsigned int ChannelIndex = 0; ChannelIndex < ChannelCount; ChannelIndex++)
     {
      Color[ChannelIndex] = 0;
     }
   }
  else
   {
    Ptr = ((P3DByte*)Data) + Y * Width * ChannelCount * sizeof(P3DByte) +
          X * ChannelCount * sizeof(P3DByte);

    for (unsigned int ChannelIndex = 0; ChannelIndex < ChannelCount; ChannelIndex++)
     {
      Color[ChannelIndex] = Ptr[ChannelIndex];
     }
   }
 }

void              *P3DImageData::DetachData
                                      ()
 {
  void                                *Result;

  Result = Data;

  Width        = 0;
  Height       = 0;
  ChannelCount = 0;
  ChannelType  = P3D_BYTE;
  Data         = 0;

  return(Result);
 }

bool               P3DImageData::RemoveAlpha
                                      (P3DImageData       *TargetImage,
                                       const P3DImageData *SourceImage)
 {
  P3DByte                              Pixel[4];

  if (SourceImage->GetChannelCount() != 4)
   {
    return(false);
   }

  if (SourceImage->GetChannelType() != P3D_BYTE)
   {
    return(false);
   }

  if (!TargetImage->Create(SourceImage->GetWidth(),
                           SourceImage->GetHeight(),
                           3,
                           SourceImage->GetChannelType()))
   {
    return(false);
   }

  for (unsigned int Y = 0; Y < SourceImage->GetHeight(); Y++)
   {
    for (unsigned int X = 0; X < SourceImage->GetWidth(); X++)
     {
      SourceImage->GetPixel(X,Y,Pixel);
      TargetImage->PutPixel(X,Y,Pixel);
     }
   }

  return(true);
 }

bool               P3DImageData::Copy (P3DImageData       *TargetImage,
                                       const P3DImageData *SourceImage)
 {
  P3DByte                              Pixel[4];

  if (SourceImage->GetChannelType() != P3D_BYTE)
   {
    return(false);
   }

  if (!TargetImage->Create(SourceImage->GetWidth(),
                           SourceImage->GetHeight(),
                           SourceImage->GetChannelCount(),
                           SourceImage->GetChannelType()))
   {
    return(false);
   }

  for (unsigned int Y = 0; Y < SourceImage->GetHeight(); Y++)
   {
    for (unsigned int X = 0; X < SourceImage->GetWidth(); X++)
     {
      SourceImage->GetPixel(X,Y,Pixel);
      TargetImage->PutPixel(X,Y,Pixel);
     }
   }

  return(true);
 }

void               P3DImageData::FlipVertical
                                      ()
 {
  unsigned int                         X;
  unsigned int                         Y;
  P3DByte                              TopPixel[4];
  P3DByte                              BotPixel[4];

  if (Data == 0)
   {
    return;
   }

  if (ChannelType != P3D_BYTE)
   {
    return;
   }

  for (Y = 0; (Y < Height / 2); Y++)
   {
    for (X = 0; X < Width; X++)
     {
      GetPixel(X,Y,TopPixel);
      GetPixel(X,Height - Y - 1,BotPixel);
      PutPixel(X,Y,BotPixel);
      PutPixel(X,Height - Y - 1,TopPixel);
     }
   }
 }

                   P3DImageFmtHandlerComposite::P3DImageFmtHandlerComposite
                                      ()
 {
 }

                   P3DImageFmtHandlerComposite::~P3DImageFmtHandlerComposite
                                      ()
 {
  for (unsigned int Index = 0; Index < Handlers.size(); Index++)
   {
    delete Handlers[Index];
   }
 }

void               P3DImageFmtHandlerComposite::AddHandler
                                      (P3DImageFmtHandler *Handler)
 {
  Handlers.push_back(Handler);
 }

unsigned int       P3DImageFmtHandlerComposite::FormatCount
                                      () const
 {
  unsigned int                         Count;

  Count = 0;

  for (unsigned int Index = 0; Index < Handlers.size(); Index++)
   {
    Count += Handlers[Index]->FormatCount();
   }

  return(Count);
 }

const char        *P3DImageFmtHandlerComposite::FormatExt
                                      (unsigned int        FormatIndex) const
 {
  for (unsigned int HandlerIndex = 0; HandlerIndex < Handlers.size(); HandlerIndex++)
   {
    if (FormatIndex < Handlers[HandlerIndex]->FormatCount())
     {
      return(Handlers[HandlerIndex]->FormatExt(FormatIndex));
     }
    else
     {
      FormatIndex -= Handlers[HandlerIndex]->FormatCount();
     }
   }

  return(0);
 }

bool               P3DImageFmtHandlerComposite::LoadImageData
                                      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt) const
 {
  for (unsigned int HandlerIndex = 0; HandlerIndex < Handlers.size(); HandlerIndex++)
   {
    for (unsigned int FormatIndex = 0;
         FormatIndex < Handlers[HandlerIndex]->FormatCount();
         FormatIndex++)
     {
      if (strcmp(Handlers[HandlerIndex]->FormatExt(FormatIndex),FileExt) == 0)
       {
        return(Handlers[HandlerIndex]->LoadImageData(ImageData,FileName,FileExt));
       }
     }
   }

  return(false);
 }

