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

#include <Python.h>

#include <ngpcore/p3dhli.h>

#include <ngppyplantinstance.h>
#include <ngppymaterialdef.h>
#include <ngppybranchgroup.h>

static void        BranchGroupDealloc(BranchGroupObject   *self)
 {
  Py_XDECREF(self->PlantInstance);

  self->ob_type->tp_free((PyObject*)self);
 }

static PyObject   *BranchGroupNew     (PyTypeObject       *type,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  BranchGroupObject                   *self;

  self = (BranchGroupObject*)type->tp_alloc(type,0);

  if (self != NULL)
   {
    self->PlantInstance = NULL;
    self->GroupIndex    = 0;
   }

  return((PyObject*)self);
 }

static int         BranchGroupInit    (BranchGroupObject  *self,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  PlantInstanceObject                 *InstanceObject;
  unsigned int                         GroupIndex;

  if (!PyArg_ParseTuple(args,"O!I",&PlantInstanceType,&InstanceObject,&GroupIndex))
   {
    return(-1);
   }

  if (!PlantInstanceCheck(InstanceObject))
   {
    return(-1);
   }

  if (GroupIndex >= InstanceObject->Template->GetGroupCount())
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid group index");

    return(-1);
   }

  Py_INCREF(InstanceObject);

  self->PlantInstance = InstanceObject;
  self->GroupIndex    = GroupIndex;

  return(0);
 }

static PyObject    *BranchGroupGetName(PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  return(Py_BuildValue("s",BranchGroup->PlantInstance->Template->
                            GetGroupName(BranchGroup->GroupIndex)));
 }


static PyObject    *BranchGroupGetMaterial
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  MaterialDefObject                   *MaterialDef;
  const P3DMaterialDef                *MaterialDefLow;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  MaterialDefLow = BranchGroup->PlantInstance->Template->GetMaterial
                    (BranchGroup->GroupIndex);

  MaterialDef = PyObject_New(MaterialDefObject,&MaterialDefType);

  if (MaterialDef != NULL)
   {
    float          R,G,B;

    MaterialDefLow->GetColor(&R,&G,&B);

    MaterialDef->Color = Py_BuildValue("(fff)",R,G,B);

    PyObject *TexNames;

    TexNames = PyList_New(P3D_MAX_TEX_LAYERS);

    if (TexNames == NULL)
     {
      /*FIXME: */
     }

    for (unsigned int TexLayer = 0; TexLayer < P3D_MAX_TEX_LAYERS; TexLayer++)
     {
      if (MaterialDefLow->GetTexName(TexLayer) == 0)
       {
        Py_INCREF(Py_None);
        PyList_SET_ITEM(TexNames,TexLayer,Py_None);
       }
      else
       {
        PyList_SET_ITEM(TexNames,TexLayer,Py_BuildValue("s",MaterialDefLow->GetTexName(TexLayer)));
       }
     }

    MaterialDef->TexNames = TexNames;

    MaterialDef->DoubleSided = Py_BuildValue("i",MaterialDefLow->IsDoubleSided());
    MaterialDef->Transparent = Py_BuildValue("i",MaterialDefLow->IsTransparent());
    MaterialDef->Billboard   = Py_BuildValue("i",MaterialDefLow->IsBillboard());
    MaterialDef->BillboardMode = Py_BuildValue("i",MaterialDefLow->GetBillboardMode());
    MaterialDef->AlphaCtrlEnabled = Py_BuildValue("i",MaterialDefLow->IsAlphaCtrlEnabled());
    MaterialDef->AlphaFadeRange = Py_BuildValue("(ff)",MaterialDefLow->GetAlphaFadeIn(),MaterialDefLow->GetAlphaFadeOut());
   }

  return((PyObject*)MaterialDef);
 }

static PyObject    *BranchGroupIsLODVisRangeEnabled
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  return(Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                            IsLODVisRangeEnabled(BranchGroup->GroupIndex)));
 }

static PyObject    *BranchGroupGetLODVisRange
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  float                                MinLOD;
  float                                MaxLOD;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  BranchGroup->PlantInstance->Template->
   GetLODVisRange(&MinLOD,&MaxLOD,BranchGroup->GroupIndex);

  return(Py_BuildValue("(ff)",MinLOD,MaxLOD));
 }

static PyObject    *BranchGroupGetBranchCount
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  return(Py_BuildValue("l",(long)BranchGroup->PlantInstance->Instance->
                            GetBranchCount(BranchGroup->GroupIndex)));
 }

