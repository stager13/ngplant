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

#include <stdio.h>
#include <stdlib.h>

#include <new>
#include <string>
#include <map>

#include <ngput/p3dglext.h>

#if defined(_WIN32)
 #if defined(GetMessage)
  #undef GetMessage
 #endif
#endif

#include <ngpcore/p3dhli.h>

#include <ngput/p3dimage.h>
#include <ngput/p3dimagetga.h>

#include <ngput/p3dospath.h>

#include <ngput/p3dglmemcntx.h>

#include <p3dshaders.h>

typedef struct
 {
  float            c[3];
 } NGPShotColor3f;

enum
 {
  GLOffScreenTargetAuto   ,
  GLOffScreenTargetPixmap ,
  GLOffScreenTargetPBuffer
 };

class NGPShotGLContextWrapper
 {
  public           :

                   NGPShotGLContextWrapper
                                      ()
   #if !defined(_WIN32)
   : ContextPixmap(NULL),ContextPBuffer(NULL)
   #endif
   {
   }

  bool             CreateBaseContext  ()
   {
    #if defined(_WIN32)
    if (BaseContext.Create(1,1))
     {
      return(BaseContext.MakeCurrent());
     }
    else
     {
      return(false);
     }
    #else
    return(true);
    #endif
   }

  bool             IsPBufferSupported () const
   {
    #if defined(_WIN32)
    return(P3DGLMemoryContextPBuffer::IsSupported());
    #else
    return(true); /* FIXME: check p-buffer support here */
    #endif
   }

  bool             Create             (unsigned int        Width,
                                       unsigned int        Height,
                                       bool                NeedAlpha,
                                       bool                PBuffer)
   {
    this->PBuffer = PBuffer;

    if (PBuffer)
     {
      return(ContextPBuffer.Create(Width,Height,NeedAlpha));
     }
    else
     {
      #if defined(_WIN32)
      return(ContextBitmap.Create(Width,Height,NeedAlpha));
      #else
      return(ContextPixmap.Create(Width,Height,NeedAlpha));
      #endif
     }
   }

  bool             MakeCurrent        ()
   {
    if (PBuffer)
     {
      return(ContextPBuffer.MakeCurrent());
     }
    else
     {
      #if defined(_WIN32)
      return(ContextBitmap.MakeCurrent());
      #else
      return(ContextPixmap.MakeCurrent());
      #endif
     }
   }

  private          :

  bool                                 PBuffer;

  #if defined(_WIN32)
  P3DGLWindowContext                   BaseContext;
  P3DGLMemoryContextBitmap             ContextBitmap;
  P3DGLMemoryContextPBuffer            ContextPBuffer;
  #else
  P3DGLMemoryContextPixmap             ContextPixmap;
  P3DGLMemoryContextPBuffer            ContextPBuffer;
  #endif
 };

class NGPTexManager
 {
  public           :

                   NGPTexManager      (const char         *TexPath);
                  ~NGPTexManager      ();

  GLuint           GetTextureHandle   (const char         *GenericName);

  private          :

  std::string                          TexPath;
  std::map<std::string,GLuint>         Handles;
  P3DImageFmtHandlerComposite          ImageFmtHandler;
 };

                   NGPTexManager::NGPTexManager
                                      (const char         *TexPath)
 {
  this->TexPath = TexPath;

  ImageFmtHandler.AddHandler(new P3DImageFmtHandlerTGA());
 }

                   NGPTexManager::~NGPTexManager
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

GLuint             NGPTexManager::GetTextureHandle
                                      (const char         *GenericName)
 {
  std::map<std::string,GLuint>::iterator Iter = Handles.find(GenericName);

  if (Iter != Handles.end())
   {
    return(Iter->second);
   }
  else
   {
    std::string                        FullPathStr;
    GLuint                             Handle;

    FullPathStr = TexPath + std::string("/") + GenericName;

    P3DImageData                       ImageData;

    if (!ImageFmtHandler.LoadImageData(&ImageData,FullPathStr.c_str(),"tga"))
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

    Handles[GenericName] = Handle;

    return(Handle);
   }
 }

