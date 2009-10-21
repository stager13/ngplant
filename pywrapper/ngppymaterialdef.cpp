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

#include <ngppymaterialdef.h>

static void        MaterialDefDealloc (MaterialDefObject  *self)
 {
  Py_XDECREF(self->Color);
  Py_XDECREF(self->TexNames);
  Py_XDECREF(self->DoubleSided);
  Py_XDECREF(self->Transparent);
  Py_XDECREF(self->Billboard);
  Py_XDECREF(self->BillboardMode);
  Py_XDECREF(self->AlphaCtrlEnabled);
  Py_XDECREF(self->AlphaFadeRange);

  self->ob_type->tp_free((PyObject*)self);
 }

static PyObject   *MaterialDefNew     (PyTypeObject       *type,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  MaterialDefObject                   *self;

  self = (MaterialDefObject*)type->tp_alloc(type,0);

  if (self != NULL)
   {
    self->Color       = Py_BuildValue("(fff)",0.0f,0.0f,0.0f);
    Py_INCREF(Py_None);
    self->TexNames    = Py_BuildValue("[OOOO]",Py_None,Py_None,Py_None,Py_None);
    self->DoubleSided = Py_BuildValue("i",0);

    Py_INCREF(self->DoubleSided);
    Py_INCREF(self->DoubleSided);
    Py_INCREF(self->DoubleSided);

    self->Transparent = self->DoubleSided;
    self->Billboard   = self->DoubleSided;
    self->BillboardMode = Py_BuildValue("i",P3D_BILLBOARD_MODE_NONE);
    self->AlphaCtrlEnabled  = self->DoubleSided;

    self->AlphaFadeRange = Py_BuildValue("(ff)",0.0f,1.0f);
   }

  return((PyObject*)self);
 }

static int         MaterialDefInit    (MaterialDefObject  *self,
                                       PyObject           *args,
                                       PyObject           *kwds)
 {
  float                                R,G,B;
  PyObject                            *TempTexNames;
  int                                  TempDoubleSided;
  int                                  TempTransparent;
  int                                  TempBillboard;
  int                                  TempBillboardMode;
  int                                  AlphaCtrlEnabled;
  float                                AlphaFadeIn;
  float                                AlphaFadeOut;

  if (!PyArg_ParseTuple
        (args,
         "(fff)Oiiiii(ff)",
         &R,&G,&B,
         &TempTexNames,
         &TempDoubleSided,
         &TempTransparent,
         &TempBillboard,
         &TempBillboardMode,
         &AlphaCtrlEnabled,
         &AlphaFadeIn,
         &AlphaFadeOut))
   {
    return(-1);
   }

  if      (PyList_Check(TempTexNames))
   {
   }
  else
   {
    PyErr_SetString(PyExc_RuntimeError,"texture names argument must be a list");

    return(-1);
   }

  Py_XDECREF(self->Color);
  self->Color = Py_BuildValue("(fff)",R,G,B);
  Py_XDECREF(self->TexNames);
  Py_INCREF(TempTexNames);
  self->TexNames = TempTexNames;

  Py_XDECREF(self->DoubleSided);
  self->DoubleSided = Py_BuildValue("i",TempDoubleSided);
  Py_XDECREF(self->Transparent);
  self->Transparent = Py_BuildValue("i",TempTransparent);
  Py_XDECREF(self->Billboard);
  self->Billboard = Py_BuildValue("i",TempBillboard);
  Py_XDECREF(self->BillboardMode);
  self->BillboardMode = Py_BuildValue("i",TempBillboardMode);
  Py_XDECREF(self->AlphaCtrlEnabled);
  self->AlphaCtrlEnabled = Py_BuildValue("i",AlphaCtrlEnabled);
  Py_XDECREF(self->AlphaFadeRange);
  self->AlphaFadeRange = Py_BuildValue("(ff)",AlphaFadeIn,AlphaFadeOut);

  return(0);
 }

static PyObject   *MaterialDefGetColor(MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->Color);

  return(self->Color);
 }

