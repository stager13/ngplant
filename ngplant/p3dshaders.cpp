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

#include <vector>

#include <ngput/p3dglext.h>

#include <p3dlog.h>

#include <shaders/default_vs.h>
#include <shaders/default_fs.h>

#include <p3dshaders.h>

                   P3DShaderManager::P3DShaderManager
                                      ()
 {
  ShadersEnabled = true;
 }

                   P3DShaderManager::~P3DShaderManager
                                      ()
 {
 }

#define ShaderSrcDefineCount     3
#define ShaderSrcHeaderLineCount  (ShaderSrcDefineCount + 1)

static const char ShaderSrcEmptyLine[]            = "\n";
static const char ShaderSrcDefineHaveDiffuseTex[] = "#define HAVE_DIFFUSE_TEX\n";
static const char ShaderSrcDefineHaveNormalMap[]  = "#define HAVE_NORMAL_MAP\n";
static const char ShaderSrcDefineTwoSided[]       = "#define TWO_SIDED\n";
static const char ShaderSrcLineNumberSetup[]      = "#line 1\n";

P3DShaderHandle    P3DShaderManager::GenShader
                                      (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided)
 {
  P3DShaderHandle                      Handle;
  P3DShaderManagerEntry               *Entry;

  Handle = FindByProps(HaveDiffuseTex,HaveNormalMap,TwoSided);

  if (Handle == P3DShaderHandleNULL)
   {
    Handle = GetUnusedSlot();

    Entry = &ShaderSet[Handle - 1];

    Entry->RefCount       = 0;
    Entry->ProgramReady   = false;
    Entry->ProgramHandle  = 0;
    Entry->HaveDiffuseTex = HaveDiffuseTex;
    Entry->HaveNormalMap  = HaveNormalMap;
    Entry->TwoSided       = TwoSided;
   }
  else
   {
    Entry = &ShaderSet[Handle - 1];
   }

  Entry->RefCount++;

  return(Handle);
 }

void               P3DShaderManager::FreeShader
                                      (P3DShaderHandle     ShaderHandle)
 {
  if ((ShaderHandle > ShaderSet.size()) || (ShaderHandle < 1))
   {
    return; /* FIXME: throw something here */
   }

  P3DShaderManagerEntry *Entry = &ShaderSet[ShaderHandle - 1];

  if (Entry->RefCount == 0)
   {
    return; /* FIXME: throw something here */
   }

  Entry->RefCount--;

  if (Entry->RefCount == 0)
   {
    if (Entry->ProgramHandle != 0)
     {
      glDeleteObjectARB(Entry->ProgramHandle);
     }
   }
 }

static wxString    GetInfoLog         (GLhandleARB         Handle)
 {
  wxString         Result;
  GLint            LogSize;
  GLcharARB       *Log;

  glGetObjectParameterivARB(Handle,GL_OBJECT_INFO_LOG_LENGTH_ARB,&LogSize);

  if (LogSize > 1)
   {
    Log = (GLcharARB*)malloc(LogSize);

    if (Log != NULL)
     {
      glGetInfoLogARB(Handle,LogSize,NULL,Log);

      Result = wxString(Log,wxConvUTF8);

      free(Log);
     }
    else
     {
      P3DLogError(wxT("unable to get shader information log (out of memory)\n"));
     }
   }

  return(Result);
 }

static bool        CompileShaderObject(GLhandleARB        *ShaderHandle,
                                       GLenum              ShaderType,
                                       GLsizei             SrcStringsCount,
                                       const GLcharARB   **SrcStrings)
 {
  bool             Result;

  *ShaderHandle = glCreateShaderObjectARB(ShaderType);

  if (*ShaderHandle != 0)
   {
    GLint          CompileStatus;
    wxString       LogStr;

    P3DLogInfo(wxT("Compiling shader...\n"));

    glShaderSourceARB(*ShaderHandle,SrcStringsCount,SrcStrings,NULL);
    glCompileShaderARB(*ShaderHandle);
    glGetObjectParameterivARB( *ShaderHandle,
                               GL_OBJECT_COMPILE_STATUS_ARB,
                              &CompileStatus);

    Result = CompileStatus != 0;

    LogStr = GetInfoLog(*ShaderHandle);

    if (Result)
     {
      if (!LogStr.IsEmpty())
       {
        P3DLogInfo(wxT("Compilation log:\n"));
        P3DLogInfo(LogStr);
       }
     }
    else
     {
      if (!LogStr.IsEmpty())
       {
        P3DLogError(wxString(wxT("Shader compilation failed. ")) + LogStr);
       }
      else
       {
        P3DLogError(wxT("Shader compilation failed. Log is not available"));
       }

      glDeleteObjectARB(*ShaderHandle);

      *ShaderHandle = 0;
     }
   }
  else
   {
    Result = false;

    P3DLogError(wxT("unable to create GLSL shader object"));
   }

  return(Result);
 }

static void        InitShaderHeader   (const GLcharARB    *Strings[],
                                       const P3DShaderManagerEntry
                                                          *Entry)
 {
  Strings[0] = Entry->HaveDiffuseTex ? ShaderSrcDefineHaveDiffuseTex :
                                       ShaderSrcEmptyLine;
  Strings[1] = Entry->HaveNormalMap  ? ShaderSrcDefineHaveNormalMap  :
                                       ShaderSrcEmptyLine;
  Strings[2] = Entry->TwoSided ? ShaderSrcDefineTwoSided :
                                 ShaderSrcEmptyLine;
  Strings[3] = ShaderSrcLineNumberSetup;
 }