static char       *LoadFileContent    (const char         *FileName)
 {
  FILE                                *SrcFile;
  char                                 Buffer[256];
  char                                *Block;
  size_t                               BlockSize;

  Block     = 0;
  BlockSize = 0;
  SrcFile   = fopen(FileName,"rb");

  if (SrcFile != NULL)
   {
    size_t                             ReadSize;
    size_t                             NewBlockSize;
    bool                               Ok;

    Ok = true;

    do
     {
      ReadSize = fread(Buffer,1,sizeof(Buffer),SrcFile);

      NewBlockSize = BlockSize + ReadSize + (Block == 0 ? 1 : 0);

      void *NewBlock = realloc(Block,NewBlockSize);

      if (NewBlock != 0)
       {
        Block = (char*)NewBlock;

        if (BlockSize == 0)
         {
          memcpy(Block,Buffer,ReadSize);
         }
        else
         {
          memcpy(&Block[BlockSize - 1],Buffer,ReadSize);
         }

        BlockSize = NewBlockSize;

        Block[NewBlockSize - 1] = 0;
       }
      else
       {
        Ok = false;

        fprintf(stderr,"error: out of memory\n");
       }
     } while ((ReadSize == sizeof(Buffer)) && (Ok));

    if (Ok)
     {
      if (ferror(SrcFile))
       {
        fprintf(stderr,"error: file read error (%s)\n",FileName);

        Ok = false;
       }
     }

    if (!Ok)
     {
      free(Block);

      Block = 0;
     }

    fclose(SrcFile);
   }
  else
   {
    fprintf(stderr,"error: unable to open file %s\n",FileName);
   }

  return(Block);
 }

/* almost exact copy of UpdateBillboardsInfo from ngplant/p3dpobject.cpp */
static void        UpdateBillboardsInfo
                                      (float              *PosBuffer,
                                       float              *BillboardNormal,
                                       unsigned int        BillboardMode,
                                       float               BillboardWidth,
                                       float               BillboardHeight,
                                       unsigned int        BranchCount)
 {
  GLfloat                              ModelViewMatrix[16];
  float                                HalfWidth;
  float                                HalfHeight;
  float                                Up[3];
  float                                Right[3];

  glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);

  HalfWidth  = BillboardWidth  * 0.5f;
  HalfHeight = BillboardHeight * 0.5f;

  Right[0] = ModelViewMatrix[0] * HalfWidth;
  Right[1] = ModelViewMatrix[4] * HalfWidth;
  Right[2] = ModelViewMatrix[8] * HalfWidth;

  Up[1] = ModelViewMatrix[5] * HalfHeight;

  if (BillboardMode == P3D_BILLBOARD_MODE_CYLINDRICAL)
   {
    Up[0] = Up[2] = 0.0f;
   }
  else
   {
    Up[0] = ModelViewMatrix[1] * HalfHeight;
    Up[2] = ModelViewMatrix[9] * HalfHeight;
   }

  BillboardNormal[0] = ModelViewMatrix[2];
  BillboardNormal[1] = ModelViewMatrix[6];
  BillboardNormal[2] = ModelViewMatrix[10];

  for (unsigned int BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
   {
    PosBuffer[0] = PosBuffer[0] - Up[0] - Right[0];
    PosBuffer[1] = PosBuffer[1] - Up[1] - Right[1];
    PosBuffer[2] = PosBuffer[2] - Up[2] - Right[2];

    PosBuffer += 3;

    PosBuffer[0] = PosBuffer[0] - Up[0] + Right[0];
    PosBuffer[1] = PosBuffer[1] - Up[1] + Right[1];
    PosBuffer[2] = PosBuffer[2] - Up[2] + Right[2];

    PosBuffer += 3;

    PosBuffer[0] = PosBuffer[0] + Up[0] - Right[0];
    PosBuffer[1] = PosBuffer[1] + Up[1] - Right[1];
    PosBuffer[2] = PosBuffer[2] + Up[2] - Right[2];

    PosBuffer += 3;

    PosBuffer[0] = PosBuffer[0] + Up[0] + Right[0];
    PosBuffer[1] = PosBuffer[1] + Up[1] + Right[1];
    PosBuffer[2] = PosBuffer[2] + Up[2] + Right[2];

    PosBuffer += 3;
   }
 }