static PyObject   *MaterialDefGetTexName
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->TexNames);

  return(self->TexNames);
 }

static PyObject   *MaterialDefGetDoubleSided
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->DoubleSided);

  return(self->DoubleSided);
 }

static PyObject   *MaterialDefGetTransparent
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->Transparent);

  return(self->Transparent);
 }

static PyObject   *MaterialDefGetBillboard
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->Billboard);

  return(self->Billboard);
 }

static PyObject   *MaterialDefGetBillboardMode
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->BillboardMode);

  return(self->BillboardMode);
 }

static PyObject   *MaterialDefGetAlphaCtrlEnabled
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->AlphaCtrlEnabled);

  return(self->AlphaCtrlEnabled);
 }

static PyObject   *MaterialDefGetAlphaFadeRange
                                      (MaterialDefObject  *self,
                                       void               *closure)
 {
  Py_INCREF(self->AlphaFadeRange);

  return(self->AlphaFadeRange);
 }

static int         MaterialDefSetAttrRO
                                      (MaterialDefObject  *self,
                                       PyObject           *value,
                                       void               *closure)
 {
  PyErr_SetString(PyExc_RuntimeError,"attribute is read-only");

  return(-1);
 }

static PyGetSetDef MaterialDefGetSeters[] =
 {
  {
   "Color",
   (getter)MaterialDefGetColor,
   (setter)MaterialDefSetAttrRO,
   "Base color",
   NULL
  },
  {
   "TexNames",
   (getter)MaterialDefGetTexName,
   (setter)MaterialDefSetAttrRO,
   "List of texture names",
   NULL
  },
  {
   "DoubleSided",
   (getter)MaterialDefGetDoubleSided,
   (setter)MaterialDefSetAttrRO,
   "Double-sided flag",
   NULL
  },
  {
   "Transparent",
   (getter)MaterialDefGetTransparent,
   (setter)MaterialDefSetAttrRO,
   "Transparency flag",
   NULL
  },
  {
   "Billboard",
   (getter)MaterialDefGetBillboard,
   (setter)MaterialDefSetAttrRO,
   "Billboard flag",
   NULL
  },
  {
   "BillboardMode",
   (getter)MaterialDefGetBillboardMode,
   (setter)MaterialDefSetAttrRO,
   "Billboard mode",
   NULL
  },
  {
   "AlphaCtrlEnabled",
   (getter)MaterialDefGetAlphaCtrlEnabled,
   (setter)MaterialDefSetAttrRO,
   "Alpha control state flag",
   NULL
  },
  {
   "AlphaFadeRange",
   (getter)MaterialDefGetAlphaFadeRange,
   (setter)MaterialDefSetAttrRO,
   "Alpha fade range",
   NULL
  },
  {
   NULL
  }
 };

static PyMethodDef MaterialDefMethods[] =
 {
  { NULL }
 };

PyTypeObject MaterialDefType =
 {
  PyObject_HEAD_INIT(NULL)
  0,                                /*ob_size*/
  "_ngp.MaterialDef",               /*tp_name*/
  sizeof(MaterialDefObject),        /*tp_basicsize*/
  0,                                /*tp_itemsize*/
  (destructor)MaterialDefDealloc,   /*tp_dealloc*/
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
  "MaterialDef objects",            /*tp_doc*/
  0,                                /*tp_traverse*/
  0,                                /*tp_clear*/
  0,                                /*tp_richcompare*/
  0,                                /*tp_weaklistoffset*/
  0,                                /*tp_iter*/
  0,                                /*tp_iternext*/
  MaterialDefMethods,               /*tp_methods*/
  0,                                /*tp_members*/
  MaterialDefGetSeters,             /*tp_getset*/
  0,                                /*tp_base*/
  0,                                /*tp_dict*/
  0,                                /*tp_descr_get*/
  0,                                /*tp_descr_set*/
  0,                                /*tp_dictoffset*/
  (initproc)MaterialDefInit,        /*tp_init*/
  0,                                /*tp_alloc*/
  MaterialDefNew                    /*tp_new*/
 };

