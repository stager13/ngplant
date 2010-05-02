/***************************************************************************

 Copyright (C) 2007  Sergey Prokhorchuk

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

***************************************************************************/

#include <string>
#include <map>

#include <ngput/p3dglext.h>
#include <ngput/p3dimage.h>
#include <ngput/p3dimagetga.h>

#ifdef WITH_LIBPNG
 #include <ngput/p3dimagepng.h>
#endif
#ifdef WITH_LIBJPEG
 #include <ngput/p3dimagejpg.h>
#endif

#include <ngput/p3dospath.h>

#include <ngpviewtexman.h>


                   NGPViewTexManager::NGPViewTexManager
                                      (const char         *TexPath)
 {
  this->TexPath = TexPath;

  ImageFmtHandler.AddHandler(new P3DImageFmtHandlerTGA());
  #ifdef WITH_LIBPNG
  ImageFmtHandler.AddHandler(new P3DImageFmtHandlerPNG());
  #endif
  #ifdef WITH_LIBJPEG
  ImageFmtHandler.AddHandler(new P3DImageFmtHandlerJPG());
  #endif
 }

                   NGPViewTexManager::~NGPViewTexManager
                                      ()
 {
  for (std::map<std::string,GLuint>::iterator Iter = Handles.begin();
       Iter != Handles.end();
       ++Iter)
   {
    GLuint         Handle;

    Handle = Iter->second;

    if (Handle != 0)
     {
      glDeleteTextures(1,&Handle);
     }
   }
 }

GLuint             NGPViewTexManager::LoadTexture
                                      (const char         *TexName)
 {
  std::map<std::string,GLuint>::iterator Iter = Handles.find(TexName);

  if (Iter != Handles.end())
   {
    return(Iter->second);
   }
  else
   {
    std::string                        FullPathStr;
    GLuint                             Handle;

    FullPathStr = TexPath + std::string("/") + TexName;

    P3DImageData                       ImageData;
    P3DPathName                        FullPathName(FullPathStr.c_str());
    std::string                        Ext;

    Ext = FullPathName.GetExtension();

    if (!ImageFmtHandler.LoadImageData
          (&ImageData,FullPathName.c_str(),Ext.c_str()))
     {
      return(0);
     }

    glGenTextures(1,&Handle);

    glBindTexture(GL_TEXTURE_2D,Handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    if      (ImageData.GetChannelCount() == 3)
     {
      gluBuild2DMipmaps(GL_TEXTURE_2D,
                        3,
                        ImageData.GetWidth(),
                        ImageData.GetHeight(),
                        GL_RGB,
                        GL_UNSIGNED_BYTE,
                        ImageData.GetData());
     }
    else if (ImageData.GetChannelCount() == 4)
     {
      gluBuild2DMipmaps(GL_TEXTURE_2D,
                        4,
                        ImageData.GetWidth(),
                        ImageData.GetHeight(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        ImageData.GetData());
     }
    else
     {
      glDeleteTextures(1,&Handle);

      return(0);
     }

    glBindTexture(GL_TEXTURE_2D,0);

    Handles[TexName] = Handle;

    return(Handle);
   }
 }

