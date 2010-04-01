/***************************************************************************

 Copyright (c) 2008 Sergey Prokhorchuk.
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
#include <stdlib.h>

#include <png.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>
#include <ngput/p3dimage.h>
#include <ngput/p3dimagepng.h>

static char P3DImageFmtPNGExt[] = "png";

#define PNG_SIGNATURE_SIZE (8)

unsigned int       P3DImageFmtHandlerPNG::FormatCount
                                      () const
 {
  return(1);
 }

const char        *P3DImageFmtHandlerPNG::FormatExt
                                      (unsigned int        FormatIndex P3D_UNUSED_ATTR) const
 {
  return(P3DImageFmtPNGExt);
 }

bool               P3DImageFmtHandlerPNG::LoadImageData
                                      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt P3D_UNUSED_ATTR) const
 {
  bool                                 Result;
  FILE                                *Source;
  unsigned char                        Sig[PNG_SIGNATURE_SIZE];
  png_structp                          PngStruct;
  png_infop                            PngInfo;

  Source = fopen(FileName,"rb");

  if (Source == NULL)
   {
    return(false);
   }

  Result = false;

  if (fread(Sig,1,sizeof(Sig),Source) == sizeof(Sig))
   {
    Result = png_check_sig(Sig,sizeof(Sig));
   }

  PngStruct = NULL;
  PngInfo   = NULL;

  if (Result)
   {
    Result = (PngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL)) != NULL;
   }

  if (Result)
   {
    Result = (PngInfo = png_create_info_struct(PngStruct)) != NULL;
   }

  if (Result)
   {
    if (setjmp(png_jmpbuf(PngStruct)))
     {
      png_destroy_read_struct(&PngStruct,&PngInfo,NULL);
      fclose(Source);

      return(false);
     }
   }

  if (Result)
   {
    png_init_io(PngStruct,Source);
    png_set_sig_bytes(PngStruct,sizeof(Sig));

    png_read_png(PngStruct,PngInfo,
                 PNG_TRANSFORM_STRIP_16 |
                 PNG_TRANSFORM_PACKING |
                 PNG_TRANSFORM_EXPAND,
                 NULL);

    unsigned int             ImageWidth,ImageHeight;
    unsigned int             ImageColorType;
    unsigned int             ImageChannelCount;

    ImageWidth  = png_get_image_width(PngStruct,PngInfo);
    ImageHeight = png_get_image_height(PngStruct,PngInfo);

    ImageColorType = png_get_color_type(PngStruct,PngInfo);

    if ((ImageWidth > 0) && (ImageHeight > 0) &&
        (ImageColorType == PNG_COLOR_TYPE_RGB ||
         ImageColorType == PNG_COLOR_TYPE_RGB_ALPHA) &&
        (png_get_bit_depth(PngStruct,PngInfo) == 8))
     {
      ImageChannelCount = ImageColorType == PNG_COLOR_TYPE_RGB ? 3 : 4;

      Result = ImageData->Create
                (ImageWidth,ImageHeight,ImageChannelCount,P3D_BYTE);
     }
    else
     {
      Result = false;
     }

    if (Result)
     {
      png_bytepp   row_pointers;

      row_pointers = png_get_rows(PngStruct,PngInfo);

      for (unsigned int Y = 0; Y < ImageHeight; Y++)
       {
        png_bytep  row;

        row = row_pointers[Y];

        for (unsigned int X = 0; X < ImageWidth; X++)
         {
          ImageData->PutPixel(X,ImageHeight - Y - 1,row);

          row += ImageChannelCount;
         }
       }
     }
   }

  if (PngInfo != NULL)
   {
    /*FIXME: will it work ok if PngInfo == NULL? */
    png_destroy_read_struct(&PngStruct,&PngInfo,NULL);
   }

  fclose(Source);

  return(Result);
 }

