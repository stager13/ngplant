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

#include <string.h>
#include <stdlib.h>

#include <ngpcore/p3dexcept.h>

#include <ngpcore/p3dconststr.h>

                   P3DConstStr::P3DConstStr     ()
 {
  InitEmpty();
 }

                   P3DConstStr::P3DConstStr     (const char         *SourceStr)
 {
  InitEmpty();
  Set(SourceStr);
 }

                   P3DConstStr::P3DConstStr     (const P3DConstStr  &SourceStr)
 {
  InitEmpty();
  Set(SourceStr.c_str());
 }

                   P3DConstStr::~P3DConstStr    ()
 {
  Free();
 }

const P3DConstStr& P3DConstStr::operator =      (const P3DConstStr  &SourceStr)
 {
  if (this != &SourceStr)
   {
    Set(SourceStr.c_str());
   }

  return *this;
 }

const
P3DConstStr       &P3DConstStr::operator =      (const char         *SourceStr)
 {
  Set(SourceStr);

  return *this;
 }

const char*        P3DConstStr::c_str           () const
 {
  return Str;
 }

void               P3DConstStr::InitEmpty       ()
 {
  Str = 0;
 }

void               P3DConstStr::Set             (const char         *SourceStr)
 {
  const char *NewStr = SourceStr != 0 ? strdup(SourceStr) : 0;

  if (SourceStr == 0 || NewStr != 0)
   {
    Free();

    Str = NewStr;
   }
  else
   {
    throw P3DExceptionGeneric("out of memory");
   }
 }

void               P3DConstStr::Free            ()
 {
  free(const_cast<char*>(Str));

  Str = 0;
 }