static void        RenderBranchGroup  (P3DHLIPlantTemplate*PlantTemplate,
                                       P3DHLIPlantInstance*PlantInstance,
                                       unsigned int        GroupIndex,
                                       float               LOD,
                                       NGPTexManager      *TexManager,
                                       P3DShaderLoader    *ShaderLoader)
 {
  unsigned int                         BranchIndex;
  unsigned int                         BranchCount;
  const P3DMaterialDef                *MaterialDef;
  GLuint                               DiffuseTexHandle;
  GLuint                               NormalTexHandle;
  float                                BillboardNormal[3];
  GLint                                BiNormalLocation;

  BranchCount = PlantInstance->GetBranchCount(GroupIndex);

  if (BranchCount == 0)
   {
    return;
   }

  MaterialDef = PlantTemplate->GetMaterial(GroupIndex);

  if (PlantTemplate->IsLODVisRangeEnabled(GroupIndex))
   {
    float          MinLOD,MaxLOD;

    PlantTemplate->GetLODVisRange(&MinLOD,&MaxLOD,GroupIndex);

    if ((LOD < MinLOD) || (LOD > MaxLOD))
     {
      return;
     }
   }

  float            R,G,B;

  MaterialDef->GetColor(&R,&G,&B);

  glColor3f(R,G,B);

  if (MaterialDef->IsDoubleSided())
   {
    glDisable(GL_CULL_FACE);
   }
  else
   {
    glEnable(GL_CULL_FACE);
   }

  if (MaterialDef->IsTransparent())
   {
    glAlphaFunc(GL_GREATER,0.5);
    glEnable(GL_ALPHA_TEST);
   }
  else
   {
    glDisable(GL_ALPHA_TEST);
   }

  DiffuseTexHandle = 0;
  NormalTexHandle  = 0;

  if (MaterialDef->GetTexName(P3D_TEX_DIFFUSE) != 0)
   {
    DiffuseTexHandle = TexManager->GetTextureHandle
                        (MaterialDef->GetTexName(P3D_TEX_DIFFUSE));

    if (DiffuseTexHandle == 0)
     {
      fprintf(stderr,"warning: unable to load texture %s\n",
              MaterialDef->GetTexName(P3D_TEX_DIFFUSE));
     }
   }

  if (MaterialDef->GetTexName(P3D_TEX_NORMAL_MAP) != 0)
   {
    NormalTexHandle = TexManager->GetTextureHandle
                        (MaterialDef->GetTexName(P3D_TEX_NORMAL_MAP));

    if (NormalTexHandle == 0)
     {
      fprintf(stderr,"warning: unable to load texture %s\n",
              MaterialDef->GetTexName(P3D_TEX_NORMAL_MAP));
     }
   }

  if (DiffuseTexHandle == 0)
   {
    glDisable(GL_TEXTURE_2D);
   }
  else
   {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D,DiffuseTexHandle);
   }

  glActiveTexture(GL_TEXTURE1);

  if (NormalTexHandle == 0)
   {
    glDisable(GL_TEXTURE_2D);
   }
  else
   {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D,NormalTexHandle);
   }

  glActiveTexture(GL_TEXTURE0);

  BiNormalLocation = -1;

  if (ShaderLoader != 0)
   {
    GLhandleARB              ProgHandle;

    ProgHandle = ShaderLoader->GetProgramHandle
                  (DiffuseTexHandle != 0,
                   NormalTexHandle != 0,
                   MaterialDef->IsDoubleSided());

    if (ProgHandle != 0)
     {
      #ifdef USE_OPENGL_20
      glUseProgram(ProgHandle);

      BiNormalLocation = glGetAttribLocation(ProgHandle,"ngp_BiNormal");
      #else
      glUseProgramObjectARB(ProgHandle);

      BiNormalLocation = glGetAttribLocationARB(ProgHandle,"ngp_BiNormal");
      #endif
     }
   }

  unsigned int     BranchVAttrCount;
  unsigned int     BranchIndexCount;
  unsigned int     TotalVAttrCount;
  unsigned int     TotalIndexCount;

  BranchVAttrCount = PlantTemplate->GetVAttrCountI(GroupIndex);
  BranchIndexCount = PlantTemplate->GetIndexCount(GroupIndex,P3D_TRIANGLE_LIST);

  TotalVAttrCount = BranchVAttrCount * BranchCount;
  TotalIndexCount = BranchIndexCount * BranchCount;

  float           *PosBuffer      = 0;
  float           *NormalBuffer   = 0;
  float           *BiNormalBuffer = 0;
  float           *TexCoordBuffer = 0;
  unsigned int    *IndexBuffer    = 0;

  PosBuffer      = new(std::nothrow) float[3 * TotalVAttrCount];
  NormalBuffer   = new(std::nothrow) float[3 * TotalVAttrCount];
  TexCoordBuffer = new(std::nothrow) float[2 * TotalVAttrCount];
  IndexBuffer    = new(std::nothrow) unsigned int[TotalIndexCount];

  if (BiNormalLocation != -1)
   {
    BiNormalBuffer = new(std::nothrow) float[3 * TotalVAttrCount];
   }

  if ((PosBuffer != 0) && (NormalBuffer != 0) &&
      (TexCoordBuffer != 0) && (IndexBuffer != 0) &&
      ((BiNormalLocation == -1) || (BiNormalBuffer != 0)))
   {
    P3DHLIVAttrBuffers                 VAttrBuffers;

    if (MaterialDef->IsBillboard())
     {
      VAttrBuffers.AddAttr(P3D_ATTR_BILLBOARD_POS,PosBuffer,0,sizeof(float) * 3);
     }
    else
     {
      VAttrBuffers.AddAttr(P3D_ATTR_VERTEX,PosBuffer,0,sizeof(float) * 3);
     }

    VAttrBuffers.AddAttr(P3D_ATTR_NORMAL,NormalBuffer,0,sizeof(float) * 3);
    VAttrBuffers.AddAttr(P3D_ATTR_TEXCOORD0,TexCoordBuffer,0,sizeof(float) * 2);

    if (BiNormalBuffer != 0)
     {
      VAttrBuffers.AddAttr(P3D_ATTR_BINORMAL,BiNormalBuffer,0,sizeof(float) * 3);
     }

    PlantInstance->FillVAttrBuffersI(&VAttrBuffers,GroupIndex);

    if (MaterialDef->IsBillboard())
     {
      float        BillboardWidth;
      float        BillboardHeight;

      PlantTemplate->GetBillboardSize(&BillboardWidth,&BillboardHeight,GroupIndex);

      UpdateBillboardsInfo(PosBuffer,
                           BillboardNormal,
                           MaterialDef->GetBillboardMode(),
                           BillboardWidth,
                           BillboardHeight,
                           BranchCount);
     }

    for (BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
     {
      PlantTemplate->FillIndexBuffer
       (&IndexBuffer[BranchIndex * BranchIndexCount],
         GroupIndex,
         P3D_TRIANGLE_LIST,
         P3D_UNSIGNED_INT,
         BranchVAttrCount * BranchIndex);
     }

    glVertexPointer(3,GL_FLOAT,0,PosBuffer);
    glEnableClientState(GL_VERTEX_ARRAY);

    if (MaterialDef->IsBillboard())
     {
      glNormal3fv(BillboardNormal);
     }
    else
     {
      glNormalPointer(GL_FLOAT,0,NormalBuffer);
      glEnableClientState(GL_NORMAL_ARRAY);
     }

    if ((DiffuseTexHandle != 0) || (NormalTexHandle != 0))
     {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2,GL_FLOAT,0,TexCoordBuffer);
     }

    if (BiNormalLocation != -1)
     {
      glEnableVertexAttribArrayARB(BiNormalLocation);
      glVertexAttribPointerARB(BiNormalLocation,3,GL_FLOAT,0,GL_FALSE,BiNormalBuffer);
     }

    glDrawElements(GL_TRIANGLES,TotalIndexCount,GL_UNSIGNED_INT,IndexBuffer);

    if (BiNormalLocation != -1)
     {
      glDisableVertexAttribArrayARB(BiNormalLocation);
     }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
   }
  else
   {
    fprintf(stderr,"error: out of memory\n");
   }

  if (ShaderLoader != 0)
   {
    #ifdef USE_OPENGL_20
    glUseProgram(0);
    #else
    glUseProgramObjectARB(0);
    #endif
   }

  delete[] IndexBuffer;
  delete[] TexCoordBuffer;
  delete[] BiNormalBuffer;
  delete[] NormalBuffer;
  delete[] PosBuffer;
 }

