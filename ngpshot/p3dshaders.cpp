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
#include <cstdio>
#include <cstdlib>

#include <ngput/p3dglext.h>

#include <p3dshaders.h>

                   P3DShaderLoader::P3DShaderLoader
                                      (const char         *VertexProgramSrc,
                                       const char         *FragmentProgramSrc)
 {
  this->VertexProgramSrc   = VertexProgramSrc;
  this->FragmentProgramSrc = FragmentProgramSrc;
 }

                   P3DShaderLoader::~P3DShaderLoader
                                      ()
 {
  for (std::vector<P3DShaderEntry>::iterator Iter = ShaderSet.begin();
       Iter != ShaderSet.end();
       ++Iter)
   {
    if (Iter->ProgramHandle != 0)
     {
      #ifdef USE_OPENGL_20
      glDeleteProgram(Iter->ProgramHandle);
      #else
      glDeleteObjectARB(Iter->ProgramHandle);
      #endif
     }
   }
 }

#define ShaderSrcDefineCount     3
#define ShaderSrcHeaderLineCount  (ShaderSrcDefineCount + 1)

static const char ShaderSrcEmptyLine[]            = "\n";
static const char ShaderSrcDefineHaveDiffuseTex[] = "#define HAVE_DIFFUSE_TEX\n";
static const char ShaderSrcDefineHaveNormalMap[]  = "#define HAVE_NORMAL_MAP\n";
static const char ShaderSrcDefineTwoSided[]       = "#define TWO_SIDED\n";
static const char ShaderSrcLineNumberSetup[]      = "#line 1\n";

static void        DumpInfoLog        (GLhandleARB         Handle)
 {
  GLint            LogSize;
  GLcharARB       *Log;

  #ifdef USE_OPENGL_20
  if (glIsProgram(Handle))
   {
    glGetProgramiv(Handle,GL_INFO_LOG_LENGTH,&LogSize);
   }
  else
   {
    glGetShaderiv(Handle,GL_INFO_LOG_LENGTH,&LogSize);
   }
  #else
  glGetObjectParameterivARB(Handle,GL_OBJECT_INFO_LOG_LENGTH_ARB,&LogSize);
  #endif

  if (LogSize > 1)
   {
    Log = (GLcharARB*)malloc(LogSize);

    if (Log != NULL)
     {
      #ifdef USE_OPENGL_20
      if (glIsProgram(Handle))
       {
        glGetProgramInfoLog(Handle,LogSize,NULL,Log);
       }
      else
       {
        glGetShaderInfoLog(Handle,LogSize,NULL,Log);
       }
      #else
      glGetInfoLogARB(Handle,LogSize,NULL,Log);
      #endif

      fprintf(stderr,"compilation log:\n");
      fprintf(stderr,"%s\n",Log);

      free(Log);
     }
    else
     {
      fprintf(stderr,"error: unable to get shader information log (out of memory)\n");
     }
   }
 }

static bool        CompileShaderObject(GLhandleARB        *ShaderHandle,
                                       GLenum              ShaderType,
                                       GLsizei             SrcStringsCount,
                                       const GLcharARB   **SrcStrings)
 {
  bool             Result;

  #ifdef USE_OPENGL_20
  *ShaderHandle = glCreateShader(ShaderType);
  #else
  *ShaderHandle = glCreateShaderObjectARB(ShaderType);
  #endif

  if (*ShaderHandle != 0)
   {
    GLint          CompileStatus;

    #ifdef USE_OPENGL_20
    glShaderSource(*ShaderHandle,SrcStringsCount,SrcStrings,NULL);
    glCompileShader(*ShaderHandle);
    glGetShaderiv(*ShaderHandle,GL_COMPILE_STATUS,&CompileStatus);
    #else
    glShaderSourceARB(*ShaderHandle,SrcStringsCount,SrcStrings,NULL);
    glCompileShaderARB(*ShaderHandle);
    glGetObjectParameterivARB( *ShaderHandle,
                               GL_OBJECT_COMPILE_STATUS_ARB,
                              &CompileStatus);
    #endif

    Result = CompileStatus;

    DumpInfoLog(*ShaderHandle);

    if (!Result)
     {
      #ifdef USE_OPENGL_20
      glDeleteShader(*ShaderHandle);
      #else
      glDeleteObjectARB(*ShaderHandle);
      #endif

      *ShaderHandle = 0;
     }
   }
  else
   {
    Result = false;

    fprintf(stderr,"error: unable to create GLSL shader object\n");
   }

  return(Result);
 }

