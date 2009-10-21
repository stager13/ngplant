/***************************************************************************

 Copyright (C) 2008  Sergey Prokhorchuk

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

/* ngpcore benchmark utility (based on ngpshot.c code) */

#include <stdio.h>
#include <string.h>

#include <new>
/*
#include <stdlib.h>

#include <string>
#include <map>
*/

#include <ngpcore/p3dhli.h>

static void        RenderBranchGroup  (P3DHLIPlantTemplate*PlantTemplate,
                                       P3DHLIPlantInstance*PlantInstance,
                                       unsigned int        GroupIndex)
 {
  unsigned int                         BranchIndex;
  unsigned int                         BranchCount;
  const P3DMaterialDef                *MaterialDef;

  BranchCount = PlantInstance->GetBranchCount(GroupIndex);

  if (BranchCount == 0)
   {
    return;
   }

  MaterialDef = PlantTemplate->GetMaterial(GroupIndex);

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
  BiNormalBuffer = new(std::nothrow) float[3 * TotalVAttrCount];

  if ((PosBuffer != 0) && (NormalBuffer != 0) &&
      (TexCoordBuffer != 0) && (IndexBuffer != 0) &&
      (BiNormalBuffer != 0))
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
   }
  else
   {
    fprintf(stderr,"error: out of memory\n");
   }

  delete[] IndexBuffer;
  delete[] TexCoordBuffer;
  delete[] BiNormalBuffer;
  delete[] NormalBuffer;
  delete[] PosBuffer;
 }

static void        Render             (P3DHLIPlantTemplate*PlantTemplate,
                                       P3DHLIPlantInstance*PlantInstance)
 {
  P3DVector3f                          BBoxMin;
  P3DVector3f                          BBoxMax;
  unsigned int                         GroupIndex;
  unsigned int                         GroupCount;

  PlantInstance->GetBoundingBox(BBoxMin.v,BBoxMax.v);

  GroupCount = PlantTemplate->GetGroupCount();

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    RenderBranchGroup(PlantTemplate,PlantInstance,GroupIndex);
   }
 }

static bool        MakeShot           (const char         *ModelFileName,
                                       unsigned int        RepeatCount)
 {
  bool                                 Result;
  P3DInputStringStreamFile             SourceStream;
  P3DHLIPlantTemplate                 *PlantTemplate;
  P3DHLIPlantInstance                 *PlantInstance;

  Result = true;

  PlantTemplate = 0;
  PlantInstance = 0;

  try
   {
    SourceStream.Open(ModelFileName);

    PlantTemplate = new P3DHLIPlantTemplate(&SourceStream);

    SourceStream.Close();

    PlantInstance = PlantTemplate->CreateInstance();

    for (unsigned int Index = 0; Index < RepeatCount; Index++)
     {
      Render(PlantTemplate,PlantInstance);
     }
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
  printf("Usage: ngpbench [options] modelfile\n");
  printf("Options:\n");
  printf("  -h            Display this information\n");
  printf("  -r <count>    Repeat <count> times (1 by default)\n");
 }

static bool        ParseArgs          (char              **ModelFileName,
                                       unsigned int       *RepeatCount,
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
  *RepeatCount   = 1;
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
        else if (strcmp(ArgStr,"-r") == 0)
         {
          ArgIndex++;

          if (ArgIndex < ArgCount)
           {
            if (sscanf(ArgValues[ArgIndex],"%u",RepeatCount) == 1)
             {
              if ((*RepeatCount) > 0)
               {
               }
              else
               {
                Result = false;

                fprintf(stderr,"error: repeat count must be greater than zero\n");
               }
             }
            else
             {
              Result = false;

              fprintf(stderr,"error: invalid repeat count (%s)\n",ArgValues[ArgIndex]);
             }
           }
          else
           {
            Result = false;

            fprintf(stderr,"error: image size required\n");
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
    if ((*ModelFileName) == 0)
     {
      Result = false;

      fprintf(stderr,"error: model file name required\n");
     }
   }

  return(Result);
 }

int                main               (int                 argc,
                                       char               *argv[])
 {
  bool                                 Result;
  char                                *ModelFileName;
  unsigned int                         RepeatCount;
  bool                                 ShowHelp;

  Result = ParseArgs(&ModelFileName,&RepeatCount,&ShowHelp,argc,argv);

  if (Result)
   {
    if (ShowHelp)
     {
      ShowHelpMessage();
     }
    else
     {
      Result = MakeShot(ModelFileName,RepeatCount);
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

