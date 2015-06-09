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

#include <ngput/p3dglext.h>
#include <ngput/p3dglmemcntx.h>

#ifdef _WIN32

static LRESULT CALLBACK
                   GLWindowClassWndProc
                                      (HWND hwnd,
                                       UINT uMsg,
                                       WPARAM wParam,
                                       LPARAM lParam)
 {
  return(DefWindowProc(hwnd,uMsg,wParam,lParam));
 }

static TCHAR       GLWindowClassName[] = TEXT("NGPGLWindowClass");

static bool        GLWindowClassRegister
                                      ()
 {
  WNDCLASS         ClassInfo;

  if (GetClassInfo(GetModuleHandle(NULL),GLWindowClassName,&ClassInfo) != 0)
   {
    /* class already registered */

    return(true);
   }
  else
   {
    SetLastError(0);
   }

  memset(&ClassInfo,0,sizeof(ClassInfo));

  ClassInfo.style         = CS_OWNDC;
  ClassInfo.lpfnWndProc   = GLWindowClassWndProc;
  ClassInfo.hInstance     = GetModuleHandle(NULL);
  ClassInfo.lpszClassName = GLWindowClassName;

  if (RegisterClass(&ClassInfo) != 0)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }


static bool        GLWindowCreate     (HWND               *WindowHandle,
                                       unsigned int        Width,
                                       unsigned int        Height)
 {
  *WindowHandle = CreateWindow(GLWindowClassName,
                               TEXT("NGPGLWindow"),
                               WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0,
                               0,
                               Width,
                               Height,
                               NULL,
                               NULL,
                               GetModuleHandle(0),
                               NULL);

  if (*WindowHandle != NULL)
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }


                   P3DGLWindowContext::P3DGLWindowContext
                                      ()
 {
  WindowHandle = NULL;
  GLContext    = NULL;
 }

                   P3DGLWindowContext::~P3DGLWindowContext
                                      ()
 {
  if (GLContext != NULL)
   {
    wglDeleteContext(GLContext);
   }

  if (WindowHandle != NULL)
   {
    DestroyWindow(WindowHandle);
   }
 }

bool               P3DGLWindowContext::Create
                                      (unsigned int        Width,
                                       unsigned int        Height)
 {
  bool                  Result;
  PIXELFORMATDESCRIPTOR PixelFormatDesc;
  int                   PixelFormat;
  HDC                   DeviceContext;

  DeviceContext = NULL;

  Result = GLWindowClassRegister();

  if (Result)
   {
    Result = GLWindowCreate(&WindowHandle,Width,Height);
   }

  if (Result)
   {
    DeviceContext = GetDC(WindowHandle);

    if (DeviceContext == NULL)
     {
      Result = false;
     }
   }

  if (Result)
   {
    memset(&PixelFormatDesc,0,sizeof(PixelFormatDesc));

    PixelFormatDesc.nSize      = sizeof(PixelFormatDesc);
    PixelFormatDesc.nVersion   = 1;
    PixelFormatDesc.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    PixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    PixelFormatDesc.cColorBits = 24;

    PixelFormat = ChoosePixelFormat(DeviceContext,&PixelFormatDesc);

    if (PixelFormat == 0)
     {
      Result = false;
     }
   }

  if (Result)
   {
    if (!SetPixelFormat(DeviceContext,PixelFormat,&PixelFormatDesc))
     {
      Result = false;
     }
   }

  if (Result)
   {
    GLContext = wglCreateContext(DeviceContext);

    if (GLContext == NULL)
     {
      Result = false;
     }
   }

  if (wglMakeCurrent(DeviceContext,GLContext))
   {
    Result = P3DGLExtInit();
   }
  else
   {
    Result = false;
   }

  if (DeviceContext != NULL)
   {
    ReleaseDC(WindowHandle,DeviceContext);
   }

  return(Result);
 }

bool               P3DGLWindowContext::MakeCurrent
                                      ()
 {
  bool                                 Result;
  HDC                                  DeviceContext;

  Result        = false;
  DeviceContext = GetDC(WindowHandle);

  if (DeviceContext != NULL)
   {
    if (wglMakeCurrent(DeviceContext,GLContext))
     {
      Result = true;
     }

    ReleaseDC(WindowHandle,DeviceContext);
   }

  return(Result);
 }


                   P3DGLMemoryContextBitmap::P3DGLMemoryContextBitmap
                                      ()
 {
  MemoryDC     = NULL;
  BitmapHandle = NULL;
  GLContext    = NULL;
 }

                   P3DGLMemoryContextBitmap::~P3DGLMemoryContextBitmap
                                      ()
 {
  if (GLContext != NULL)
   {
    wglDeleteContext(GLContext);
   }

  if (BitmapHandle != NULL)
   {
    DeleteObject(BitmapHandle);
   }

  if (MemoryDC != NULL)
   {
    DeleteDC(MemoryDC);
   }
 }