static PyObject    *BranchGroupGetBillboardSize
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  PyObject                            *Result;
  BranchGroupObject                   *BranchGroup;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  try
   {
    float                    Width,Height;

    BranchGroup->PlantInstance->Template->GetBillboardSize
     (&Width,&Height,BranchGroup->GroupIndex);

    Result = Py_BuildValue("(ff)",Width,Height);
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupIsCloneable
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  PyObject                            *Result;
  BranchGroupObject                   *BranchGroup;
  int                                  AllowScaling;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  AllowScaling = 0;

  if (!PyArg_ParseTuple(args,"|i",&AllowScaling))
   {
    return(NULL);
   }

  try
   {
    bool IsCloneable;

    IsCloneable = BranchGroup->PlantInstance->Template->IsCloneable
                   (BranchGroup->GroupIndex,(bool)AllowScaling);

    Result = Py_BuildValue("i",(int)IsCloneable);
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrCount
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         Attr;
  int                                  Total;
  PyObject                            *Result;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  Total = 0;

  if (!PyArg_ParseTuple(args,"I|i",&Attr,&Total))
   {
    return(NULL);
   }

  try
   {
    if (Total)
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Instance->
                                  GetVAttrCount(BranchGroup->GroupIndex,Attr));
     }
    else
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                                  GetVAttrCount(BranchGroup->GroupIndex,Attr));
     }
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *CreateListFromFloatBuffer
                                      (float              *Buffer,
                                       unsigned int        ItemCount,
                                       unsigned int        Size)
 {
  PyObject                            *Result;
  unsigned int                         Index;
  float                               *Ptr;
  int                                  Ok;

  Result = PyList_New(Size);

  if (Result == NULL)
   {
    return(PyErr_NoMemory());
   }

  Ptr   = Buffer;
  Ok    = 1;
  Index = 0;

  while ((Index < Size) && (Ok))
   {
    PyObject      *Element;

    Element = PyTuple_New(ItemCount);

    if (Element != NULL)
     {
      unsigned int      ItemIndex;

      ItemIndex = 0;

      while ((ItemIndex < ItemCount) && (Ok))
       {
        if (PyTuple_SetItem(Element,ItemIndex,Py_BuildValue("f",*Ptr)) == 0)
         {
          Ptr++;

          ItemIndex++;
         }
        else
         {
          Ok = 0;
         }
       }

      if (Ok)
       {
        if (PyList_SetItem(Result,Index,Element) != 0)
         {
          Ok = 0;
         }
       }

      if (!Ok)
       {
        Py_DECREF(Element);
       }
     }
    else
     {
      Ok = 0;
     }

    Index++;
   }

  if (!Ok)
   {
    Py_DECREF(Result);

    Result = NULL;
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrBufferImpl
                                      (PyObject           *self,
                                       PyObject           *args,
                                       bool                CloneMode)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         Attr;
  PyObject                            *Result;
  unsigned int                         AttrItemCount;
  float                               *Buffer;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  if (!PyArg_ParseTuple(args,"I",&Attr))
   {
    return(NULL);
   }

  if (CloneMode && !BranchGroup->PlantInstance->Template->IsCloneable
                     (BranchGroup->GroupIndex,true))
   {
    PyErr_SetString(PyExc_RuntimeError,"trying to get clone attributes for non-cloneable group");

    return(NULL);
   }

  if      ((Attr == P3D_ATTR_VERTEX)  ||
           (Attr == P3D_ATTR_NORMAL)  ||
           (Attr == P3D_ATTR_TANGENT) ||
           (Attr == P3D_ATTR_BINORMAL))
   {
    AttrItemCount = 3;
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    AttrItemCount = 2;
   }
  else
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid attribute");

    return(NULL);
   }

  Buffer = NULL;

  try
   {
    unsigned int                       TotalAttrCount;

    if (CloneMode)
     {
      TotalAttrCount = BranchGroup->PlantInstance->Template->
                         GetVAttrCount(BranchGroup->GroupIndex,Attr);
     }
    else
     {
      TotalAttrCount = BranchGroup->PlantInstance->Instance->
                         GetVAttrCount(BranchGroup->GroupIndex,Attr);
     }

    Buffer = PyMem_New(float,AttrItemCount * TotalAttrCount);

    if (Buffer == NULL)
     {
      return(PyErr_NoMemory());
     }

    if (CloneMode)
     {
      BranchGroup->PlantInstance->Template->FillCloneVAttrBuffer(Buffer,BranchGroup->GroupIndex,Attr);
     }
    else
     {
      BranchGroup->PlantInstance->Instance->FillVAttrBuffer(Buffer,BranchGroup->GroupIndex,Attr);
     }

    Result = CreateListFromFloatBuffer(Buffer,AttrItemCount,TotalAttrCount);

    PyMem_Del(Buffer);
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    if (Buffer != NULL)
     {
      PyMem_Del(Buffer);
     }

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrBuffer
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  return BranchGroupGetVAttrBufferImpl(self,args,false);
 }

static PyObject    *BranchGroupGetCloneVAttrBuffer
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  return BranchGroupGetVAttrBufferImpl(self,args,true);
 }

