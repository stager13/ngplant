/***************************************************************************

 Copyright (C) 2007  Sergey Prokhorchuk

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
#include <cstring>

#include <ngput/p3dglext.h>

#ifndef GLAPIENTRY
 #define GLAPIENTRY
#endif

#include <GL/glut.h>

#include <vector>
#include <string>

#include <ngpcore/p3dhli.h>

#include <ngput/p3dospath.h>

#include <ngpviewtexman.h>
#include <ngpviewdata.h>

static NGPViewMeshData      *PlantMesh = 0;
static float                 RotAngle = 0.0f;
static float                 StartAngle = 0.0f;
static int                   AnimationStartTime = 0;
static bool                  AnimationRunning = false;

const float AnimationSpeed = 15.0; /* degrees per second */

static void        PrepareMaterialState
                                      (const NGPViewSubMeshData
                                                          *SubMesh)
 {
  if (SubMesh->TexHandle == 0)
   {
    glDisable(GL_TEXTURE_2D);
   }
  else
   {
    glBindTexture(GL_TEXTURE_2D,SubMesh->TexHandle);
    glEnable(GL_TEXTURE_2D);
   }

  if (SubMesh->DoubleSided)
   {
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
   }
  else
   {
    glEnable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
   }

  if (SubMesh->Transparent)
   {
    glAlphaFunc(GL_GREATER,0.5f);
    glEnable(GL_ALPHA_TEST);
   }
  else
   {
    glDisable(GL_ALPHA_TEST);
   }
 }

static void        RenderBillboards   (const NGPViewSubMeshData
                                                          *SubMesh)
 {
  float          ModelViewMatrix[16];
  float          Right[3];
  float          Left[3];
  float          Up[3];
  float          Down[3];
  float          HalfWidth;
  float          HalfHeight;
  unsigned int   Index;
  unsigned int   Count;
  const float   *Pos;
  float          TexCoordUL[2];
  float          TexCoordDL[2];
  float          TexCoordUR[2];
  float          TexCoordDR[2];
  const float   *TexCoordBuffer;

  if (SubMesh->TexHandle == 0)
   {
    TexCoordDL[0] = 0.0f;
    TexCoordDL[1] = 0.0f;
    TexCoordDR[0] = 0.0f;
    TexCoordDR[1] = 0.0f;
    TexCoordUR[0] = 0.0f;
    TexCoordUR[1] = 0.0f;
    TexCoordUL[0] = 0.0f;
    TexCoordUL[1] = 0.0f;
   }
  else
   {
    TexCoordBuffer = (const float*)SubMesh->TexCoordBuffer;

    TexCoordDL[0] = TexCoordBuffer[0];
    TexCoordDL[1] = TexCoordBuffer[1];
    TexCoordDR[0] = TexCoordBuffer[2];
    TexCoordDR[1] = TexCoordBuffer[3];
    TexCoordUL[0] = TexCoordBuffer[4];
    TexCoordUL[1] = TexCoordBuffer[5];
    TexCoordUR[0] = TexCoordBuffer[6];
    TexCoordUR[1] = TexCoordBuffer[7];
   }

  PrepareMaterialState(SubMesh);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor3f(SubMesh->R,SubMesh->G,SubMesh->B);

  glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);

  HalfWidth  = SubMesh->BillboardWidth  * 0.5f;
  HalfHeight = SubMesh->BillboardHeight * 0.5f;

  Right[0] = ModelViewMatrix[0] * HalfWidth;
  Right[1] = ModelViewMatrix[4] * HalfWidth;
  Right[2] = ModelViewMatrix[8] * HalfWidth;

  Up[0] = ModelViewMatrix[1] * HalfHeight;
  Up[1] = ModelViewMatrix[5] * HalfHeight;
  Up[2] = ModelViewMatrix[9] * HalfHeight;

  Left[0] = -Right[0];
  Left[1] = -Right[1];
  Left[2] = -Right[2];

  Down[0] = -Up[0];
  Down[1] = -Up[1];
  Down[2] = -Up[2];

  if (SubMesh->BillboardMode == P3D_BILLBOARD_MODE_CYLINDRICAL)
   {
    Up[0] = Up[2] = Down[0] = Down[2] = 0.0f;
   }

  Count = SubMesh->IndexCount;
  Pos   = (float*)SubMesh->PosBuffer;

  glBegin(GL_QUADS);

  glNormal3f(ModelViewMatrix[2],ModelViewMatrix[6],ModelViewMatrix[10]);

  for (Index = 0; Index < Count; Index++)
   {
    glTexCoord2fv(TexCoordUL);

    glVertex3f(Pos[0] + Left[0] + Up[0],
               Pos[1] + Left[1] + Up[1],
               Pos[2] + Left[2] + Up[2]);

    glTexCoord2fv(TexCoordDL);

    glVertex3f(Pos[0] + Left[0] + Down[0],
               Pos[1] + Left[1] + Down[1],
               Pos[2] + Left[2] + Down[2]);

    glTexCoord2fv(TexCoordDR);

    glVertex3f(Pos[0] + Right[0] + Down[0],
               Pos[1] + Right[1] + Down[1],
               Pos[2] + Right[2] + Down[2]);

    glTexCoord2fv(TexCoordUR);

    glVertex3f(Pos[0] + Right[0] + Up[0],
               Pos[1] + Right[1] + Up[1],
               Pos[2] + Right[2] + Up[2]);

    Pos += 12;
   }

  glEnd();
 }

