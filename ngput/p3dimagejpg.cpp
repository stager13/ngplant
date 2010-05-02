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

#ifdef LIBJPEG_NO_EXTERN_C
 extern "C" {
#endif

#include <jpeglib.h>

#ifdef LIBJPEG_NO_EXTERN_C
 }
#endif

#include <setjmp.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>
#include <ngput/p3dimage.h>
#include <ngput/p3dimagejpg.h>

static char P3DImageFmtJPGExt[] = "jpg";

unsigned int       P3DImageFmtHandlerJPG::FormatCount
                                      () const
 {
  return(1);
 }

const char        *P3DImageFmtHandlerJPG::FormatExt
                                      (unsigned int        FormatIndex P3D_UNUSED_ATTR) const
 {
  return(P3DImageFmtJPGExt);
 }

typedef struct
 {
  struct jpeg_error_mgr      Base;
  jmp_buf                    SetJmpBuffer;
 } JpegErrorHandlerStruct;

static void        JpegErrorHandlerFunc
                                      (j_common_ptr        HandlerData)
 {
  longjmp(((JpegErrorHandlerStruct*)HandlerData)->SetJmpBuffer,1);
 }

bool               P3DImageFmtHandlerJPG::LoadImageData
                                      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt P3D_UNUSED_ATTR) const
 {
  bool                                 Result;
  FILE                                *Source;
  struct jpeg_decompress_struct        JpegInfo;
  JpegErrorHandlerStruct               JpegErrorHandler;

  Source = fopen(FileName,"rb");

  if (Source == NULL)
   {
    return(false);
   }

  JpegInfo.err = jpeg_std_error(&JpegErrorHandler.Base);
  JpegErrorHandler.Base.error_exit = JpegErrorHandlerFunc;

  if (setjmp(JpegErrorHandler.SetJmpBuffer))
   {
    jpeg_destroy_decompress(&JpegInfo);

    fclose(Source);

    return(false);
   }

  jpeg_create_decompress(&JpegInfo);
  jpeg_stdio_src(&JpegInfo,Source);
  jpeg_read_header(&JpegInfo,TRUE);

  Result = true;

  if (JpegInfo.image_width > 0 &&
      JpegInfo.image_height > 0 &&
      JpegInfo.num_components == 3)
   {
    Result = ImageData->Create
              (JpegInfo.image_width,JpegInfo.image_height,3,P3D_BYTE);

    if (Result)
     {
      unsigned char         *Ptr;
      unsigned int           Stride;

      Stride = JpegInfo.image_width * JpegInfo.num_components;

      jpeg_start_decompress(&JpegInfo);

      Ptr = (unsigned char*)ImageData->GetData() +
            Stride * JpegInfo.image_height;

      while (JpegInfo.output_scanline < JpegInfo.image_height)
       {
        Ptr -= Stride;

        jpeg_read_scanlines(&JpegInfo,&Ptr,1);
       }

      jpeg_finish_decompress(&JpegInfo);
     }
   }
  else
   {
    Result = false;
   }

  jpeg_destroy_decompress(&JpegInfo);

  fclose(Source);

  return(Result);
 }