static PyObject    *BranchGroupGetPrimitiveCount
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  int                                  All;
  PyObject                            *Result;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  All = 1;

  if (!PyArg_ParseTuple(args,"|i",&All))
   {
    return(NULL);
   }

  try
   {
    if (All)
     {
      Result = Py_BuildValue("l",(long)(BranchGroup->PlantInstance->Template->
                                  GetPrimitiveCount(BranchGroup->GroupIndex) *
                                   BranchGroup->PlantInstance->Instance->
                                  GetBranchCount(BranchGroup->GroupIndex)));
     }
    else
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                                  GetPrimitiveCount(BranchGroup->GroupIndex));
     }
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetPrimitiveType
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         PrimitiveIndex;
  int                                  All;
  PyObject                            *Result;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  All = 1;

  if (!PyArg_ParseTuple(args,"I|i",&PrimitiveIndex,&All))
   {
    return(NULL);
   }

  try
   {
    if (All)
     {
      PrimitiveIndex %= BranchGroup->PlantInstance->Template->
                         GetPrimitiveCount(BranchGroup->GroupIndex);
     }

    Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                                GetPrimitiveType(BranchGroup->GroupIndex,
                                                 PrimitiveIndex));
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrIndexBuffer
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         Attr;
  unsigned int                         IndexBase;
  int                                  All;
  PyObject                            *Result;
  unsigned int                        *Buffer;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  All       = 1;
  IndexBase = 0;

  if (!PyArg_ParseTuple(args,"I|ii",&Attr,&All,&IndexBase))
   {
    return(NULL);
   }

  Buffer = NULL;

  try
   {
    unsigned int   PrimitiveIndex;
    unsigned int   PrimitiveCount;
    unsigned int   BranchIndexCount;
    unsigned int   ReqBranchCount;

    if (All)
     {
      ReqBranchCount = BranchGroup->PlantInstance->Instance->
                        GetBranchCount(BranchGroup->GroupIndex);
     }
    else
     {
      ReqBranchCount = 1;
     }

    PrimitiveCount = BranchGroup->PlantInstance->Template->GetPrimitiveCount
                      (BranchGroup->GroupIndex);

    BranchIndexCount = 0;

    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      if (BranchGroup->PlantInstance->Template->GetPrimitiveType
           (BranchGroup->GroupIndex,PrimitiveIndex) == P3D_QUAD)
       {
        BranchIndexCount += 4;
       }
      else
       {
        BranchIndexCount += 3;
       }
     }

    Buffer = PyMem_New(unsigned int,BranchIndexCount);

    if (Buffer == NULL)
     {
      return(PyErr_NoMemory());
     }

    Result = PyList_New(PrimitiveCount * ReqBranchCount);

    if (Result != NULL)
     {
      int          Ok;
      unsigned int BranchIndex;
      unsigned int ArrayOffset;
      unsigned int*Ptr;
      unsigned int VAttrCount;

      Ok = 1;
      BranchIndex = 0;
      ArrayOffset = 0;
      VAttrCount  = BranchGroup->PlantInstance->Template->GetVAttrCount
                     (BranchGroup->GroupIndex,Attr);

      BranchGroup->PlantInstance->Template->FillVAttrIndexBuffer
       (Buffer,BranchGroup->GroupIndex,Attr,P3D_UNSIGNED_INT,0);

      while ((BranchIndex < ReqBranchCount) && (Ok))
       {
        PrimitiveIndex = 0;
        Ptr            = Buffer;

        while ((PrimitiveIndex < PrimitiveCount) && (Ok))
         {
          if (BranchGroup->PlantInstance->Template->GetPrimitiveType
               (BranchGroup->GroupIndex,PrimitiveIndex) == P3D_QUAD)
           {
            if (PyList_SetItem(Result,ArrayOffset,
                               Py_BuildValue("(llll)",
                                              (long)(IndexBase + Ptr[0]),
                                              (long)(IndexBase + Ptr[1]),
                                              (long)(IndexBase + Ptr[2]),
                                              (long)(IndexBase + Ptr[3]))) != 0)
             {
              Ok = 0;
             }

            Ptr += 4;
           }
          else
           {
            if (PyList_SetItem(Result,ArrayOffset,
                               Py_BuildValue("(lll)",
                                              (long)(IndexBase + Ptr[0]),
                                              (long)(IndexBase + Ptr[1]),
                                              (long)(IndexBase + Ptr[2]))) != 0)
             {
              Ok = 0;
             }

            Ptr += 3;
           }

          ArrayOffset++;
          PrimitiveIndex++;
         }

        IndexBase += VAttrCount;

        BranchIndex++;
       }

      if (!Ok)
       {
        Py_DECREF(Result);
       }
     }
    else
     {
      PyErr_NoMemory();
     }

    PyMem_Del(Buffer);
   }
  catch (P3DException       &Error)
   {
    if (Buffer != NULL)
     {
      PyMem_Del(Buffer);
     }

    if (Result != NULL)
     {
      Py_DECREF(Result);
     }

    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrCountI
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  int                                  Total;
  PyObject                            *Result;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  Total = 0;

  if (!PyArg_ParseTuple(args,"|i",&Total))
   {
    return(NULL);
   }

  try
   {
    if (Total)
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Instance->
                                  GetVAttrCountI(BranchGroup->GroupIndex));
     }
    else
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                                  GetVAttrCountI(BranchGroup->GroupIndex));
     }
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrBufferIImpl
                                      (PyObject           *self,
                                       PyObject           *args,
                                       bool                CloneMode)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         Attr;
  PyObject                            *Result;
  unsigned int                         AttrItemCount;
  float                               *Buffer;
  P3DHLIVAttrBuffers                   VAttrBuffers;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  if (!PyArg_ParseTuple(args,"I",&Attr))
   {
    return(NULL);
   }

  if (CloneMode && !BranchGroup->PlantInstance->Template->IsCloneable
                     (BranchGroup->GroupIndex,true))
   {
    PyErr_SetString(PyExc_RuntimeError,"trying to get clone attributes for non-cloneable group");

    return(NULL);
   }

  if      ((Attr == P3D_ATTR_VERTEX)  ||
           (Attr == P3D_ATTR_NORMAL)  ||
           (Attr == P3D_ATTR_TANGENT) ||
           (Attr == P3D_ATTR_BINORMAL))
   {
    AttrItemCount = 3;
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    AttrItemCount = 2;
   }
  else if (Attr == P3D_ATTR_BILLBOARD_POS)
   {
    AttrItemCount = 3;
   }
  else
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid attribute");

    return(NULL);
   }

  Buffer = NULL;

  try
   {
    unsigned int                       TotalAttrCount;

    if (CloneMode)
     {
      TotalAttrCount = BranchGroup->PlantInstance->Template->
                         GetVAttrCountI(BranchGroup->GroupIndex);
     }
    else
     {
      TotalAttrCount = BranchGroup->PlantInstance->Instance->
                         GetVAttrCountI(BranchGroup->GroupIndex);
     }

    Buffer = PyMem_New(float,AttrItemCount * TotalAttrCount);

    if (Buffer == NULL)
     {
      return(PyErr_NoMemory());
     }

    VAttrBuffers.AddAttr(Attr,Buffer,0,sizeof(float) * AttrItemCount);

    if (CloneMode)
     {
      BranchGroup->PlantInstance->Template->FillCloneVAttrBuffersI
       (&VAttrBuffers,BranchGroup->GroupIndex);
     }
    else
     {
      BranchGroup->PlantInstance->Instance->FillVAttrBuffersI
       (&VAttrBuffers,BranchGroup->GroupIndex);
     }

    Result = CreateListFromFloatBuffer(Buffer,AttrItemCount,TotalAttrCount);

    PyMem_Del(Buffer);
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    if (Buffer != NULL)
     {
      PyMem_Del(Buffer);
     }

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetVAttrBufferI
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  return BranchGroupGetVAttrBufferIImpl(self,args,false);
 }