static void        Render             (P3DHLIPlantTemplate*PlantTemplate,
                                       P3DHLIPlantInstance*PlantInstance,
                                       const NGPShotColor3f
                                                          *BGColor,
                                       bool                HasAlpha,
                                       float               XAngle,
                                       float               YAngle,
                                       float               LOD,
                                       NGPTexManager      *TexManager,
                                       const char         *VertexProgSrc,
                                       const char         *FragmentProgSrc)
 {
  P3DVector3f                          BBoxMin;
  P3DVector3f                          BBoxMax;
  float                                SizeX;
  float                                SizeY;
  float                                SizeZ;
  float                                OrthoSize;
  P3DShaderLoader                      ShaderLoader(VertexProgSrc,FragmentProgSrc);

  PlantInstance->GetBoundingBox(BBoxMin.v,BBoxMax.v);

  SizeX = BBoxMax.X() - BBoxMin.X();
  SizeY = BBoxMax.Y() - BBoxMin.Y();
  SizeZ = BBoxMax.Z() - BBoxMin.Z();

  if (SizeX > SizeY)
   {
    OrthoSize = SizeX * 0.5f;
   }
  else
   {
    OrthoSize = SizeY * 0.5f;
   }

  P3DVector3f                          Center(BBoxMin);

  Center += BBoxMax;
  Center *= 0.5f;

  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  if (HasAlpha)
   {
    glClearColor(0.0f,0.0f,0.0f,0.0f);
   }
  else
   {
    glClearColor(BGColor->c[0],BGColor->c[1],BGColor->c[2],1.0f);
   }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-OrthoSize,OrthoSize,-OrthoSize,OrthoSize,1.0f,2.0f + SizeZ);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0f,0.0f,-SizeZ * 0.5f - 1.0f);

  glRotatef(XAngle,1.0f,0.0f,0.0f);
  glRotatef(-YAngle,0.0f,1.0f,0.0f);
  glTranslatef(-Center.X(),-Center.Y(),0.0f);

  glDisable(GL_TEXTURE_2D);

  unsigned int GroupIndex;
  unsigned int GroupCount;

  GroupCount = PlantTemplate->GetGroupCount();

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    RenderBranchGroup(PlantTemplate,
                      PlantInstance,
                      GroupIndex,
                      LOD,
                      TexManager,
                      ((VertexProgSrc == 0) && (FragmentProgSrc == 0)) ? 0 : &ShaderLoader);
   }

  glFinish();

  #ifndef _WIN32
  glXWaitX();
  #endif
 }

