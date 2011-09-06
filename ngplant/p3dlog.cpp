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
#include <stdarg.h>

#include <wx/wx.h>

#include <p3dlog.h>

static bool StdErrEnabled  = false;
static bool VerboseEnabled = false;

extern void        P3DLogError        (const wxChar       *Format,
                                       ...)
 {
  wxString                             MessageStr;
  va_list                              Args;

  va_start(Args,Format);

  MessageStr = wxString::FormatV(Format,Args);

  va_end(Args);

  if (StdErrEnabled)
   {
    fprintf(stderr,"error: %s",(const char*)MessageStr.mb_str());
   }

  wxLogError(MessageStr);
 }

extern void        P3DLogInfo         (const wxChar       *Format,
                                       ...)
 {
  wxString                             MessageStr;
  va_list                              Args;

  if (!VerboseEnabled)
   {
    return;
   }

  va_start(Args,Format);

  MessageStr = wxString::FormatV(Format,Args);

  va_end(Args);

  if (StdErrEnabled)
   {
    fprintf(stderr,"info: %s",(const char*)MessageStr.mb_str());
   }
 }

extern void        P3DLogEnableStdErr ()
 {
  StdErrEnabled = true;
 }

extern void        P3DLogEnableVerbose()
 {
  VerboseEnabled = true;
 }

