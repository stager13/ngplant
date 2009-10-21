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

#include <vector>

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/image.h>

#include <ngput/p3dglext.h>
#include <ngput/p3dimagetga.h>

#include <p3dtexture.h>

#include <p3dapp.h>
#include <p3didevfs.h>

                   P3DTexManagerGL::P3DTexManagerGL
                                      ()
 {
  ImageFmtHandler.AddHandler(new P3DImageFmtHandlerTGA());
 }

                   P3DTexManagerGL::~P3DTexManagerGL
                                      ()
 {
 }

P3DTexHandle       P3DTexManagerGL::LoadFromFile
                                      (const char         *FileName,
                                       wxString           &ErrorMessage)
 {
  P3DTexHandle                         Handle;
  P3DTexManagerGLEntry                *Entry;
  unsigned int                         TextureChannelCount;

  Handle = FindByFileName(FileName);

  if (Handle == P3DTexHandleNULL)
   {
    P3DImageData                       ImageData;
    P3DImageData                       TempImageData;
    wxString                           FileExt = wxFileName(wxString(FileName,wxConvUTF8)).GetExt();

    if (!ImageFmtHandler.LoadImageData(&ImageData,FileName,FileExt.mb_str()))
     {
      ErrorMessage = wxT("Texture image load failed");

      return(P3DTexHandleNULL);
     }

    std::string GenericName = wxGetApp().GetTexFS()->System2Generic(FileName);

    if (GenericName.length() == 0)
     {
      ErrorMessage = wxT("Texture image file located outside texture search path(s).\nPlease setup texture search path(s) in \"Preferences\" dialog.");

      return(P3DTexHandleNULL);
     }

    TextureChannelCount = ImageData.GetChannelCount();

    if      (TextureChannelCount == 3)
     {
      if (!P3DImageData::Copy(&TempImageData,&ImageData))
       {
        ErrorMessage = wxT("Out of memory");

        return(P3DTexHandleNULL);
       }
     }
    else if (TextureChannelCount == 4)
     {
      if (!P3DImageData::RemoveAlpha(&TempImageData,&ImageData))
       {
        ErrorMessage = wxT("Out of memory");

        return(P3DTexHandleNULL);
       }

      /* draw alpha grid */

      for (unsigned int Y = 0; Y < ImageData.GetHeight(); Y++)
       {
        for (unsigned int X = 0; X < ImageData.GetWidth(); X++)
         {
          unsigned char                Pixel[4];

          ImageData.GetPixel(X,Y,Pixel);

          if (Pixel[3] == 0)
           {
            if ((((X / 8) + (Y / 8)) & 0x01) == 1)
             {
              Pixel[0] = 0xCF;
              Pixel[1] = 0xCF;
              Pixel[2] = 0xCF;
             }
            else
             {
              Pixel[0] = 0x80;
              Pixel[1] = 0x80;
              Pixel[2] = 0x80;
             }

            TempImageData.PutPixel(X,Y,Pixel);
           }
         }
       }
     }
    else
     {
      ErrorMessage = wxT("Texture must be RGB or RGBA");

      return(P3DTexHandleNULL);
     }

    Handle = GetUnusedSlot();

    Entry  = &TextureSet[Handle - 1];

    wxImage Image(TempImageData.GetWidth(),
                  TempImageData.GetHeight(),
                  (unsigned char*)(TempImageData.GetData()),
                  TRUE);

/*    TempImageData.DetachData(); */

    Entry->Bitmap      = new wxBitmap(Image.Scale(64,64).Mirror(false));
    Entry->FileName    = FileName;
    Entry->GenericName = GenericName;

    glGenTextures(1,&Entry->GLHandle);

    glBindTexture(GL_TEXTURE_2D,Entry->GLHandle);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    if (TextureChannelCount == 3)
     {
      gluBuild2DMipmaps(GL_TEXTURE_2D,
                        TextureChannelCount,
                        ImageData.GetWidth(),
                        ImageData.GetHeight(),
                        GL_RGB,
                        GL_UNSIGNED_BYTE,
                        ImageData.GetData());
     }
    else
     {
      gluBuild2DMipmaps(GL_TEXTURE_2D,
                        TextureChannelCount,
                        ImageData.GetWidth(),
                        ImageData.GetHeight(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        ImageData.GetData());
     }

    glBindTexture(GL_TEXTURE_2D,0);
   }
  else
   {
    Entry  = &TextureSet[Handle - 1];
   }

  Entry->RefCount++;

  return(Handle);
 }

P3DTexHandle       P3DTexManagerGL::GetHandleByGenericName
                                      (const char         *GenericName)
 {
  P3DTexHandle     TexHandle;

  TexHandle = FindByGenericName(GenericName);

  if (TexHandle != P3DTexHandleNULL)
   {
    P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

    Entry->RefCount++;
   }

  return(TexHandle);
 }

void               P3DTexManagerGL::FreeTexture
                                      (P3DTexHandle        TexHandle)
 {
  if ((TexHandle > TextureSet.size()) || (TexHandle < 1))
   {
    return; /* FIXME: throw something here */
   }

  P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

  if (Entry->RefCount == 0)
   {
    return; /* FIXME: throw something here */
   }

  Entry->RefCount--;

  if (Entry->RefCount == 0)
   {
    glDeleteTextures(1,&Entry->GLHandle);

    delete Entry->Bitmap;
   }
 }

GLuint             P3DTexManagerGL::GetGLHandle
                                      (P3DTexHandle        TexHandle) const
 {
  if ((TexHandle < 1) || (TexHandle > TextureSet.size()))
   {
    return(0);
   }

  const P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

  if (Entry->RefCount > 0)
   {
    return(Entry->GLHandle);
   }
  else
   {
    return(0);
   }
 }

const wxBitmap    *P3DTexManagerGL::GetBitmap
                                      (P3DTexHandle        TexHandle) const
 {
  if ((TexHandle < 1) || (TexHandle > TextureSet.size()))
   {
    return(0);
   }

  const P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

  if (Entry->RefCount > 0)
   {
    return(Entry->Bitmap);
   }
  else
   {
    return(0);
   }
 }

const char        *P3DTexManagerGL::GetGenericName
                                      (P3DTexHandle        TexHandle) const
 {
  if ((TexHandle < 1) || (TexHandle > TextureSet.size()))
   {
    return(0);
   }

  const P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

  if (Entry->RefCount > 0)
   {
    return(Entry->GenericName.c_str());
   }
  else
   {
    return(0);
   }
 }

const char        *P3DTexManagerGL::GetTexFileName
                                      (P3DTexHandle        TexHandle) const
 {
  if ((TexHandle < 1) || (TexHandle > TextureSet.size()))
   {
    return(0);
   }

  const P3DTexManagerGLEntry *Entry = &TextureSet[TexHandle - 1];

  if (Entry->RefCount > 0)
   {
    return(Entry->FileName.c_str());
   }
  else
   {
    return(0);
   }
 }

P3DTexHandle       P3DTexManagerGL::GetUnusedSlot
                                      ()
 {
  P3DTexHandle                         Handle;

  for (Handle = 0; Handle < TextureSet.size(); Handle++)
   {
    if (TextureSet[Handle].RefCount == 0)
     {
      return(Handle + 1);
     }
   }

  P3DTexManagerGLEntry                 NewEntry;

  NewEntry.RefCount = 0;
  NewEntry.GLHandle = 0;
  NewEntry.Bitmap   = 0;

  TextureSet.push_back(NewEntry);

  return(TextureSet.size());
 }

P3DTexHandle       P3DTexManagerGL::FindByFileName
                                      (const char         *FileName) const
 {
  P3DTexHandle                         Handle;

  for (Handle = 0; Handle < TextureSet.size(); Handle++)
   {
    if (TextureSet[Handle].RefCount > 0)
     {
      if (TextureSet[Handle].FileName == FileName)
       {
        return(Handle + 1);
       }
     }
   }

  return(P3DTexHandleNULL);
 }

P3DTexHandle       P3DTexManagerGL::FindByGenericName
                                      (const char         *GenericName) const
 {
  P3DTexHandle                         Handle;

  for (Handle = 0; Handle < TextureSet.size(); Handle++)
   {
    if (TextureSet[Handle].RefCount > 0)
     {
      if (TextureSet[Handle].GenericName == GenericName)
       {
        return(Handle + 1);
       }
     }
   }

  return(P3DTexHandleNULL);
 }