static PyObject    *BranchGroupGetCloneVAttrBufferI
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  return BranchGroupGetVAttrBufferIImpl(self,args,true);
 }

static PyObject    *BranchGroupGetIndexCount
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         PrimitiveType;
  int                                  Total;
  PyObject                            *Result;

  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  Total = 1;

  if (!PyArg_ParseTuple(args,"I|i",&PrimitiveType,&Total))
   {
    return(NULL);
   }

  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid primitive type");

    return(NULL);
   }

  try
   {
    if (Total)
     {
      Result = Py_BuildValue("l",(long)(BranchGroup->PlantInstance->Template->
                                  GetIndexCount(BranchGroup->GroupIndex,PrimitiveType) *
                                   BranchGroup->PlantInstance->Instance->
                                  GetBranchCount(BranchGroup->GroupIndex)));
     }
    else
     {
      Result = Py_BuildValue("l",(long)BranchGroup->PlantInstance->Template->
                                  GetIndexCount(BranchGroup->GroupIndex,PrimitiveType));
     }
   }
  catch (P3DException       &Error)
   {
    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyObject    *BranchGroupGetIndexBuffer
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  BranchGroupObject                   *BranchGroup;
  unsigned int                         PrimitiveType;
  int                                  All;
  unsigned int                         IndexBase;
  PyObject                            *Result;
  unsigned int                        *Buffer;

  Result      = NULL;
  BranchGroup = (BranchGroupObject*)self;

  if (!PlantInstanceCheck(BranchGroup->PlantInstance))
   {
    return(NULL);
   }

  All       = 1;
  IndexBase = 0;

  if (!PyArg_ParseTuple(args,"I|ii",&PrimitiveType,&All,&IndexBase))
   {
    return(NULL);
   }

  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid primitive type");

    return(NULL);
   }

  Buffer = NULL;

  try
   {
    unsigned int   BranchIndexCount;
    unsigned int   ReqBranchCount;

    if (All)
     {
      ReqBranchCount = BranchGroup->PlantInstance->Instance->
                        GetBranchCount(BranchGroup->GroupIndex);
     }
    else
     {
      ReqBranchCount = 1;
     }

    BranchIndexCount = BranchGroup->PlantInstance->Template->
                        GetIndexCount(BranchGroup->GroupIndex,PrimitiveType);

    Buffer = PyMem_New(unsigned int,BranchIndexCount * ReqBranchCount);

    if (Buffer == NULL)
     {
      return(PyErr_NoMemory());
     }

    Result = PyList_New(BranchIndexCount * ReqBranchCount);

    if (Result != NULL)
     {
      int          Ok;
      unsigned int BranchIndex;
      unsigned int ArrayOffset;
      unsigned int*Ptr;
      unsigned int VAttrIndex;
      unsigned int VAttrCountI;

      Ok = 1;
      BranchIndex  = 0;
      ArrayOffset  = 0;
      VAttrCountI  = BranchGroup->PlantInstance->Template->GetVAttrCountI
                      (BranchGroup->GroupIndex);

      BranchGroup->PlantInstance->Template->FillIndexBuffer
       (Buffer,BranchGroup->GroupIndex,PrimitiveType,P3D_UNSIGNED_INT,0);

      while ((BranchIndex < ReqBranchCount) && (Ok))
       {
        VAttrIndex = 0;
        Ptr        = Buffer;

        while ((VAttrIndex < BranchIndexCount) && (Ok))
         {
          if (PyList_SetItem(Result,ArrayOffset,
                             Py_BuildValue("l",(long)(IndexBase + Ptr[0]))) == 0)
           {
            Ptr++;
           }
          else
           {
            Ok = 0;
           }

          ArrayOffset++;
          VAttrIndex++;
         }

        IndexBase += VAttrCountI;

        BranchIndex++;
       }

      if (!Ok)
       {
        Py_DECREF(Result);
       }
     }
    else
     {
      PyErr_NoMemory();
     }

    PyMem_Del(Buffer);
   }
  catch (P3DException       &Error)
   {
    if (Buffer != NULL)
     {
      PyMem_Del(Buffer);
     }

    if (Result != NULL)
     {
      Py_DECREF(Result);
     }

    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(NULL);
   }

  return(Result);
 }

