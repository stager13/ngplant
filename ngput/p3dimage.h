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

#ifndef __P3DIMAGE_H__
#define __P3DIMAGE_H__

#include <ngpcore/p3dtypes.h>

#include <vector>

class P3DImageData
 {
  public           :

                   P3DImageData       ();
                  ~P3DImageData       ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       unsigned int        ChannelCount,
                                       unsigned int        ChannelType);

  bool             IsEmpty            () const;

  unsigned int     GetWidth           () const;
  unsigned int     GetHeight          () const;
  unsigned int     GetChannelCount    () const;
  unsigned int     GetChannelType     () const;

  void            *GetData            ();
  const void      *GetConstData       () const;

  void             PutPixel           (unsigned int        X,
                                       unsigned int        Y,
                                       const unsigned char*Color);

  void             GetPixel           (unsigned int        X,
                                       unsigned int        Y,
                                       unsigned char      *Color) const;

  void            *DetachData         ();

  /* remove alpha channel from SourceImage                              */
  /* NOTE: if source image does not have alpha, TargetImage will not be */
  /*       modified                                                     */
  static bool      RemoveAlpha        (P3DImageData       *TargetImage,
                                       const P3DImageData *SourceImage);

  static bool      Copy               (P3DImageData       *TargetImage,
                                       const P3DImageData *SourceImage);

  void             FlipVertical       ();

  private          :

  unsigned int     Width;
  unsigned int     Height;
  unsigned int     ChannelCount;
  unsigned int     ChannelType;
  void            *Data;
 };

class P3DImageFmtHandler
 {
  public           :

  virtual         ~P3DImageFmtHandler () {};

  virtual
  unsigned int     FormatCount        () const = 0;

  virtual
  const char      *FormatExt          (unsigned int        FormatIndex) const = 0;

  virtual
  bool             LoadImageData      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt) const = 0;
 };

class P3DImageFmtHandlerComposite : public P3DImageFmtHandler
 {
  public           :

                   P3DImageFmtHandlerComposite
                                      ();
  virtual         ~P3DImageFmtHandlerComposite
                                      ();

  void             AddHandler         (P3DImageFmtHandler *Handler);

  virtual
  unsigned int     FormatCount        () const;

  virtual
  const char      *FormatExt          (unsigned int        FormatIndex) const;

  virtual
  bool             LoadImageData      (P3DImageData       *ImageData,
                                       const char         *FileName,
                                       const char         *FileExt) const;

  private          :

  std::vector<P3DImageFmtHandler*>     Handlers;
 };

#endif

