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

#include <ngpcore/p3dhli.h>

#include <p3dhliext.h>

/*FIXME: very similar to CalcInternalGroupCount from p3dhli.cpp */
static
unsigned int       CalcSubGroupCount  (const P3DBranchModel
                                                        *BranchModel)
 {
  unsigned int                         GroupCount;
  unsigned int                         SubBranchIndex;
  unsigned int                         SubBranchCount;

  GroupCount = 1;

  SubBranchCount = BranchModel->GetSubBranchCount();

  for (SubBranchIndex = 0; SubBranchIndex < SubBranchCount; SubBranchIndex++)
   {
    GroupCount += CalcSubGroupCount
                   (BranchModel->GetSubBranchModel(SubBranchIndex));
   }

  return(GroupCount);
 }

class P3DHLIBranchCalculatorMulti : public P3DBranchingFactory
 {
  public           :

                   P3DHLIBranchCalculatorMulti
                                      (P3DMathRNG         *RNG,
                                       const P3DBranchModel
                                                          *BranchModel,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       unsigned int        GroupIndex,
                                       unsigned int       *Counters)
   {
    this->RNG           = RNG;
    this->BranchModel   = BranchModel;
    this->Parent        = Parent;
    this->GroupIndex    = GroupIndex;
    this->Counters      = Counters;
   }

  virtual void     GenerateBranch     (float               Offset,
                                       const P3DQuaternionf
                                                          *Orientation)
   {
    const P3DStemModel              *StemModel;
    P3DStemModelInstance            *Instance;
    unsigned int                     SubBranchIndex;
    unsigned int                     SubBranchCount;
    unsigned int                     SubGroupIndex;

    StemModel = BranchModel->GetStemModel();

    if (StemModel != 0)
     {
      Instance      = StemModel->CreateInstance(RNG,Parent,Offset,Orientation);
      SubGroupIndex = GroupIndex + 1;

      Counters[GroupIndex]++;
     }
    else
     {
      Instance      = 0;
      SubGroupIndex = 0;
     }

    SubBranchCount = BranchModel->GetSubBranchCount();

    for (SubBranchIndex = 0; SubBranchIndex < SubBranchCount; SubBranchIndex++)
     {
      P3DHLIBranchCalculatorMulti Calculator(RNG,
                                             BranchModel->GetSubBranchModel(SubBranchIndex),
                                             Instance,
                                             SubGroupIndex,
                                             Counters);

      const_cast<P3DBranchingAlg*>(BranchModel->GetSubBranchModel(SubBranchIndex)->GetBranchingAlg())
       ->CreateBranches(&Calculator,Instance,RNG);

      SubGroupIndex += CalcSubGroupCount
                        (BranchModel->GetSubBranchModel(SubBranchIndex));
     }

    if (Instance != 0)
     {
      StemModel->ReleaseInstance(Instance);
     }
   }

  private          :

  P3DMathRNG                          *RNG;
  const P3DBranchModel                *BranchModel;
  const P3DStemModelInstance          *Parent;
  unsigned int                         GroupIndex;
  unsigned int                        *Counters;
 };

void               P3DHLIGetBranchCountMulti
                                      (unsigned int       *BranchCounts,
                                       const P3DPlantModel*PlantModel)
 {
  unsigned int                         GroupIndex;
  unsigned int                         GroupCount;

  GroupCount = CalcSubGroupCount(PlantModel->GetPlantBase()) - 1;

  for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
   {
    BranchCounts[GroupIndex] = 0;
   }

  P3DMathRNGSimple                     RNG(PlantModel->GetBaseSeed());

  P3DHLIBranchCalculatorMulti Calculator((PlantModel->GetFlags() & P3D_MODEL_FLAG_NO_RANDOMNESS) ? 0 : &RNG,
                                          PlantModel->GetPlantBase(),
                                          0,
                                          0,
                                          BranchCounts);

  Calculator.GenerateBranch(0.0f,0);
 }