GLhandleARB        P3DShaderManager::GetProgramHandle
                                      (P3DShaderHandle     ShaderHandle) const
 {
  if ((ShaderHandle < 1) || (ShaderHandle > ShaderSet.size()))
   {
    return(0);
   }

  P3DShaderManagerEntry *Entry = &ShaderSet[ShaderHandle - 1];

  if (Entry->RefCount > 0)
   {
    if (!Entry->ProgramReady)
     {
      GLhandleARB    ProgHandle;

      Entry->ProgramReady  = true;
      Entry->ProgramHandle = 0;

      if (ShadersEnabled)
       {
        GLhandleARB    ShaderHandle;
        const
        GLcharARB     *SourceStrings[ShaderSrcHeaderLineCount + 1];
        GLint          LinkStatus;
        wxString       LinkLogStr;
        bool           ShaderOk;

        ProgHandle = glCreateProgramObjectARB();

        if (ProgHandle != 0)
         {
          InitShaderHeader(SourceStrings,Entry);

          SourceStrings[ShaderSrcHeaderLineCount] = P3DDefaultVertexShaderSrc;

          ShaderOk = CompileShaderObject(&ShaderHandle,GL_VERTEX_SHADER_ARB,ShaderSrcHeaderLineCount + 1,SourceStrings);

          if (ShaderOk)
           {
            glAttachObjectARB(ProgHandle,ShaderHandle);
            glDeleteObjectARB(ShaderHandle);

            SourceStrings[ShaderSrcHeaderLineCount] = P3DDefaultFragmentShaderSrc;

            ShaderOk = CompileShaderObject(&ShaderHandle,GL_FRAGMENT_SHADER_ARB,ShaderSrcHeaderLineCount + 1,SourceStrings);
           }

          if (ShaderOk)
           {
            glAttachObjectARB(ProgHandle,ShaderHandle);
            glDeleteObjectARB(ShaderHandle);

            glLinkProgramARB(ProgHandle);

            glGetObjectParameterivARB( ProgHandle,
                                       GL_OBJECT_LINK_STATUS_ARB,
                                      &LinkStatus);

            LinkLogStr = GetInfoLog(ProgHandle);

            if (LinkStatus)
             {
              if (!LinkLogStr.IsEmpty())
               {
                P3DLogInfo(wxT("Link log:\n"));
                P3DLogInfo(LinkLogStr);
               }

              GLhandleARB    CurrProgHandle;

              CurrProgHandle = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);

              glUseProgramObjectARB(ProgHandle);

              GLint                    Location;

              Location = glGetUniformLocationARB(ProgHandle,"DiffuseTexSampler");

              if (Location != -1)
               {
                glUniform1iARB(Location,0);
               }

              Location = glGetUniformLocationARB(ProgHandle,"NormalMapSampler");

              if (Location != -1)
               {
                glUniform1iARB(Location,1);
               }

              glUseProgramObjectARB(CurrProgHandle);
             }
            else
             {
              if (!LinkLogStr.IsEmpty())
               {
                P3DLogError(wxString(wxT("Shader linkage failed. ")) + LinkLogStr);
               }
              else
               {
                P3DLogError(wxT("Shader linkage failed. Log is not available"));
               }

              glDeleteObjectARB(ProgHandle);

              ProgHandle = 0;
             }
           }
          else
           {
            glDeleteObjectARB(ProgHandle);

            ProgHandle = 0;
           }
         }
        else
         {
          P3DLogError(wxT("unable to create GLSL program object"));
         }
       }
      else
       {
        ProgHandle = 0;
       }

      Entry->ProgramHandle = ProgHandle;
     }

    return(Entry->ProgramHandle);
   }
  else
   {
    return(0);
   }
 }

P3DShaderHandle    P3DShaderManager::GetUnusedSlot
                                      ()
 {
  P3DShaderHandle                     Handle;

  for (Handle = 0; Handle < ShaderSet.size(); Handle++)
   {
    if (ShaderSet[Handle].RefCount == 0)
     {
      return(Handle + 1);
     }
   }

  P3DShaderManagerEntry                NewEntry;

  NewEntry.RefCount      = 0;
  NewEntry.ProgramReady  = false;
  NewEntry.ProgramHandle = 0;

  ShaderSet.push_back(NewEntry);

  return(ShaderSet.size());
 }

P3DShaderHandle    P3DShaderManager::FindByProps
                                      (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const
 {
  P3DShaderHandle                      Handle;

  for (Handle = 0; Handle < ShaderSet.size(); Handle++)
   {
    if (ShaderSet[Handle].RefCount > 0)
     {
      if ((ShaderSet[Handle].HaveDiffuseTex == HaveDiffuseTex) &&
          (ShaderSet[Handle].HaveNormalMap  == HaveNormalMap)  &&
          (ShaderSet[Handle].TwoSided       == TwoSided))
       {
        return(Handle + 1);
       }
     }
   }

  return(P3DShaderHandleNULL);
 }

void               P3DShaderManager::DisableShaders
                                      ()
 {
  ShadersEnabled = false;
 }