static bool        SaveImage          (const char         *FileName,
                                       unsigned int        Width,
                                       unsigned int        Height,
                                       bool                HasAlpha)
 {
  bool                                 Result;
  P3DImageData                         Image;

  if (HasAlpha)
   {
    Result = Image.Create(Width,Height,4,P3D_BYTE);
   }
  else
   {
    Result = Image.Create(Width,Height,3,P3D_BYTE);
   }

  if (Result)
   {
    glReadBuffer(GL_FRONT);

    if (HasAlpha)
     {
      glReadPixels(0,0,Width,Height,GL_RGBA,GL_UNSIGNED_BYTE,Image.GetData());
     }
    else
     {
      glReadPixels(0,0,Width,Height,GL_RGB,GL_UNSIGNED_BYTE,Image.GetData());
     }

    Image.FlipVertical();

    Result = P3DImageFmtHandlerTGA::SaveAsTGA(FileName,&Image);

    if (!Result)
     {
      fprintf(stderr,"error: unable to save image\n");
     }
   }
  else
   {
    fprintf(stderr,"error: unable to create image\n");
   }

  return(Result);
 }

static bool        MakeShot           (const char         *ModelFileName,
                                       const char         *ImageFileName,
                                       const char         *TexPath,
                                       unsigned int        Width,
                                       unsigned int        Height,
                                       const NGPShotColor3f
                                                          *BGColor,
                                       bool                HasAlpha,
                                       float               XAngle,
                                       float               YAngle,
                                       float               LOD,
                                       const char         *VertexProgSrc,
                                       const char         *FragmentProgSrc)
 {
  bool                                 Result;
  P3DInputStringStreamFile             SourceStream;
  P3DHLIPlantTemplate                 *PlantTemplate;
  P3DHLIPlantInstance                 *PlantInstance;
  NGPTexManager                        TexManager(TexPath);

  Result = true;

  PlantTemplate = 0;
  PlantInstance = 0;

  try
   {
    SourceStream.Open(ModelFileName);

    PlantTemplate = new P3DHLIPlantTemplate(&SourceStream);

    SourceStream.Close();

    PlantInstance = PlantTemplate->CreateInstance();

    Render( PlantTemplate,
            PlantInstance,
            BGColor,
            HasAlpha,
            XAngle,
            YAngle,
            LOD,
           &TexManager,
            VertexProgSrc,
            FragmentProgSrc);

    Result = SaveImage(ImageFileName,Width,Height,HasAlpha);
   }
  catch (const P3DException &Exception)
   {
    fprintf(stderr,"error: %s\n",Exception.GetMessage());

    Result = false;
   }

  delete PlantInstance;
  delete PlantTemplate;

  return(Result);
 }

