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

#ifndef __P3DTEXTURE_H__
#define __P3DTEXTURE_H__

#include <vector>
#include <string>

#include <wx/wx.h>

#include <ngput/p3dglext.h>
#include <ngput/p3dimage.h>

typedef unsigned int     P3DTexHandle;

#define P3DTexHandleNULL (0)

typedef struct
 {
  unsigned int     RefCount;
  GLuint           GLHandle;
  wxBitmap        *Bitmap;
  std::string      FileName;
  std::string      GenericName;
 } P3DTexManagerGLEntry;

class P3DTexManagerGL
 {
  public           :

                   P3DTexManagerGL    ();
                  ~P3DTexManagerGL    ();

  P3DTexHandle     LoadFromFile       (const char         *FileName,
                                       wxString           &ErrorMessage);
  P3DTexHandle     GetHandleByGenericName
                                      (const char         *GenericName);

  void             FreeTexture        (P3DTexHandle        TexHandle);

  GLuint           GetGLHandle        (P3DTexHandle        TexHandle) const;
  const wxBitmap  *GetBitmap          (P3DTexHandle        TexHandle) const;
  const char      *GetGenericName     (P3DTexHandle        TexHandle) const;
  const char      *GetTexFileName     (P3DTexHandle        TexHandle) const;

  private          :

  P3DTexHandle     GetUnusedSlot      ();
  P3DTexHandle     FindByFileName     (const char         *FileName) const;
  P3DTexHandle     FindByGenericName  (const char         *GenericName) const;

  P3DImageFmtHandlerComposite          ImageFmtHandler;

  std::vector<P3DTexManagerGLEntry>    TextureSet;
 };

#endif