bool               P3DGLMemoryContextBitmap::Create
                                      (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha)
 {
  BITMAPINFOHEADER                     BitmapInfo;
  void                                *BitmapBuffer;
  PIXELFORMATDESCRIPTOR                PixelFormat;
  int                                  Descriptor;

  MemoryDC = CreateCompatibleDC(NULL);

  if (MemoryDC == NULL)
   {
    return(false);
   }

  memset(&BitmapInfo,0,sizeof(BitmapInfo));

  BitmapInfo.biSize        = sizeof(BitmapInfo);
  BitmapInfo.biWidth       = Width;
  BitmapInfo.biHeight      = Height;
  BitmapInfo.biPlanes      = 1;
  BitmapInfo.biBitCount    = 24;
  BitmapInfo.biCompression = BI_RGB;

  BitmapHandle = CreateDIBSection( MemoryDC,
                                  (BITMAPINFO*)&BitmapInfo,
                                   DIB_RGB_COLORS,
                                  &BitmapBuffer,
                                   NULL,
                                  0);

  if (BitmapHandle == NULL)
   {
    DeleteDC(MemoryDC);

    MemoryDC = NULL;

    return(false);
   }

  SelectObject(MemoryDC,BitmapHandle);

  memset(&PixelFormat,0,sizeof(PixelFormat));

  PixelFormat.nSize      = sizeof(PixelFormat);
  PixelFormat.nVersion   = 1;
  PixelFormat.dwFlags    = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL;
  PixelFormat.iPixelType = PFD_TYPE_RGBA;
  PixelFormat.cColorBits = 24;
  PixelFormat.cDepthBits = 16;
  PixelFormat.iLayerType = PFD_MAIN_PLANE;

  if (NeedAlpha)
   {
    PixelFormat.cAlphaBits = 8;
   }

  Descriptor = ChoosePixelFormat(MemoryDC,&PixelFormat);

  if (Descriptor == 0)
   {
    DeleteObject(BitmapHandle);
    DeleteDC(MemoryDC);

    MemoryDC     = NULL;
    BitmapHandle = NULL;

    return(false);
   }

  if (!SetPixelFormat(MemoryDC,Descriptor,&PixelFormat))
   {
    DeleteObject(BitmapHandle);
    DeleteDC(MemoryDC);

    MemoryDC     = NULL;
    BitmapHandle = NULL;

    return(false);
   }

  GLContext = wglCreateContext(MemoryDC);

  if (GLContext == NULL)
   {
    DeleteObject(BitmapHandle);
    DeleteDC(MemoryDC);

    MemoryDC     = NULL;
    BitmapHandle = NULL;

    return(false);
   }

  if (MakeCurrent())
   {
    return(P3DGLExtInit());
   }
  else
   {
    return(false);
   }
 }