static void        RenderSubMesh      (const NGPViewSubMeshData
                                                          *SubMesh)
 {
  PrepareMaterialState(SubMesh);

  glVertexPointer(3,GL_FLOAT,0,SubMesh->PosBuffer);
  glEnableClientState(GL_VERTEX_ARRAY);
  glNormalPointer(GL_FLOAT,0,SubMesh->NormalBuffer);
  glEnableClientState(GL_NORMAL_ARRAY);

  if (SubMesh->TexHandle != 0)
   {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2,GL_FLOAT,0,SubMesh->TexCoordBuffer);
   }
  else
   {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

  glColor3f(SubMesh->R,SubMesh->G,SubMesh->B);

  glDrawElements(GL_TRIANGLES,
                 SubMesh->IndexCount,
                 GL_UNSIGNED_INT,
                 SubMesh->IndexBuffer);
 }

static GLfloat     Light0Position[] = { 0.0f, 0.0f, 0.0f, 1.0f };

static void        RenderScene        ()
 {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glLightfv(GL_LIGHT0,GL_POSITION,Light0Position);
  glTranslatef(0.0f,(PlantMesh->BBoxMin[1] - PlantMesh->BBoxMax[1]) * 0.5f,-10.0f);
  glRotatef(RotAngle,0.0f,1.0f,0.0f);

  if (PlantMesh != 0)
   {
    unsigned int                       SubMeshIndex;
    unsigned int                       SubMeshCount;

    SubMeshCount = PlantMesh->SubMeshes.size();

    for (SubMeshIndex = 0; SubMeshIndex < SubMeshCount; SubMeshIndex++)
     {
      if (PlantMesh->SubMeshes[SubMeshIndex].Billboard)
       {
        RenderBillboards(&PlantMesh->SubMeshes[SubMeshIndex]);
       }
      else
       {
        RenderSubMesh(&PlantMesh->SubMeshes[SubMeshIndex]);
       }
     }
   }

  glFlush();
  glutSwapBuffers();
 }

static void        GetBranchGroupMaterial
                                      (NGPViewSubMeshData *SubMeshData,
                                       const P3DMaterialDef
                                                          *MaterialDef,
                                       NGPViewTexManager  *TextureManager)
 {
  const char                          *TexName;

  MaterialDef->GetColor(&SubMeshData->R,&SubMeshData->G,&SubMeshData->B);

  TexName = MaterialDef->GetTexName(P3D_TEX_DIFFUSE);

  if (TexName != 0)
   {
    SubMeshData->TexHandle = TextureManager->LoadTexture(TexName);

    if (SubMeshData->TexHandle == 0)
     {
      fprintf(stderr,"warning: unable to load texture \"%s\" - texture will not be used\n",TexName);
     }
   }
  else
   {
    SubMeshData->TexHandle = 0;
   }

  SubMeshData->DoubleSided   = MaterialDef->IsDoubleSided();
  SubMeshData->Transparent   = MaterialDef->IsTransparent();
  SubMeshData->Billboard     = MaterialDef->IsBillboard();
  SubMeshData->BillboardMode = MaterialDef->GetBillboardMode();
 }

