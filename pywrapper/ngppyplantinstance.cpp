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

#include <ngppyplantinstance.h>
#include <ngppybranchgroup.h>

static void        PlantInstanceDealloc
                                      (PlantInstanceObject*self)
 {
  delete self->Instance;
  delete self->Template;

  self->ob_type->tp_free((PyObject*)self);
 }

static PyObject   *PlantInstanceNew   (PyTypeObject       *type,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  PlantInstanceObject                 *self;

  self = (PlantInstanceObject*)type->tp_alloc(type,0);

  if (self != NULL)
   {
    self->Template = NULL;
    self->Instance = NULL;
   }

  return((PyObject*)self);
 }

static int         PlantInstanceInit  (PlantInstanceObject*self,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  char                                *FileName;
  unsigned int                         Seed;
  int                                  EnableDummies;

  Seed          = 0;
  EnableDummies = 0;

  if (!PyArg_ParseTuple(args,"s|Ii",&FileName,&Seed,&EnableDummies))
   {
    return(-1);
   }

  try
   {
    P3DInputStringStreamFile           SourceStream;

    SourceStream.Open(FileName);

    self->Template = new P3DHLIPlantTemplate(&SourceStream);

    if (EnableDummies)
     {
      self->Template->SetDummiesEnabled(true);
     }

    self->Instance = self->Template->CreateInstance(Seed);

    SourceStream.Close();
   }
  catch (P3DException       &Error)
   {
    delete self->Template;
    delete self->Instance;

    self->Template = NULL;
    self->Instance = NULL;

    PyErr_SetString(PyExc_RuntimeError,Error.GetMessage());

    return(-1);
   }
  catch (...)
   {
    delete self->Template;
    delete self->Instance;

    self->Template = NULL;
    self->Instance = NULL;

    PyErr_SetString(PyExc_RuntimeError,"undefined error");

    return(-1);
   }

  return(0);
 }

extern "C" int    PlantInstanceCheck(PlantInstanceObject*InstanceObject)
 {
  if ((InstanceObject->Template == NULL) ||
      (InstanceObject->Instance == NULL))
   {
    PyErr_SetString(PyExc_RuntimeError,"object is not initialized");

    return(0);
   }
  else
   {
    return(1);
   }
 }

static PyObject    *PlantInstanceGetGroupCount
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  PlantInstanceObject                 *InstanceObject;

  InstanceObject = (PlantInstanceObject*)self;

  if (!PlantInstanceCheck(InstanceObject))
   {
    return(NULL);
   }

  return(Py_BuildValue("l",(long int)InstanceObject->Template->GetGroupCount()));
 }

static PyObject    *PlantInstanceGetGroup
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  PlantInstanceObject                 *InstanceObject;
  unsigned int                         GroupIndex;
  BranchGroupObject                   *GroupObject;

  InstanceObject = (PlantInstanceObject*)self;

  if (!PlantInstanceCheck(InstanceObject))
   {
    return(NULL);
   }

  if (!PyArg_ParseTuple(args,"I",&GroupIndex))
   {
    return(NULL);
   }

  if (GroupIndex >= InstanceObject->Template->GetGroupCount())
   {
    PyErr_SetString(PyExc_RuntimeError,"invalid group index");

    return(NULL);
   }

  GroupObject = PyObject_New(BranchGroupObject,&BranchGroupType);

  if (GroupObject != NULL)
   {
    Py_INCREF(self);

    GroupObject->PlantInstance = (PlantInstanceObject*)self;
    GroupObject->GroupIndex    = GroupIndex;
   }

  return((PyObject*)GroupObject);
 }

static PyObject    *PlantInstanceGetBoundingBox
                                      (PyObject           *self,
                                       PyObject           *args)
 {
  PlantInstanceObject                 *InstanceObject;
  float                                Min[3];
  float                                Max[3];

  InstanceObject = (PlantInstanceObject*)self;

  if (!PlantInstanceCheck(InstanceObject))
   {
    return(NULL);
   }

  InstanceObject->Instance->GetBoundingBox(Min,Max);

  return(Py_BuildValue("((fff)(fff))",Min[0],Min[1],Min[2],Max[0],Max[1],Max[2]));
 }

static PyMethodDef PlantInstanceMethods[] =
 {
  {
   "GetGroupCount",
   (PyCFunction)PlantInstanceGetGroupCount,
   METH_NOARGS,
   "Return plant branch group count"
  },
  {
   "GetGroup",
   (PyCFunction)PlantInstanceGetGroup,
   METH_VARARGS,
   "Return branch group by GroupIndex"
  },
  {
   "GetBoundingBox",
   (PyCFunction)PlantInstanceGetBoundingBox,
   METH_VARARGS,
   "Return plant bounding box"
  },
  { NULL }
 };

PyTypeObject PlantInstanceType =
 {
  PyObject_HEAD_INIT(NULL)
  0,                                /*ob_size*/
  "_ngp.PlantInstance",             /*tp_name*/
  sizeof(PlantInstanceObject),      /*tp_basicsize*/
  0,                                /*tp_itemsize*/
  (destructor)PlantInstanceDealloc, /*tp_dealloc*/
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
  "PlantInstance objects",          /*tp_doc*/
  0,                                /*tp_traverse */
  0,                                /*tp_clear*/
  0,                                /*tp_richcompare*/
  0,                                /*tp_weaklistoffset*/
  0,                                /*tp_iter*/
  0,                                /*tp_iternext*/
  PlantInstanceMethods,             /*tp_methods*/
  0,                                /*tp_members*/
  0,                                /*tp_getset*/
  0,                                /*tp_base*/
  0,                                /*tp_dict*/
  0,                                /*tp_descr_get*/
  0,                                /*tp_descr_set */
  0,                                /*tp_dictoffset */
  (initproc)PlantInstanceInit,      /*tp_init */
  0,                                /*tp_alloc */
  PlantInstanceNew                  /*tp_new */
 };

