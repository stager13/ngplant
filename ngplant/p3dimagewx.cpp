/***************************************************************************

 Copyright (C) 2010  Sergey Prokhorchuk

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

#include "wx/wx.h"

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>
#include <ngput/p3dimage.h>
#include <p3dimagewx.h>

static const char *P3DImageFmtWxExts[] =
 {
  "png","jpg"
 };

unsigned int       P3DImageFmtHandlerWx::FormatCount
                                      () const
 {
  return(sizeof(P3DImageFmtWxExts) / sizeof(P3DImageFmtWxExts[0]));
 }

const char        *P3DImageFmtHandlerWx::FormatExt
                                      (unsigned int        FormatIndex) const
 {
  return P3DImageFmtWxExts[FormatIndex < FormatCount() ? FormatIndex : 0];
 }

bool               P3DImageFmtHandlerWx::LoadImageData
                                      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt P3D_UNUSED_ATTR) const
 {
  bool                                 Result;
  wxImage                             *Image;
  int                                  Width;
  int                                  Height;
  bool                                 HasAlpha;

  Image  = new wxImage();
  Result = Image->LoadFile(wxString(FileName,wxConvUTF8));

  if (Result)
   {
    Width    = Image->GetWidth();
    Height   = Image->GetHeight();
    HasAlpha = Image->HasAlpha();

    Result = ImageData->Create(Width,Height,HasAlpha ? 4 : 3,P3D_BYTE);

    if (Result)
     {
      unsigned char *Ptr;

      Ptr = (unsigned char*)ImageData->GetData();

      for (int Y = Height - 1; Y >= 0; Y--)
       {
        for (int X = 0; X < Width; X++)
         {
          *Ptr++ = Image->GetRed(X,Y);
          *Ptr++ = Image->GetGreen(X,Y);
          *Ptr++ = Image->GetBlue(X,Y);

          if (HasAlpha)
           {
            *Ptr++ = Image->GetAlpha(X,Y);
           }
         }
       }
     }
   }

  delete Image;

  return(Result);
 }

