/***************************************************************************

 Copyright (c) 2014 Sergey Prokhorchuk.
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

#include <ngppymodelmetainfo.h>

static void        ModelMetaInfoDealloc    (ModelMetaInfoObject     *self)
 {
  Py_XDECREF(self->Author);
  Py_XDECREF(self->AuthorURL);
  Py_XDECREF(self->LicenseName);
  Py_XDECREF(self->LicenseURL);
  Py_XDECREF(self->PlantInfoURL);

  self->ob_type->tp_free((PyObject*)self);
 }

static PyObject   *ModelMetaInfoNew   (PyTypeObject       *type,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  ModelMetaInfoObject                 *self;

  self = (ModelMetaInfoObject*)type->tp_alloc(type,0);

  if (self != NULL)
   {
    Py_INCREF(Py_None); self->Author       = Py_None;
    Py_INCREF(Py_None); self->AuthorURL    = Py_None;
    Py_INCREF(Py_None); self->LicenseName  = Py_None;
    Py_INCREF(Py_None); self->LicenseURL   = Py_None;
    Py_INCREF(Py_None); self->PlantInfoURL = Py_None;
   }

  return((PyObject*)self);
 }

static void        AssignPyObject     (PyObject          **dest,
                                       PyObject           *src)
 {
  Py_XDECREF(*dest);
  Py_INCREF(src);

  *dest = src;
 }

static int         ModelMetaInfoInit  (ModelMetaInfoObject*self,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  if (PyTuple_Size(args) != 5)
   {
    PyErr_SetString(PyExc_RuntimeError,"ModelMetaInfo constructor requires exactly 4 arguments");

    return -1;
   }

  for (int Index = 0; Index < 5; Index++)
   {
    PyObject *Arg = PyTuple_GetItem(args,Index);

    if (Arg != Py_None && !PyString_Check(Arg))
     {
      PyErr_SetString(PyExc_RuntimeError,"String argument required");

      return -1;
     }
   }

  AssignPyObject(&self->Author,PyTuple_GetItem(args,0));
  AssignPyObject(&self->AuthorURL,PyTuple_GetItem(args,1));
  AssignPyObject(&self->LicenseName,PyTuple_GetItem(args,2));
  AssignPyObject(&self->LicenseURL,PyTuple_GetItem(args,3));
  AssignPyObject(&self->PlantInfoURL,PyTuple_GetItem(args,4));

  return(0);
 }

static PyObject   *ModelMetaInfoGetAuthor
                                      (ModelMetaInfoObject*self,
                                       void               *closure)
 {
  Py_INCREF(self->Author);

  return(self->Author);
 }

static PyObject   *ModelMetaInfoGetAuthorURL
                                      (ModelMetaInfoObject*self,
                                       void               *closure)
 {
  Py_INCREF(self->AuthorURL);

  return(self->AuthorURL);
 }

static PyObject   *ModelMetaInfoGetLicenseName
                                      (ModelMetaInfoObject*self,
                                       void               *closure)
 {
  Py_INCREF(self->LicenseName);

  return(self->LicenseName);
 }

static PyObject   *ModelMetaInfoGetLicenseURL
                                      (ModelMetaInfoObject*self,
                                       void               *closure)
 {
  Py_INCREF(self->LicenseURL);

  return(self->LicenseURL);
 }

static PyObject   *ModelMetaInfoGetPlantInfoURL
                                      (ModelMetaInfoObject*self,
                                       void               *closure)
 {
  Py_INCREF(self->PlantInfoURL);

  return(self->PlantInfoURL);
 }

static int         ModelMetaInfoSetAttrRO
                                      (ModelMetaInfoObject*self,
                                       PyObject           *value,
                                       void               *closure)
 {
  PyErr_SetString(PyExc_RuntimeError,"attribute is read-only");

  return(-1);
 }

static PyGetSetDef ModelMetaInfoGetSetters[] =
 {
  {
   "Author",
   (getter)ModelMetaInfoGetAuthor,
   (setter)ModelMetaInfoSetAttrRO,
   "Author",
   NULL
  },
  {
   "AuthorURL",
   (getter)ModelMetaInfoGetAuthorURL,
   (setter)ModelMetaInfoSetAttrRO,
   "AuthorURL",
   NULL
  },
  {
   "LicenseName",
   (getter)ModelMetaInfoGetLicenseName,
   (setter)ModelMetaInfoSetAttrRO,
   "License name",
   NULL
  },
  {
   "LicenseURL",
   (getter)ModelMetaInfoGetLicenseURL,
   (setter)ModelMetaInfoSetAttrRO,
   "License URL",
   NULL
  },
  {
   "PlantInfoURL",
   (getter)ModelMetaInfoGetPlantInfoURL,
   (setter)ModelMetaInfoSetAttrRO,
   "Plant information URL",
   NULL
  },
  {
   NULL
  }
 };

static PyMethodDef ModelMetaInfoMethods[] =
 {
  { NULL }
 };

PyTypeObject ModelMetaInfoType =
 {
  PyObject_HEAD_INIT(NULL)
  0,                                /*ob_size*/
  "_ngp.ModelMetaInfo",             /*tp_name*/
  sizeof(ModelMetaInfoObject),      /*tp_basicsize*/
  0,                                /*tp_itemsize*/
  (destructor)ModelMetaInfoDealloc, /*tp_dealloc*/
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
  "ModelMetaInfo objects",          /*tp_doc*/
  0,                                /*tp_traverse*/
  0,                                /*tp_clear*/
  0,                                /*tp_richcompare*/
  0,                                /*tp_weaklistoffset*/
  0,                                /*tp_iter*/
  0,                                /*tp_iternext*/
  ModelMetaInfoMethods,             /*tp_methods*/
  0,                                /*tp_members*/
  ModelMetaInfoGetSetters,          /*tp_getset*/
  0,                                /*tp_base*/
  0,                                /*tp_dict*/
  0,                                /*tp_descr_get*/
  0,                                /*tp_descr_set*/
  0,                                /*tp_dictoffset*/
  (initproc)ModelMetaInfoInit,      /*tp_init*/
  0,                                /*tp_alloc*/
  ModelMetaInfoNew                  /*tp_new*/
 };

