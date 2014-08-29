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
#include <ngppybranchgroup.h>
#include <ngppymaterialdef.h>
#include <ngppymodelmetainfo.h>

static PyMethodDef ModuleMethods[] =
 {
  { NULL }
 };

#ifndef PyMODINIT_FUNC
 #define PyMODINIT_FUNC extern "C" void
#endif

PyMODINIT_FUNC     init_ngp            (void)
 {
  PyObject        *ModuleObject;

  PlantInstanceType.tp_new = PyType_GenericNew;

  if (PyType_Ready(&PlantInstanceType) < 0)
   {
    return;
   }

  BranchGroupType.tp_new = PyType_GenericNew;

  if (PyType_Ready(&BranchGroupType) < 0)
   {
    return;
   }

  MaterialDefType.tp_new = PyType_GenericNew;

  if (PyType_Ready(&MaterialDefType) < 0)
   {
    return;
   }

  ModelMetaInfoType.tp_new = PyType_GenericNew;

  if (PyType_Ready(&ModelMetaInfoType) < 0)
   {
    return;
   }

  ModuleObject = Py_InitModule3("_ngp",ModuleMethods,"ngPlant library python extension module");

  Py_INCREF(&PlantInstanceType);
  Py_INCREF(&BranchGroupType);
  Py_INCREF(&MaterialDefType);
  Py_INCREF(&ModelMetaInfoType);

  PyModule_AddObject(ModuleObject,"PlantInstance",(PyObject*)&PlantInstanceType);
  PyModule_AddObject(ModuleObject,"BranchGroup",(PyObject*)&BranchGroupType);
  PyModule_AddObject(ModuleObject,"MaterialDef",(PyObject*)&MaterialDefType);
  PyModule_AddObject(ModuleObject,"ModelMetaInfo",(PyObject*)&ModelMetaInfoType);

  PyModule_AddIntConstant(ModuleObject,"ATTR_VERTEX",(long)P3D_ATTR_VERTEX);
  PyModule_AddIntConstant(ModuleObject,"ATTR_NORMAL",(long)P3D_ATTR_NORMAL);
  PyModule_AddIntConstant(ModuleObject,"ATTR_TEXCOORD0",(long)P3D_ATTR_TEXCOORD0);
  PyModule_AddIntConstant(ModuleObject,"ATTR_TANGENT",(long)P3D_ATTR_TANGENT);
  PyModule_AddIntConstant(ModuleObject,"ATTR_BINORMAL",(long)P3D_ATTR_BINORMAL);
  PyModule_AddIntConstant(ModuleObject,"ATTR_BILLBOARD_POS",(long)P3D_ATTR_BILLBOARD_POS);

  PyModule_AddIntConstant(ModuleObject,"BILLBOARD_MODE_NONE",(long)P3D_BILLBOARD_MODE_NONE);
  PyModule_AddIntConstant(ModuleObject,"BILLBOARD_MODE_SPHERICAL",(long)P3D_BILLBOARD_MODE_SPHERICAL);
  PyModule_AddIntConstant(ModuleObject,"BILLBOARD_MODE_CYLINDRICAL",(long)P3D_BILLBOARD_MODE_CYLINDRICAL);

  PyModule_AddIntConstant(ModuleObject,"TRIANGLE",(long)P3D_TRIANGLE);
  PyModule_AddIntConstant(ModuleObject,"TRIANGLE_LIST",(long)P3D_TRIANGLE_LIST);
  PyModule_AddIntConstant(ModuleObject,"TRIANGLE_STRIP",(long)P3D_TRIANGLE_STRIP);
  PyModule_AddIntConstant(ModuleObject,"QUAD",(long)P3D_QUAD);

  PyModule_AddIntConstant(ModuleObject,"TEX_DIFFUSE",(long)P3D_TEX_DIFFUSE);
  PyModule_AddIntConstant(ModuleObject,"TEX_NORMAL_MAP",(long)P3D_TEX_NORMAL_MAP);
  PyModule_AddIntConstant(ModuleObject,"TEX_AUX0",(long)P3D_TEX_AUX0);
  PyModule_AddIntConstant(ModuleObject,"TEX_AUX1",(long)P3D_TEX_AUX1);
 }

