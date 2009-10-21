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

#ifndef __P3DGLMEMCNTX_H__
#define __P3DGLMEMCNTX_H__

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

class P3DGLWindowContext
 {
  public           :

                   P3DGLWindowContext ();
                  ~P3DGLWindowContext ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height);

  bool             MakeCurrent        ();

  private          :


  HWND             WindowHandle;
  HGLRC            GLContext;
 };

class P3DGLMemoryContextBitmap
 {
  public           :

                   P3DGLMemoryContextBitmap
                                      ();
                  ~P3DGLMemoryContextBitmap
                                      ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha);
  bool             MakeCurrent        ();

  private          :

  HDC              MemoryDC;
  HBITMAP          BitmapHandle;
  HGLRC            GLContext;
 };

class P3DGLMemoryContextPBuffer
 {
  public           :

                   P3DGLMemoryContextPBuffer
                                      ();
                  ~P3DGLMemoryContextPBuffer
                                      ();

  static bool      IsSupported        ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha);
  bool             MakeCurrent        ();

  private          :

  HPBUFFERARB      PBufferHandle;
  HDC              PBufferDC;
  HGLRC            GLContext;
 };

#else

#include <X11/Xlib.h>

#include <GL/glx.h>

class P3DGLMemoryContextPixmap
 {
  public           :

                   P3DGLMemoryContextPixmap
                                      (Display            *Connection);
                  ~P3DGLMemoryContextPixmap ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha);
  bool             MakeCurrent        ();

  private          :

  Display                             *Connection;
  bool                                 OwnConnection;
  Pixmap                               TargetPixmap;
  GLXPixmap                            TargetGLXPixmap;
  GLXContext                           OffScreenGLXContext;
  bool                                 Ok;
 };

class P3DGLMemoryContextPBuffer
 {
  public           :

                   P3DGLMemoryContextPBuffer
                                      (Display            *Connection);
                  ~P3DGLMemoryContextPBuffer
                                      ();

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha);
  bool             MakeCurrent        ();

  private          :

  Display                             *Connection;
  bool                                 OwnConnection;
  GLXContext                           OffScreenGLXContext;
  bool                                 Ok;

  GLXPbuffer                           PBuffer;
 };

#endif

#endif

