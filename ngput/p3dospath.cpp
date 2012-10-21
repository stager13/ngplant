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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include <string>

#ifdef _WIN32
 #include <windows.h>
#endif

#include <ngput/p3dospath.h>

static bool        P3DOSGetEnv        (std::string        &Value,
                                       const char         *Name)
 {
  char                                *EnvValue;

  EnvValue = getenv(Name);

  if (EnvValue != NULL)
   {
    Value = EnvValue;

    return(true);
   }
  else
   {
    return(false);
   }
 }

#ifdef _WIN32
std::string   P3DPathInfo::GetUserDir ()
 {
  std::string                          Result;
  std::string                          HomeDrive;
  std::string                          HomePath;

  if (P3DOSGetEnv(Result,"HOME"))
   {
    return(Result);
   }

  if (P3DOSGetEnv(HomeDrive,"HOMEDRIVE") && P3DOSGetEnv(HomePath,"HOMEPATH"))
   {
    Result = HomeDrive + std::string("\\") + HomePath;

    return(Result);
   }

  if (P3DOSGetEnv(Result,"USERPROFILE"))
   {
    return(Result);
   }

  return(std::string(""));
 }
#else
std::string   P3DPathInfo::GetUserDir ()
 {
  std::string                          Result;

  if (P3DOSGetEnv(Result,"HOME"))
   {
    return(Result);
   }
  else
   {
    return(std::string(""));
   }
 }
#endif

#ifdef _WIN32
std::string P3DPathInfo::GetCurrentDir()
 {
  DWORD                                Result;
  char                                 CurrentDirBuf[FILENAME_MAX];

  Result = GetCurrentDirectory(sizeof(CurrentDirBuf),CurrentDirBuf);

  if ((Result == 0) || (Result > sizeof(CurrentDirBuf)))
   {
    return(std::string(""));
   }
  else
   {
    return(std::string(CurrentDirBuf));
   }
 }
#else
std::string P3DPathInfo::GetCurrentDir()
 {
  char                                 CurrentDirBuf[FILENAME_MAX];

  if (getcwd(CurrentDirBuf,sizeof(CurrentDirBuf)) == NULL)
   {
    return(std::string(""));
   }
  else
   {
    return(std::string(CurrentDirBuf));
   }
 }
#endif

                   P3DPathName::P3DPathName
                                      (const char         *PathName)
                   : Path(PathName)
 {
 }

bool               P3DPathName::IsAbsolute
                                      () const
 {
  if (Path.empty())
   {
    return(false);
   }

  #ifdef _WIN32
  if (Path.length() >= 3)
   {
    if (((Path[0] >= 'a') && (Path[0] <= 'z')) ||
        ((Path[0] >= 'A') && (Path[0] <= 'Z')))
     {
      if (Path[1] == ':')
       {
        if (Path[2] == '\\')
         {
          return(true);
         }
       }
     }
   }
  #else
  if (Path[0] == '/')
   {
    return(true);
   }
  #endif

  return(false);
 }

bool               P3DPathName::IsRelative
                                      () const
 {
  return(!IsAbsolute());
 }

const char        *P3DPathName::c_str () const
 {
  return(Path.c_str());
 }

void               P3DPathName::Normalize
                                      ()
 {
  if (!Path.empty())
   {
    if (Path[Path.length() - 1] == '/')
     {
      Path.erase(Path.length() - 1);
     }
   }
 }

bool               P3DPathName::ExpandUserDir
                                      ()
 {
  if (!Path.empty())
   {
    if (Path[0] == '~')
     {
      std::string                      HomeDir;

      HomeDir = P3DPathInfo::GetUserDir();

      if (HomeDir.empty())
       {
        return(false);
       }
      else
       {
        Path[0] = '/';

        Path = HomeDir + Path;
       }
     }
   }

  return(true);
 }

bool               P3DPathName::MakeAbsolute
                                      ()
 {
  std::string                          CurrentDir;

  if (IsAbsolute())
   {
    return(true);
   }

  CurrentDir = P3DPathInfo::GetCurrentDir();

  if (CurrentDir.empty())
   {
    return(false);
   }

  P3DPathName                          WorkingDir(CurrentDir.c_str());

  WorkingDir.Normalize();

  Path = std::string(WorkingDir.c_str()) + std::string("/") + std::string(Path.c_str());

  Normalize();

  return(true);
 }

static int         p3d_memeq          (const char         *A1,
                                       const char         *A2,
                                       size_t              SIZE)
 {
  return(!memcmp(A1,A2,SIZE));
 }

static int         p3d_memcaseeq      (const char         *A1,
                                       const char         *A2,
                                       size_t              SIZE)
 {
  int              d;

  d = 1;

  while ((SIZE-- != 0) && (d = tolower(*A1++) - tolower(*A2++) == 0))
   ;

  return d;
 }

bool               P3DPathName::MakeRelativeTo
                                      (const P3DPathName *BasePath)
 {
  if (BasePath->Path.length() >= Path.length())
   {
    return(false);
   }

  #ifdef _WIN32
  if (p3d_memcaseeq(BasePath->Path.c_str(),Path.c_str(),BasePath->Path.length()))
  #else
  if (p3d_memeq(BasePath->Path.c_str(),Path.c_str(),BasePath->Path.length()))
  #endif
   {
    Path.erase(0,BasePath->Path.length());

    while ((!Path.empty()) && ((Path[0] == '/') || (Path[0] == '\\')))
     {
      Path.erase(0,1);
     }

    return(true);
   }
  else
   {
    return(false);
   }
 }

bool               P3DPathName::ToUNIX()
 {
  std::string::iterator Index;

  for (Index = Path.begin(); Index != Path.end(); Index++)
   {
    if (*Index == '\\')
     {
      *Index = '/';
     }
   }

  return(true);
 }

std::string        P3DPathName::GetExtension
                                      () const
 {
  std::string::const_reverse_iterator  Index = Path.rbegin();

  while ((Index != Path.rend()) &&
         (*Index != '\\') && (*Index != '/') && (*Index != ':'))
   {
    if (*Index == '.')
     {
      return(std::string(Index.base(),Path.end()));
     }
    else
     {
      ++Index;
     }
   }

  return(std::string(""));
 }