static PyMethodDef BranchGroupMethods[] =
 {
  {
   "GetName",
   (PyCFunction)BranchGroupGetName,
   METH_NOARGS,
   "Return branch group name"
  },
  {
   "GetMaterial",
   (PyCFunction)BranchGroupGetMaterial,
   METH_NOARGS,
   "Return branch group material"
  },
  {
   "GetBillboardSize",
   (PyCFunction)BranchGroupGetBillboardSize,
   METH_NOARGS,
   "Return branch group billboards size"
  },
  {
   "IsCloneable",
   (PyCFunction)BranchGroupIsCloneable,
   METH_VARARGS,
   "Return boolean indicating if branch group is cloneable"
  },
  {
   "GetBranchCount",
   (PyCFunction)BranchGroupGetBranchCount,
   METH_NOARGS,
   "Return branch count in group"
  },
  {
   "IsLODVisRangeEnabled",
   (PyCFunction)BranchGroupIsLODVisRangeEnabled,
   METH_NOARGS,
   "Return state of LOD visibility range (enabled/disabled)"
  },
  {
   "GetLODVisRange",
   (PyCFunction)BranchGroupGetLODVisRange,
   METH_NOARGS,
   "Return tuple of two floats which describe LOD visibility range (MinLOD,MaxLOD)"
  },
  {
   "GetVAttrCount",
   (PyCFunction)BranchGroupGetVAttrCount,
   METH_VARARGS,
   "Return count of different attribute values"
  },
  {
   "GetVAttrBuffer",
   (PyCFunction)BranchGroupGetVAttrBuffer,
   METH_VARARGS,
   "Return array of attributes"
  },
  {
   "GetCloneVAttrBuffer",
   (PyCFunction)BranchGroupGetCloneVAttrBuffer,
   METH_VARARGS,
   "Return array of attributes for cloneable branch group"
  },
  {
   "GetPrimitiveCount",
   (PyCFunction)BranchGroupGetPrimitiveCount,
   METH_VARARGS,
   "Return count of primitives in single branch or in branch group"
  },
  {
   "GetPrimitiveType",
   (PyCFunction)BranchGroupGetPrimitiveType,
   METH_VARARGS,
   "Return type of primitive by its index in a single branch or in branch group"
  },
  {
   "GetVAttrIndexBuffer",
   (PyCFunction)BranchGroupGetVAttrIndexBuffer,
   METH_VARARGS,
   "Return array of indices"
  },
  {
   "GetVAttrCountI",
   (PyCFunction)BranchGroupGetVAttrCountI,
   METH_VARARGS,
   "Return branch group vertex count"
  },
  {
   "GetVAttrBufferI",
   (PyCFunction)BranchGroupGetVAttrBufferI,
   METH_VARARGS,
   "Return array of attributes (indexed mode)"
  },
  {
   "GetCloneVAttrBufferI",
   (PyCFunction)BranchGroupGetCloneVAttrBufferI,
   METH_VARARGS,
   "Return array of attributes for cloneable group (indexed mode)"
  },

  {
   "GetIndexCount",
   (PyCFunction)BranchGroupGetIndexCount,
   METH_VARARGS,
   "Return branch group index count"
  },
  {
   "GetIndexBuffer",
   (PyCFunction)BranchGroupGetIndexBuffer,
   METH_VARARGS,
   "Return branch group index buffer"
  },
  { NULL }
 };