static void        ShowHelpMessage    ()
 {
  printf("Usage: ngpshot [options] modelfile imagefile\n");
  printf("Options:\n");
  printf("  -h            Display this information\n");
  printf("  -t <path>     Use <path> for texture search (current dir by default)\n");
  printf("  -s <size>     Set image size to <size>x<size> (256x256 by default)\n");
  printf("  -b <RRGGBB>   Use <RRGGBB> for background (transparent by default)\n");
  printf("  -x <degrees>  Rotate model by <degrees> around X axis (0 by default)\n");
  printf("  -y <degress>  Rotate model by <degrees> around Y axis (0 by default)\n");
  printf("  -l <LOD>      Render model using <LOD> LOD (1.0 by default)\n");
  printf("  -vs <name>    Render model using vertex program from file <name>\n");
  printf("  -fs <name>    Render model using fragment program from file <name>\n");
  printf("  -o auto       Use pbuffer or pixmap offscreen rendering method (autodetect)\n");
  printf("  -o pixmap     Use pixmap for offscreen rendering (auto by default)\n");
  printf("  -o pbuffer    Use pbuffer for offscreen rendering (auto by default)\n");
 }

static bool        ParseColorString   (NGPShotColor3f     *Color,
                                       const char         *ColorStr)
 {
  bool                                 Result;
  char                                 CompStr[2 + 1];
  unsigned int                         Index;

  Result = true;

  if (strlen(ColorStr) != 6)
   {
    Result = false;
   }

  if (Result)
   {
    Index = 0;
    CompStr[2] = 0;

    while ((Index < 3) && (Result))
     {
      unsigned int                     CompValue;

      memcpy(CompStr,&ColorStr[Index * 2],2);

      if (sscanf(CompStr,"%x",&CompValue) == 1)
       {
        Color->c[Index] = ((float)CompValue) / 255.0f;

        Index++;
       }
      else
       {
        Result = false;
       }
     }
   }

  if (!Result)
   {
    fprintf(stderr,"error: invalid color value (%s)\n",ColorStr);
   }

  return(Result);
 }