static bool        GetBranchGroupBillboards
                                      (NGPViewSubMeshData *SubMeshData,
                                       const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       const P3DHLIPlantInstance
                                                          *PlantInstance,
                                       unsigned int        GroupIndex)
 {
  P3DHLIVAttrBuffers                   VAttrBuffers;
  unsigned int                         BranchCount;
  unsigned int                         AttrCount;

  AttrCount   = PlantInstance->GetVAttrCountI(GroupIndex);
  BranchCount = PlantInstance->GetBranchCount(GroupIndex);

  if (AttrCount == 0)
   {
    return(true);
   }

  SubMeshData->PosBuffer = malloc(sizeof(float) * 3 * AttrCount);

  if (SubMeshData->PosBuffer == 0)
   {
    return(false);
   }

  VAttrBuffers.AddAttr(P3D_ATTR_BILLBOARD_POS,SubMeshData->PosBuffer,0,sizeof(float) * 3);

  if (SubMeshData->TexHandle != 0)
   {
    SubMeshData->TexCoordBuffer = malloc(sizeof(float) * 2 * AttrCount);

    if (SubMeshData->TexCoordBuffer != 0)
     {
      VAttrBuffers.AddAttr(P3D_ATTR_TEXCOORD0,SubMeshData->TexCoordBuffer,0,sizeof(float) * 2);
     }
    else
     {
      return(false);
     }
   }

  SubMeshData->IndexCount = BranchCount;

  PlantInstance->FillVAttrBuffersI(&VAttrBuffers,GroupIndex);

  PlantTemplate->GetBillboardSize(&SubMeshData->BillboardWidth,
                                  &SubMeshData->BillboardHeight,
                                   GroupIndex);

  return(true);
 }