class P3DHLIFillVAttrBuffersIMultiHelper : public P3DBranchingFactory
 {
  public           :

                   P3DHLIFillVAttrBuffersIMultiHelper
                                      (P3DMathRNG         *RNG,
                                       const P3DBranchModel
                                                          *BranchModel,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       unsigned int        GroupIndex,
                                       P3DHLIVAttrBufferSet
                                                          *VAttrBufferSetArray)
   {
    this->RNG                 = RNG;
    this->BranchModel         = BranchModel;
    this->Parent              = Parent;
    this->GroupIndex          = GroupIndex;
    this->VAttrBufferSetArray = VAttrBufferSetArray;
   }

  virtual void     GenerateBranch     (float               Offset,
                                       const P3DQuaternionf
                                                          *Orientation)
   {
    const P3DStemModel                *StemModel;
    P3DStemModelInstance              *Instance;

    StemModel = BranchModel->GetStemModel();

    if (StemModel != 0)
     {
      Instance = StemModel->CreateInstance(RNG,Parent,Offset,Orientation);
     }
    else
     {
      Instance = 0;
     }

    if (Instance != 0)
     {
      unsigned int                     VAttrIndex;
      unsigned int                     VAttrCount;

      VAttrCount = Instance->GetVAttrCountI();

      for (VAttrIndex = 0; VAttrIndex < VAttrCount; VAttrIndex++)
       {
        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_VERTEX] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_VERTEX],
            P3D_ATTR_VERTEX,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_VERTEX] += 3;
         }

        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_NORMAL] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_NORMAL],
            P3D_ATTR_NORMAL,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_NORMAL] += 3;
         }

        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_TEXCOORD0] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_TEXCOORD0],
            P3D_ATTR_TEXCOORD0,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_TEXCOORD0] += 2;
         }

        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_TANGENT] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_TANGENT],
            P3D_ATTR_TANGENT,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_TANGENT] += 3;
         }

        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_BINORMAL] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_BINORMAL],
            P3D_ATTR_BINORMAL,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_BINORMAL] += 3;
         }

        if (VAttrBufferSetArray[GroupIndex][P3D_ATTR_BILLBOARD_POS] != 0)
         {
          Instance->GetVAttrValueI
           (VAttrBufferSetArray[GroupIndex][P3D_ATTR_BILLBOARD_POS],
            P3D_ATTR_BILLBOARD_POS,
            VAttrIndex);

          VAttrBufferSetArray[GroupIndex][P3D_ATTR_BILLBOARD_POS] += 3;
         }
       }
     }

    unsigned int                     SubBranchIndex;
    unsigned int                     SubBranchCount;
    unsigned int                     SubGroupIndex;

    if (StemModel != 0)
     {
      SubGroupIndex = GroupIndex + 1;
     }
    else
     {
      SubGroupIndex = 0;
     }

    SubBranchCount = BranchModel->GetSubBranchCount();

    for (SubBranchIndex = 0; SubBranchIndex < SubBranchCount; SubBranchIndex++)
     {
      P3DHLIFillVAttrBuffersIMultiHelper
                                       Helper(RNG,
                                              BranchModel->GetSubBranchModel(SubBranchIndex),
                                              Instance,
                                              SubGroupIndex,
                                              VAttrBufferSetArray);

      const_cast<P3DBranchingAlg*>(BranchModel->GetSubBranchModel(SubBranchIndex)->GetBranchingAlg())
       ->CreateBranches(&Helper,Instance,RNG);

      SubGroupIndex += CalcSubGroupCount
                        (BranchModel->GetSubBranchModel(SubBranchIndex));
     }

    if (Instance != 0)
     {
      StemModel->ReleaseInstance(Instance);
     }
   }

  private          :

  P3DMathRNG                          *RNG;
  const P3DBranchModel                *BranchModel;
  const P3DStemModelInstance          *Parent;
  unsigned int                         GroupIndex;
  P3DHLIVAttrBufferSet                *VAttrBufferSetArray;
 };

void               P3DHLIFillVAttrBuffersIMulti
                                      (P3DHLIVAttrBufferSet
                                                          *VAttrBufferSet,
                                       const P3DPlantModel*PlantModel)
 {
  unsigned int                         GroupIndex;
  unsigned int                         GroupCount;

  GroupCount = CalcSubGroupCount(PlantModel->GetPlantBase()) - 1;

  if (GroupCount > 0)
   {
    P3DHLIVAttrBufferSet              *TempVAttrBufferSet;

    TempVAttrBufferSet = new P3DHLIVAttrBufferSet[GroupCount];

    for (GroupIndex = 0; GroupIndex < GroupCount; GroupIndex++)
     {
      for (unsigned int AttrIndex = 0; AttrIndex < P3D_MAX_ATTRS; AttrIndex++)
       {
        TempVAttrBufferSet[GroupIndex][AttrIndex] =
         VAttrBufferSet[GroupIndex][AttrIndex];
       }
     }

    P3DMathRNGSimple                     RNG(PlantModel->GetBaseSeed());
    P3DHLIFillVAttrBuffersIMultiHelper   Helper((PlantModel->GetFlags() & P3D_MODEL_FLAG_NO_RANDOMNESS) ? 0 : &RNG,
                                                 PlantModel->GetPlantBase(),
                                                 0,
                                                 0,
                                                 TempVAttrBufferSet);

    Helper.GenerateBranch(0.0f,0);

    delete TempVAttrBufferSet;
   }
 }