static bool        ParseArgs          (char              **ModelFileName,
                                       char              **ImageFileName,
                                       char              **TexPath,
                                       unsigned int       *ImageSize,
                                       NGPShotColor3f     *BGColor,
                                       bool               *NeedAlpha,
                                       float              *XAngle,
                                       float              *YAngle,
                                       float              *LOD,
                                       char              **VertexProgFileName,
                                       char              **FragmentProgFileName,
                                       unsigned int       *OffScreenTarget,
                                       bool               *ShowHelp,
                                       unsigned int        ArgCount,
                                       char               *ArgValues[])
 {
  bool                                 Result;
  unsigned int                         ArgIndex;
  char                                *ArgStr;
  unsigned int                         ArgStrLen;

  Result = true;

  *ModelFileName = 0;
  *ImageFileName = 0;
  *TexPath       = 0;
  *ImageSize     = 256;
  BGColor->c[0]  = 0.0f;
  BGColor->c[1]  = 0.0f;
  BGColor->c[2]  = 0.0f;
  *NeedAlpha     = true;
  *XAngle        = 0.0f;
  *YAngle        = 0.0f;
  *LOD           = 1.0f;
  *VertexProgFileName   = 0;
  *FragmentProgFileName = 0;
  *OffScreenTarget      = GLOffScreenTargetAuto;
  *ShowHelp             = false;

  ArgIndex = 1;

  while ((ArgIndex < ArgCount) && (Result))
   {
    ArgStr    = ArgValues[ArgIndex];
    ArgStrLen = strlen(ArgStr);

    if (ArgStrLen > 0)
     {
      if (ArgStr[0] == '-')
       {
        if      (strcmp(ArgStr,"-h") == 0)
         {
          *ShowHelp = true;
         }
        else if (strcmp(ArgStr,"-t") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            *TexPath = ArgValues[ArgIndex];
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: texture search path required\n");
           }
         }
        else if (strcmp(ArgStr,"-s") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if (sscanf(ArgValues[ArgIndex],"%u",ImageSize) == 1)
             {
              if ((*ImageSize) > 0)
               {
               }
              else
               {
                Result = false;

                fprintf(stderr,"error: image size must be greater than zero\n");
               }
             }
            else
             {
              Result = false;

              fprintf(stderr,"error: invalid image size (%s)\n",ArgValues[ArgIndex]);
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: image size required\n");
           }
         }
        else if (strcmp(ArgStr,"-x") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if (sscanf(ArgValues[ArgIndex],"%f",XAngle) != 1)
             {
              Result = false;

              fprintf(stderr,"error: invalid degrees value(%s)\n",ArgValues[ArgIndex]);
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: angle required\n");
           }
         }
        else if (strcmp(ArgStr,"-y") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if (sscanf(ArgValues[ArgIndex],"%f",YAngle) != 1)
             {
              Result = false;

              fprintf(stderr,"error: invalid degrees value(%s)\n",ArgValues[ArgIndex]);
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: angle required\n");
           }
         }
        else if (strcmp(ArgStr,"-l") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if (sscanf(ArgValues[ArgIndex],"%f",LOD) != 1)
             {
              Result = false;

              fprintf(stderr,"error: invalid LOD value(%s)\n",ArgValues[ArgIndex]);
             }
            else
             {
              if (((*LOD) < 0.0f) || ((*LOD) > 1.0f))
               {
                Result = false;

                fprintf(stderr,"error: LOD value must be in [0.0 - 1.0] range\n");
               }
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: angle required\n");
           }
         }
        else if (strcmp(ArgStr,"-b") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            *NeedAlpha = false;

            Result = ParseColorString(BGColor,ArgValues[ArgIndex]);
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: color value required\n");
           }
         }
        else if (strcmp(ArgStr,"-vs") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            *VertexProgFileName = ArgValues[ArgIndex];
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: vertex program file name required\n");
           }
         }
        else if (strcmp(ArgStr,"-fs") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            *FragmentProgFileName = ArgValues[ArgIndex];
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: fragment program file name required\n");
           }
         }
        else if (strcmp(ArgStr,"-o") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if      (strcmp(ArgValues[ArgIndex],"auto") == 0)
             {
              *OffScreenTarget = GLOffScreenTargetAuto;
             }
            else if (strcmp(ArgValues[ArgIndex],"pixmap") == 0)
             {
              *OffScreenTarget = GLOffScreenTargetPixmap;
             }
            else if (strcmp(ArgValues[ArgIndex],"pbuffer") == 0)
             {
              *OffScreenTarget = GLOffScreenTargetPBuffer;
             }
            else
             {
              Result = false;

              fprintf(stderr,"error: invalid off-screen target  (must be one of: auto, pixmap or pbuffer)\n");
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: off-screen target required (auto, pixmap or pbuffer)\n");
           }
         }
        else
         {
          Result = false;

          fprintf(stderr,"error: invalid option \"%s\"\n",ArgStr);
         }
       }
      else
       {
        if      ((*ModelFileName) == 0)
         {
          *ModelFileName = ArgStr;
         }
        else if ((*ImageFileName) == 0)
         {
          *ImageFileName = ArgStr;
         }
        else
         {
          Result = false;

          fprintf(stderr,"error: extra argument passed\n");
         }
       }
     }

    ArgIndex++;
   }

  if (!(*ShowHelp))
   {
    if      ((*ModelFileName) == 0)
     {
      Result = false;

      fprintf(stderr,"error: model file name required\n");
     }
    else if ((*ImageFileName) == 0)
     {
      Result = false;

      fprintf(stderr,"error: image file name required\n");
     }
   }

  return(Result);
 }