static bool        GetBranchGroupGeometry
                                      (NGPViewSubMeshData *SubMeshData,
                                       const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       const P3DHLIPlantInstance
                                                          *PlantInstance,
                                       unsigned int        GroupIndex)
 {
  unsigned int                         AttrCount;
  unsigned int                         BranchAttrCount;
  P3DHLIVAttrBuffers                   VAttrBuffers;
  unsigned int                         IndexCount;
  unsigned int                         BranchIndex;
  unsigned int                         BranchCount;

  AttrCount = PlantInstance->GetVAttrCountI(GroupIndex);

  if (AttrCount == 0)
   {
    return(true);
   }

  SubMeshData->PosBuffer    = malloc(sizeof(float) * 3 * AttrCount);
  SubMeshData->NormalBuffer = malloc(sizeof(float) * 3 * AttrCount);

  if ((SubMeshData->PosBuffer    != 0) &&
      (SubMeshData->NormalBuffer != 0))
   {
    VAttrBuffers.AddAttr(P3D_ATTR_VERTEX,SubMeshData->PosBuffer,0,sizeof(float) * 3);
    VAttrBuffers.AddAttr(P3D_ATTR_NORMAL,SubMeshData->NormalBuffer,0,sizeof(float) * 3);
   }
  else
   {
    return(false);
   }

  if (SubMeshData->TexHandle != 0)
   {
    SubMeshData->TexCoordBuffer = malloc(sizeof(float) * 2 * AttrCount);

    if (SubMeshData->TexCoordBuffer != 0)
     {
      VAttrBuffers.AddAttr(P3D_ATTR_TEXCOORD0,SubMeshData->TexCoordBuffer,0,sizeof(float) * 2);
     }
    else
     {
      return(false);
     }
   }

  PlantInstance->FillVAttrBuffersI(&VAttrBuffers,GroupIndex);

  BranchCount = PlantInstance->GetBranchCount(GroupIndex);
  IndexCount  = PlantTemplate->GetIndexCount(GroupIndex,P3D_TRIANGLE_LIST);

  SubMeshData->IndexCount = IndexCount * BranchCount;

  SubMeshData->IndexBuffer =
   (unsigned int*)malloc(SubMeshData->IndexCount * sizeof(unsigned int));

  if (SubMeshData->IndexBuffer == 0)
   {
    return(false);
   }

  BranchAttrCount = PlantTemplate->GetVAttrCountI(GroupIndex);

  for (BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
   {
    PlantTemplate->FillIndexBuffer
     (&(SubMeshData->IndexBuffer[BranchIndex * IndexCount]),
       GroupIndex,
       P3D_TRIANGLE_LIST,
       P3D_UNSIGNED_INT,
       BranchAttrCount * BranchIndex);
   }

  return(true);
 }

static bool        GetBranchGroupData (NGPViewSubMeshData *SubMeshData,
                                       const P3DHLIPlantTemplate
                                                          *PlantTemplate,
                                       const P3DHLIPlantInstance
                                                          *PlantInstance,
                                       unsigned int        GroupIndex,
                                       NGPViewTexManager  *TextureManager)
 {
  GetBranchGroupMaterial
   (SubMeshData,PlantTemplate->GetMaterial(GroupIndex),TextureManager);

  if (SubMeshData->Billboard)
   {
    return(GetBranchGroupBillboards
            (SubMeshData,PlantTemplate,PlantInstance,GroupIndex));
   }
  else
   {
    return(GetBranchGroupGeometry
            (SubMeshData,PlantTemplate,PlantInstance,GroupIndex));
   }
 }

static bool        LoadModel          (NGPViewMeshData   **MeshData,
                                       const char         *SourceFileName,
                                       NGPViewTexManager  *TextureManager)
 {
  bool                                 Result;
  P3DInputStringStreamFile             SourceStream;
  unsigned int                         BranchGroupIndex;
  unsigned int                         BranchGroupCount;

  Result = true;

  try
   {
    SourceStream.Open(SourceFileName);

    P3DHLIPlantTemplate                  PlantTemplate(&SourceStream);

    SourceStream.Close();

    P3DHLIPlantInstance                 *PlantInstance;

    PlantInstance = PlantTemplate.CreateInstance();

    BranchGroupCount = PlantTemplate.GetGroupCount();

    if (BranchGroupCount > 0)
     {
      *MeshData = new NGPViewMeshData(BranchGroupCount);

      PlantInstance->GetBoundingBox((*MeshData)->BBoxMin,(*MeshData)->BBoxMax);

      BranchGroupIndex = 0;

      while ((BranchGroupIndex < BranchGroupCount) && (Result))
       {
        Result = GetBranchGroupData(&((*MeshData)->SubMeshes[BranchGroupIndex]),
                                    &PlantTemplate,
                                     PlantInstance,
                                     BranchGroupIndex,
                                     TextureManager);

        BranchGroupIndex++;
       }
     }
    else
     {
      fprintf(stderr,"warning: plant model is empty\n");
     }
   }
  catch (const P3DException &Exc)
   {
    Result = false;

    fprintf(stderr,"error: %s\n",Exc.GetMessage());
   }

  return(Result);
 }

static GLfloat     Light0Ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat     Light0Diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat     Light0Specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat     GlobalAmbientLight[]  = { 0.4f, 0.4f, 0.4f, 1.0f };

static void        GLInit             ()
 {
  glEnable(GL_DEPTH_TEST);

  glClearColor(0.0f,0.0f,0.0f,1.0f);

  glCullFace(GL_BACK);

  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT0,GL_AMBIENT,Light0Ambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,Light0Diffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,Light0Specular);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,GlobalAmbientLight);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-25.0f,25.0f,-25.0f,25.0f,1.0f,50.0f);
 }