PyTypeObject BranchGroupType =
 {
  PyObject_HEAD_INIT(NULL)
  0,                                /*ob_size*/
  "_ngp.BranchGroup",               /*tp_name*/
  sizeof(BranchGroupObject),        /*tp_basicsize*/
  0,                                /*tp_itemsize*/
  (destructor)BranchGroupDealloc,   /*tp_dealloc*/
  0,                                /*tp_print*/
  0,                                /*tp_getattr*/
  0,                                /*tp_setattr*/
  0,                                /*tp_compare*/
  0,                                /*tp_repr*/
  0,                                /*tp_as_number*/
  0,                                /*tp_as_sequence*/
  0,                                /*tp_as_mapping*/
  0,                                /*tp_hash*/
  0,                                /*tp_call*/
  0,                                /*tp_str*/
  0,                                /*tp_getattro*/
  0,                                /*tp_setattro*/
  0,                                /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,               /*tp_flags*/
  "BranchGroup objects",            /*tp_doc*/
  0,                                /*tp_traverse */
  0,                                /*tp_clear*/
  0,                                /*tp_richcompare*/
  0,                                /*tp_weaklistoffset*/
  0,                                /*tp_iter*/
  0,                                /*tp_iternext*/
  BranchGroupMethods,               /*tp_methods*/
  0,                                /*tp_members*/
  0,                                /*tp_getset*/
  0,                                /*tp_base*/
  0,                                /*tp_dict*/
  0,                                /*tp_descr_get*/
  0,                                /*tp_descr_set */
  0,                                /*tp_dictoffset */
  (initproc)BranchGroupInit,        /*tp_init */
  0,                                /*tp_alloc */
  BranchGroupNew                    /*tp_new */
 };