GLhandleARB        P3DShaderLoader::GetProgramHandle
                                      (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const
 {
  GLhandleARB                          ProgHandle;
  GLhandleARB                          ShaderHandle;
  const GLcharARB                     *SourceStrings[ShaderSrcHeaderLineCount + 1];
  GLint                                LinkStatus;
  bool                                 ShaderOk;

  ProgHandle = FindByProps(HaveDiffuseTex,HaveNormalMap,TwoSided);

  if (ProgHandle != 0)
   {
    return(ProgHandle);
   }

  #ifdef USE_OPENGL_20
  ProgHandle = glCreateProgram();
  #else
  ProgHandle = glCreateProgramObjectARB();
  #endif

  if (ProgHandle != 0)
   {
    SourceStrings[0] = HaveDiffuseTex ? ShaderSrcDefineHaveDiffuseTex :
                                        ShaderSrcEmptyLine;
    SourceStrings[1] = HaveNormalMap  ? ShaderSrcDefineHaveNormalMap  :
                                        ShaderSrcEmptyLine;
    SourceStrings[2] = TwoSided ? ShaderSrcDefineTwoSided :
                                  ShaderSrcEmptyLine;
    SourceStrings[3] = ShaderSrcLineNumberSetup;

    ShaderOk = true;

    if (VertexProgramSrc != 0)
     {
      SourceStrings[ShaderSrcHeaderLineCount] = VertexProgramSrc;

      #ifdef USE_OPENGL_20
      ShaderOk = CompileShaderObject(&ShaderHandle,GL_VERTEX_SHADER,ShaderSrcHeaderLineCount + 1,SourceStrings);
      #else
      ShaderOk = CompileShaderObject(&ShaderHandle,GL_VERTEX_SHADER_ARB,ShaderSrcHeaderLineCount + 1,SourceStrings);
      #endif

      if (ShaderOk)
       {
        #ifdef USE_OPENGL_20
        glAttachShader(ProgHandle,ShaderHandle);
        glDeleteShader(ShaderHandle);
        #else
        glAttachObjectARB(ProgHandle,ShaderHandle);
        glDeleteObjectARB(ShaderHandle);
        #endif
       }
     }

    if (ShaderOk)
     {
      if (FragmentProgramSrc != 0)
       {
        SourceStrings[ShaderSrcHeaderLineCount] = FragmentProgramSrc;

        #ifdef USE_OPENGL_20
        ShaderOk = CompileShaderObject(&ShaderHandle,GL_FRAGMENT_SHADER,ShaderSrcHeaderLineCount + 1,SourceStrings);
        #else
        ShaderOk = CompileShaderObject(&ShaderHandle,GL_FRAGMENT_SHADER_ARB,ShaderSrcHeaderLineCount + 1,SourceStrings);
        #endif

        if (ShaderOk)
         {
          #ifdef USE_OPENGL_20
          glAttachShader(ProgHandle,ShaderHandle);
          glDeleteShader(ShaderHandle);
          #else
          glAttachObjectARB(ProgHandle,ShaderHandle);
          glDeleteObjectARB(ShaderHandle);
          #endif
         }
       }
     }

    if (ShaderOk)
     {
      #ifdef USE_OPENGL_20
      glLinkProgram(ProgHandle);
      #else
      glLinkProgramARB(ProgHandle);
      #endif

      #ifdef USE_OPENGL_20
      glGetProgramiv( ProgHandle,GL_LINK_STATUS,&LinkStatus);
      #else
      glGetObjectParameterivARB( ProgHandle,
                                 GL_OBJECT_LINK_STATUS_ARB,
                                &LinkStatus);
      #endif

      DumpInfoLog(ProgHandle);

      if (LinkStatus)
       {
        GLhandleARB    CurrProgHandle;

        #ifdef USE_OPENGL_20
        glGetIntegerv(GL_CURRENT_PROGRAM,(GLint*)&CurrProgHandle);
        #else
        CurrProgHandle = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
        #endif

        #ifdef USE_OPENGL_20
        glUseProgram(ProgHandle);
        #else
        glUseProgramObjectARB(ProgHandle);
        #endif

        GLint                    Location;

        #ifdef USE_OPENGL_20
        Location = glGetUniformLocation(ProgHandle,"DiffuseTexSampler");
        #else
        Location = glGetUniformLocationARB(ProgHandle,"DiffuseTexSampler");
        #endif

        if (Location != -1)
         {
          #ifdef USE_OPENGL_20
          glUniform1i(Location,0);
          #else
          glUniform1iARB(Location,0);
          #endif
         }

        #ifdef USE_OPENGL_20
        Location = glGetUniformLocation(ProgHandle,"NormalMapSampler");
        #else
        Location = glGetUniformLocationARB(ProgHandle,"NormalMapSampler");
        #endif

        if (Location != -1)
         {
          #ifdef USE_OPENGL_20
          glUniform1i(Location,1);
          #else
          glUniform1iARB(Location,1);
          #endif
         }

        #ifdef USE_OPENGL_20
        glUseProgram(CurrProgHandle);
        #else
        glUseProgramObjectARB(CurrProgHandle);
        #endif
       }
      else
       {
        fprintf(stderr,"error: shader linkage failed\n");

        #ifdef USE_OPENGL_20
        glDeleteProgram(ProgHandle);
        #else
        glDeleteObjectARB(ProgHandle);
        #endif

        ProgHandle = 0;
       }
     }
    else
     {
      #ifdef USE_OPENGL_20
      glDeleteProgram(ProgHandle);
      #else
      glDeleteObjectARB(ProgHandle);
      #endif

      ProgHandle = 0;
     }
   }
  else
   {
    fprintf(stderr,"error: unable to create GLSL program object\n");
   }

  if (ProgHandle != 0)
   {
    P3DShaderEntry Entry;

    Entry.ProgramHandle  = ProgHandle;
    Entry.HaveDiffuseTex = HaveDiffuseTex;
    Entry.HaveNormalMap  = HaveNormalMap;
    Entry.TwoSided       = TwoSided;

    ShaderSet.push_back(Entry);
   }

  return(ProgHandle);
 }

GLhandleARB        P3DShaderLoader::FindByProps
                                      (bool                HaveDiffuseTex,
                                       bool                HaveNormalMap,
                                       bool                TwoSided) const
 {

  for (std::vector<P3DShaderEntry>::iterator Iter = ShaderSet.begin();
       Iter != ShaderSet.end();
       ++Iter)
   {
    if ((Iter->HaveDiffuseTex == HaveDiffuseTex) &&
        (Iter->HaveNormalMap  == HaveNormalMap)  &&
        (Iter->TwoSided       == TwoSided))
     {
      return(Iter->ProgramHandle);
     }
   }

  return(0);
 }