static void        ReshapeFunc        (int                 width,
                                       int                 height)
 {
  glViewport(0,0,width,height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (PlantMesh != 0)
   {
    float                              BBoxSize[3];
    float                              MaxValue;

    BBoxSize[0] = (PlantMesh->BBoxMax[0] - PlantMesh->BBoxMin[0]) / 2.0f;
    BBoxSize[1] = (PlantMesh->BBoxMax[1] - PlantMesh->BBoxMin[1]) / 2.0f;
    BBoxSize[2] = (PlantMesh->BBoxMax[2] - PlantMesh->BBoxMin[2]) / 2.0f;

    if (BBoxSize[0] > BBoxSize[1])
     {
      if (BBoxSize[0] > BBoxSize[2])
       {
        MaxValue = BBoxSize[0];
       }
      else
       {
        MaxValue = BBoxSize[2];
       }
     }
    else
     {
      if (BBoxSize[1] > BBoxSize[2])
       {
        MaxValue = BBoxSize[1];
       }
      else
       {
        MaxValue = BBoxSize[2];
       }
     }

    MaxValue *= 1.5f;

    if (width > height)
     {
      glOrtho(-MaxValue * ((float)width / height),
               MaxValue * ((float)width / height),
              -MaxValue,
               MaxValue,
               1.0f,
               50.0f);
     }
    else
     {
      glOrtho(-MaxValue,
               MaxValue,
              -MaxValue * ((float)height / width),
               MaxValue * ((float)height / width),
               1.0f,
               50.0f);
     }
   }
  else
   {
    glOrtho(-25.0f,25.0f,-25.0f,25.0f,1.0f,50.0f);
   }
 }

static void        IdleFunc           ()
 {
  RotAngle = StartAngle + (glutGet(GLUT_ELAPSED_TIME) - AnimationStartTime) * AnimationSpeed / 1000.0f;

  glutPostRedisplay();
 }

static void        KeyboardFunc       (unsigned char       key,
                                       int                 width P3D_UNUSED_ATTR,
                                       int                 height P3D_UNUSED_ATTR)
 {
  if ((key == 'q') || (key == 0x1B))
   {
    exit(0);
   }
  else if (key == 'a')
   {
    if (AnimationRunning)
     {
      StartAngle       = RotAngle;
      AnimationRunning = false;

      glutIdleFunc(NULL);
     }
    else
     {
      AnimationStartTime = glutGet(GLUT_ELAPSED_TIME);
      AnimationRunning   = true;

      glutIdleFunc(IdleFunc);
     }
   }
 }

static bool        ParseArgs          (char              **ModelFileName,
                                       char              **TexPath,
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
  *TexPath       = 0;
  *ShowHelp      = false;

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
   }

  return(Result);
 }

static void        ShowHelpMessage    ()
 {
  printf("Usage: ngpview [options] modelfile\n");
  printf("Options:\n");
  printf("  -h            Display this information\n");
  printf("  -t <path>     Use <path> for texture search (current dir by default)\n");
  printf("\nUse 'ESC' or 'q' key to exit\n");
  printf("Use 'a' key to start/stop animation\n\n");
 }

int                main               (int                 argc,
                                       char               *argv[])
 {
  char                                *ModelFileName;
  char                                *TexPathOpt;
  bool                                 ShowHelp;

  glutInitWindowSize(640,480);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInit(&argc,argv);

  if (ParseArgs(&ModelFileName,&TexPathOpt,&ShowHelp,argc,argv))
   {
    if (ShowHelp)
     {
      ShowHelpMessage();

      exit(0);
     }

    std::string                    TexPath;

    if (TexPathOpt != 0)
     {
      TexPath = TexPathOpt;
     }
    else
     {
      TexPath = P3DPathInfo::GetCurrentDir();
     }

    NGPViewTexManager                    TextureManager(TexPath.c_str());

    glutCreateWindow("ngpview");

    if (LoadModel(&PlantMesh,ModelFileName,&TextureManager))
     {
      GLInit();

      glutReshapeFunc(ReshapeFunc);
      glutDisplayFunc(RenderScene);
      glutKeyboardFunc(KeyboardFunc);

      glutMainLoop();
     }

    if (PlantMesh != 0)
     {
      delete PlantMesh;
     }
   }

  return(0);
 }