int                main               (int                 argc,
                                       char               *argv[])
 {
  bool                                 Result;
  NGPShotGLContextWrapper              GLMemoryContext;
  char                                *ModelFileName;
  char                                *ImageFileName;
  char                                *TexPathOpt;
  unsigned int                         ImageSize;
  NGPShotColor3f                       BGColor;
  bool                                 NeedAlpha;
  float                                XAngle;
  float                                YAngle;
  float                                LOD;
  char                                *VertexProgFileName;
  char                                *FragmentProgFileName;
  unsigned int                         OffScreenTarget;
  bool                                 ShowHelp;
  char                                *VertexProgramSrc;
  char                                *FragmentProgramSrc;

  Result = ParseArgs(&ModelFileName,
                     &ImageFileName,
                     &TexPathOpt,
                     &ImageSize,
                     &BGColor,
                     &NeedAlpha,
                     &XAngle,
                     &YAngle,
                     &LOD,
                     &VertexProgFileName,
                     &FragmentProgFileName,
                     &OffScreenTarget,
                     &ShowHelp,
                      argc,argv);

  if (Result)
   {
    if (ShowHelp)
     {
      ShowHelpMessage();
     }
    else
     {
      Result = GLMemoryContext.CreateBaseContext();

      if (Result)
       {
        if (GLMemoryContext.IsPBufferSupported())
         {
          if (OffScreenTarget == GLOffScreenTargetAuto)
           {
            OffScreenTarget = GLOffScreenTargetPBuffer;
           }
         }
        else
         {
          if      (OffScreenTarget == GLOffScreenTargetAuto)
           {
            OffScreenTarget = GLOffScreenTargetPixmap;
           }
          else if (OffScreenTarget == GLOffScreenTargetPBuffer)
           {
            Result = false;
           }
         }
       }

      if (Result)
       {
        Result = GLMemoryContext.Create(ImageSize,ImageSize,NeedAlpha,OffScreenTarget == GLOffScreenTargetPBuffer);
       }

      if (Result)
       {
        GLMemoryContext.MakeCurrent();

        P3DGLExtInit();

        VertexProgramSrc   = 0;
        FragmentProgramSrc = 0;

        if (VertexProgFileName != 0)
         {
          VertexProgramSrc = LoadFileContent(VertexProgFileName);

          if (VertexProgramSrc == 0)
           {
            Result = false;
           }
         }

        if (FragmentProgFileName != 0)
         {
          FragmentProgramSrc = LoadFileContent(FragmentProgFileName);

          if (FragmentProgramSrc == 0)
           {
            Result = false;
           }
         }

        if (VertexProgFileName != 0)
         {
          if ((!GLEW_ARB_shader_objects) || (!GLEW_ARB_vertex_shader))
           {
            fprintf(stderr,"error: hardware/driver lacks GLSL support - unable to use vertex shaders\n");

            Result = false;
           }
         }

        if (FragmentProgFileName != 0)
         {
          if ((!GLEW_ARB_shader_objects) || (!GLEW_ARB_fragment_shader))
           {
            fprintf(stderr,"error: hardware/driver lacks GLSL support - unable to use fragment shaders\n");

            Result = false;
           }
         }

        if (Result)
         {
          std::string                    TexPath;

          if (TexPathOpt != 0)
           {
            TexPath = TexPathOpt;
           }
          else
           {
            TexPath = P3DPathInfo::GetCurrentDir();
           }

          Result = MakeShot( ModelFileName,
                             ImageFileName,
                             TexPath.c_str(),
                             ImageSize,
                             ImageSize,
                            &BGColor,
                             NeedAlpha,
                             XAngle,
                             YAngle,
                             LOD,
                             VertexProgramSrc,
                             FragmentProgramSrc);
         }

        free(FragmentProgramSrc);
        free(VertexProgramSrc);
       }
      else
       {
        fprintf(stderr,"error: unable to create OpenGL memory context\n");
       }
     }
   }

  if (Result)
   {
    return(0);
   }
  else
   {
    return(1);
   }
 }