bool               P3DGLMemoryContextBitmap::MakeCurrent
                                      ()
 {
  if (GLContext == NULL)
   {
    return(false);
   }

  if (wglMakeCurrent(MemoryDC,GLContext))
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

                   P3DGLMemoryContextPBuffer::P3DGLMemoryContextPBuffer
                                      ()
 {
  PBufferHandle = NULL;
  PBufferDC     = NULL;
  GLContext     = NULL;
 }

                   P3DGLMemoryContextPBuffer::~P3DGLMemoryContextPBuffer
                                      ()
 {
  if (GLContext != NULL)
   {
    wglDeleteContext(GLContext);
   }

  if (PBufferDC != NULL)
   {
    wglReleasePbufferDCARB(PBufferHandle,PBufferDC);
   }

  if (PBufferHandle != NULL)
   {
    wglDestroyPbufferARB(PBufferHandle);
   }
 }

bool               P3DGLMemoryContextPBuffer::IsSupported
                                      ()
 {
  return(WGLEW_ARB_pbuffer);
 }

static int         PBufferPixelFormatIntAttrsNoAlpha[] =
 {
  WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
  WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
  WGL_RED_BITS_ARB, 8,
  WGL_GREEN_BITS_ARB, 8,
  WGL_BLUE_BITS_ARB, 8,
  WGL_DEPTH_BITS_ARB, 16,
  0, 0
 };

static int         PBufferPixelFormatIntAttrsAlpha[] =
 {
  WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
  WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
  WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
  WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
  WGL_RED_BITS_ARB, 8,
  WGL_GREEN_BITS_ARB, 8,
  WGL_BLUE_BITS_ARB, 8,
  WGL_ALPHA_BITS_ARB, 8,
  WGL_DEPTH_BITS_ARB, 16,
  0, 0
 };

static int         PBufferAttrs[] = { 0,0 };

bool               P3DGLMemoryContextPBuffer::Create
                                      (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha)
 {
  int              PixelFormat;
  UINT             FormatCount;
  HDC              CurrDeviceContext;

  CurrDeviceContext = wglGetCurrentDC();

  if (wglChoosePixelFormatARB( CurrDeviceContext,
                               NeedAlpha ? PBufferPixelFormatIntAttrsAlpha :
                                           PBufferPixelFormatIntAttrsNoAlpha,
                               NULL,
                               1,
                              &PixelFormat,
                              &FormatCount))
   {
    if (FormatCount == 0)
     {
      return(false);
     }
   }
  else
   {
    return(false);
   }

  PBufferHandle = wglCreatePbufferARB
                   (CurrDeviceContext,
                    PixelFormat, 
                    Width,
                    Height,
                    PBufferAttrs);

  if (PBufferHandle == NULL)
   {
    return(false);
   }

  PBufferDC = wglGetPbufferDCARB(PBufferHandle);
  GLContext = wglCreateContext(PBufferDC);

  if (MakeCurrent())
   {
    return(P3DGLExtInit());
   }
  else
   {
    return(false);
   }

  return(true);
 }

bool               P3DGLMemoryContextPBuffer::MakeCurrent
                                      ()
 {
  if (wglMakeCurrent(PBufferDC,GLContext))
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

#else

static int         VisualAttrArray[] =
 {
  GLX_RGBA,
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8,
  GLX_DEPTH_SIZE, 16,
  None
 };

static int         VisualAttrArrayAlpha[] =
 {
  GLX_RGBA,
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8,
  GLX_ALPHA_SIZE, 8,
  GLX_DEPTH_SIZE, 16,
  None
 };

                   P3DGLMemoryContextPixmap::P3DGLMemoryContextPixmap
                                      (Display            *Connection)
 {
  this->Connection = Connection;
  OwnConnection    = false;
  Ok               = false;
 }

                   P3DGLMemoryContextPixmap::~P3DGLMemoryContextPixmap
                                      ()
 {
  if (Ok)
   {
    glXDestroyContext(Connection,OffScreenGLXContext);
    glXDestroyGLXPixmap(Connection,TargetGLXPixmap);
    XFreePixmap(Connection,TargetPixmap);
   }

  if (OwnConnection)
   {
    if (Connection != NULL)
     {
      XCloseDisplay(Connection);
     }
   }
 }

bool               P3DGLMemoryContextPixmap::Create
                                      (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha)
 {
  bool                                 Result;
  XVisualInfo                         *VisualInfo;

  Result = true;

  if (Connection == NULL)
   {
    OwnConnection = true;
    Connection = XOpenDisplay(NULL);

    if (Connection == NULL)
     {
      return(false);
     }
   }
  else
   {
    OwnConnection = false;
   }

  Result = true;

  if (NeedAlpha)
   {
    VisualInfo = glXChooseVisual(Connection,
                                 DefaultScreen(Connection),
                                 VisualAttrArrayAlpha);
   }
  else
   {
    VisualInfo = glXChooseVisual(Connection,
                                 DefaultScreen(Connection),
                                 VisualAttrArray);
   }

  if (VisualInfo != NULL)
   {
    TargetPixmap = XCreatePixmap(Connection,
                                 DefaultRootWindow(Connection),
                                 Width,
                                 Height,
                                 VisualInfo->depth);

    TargetGLXPixmap = glXCreateGLXPixmap(Connection,VisualInfo,TargetPixmap);

    /*FIXME: check for errors here */

    OffScreenGLXContext = glXCreateContext(Connection,VisualInfo,NULL,False);

    if (OffScreenGLXContext != NULL)
     {
      Ok = true;

      Result = MakeCurrent();

      if (Result)
       {
        Result = P3DGLExtInit();
       }

      Ok = Result;
     }
    else
     {
      Result = false;
     }

    if (!Result)
     {
      glXDestroyGLXPixmap(Connection,TargetGLXPixmap);
      XFreePixmap(Connection,TargetPixmap);
     }

    XFree(VisualInfo);
   }
  else
   {
    Result = false;
   }

  if (!Result)
   {
    if (OwnConnection)
     {
      XCloseDisplay(Connection);
     }

    Connection = NULL;
   }

  Ok = Result;

  return(Result);
 }

bool               P3DGLMemoryContextPixmap::MakeCurrent
                                      ()
 {
  if (!Ok)
   {
    return(false);
   }

  if (glXMakeCurrent(Connection,TargetGLXPixmap,OffScreenGLXContext))
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

                   P3DGLMemoryContextPBuffer::P3DGLMemoryContextPBuffer
                                      (Display            *Connection)
 {
  this->Connection = Connection;
  OwnConnection    = false;
  Ok               = false;
 }

                   P3DGLMemoryContextPBuffer::~P3DGLMemoryContextPBuffer
                                      ()
 {
  if (Ok)
   {
    glXDestroyContext(Connection,OffScreenGLXContext);
    glXDestroyPbuffer(Connection,PBuffer);
   }

  if (OwnConnection)
   {
    if (Connection != NULL)
     {
      XCloseDisplay(Connection);
     }
   }
 }

static int         FBAttrsArray[] =
 {
  GLX_BUFFER_SIZE  , 24,
  GLX_DEPTH_SIZE   , 16,
  GLX_DOUBLEBUFFER , False,
  GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
  None, None
 };

static int         FBAttrsAlphaArray[] =
 {
  GLX_BUFFER_SIZE  , 32,
  GLX_ALPHA_SIZE   , 8,
  GLX_DEPTH_SIZE   , 16,
  GLX_DOUBLEBUFFER , False,
  GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
  None, None
 };

bool               P3DGLMemoryContextPBuffer::Create
                                      (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha)
 {
  bool                                 Result;
  GLXFBConfig                         *FBConfigList;
  int                                  FBConfigCount;

  Result = true;

  if (Connection == NULL)
   {
    OwnConnection = true;
    Connection = XOpenDisplay(NULL);

    if (Connection == NULL)
     {
      return(false);
     }
   }
  else
   {
    OwnConnection = false;
   }

  Result = true;

  FBConfigList = glXChooseFBConfig( Connection,
                                    DefaultScreen(Connection),
                                    NeedAlpha ? FBAttrsAlphaArray : FBAttrsArray,
                                   &FBConfigCount);


  if (FBConfigList != NULL)
   {
    int            AttrList[8];

    AttrList[0] = GLX_PBUFFER_WIDTH;      AttrList[1] = Width;
    AttrList[2] = GLX_PBUFFER_HEIGHT;     AttrList[3] = Height;
    AttrList[4] = GLX_PRESERVED_CONTENTS; AttrList[5] = True;
    AttrList[6] = None; AttrList[7] = None;

    PBuffer = glXCreatePbuffer(Connection,FBConfigList[0],AttrList);

    /*FIXME: check for errors here */

    OffScreenGLXContext = glXCreateNewContext(Connection,FBConfigList[0],GLX_RGBA_TYPE,NULL,True);

    if (OffScreenGLXContext != NULL)
     {
      Ok = true;

      Result = MakeCurrent();

      if (Result)
       {
        Result = P3DGLExtInit();
       }

      Ok = Result;
     }
    else
     {
      Result = false;
     }

    if (!Result)
     {
      glXDestroyPbuffer(Connection,PBuffer);
     }

    XFree(FBConfigList);
   }
  else
   {
    Result = false;
   }

  if (!Result)
   {
    if (OwnConnection)
     {
      XCloseDisplay(Connection);
     }

    Connection = NULL;
   }

  Ok = Result;

  return(Result);
 }

bool               P3DGLMemoryContextPBuffer::MakeCurrent
                                      ()
 {
  if (!Ok)
   {
    return(false);
   }

  if (glXMakeCurrent(Connection,PBuffer,OffScreenGLXContext))
   {
    return(true);
   }
  else
   {
    return(false);
   }
 }

#endif

