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

#include <stdio.h>
#include <string.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>
#include <ngput/p3dimage.h>
#include <ngput/p3dimagetga.h>

static char P3DImageFmtTGAExt[] = "tga";

unsigned int       P3DImageFmtHandlerTGA::FormatCount
                                      () const
 {
  return(1);
 }

const char        *P3DImageFmtHandlerTGA::FormatExt
                                      (unsigned int        FormatIndex P3D_UNUSED_ATTR) const
 {
  return(P3DImageFmtTGAExt);
 }

bool               ReadByte           (P3DByte            *Value,
                                       FILE               *Source)
 {
  if (fread(Value,sizeof(*Value),1,Source) == 1)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

bool               ReadUint16         (P3Duint16          *Value,
                                       FILE               *Source)
 {
  if (fread(Value,sizeof(*Value),1,Source) == 1)
   {
    #ifdef P3D_BIG_ENDIAN
     {
      *Value = (((*Value) & 0xFF) << 8) | (((*Value) >> 8) & 0xFF);
     }
    #endif
    return(true);
   }
  else
   {
    return(false);
   }
 }

bool               ReadBlock          (void               *Data,
                                       unsigned int        Size,
                                       FILE               *Source)
 {
  if (fread(Data,1,Size,Source) == Size)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

class P3DTGADataReader
 {
  public           :

                   P3DTGADataReader   () {}
  virtual         ~P3DTGADataReader   () {}

  virtual bool     ReadPixel          (P3DByte            *PixelData) = 0;
 };

class P3DTGADataReaderRaw : public P3DTGADataReader
 {
  public           :

                   P3DTGADataReaderRaw(FILE               *Source,
                                       unsigned int        ChannelCount);

  virtual bool     ReadPixel          (P3DByte            *PixelData);

  private          :

  FILE            *Source;
  unsigned int     ChannelCount;
 };

                   P3DTGADataReaderRaw::P3DTGADataReaderRaw
                                      (FILE               *Source,
                                       unsigned int        ChannelCount)
 {
  this->Source       = Source;
  this->ChannelCount = ChannelCount;
 }

bool               P3DTGADataReaderRaw::ReadPixel
                                     (P3DByte            *PixelData)
 {
  bool                                Result;

  if      (ChannelCount == 1)
   {
    Result = ReadByte(&PixelData[0],Source);
   }
  else if (ChannelCount == 3)
   {
    Result = ReadByte(&PixelData[2],Source);

    if (Result)
     {
      Result = ReadByte(&PixelData[1],Source);
     }

    if (Result)
     {
      Result = ReadByte(&PixelData[0],Source);
     }
   }
  else // (ChannelCount == 4)
   {
    Result = ReadByte(&PixelData[2],Source);

    if (Result)
     {
      Result = ReadByte(&PixelData[1],Source);
     }

    if (Result)
     {
      Result = ReadByte(&PixelData[0],Source);
     }

    if (Result)
     {
      Result = ReadByte(&PixelData[3],Source);
     }
   }

  return(Result);
 }

class P3DTGADataReaderRLE : public P3DTGADataReader
 {
  public           :

                   P3DTGADataReaderRLE(FILE               *Source,
                                       unsigned int        ChannelCount);

  virtual bool     ReadPixel          (P3DByte            *PixelData);

  private          :

  FILE                               *Source;
  unsigned int                        ChannelCount;
  P3DByte                             PixelValue[4];
  P3DByte                             RepCount;
  bool                                RunLengthFlag;
  P3DTGADataReaderRaw                 DataReaderRaw;
 };

                   P3DTGADataReaderRLE::P3DTGADataReaderRLE
                                      (FILE               *Source,
                                       unsigned int        ChannelCount)
                   : DataReaderRaw(Source,ChannelCount)
 {
  this->Source       = Source;
  this->ChannelCount = ChannelCount;

  RepCount = 0;
 }

bool               P3DTGADataReaderRLE::ReadPixel
                                      (P3DByte            *PixelData)
 {
  bool                                 Result;

  Result = true;

  if (RepCount == 0)
   {
    Result = ReadByte(&RepCount,Source);

    if (Result)
     {
      if (RepCount & 0x80)
       {
        RunLengthFlag = true;
        RepCount     &= 0x7F;
        RepCount++;

        Result = DataReaderRaw.ReadPixel(PixelValue);
       }
      else
       {
        RunLengthFlag = false;
        RepCount++;
       }
     }
   }

  if (Result)
   {
    if (RunLengthFlag)
     {
      PixelData[0] = PixelValue[0];

      if (ChannelCount > 1)
       {
        PixelData[1] = PixelValue[1];
        PixelData[2] = PixelValue[2];

        if (ChannelCount > 3)
         {
          PixelData[3] = PixelValue[3];
         }
       }
     }
    else
     {
      Result = DataReaderRaw.ReadPixel(PixelData);
     }

    RepCount--;
   }

  return(Result);
 }

bool               P3DImageFmtHandlerTGA::LoadImageData
                                      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt) const
 {
  bool                                 Result;
  FILE                                *Source;
  P3DByte                              TGAIDLength;
  P3DByte                              TGAColorMapType;
  P3DByte                              TGAImageType;
  P3DByte                              TGAColorMapSpec[5];
  P3Duint16                            ImageOriginX;
  P3Duint16                            ImageOriginY;
  P3Duint16                            ImageWidth;
  P3Duint16                            ImageHeight;
  P3DByte                              PixelDepth;
  P3DByte                              ImageDescriptor;
  P3DByte                              AlphaChannelBits;
  unsigned int                         Index;
  P3DByte                              TempByte;
  unsigned int                         ChannelCount;
  P3DByte                              Pixel[4];
  bool                                 RLECompressed;

  if (strcmp(FileExt,P3DImageFmtTGAExt) != 0)
   {
    return(false);
   }

  Source = fopen(FileName,"rb");

  if (Source == NULL)
   {
    return(false);
   }

  Result = ReadByte(&TGAIDLength,Source);

  if (Result)
   {
    Result = ReadByte(&TGAColorMapType,Source);
   }

  if (Result)
   {
    if (TGAColorMapType != 0)
     {
      /* Color map is not supported */

      Result = false;
     }
   }

  if (Result)
   {
    Result = ReadByte(&TGAImageType,Source);
   }

  if (Result)
   {
    if      ((TGAImageType == 2) || (TGAImageType == 3))
     {
      RLECompressed = false;
     }
    else if ((TGAImageType == 10) || (TGAImageType == 11))
     {
      RLECompressed = true;
     }
    else
     {
      /* Unsupported image type */

      Result = false;
     }
   }

  if (Result)
   {
    Result = ReadBlock(TGAColorMapSpec,sizeof(TGAColorMapSpec),Source);
   }

  if (Result)
   {
    Result = ReadUint16(&ImageOriginX,Source);
   }

  if (Result)
   {
    Result = ReadUint16(&ImageOriginY,Source);
   }

  if (Result)
   {
    Result = ReadUint16(&ImageWidth,Source);
   }

  if (Result)
   {
    Result = ReadUint16(&ImageHeight,Source);
   }

  if (Result)
   {
    Result = ReadByte(&PixelDepth,Source);
   }

  if (Result)
   {
    Result = ReadByte(&ImageDescriptor,Source);
   }

  if (Result)
   {
    AlphaChannelBits = ImageDescriptor & 0x0F;

    Index = 0;

    while ((Index < TGAIDLength) && (Result))
     {
      Result = ReadByte(&TempByte,Source);

      Index++;
     }
   }

  if (Result)
   {
    if      (PixelDepth == 8)
     {
      ChannelCount = 1;
     }
    else if ((PixelDepth == 24) && (AlphaChannelBits == 0))
     {
      ChannelCount = 3;
     }
    else if ((PixelDepth == 32) && (AlphaChannelBits == 8))
     {
      ChannelCount = 4;
     }
    else
     {
      /* unsupported pixel depth */

      Result = false;
     }
   }

  if (Result)
   {
    Result = ImageData->Create(ImageWidth,ImageHeight,ChannelCount,P3D_BYTE);
   }

  if (Result)
   {
    P3DTGADataReaderRaw                DataReaderRaw(Source,ChannelCount);
    P3DTGADataReaderRLE                DataReaderRLE(Source,ChannelCount);

    P3DTGADataReader                  *DataReader;

    if (RLECompressed)
     {
      DataReader = &DataReaderRLE;
     }
    else
     {
      DataReader = &DataReaderRaw;
     }

    for (unsigned int Y = 0; (Y < ImageHeight) && (Result); Y++)
     {
      for (unsigned int X = 0; (X < ImageWidth) && (Result); X++)
       {
        Result = DataReader->ReadPixel(Pixel);

        if (Result)
         {
          if (ImageDescriptor & 0x10)
           {
            if (ImageDescriptor & 0x20)
             {
              /* top right */
              ImageData->PutPixel(ImageWidth - X - 1,ImageHeight - Y - 1,Pixel);
             }
            else
             {
              /* bottom right */
              ImageData->PutPixel(ImageWidth - X - 1,Y,Pixel);
             }
           }
          else
           {
            if (ImageDescriptor & 0x20)
             {
              /* top left */
              ImageData->PutPixel(X,ImageHeight - Y - 1,Pixel);
             }
            else
             {
              /* bottom left */
              ImageData->PutPixel(X,Y,Pixel);
             }
           }
         }
       }
     }
   }

  fclose(Source);

  return(Result);
 }

bool               WriteByte          (FILE               *Target,
                                       P3DByte             Value)
 {
  if (fwrite(&Value,sizeof(Value),1,Target) == 1)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

bool               WriteUint16        (FILE               *Target,
                                       P3Duint16           Value)
 {
  #ifdef P3D_BIG_ENDIAN
   {
    Value = ((Value & 0xFF) << 8) | ((Value >> 8) & 0xFF);
   }
  #endif
  if (fwrite(&Value,sizeof(Value),1,Target) == 1)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

bool               P3DImageFmtHandlerTGA::SaveAsTGA
                                      (const char         *FileName,
                                       P3DImageData       *ImageData)
 {
  bool                                 Result;
  FILE                                *Target;
  unsigned int                         X;
  unsigned int                         Y;

  if ((ImageData->GetChannelCount() == 1) ||
      (ImageData->GetChannelCount() == 3) ||
      (ImageData->GetChannelCount() == 4))
   {
   }
  else
   {
    return(false);
   }

  if (ImageData->GetChannelType() != P3D_BYTE)
   {
    return(false);
   }

  Target = fopen(FileName,"wb");

  if (Target == NULL)
   {
    return(false);
   }

  Result = WriteByte(Target,0x00); /* IDLength */

  if (Result)
   {
    Result = WriteByte(Target,0x00); /* ColorMapType = 0 - no color table */
   }

  if (Result)
   {
    if (ImageData->GetChannelCount() == 1)
     {
      Result = WriteByte(Target,3); /* Monochrome */
     }
    else
     {
      Result = WriteByte(Target,2); /* True-color */
     }
   }

  /* Colormap spec */

  if (Result)
   {
    Result = WriteUint16(Target,0);
   }

  if (Result)
   {
    Result = WriteUint16(Target,0);
   }

  if (Result)
   {
    Result = WriteByte(Target,0);
   }

  /* Image origin */

  if (Result)
   {
    Result = WriteUint16(Target,0);
   }

  if (Result)
   {
    Result = WriteUint16(Target,0);
   }

  /* Image size */

  if (Result)
   {
    Result = WriteUint16(Target,ImageData->GetWidth());
   }

  if (Result)
   {
    Result = WriteUint16(Target,ImageData->GetHeight());
   }

  /* Pixel depth */

  if (Result)
   {
    Result = WriteByte(Target,ImageData->GetChannelCount() * 8);
   }

  /* Image descriptor */

  if (Result)
   {
    if (ImageData->GetChannelCount() == 4)
     {
      Result = WriteByte(Target,8);
     }
    else
     {
      Result = WriteByte(Target,0);
     }
   }

  if (Result)
   {
    for (Y = 0; (Y < ImageData->GetHeight()) && (Result); Y++)
     {
      for (X = 0; (X < ImageData->GetWidth()) && (Result); X++)
       {
        unsigned char                  Pixel[4];

        ImageData->GetPixel(X,ImageData->GetHeight() - Y - 1,Pixel);

        if      (ImageData->GetChannelCount() == 1)
         {
          Result = WriteByte(Target,Pixel[0]);
         }
        else if (ImageData->GetChannelCount() == 3)
         {
          Result = WriteByte(Target,Pixel[2]);

          if (Result)
           {
            Result = WriteByte(Target,Pixel[1]);
           }

          if (Result)
           {
            Result = WriteByte(Target,Pixel[0]);
           }
         }
        else // (ChannelCount == 4)
         {
          Result = WriteByte(Target,Pixel[2]);

          if (Result)
           {
            Result = WriteByte(Target,Pixel[1]);
           }

          if (Result)
           {
            Result = WriteByte(Target,Pixel[0]);
           }

          if (Result)
           {
            Result = WriteByte(Target,Pixel[3]);
           }
         }
       }
     }
   }

  fclose(Target);

  return(Result);
 }

