from SCons.Util import *

P3DGXXSupportArgAttrUnusedSrc = """

class TestClass
 {
  public           :

  void             TestMethod         (int                 a __attribute__((unused)));
 };

int                main               (int                 argc,
                                       char               *argv[])
 {
  return(0);
 }
"""

P3DCheckEndianessSrc = """
#include <stdio.h>

typedef union
 {
  unsigned char    ByteArray[sizeof(unsigned long)];
  unsigned long    ULongValue;
 } TestUnion;

int                main               (int                 argc,
                                       char               *argv[])
 {
  TestUnion                            Value;

  Value.ULongValue = 1;

  if      (Value.ByteArray[0] == 1)
   {
    printf("little");
   }
  else if (Value.ByteArray[sizeof(unsigned long) - 1] == 1)
   {
    printf("big");
   }
  else
   {
    printf("unknown");
   }

  return(0);
 }
"""

P3DCheckLuaFuncPresenceSrc = """
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int main (int argc,char *argv[])
{
 &(%s);

 return(0);
}
"""

P3DCheckGLEWUsabilitySrc = """
#include <GL/glew.h>

int main (int argc,char *argv[])
 {
  if (glewInit() == GLEW_OK)
   {
    return(0);
   }
  else
   {
    return(1);
   }
 }
"""


def CheckGXXSupportArgAttrUnused(Context):
    Context.Message('Checking for __attribute__((unused)) support presence in g++... ')
    Result = Context.TryCompile(P3DGXXSupportArgAttrUnusedSrc,'.cpp')
    Context.Result(Result)
    return Result

def CheckTypeSizeOf(Context,TypeName):
    Context.Message("Checking %s type sizeof ... " % (TypeName))
    Src = """
#include <stdio.h>

int                main               (int                 argc,
                                       char               *argv[])
 {
  printf("%%u",(unsigned int)(sizeof(%s)));

  return(0);
 }

""" % (TypeName)
    Result = Context.TryRun(Src,".c")
    Context.Result(Result[1])
    return Result[1]

def CheckEndianess(Context):
    Context.Message("Checking endianess ... ")
    Result = Context.TryRun(P3DCheckEndianessSrc,".c")
    Context.Result(Result[1])
    return Result[1]

def CheckCXXPresence(context):
    context.Message('Checking for c++ compiler presence... ')
    cxx = context.env['CXX']
    context.Result(cxx is not None)
    return cxx

#FIXME: correct for UNIX'es only
import os
import os.path
def WhichEmul(progname):
    try:
        for dir in os.environ['PATH'].split(':'):
            fullpath = os.path.join(dir,progname)
            if os.access(fullpath,os.X_OK):
                return fullpath
    except:
        return None

    return None

# Taken from http://scons.org/wiki/AutoconfRecipes
def CheckCommand(context,cmd):
    context.Message('Checking for %s command... ' % (cmd))
    result = WhichEmul(cmd)
    context.Result(result is not None)
    return result

def GetEnvKeyOpt(Env,Key,Def = None):
    if Env.has_key(Key):
        return Env[Key]
    else:
        return Def

def CheckLuaFunc(Context,FuncName):
    Context.Message('Checking for %s function presence in Lua libraries ...' % (FuncName))

    lastLIBS    = GetEnvKeyOpt(Context.env,'LIBS')
    lastLIBPATH = GetEnvKeyOpt(Context.env,'LIBPATH')
    lastCPPPATH = GetEnvKeyOpt(Context.env,'CPPPATH')

    Context.env.Append(LIBS=Context.env['LUA_LIBS'])
    Context.env.Append(LIBPATH=Context.env['LUA_LIBPATH'])
    Context.env.Append(CPPPATH=Context.env['LUA_INC'])

    Ret = Context.TryLink(P3DCheckLuaFuncPresenceSrc % (FuncName),".c")

    Context.env.Replace(LIBS=lastLIBS)
    Context.env.Replace(LIBPATH=lastLIBPATH)
    Context.env.Replace(CPPPATH=lastCPPPATH)

    Context.Result(Ret)

    return Ret

def ConfigureGLEW(Context):
    Context.Message('Checking GLEW presence and usability ... ')

    if   not Context.env.has_key('GLEW_INC'):
        Context.env.Append(GLEW_INC=[])
    elif Context.env['GLEW_INC'] != '':
        Context.env.Replace(GLEW_INC=Split(Context.env['GLEW_INC']))
    else:
        Context.env.Replace(GLEW_INC=[])

    if   not Context.env.has_key('GLEW_LIBPATH'):
        Context.env.Append(GLEW_LIBPATH=[])
    elif Context.env['GLEW_LIBPATH'] != '':
        Context.env.Replace(GLEW_LIBPATH=Split(Context.env['GLEW_LIBPATH']))
    else:
        Context.env.Replace(GLEW_LIBPATH=[])

    if  not Context.env.has_key('GLEW_LIBS'):
        Context.env.Append(GLEW_LIBS=['GLEW'])
    elif Context.env['GLEW_LIBS'] != '':
        Context.env.Replace(GLEW_LIBS=Split(Context.env['GLEW_LIBS']))
    else:
        Context.env.Replace(GLEW_LIBS=['GLEW'])

    lastLIBS       = GetEnvKeyOpt(Context.env,'LIBS')
    lastLIBPATH    = GetEnvKeyOpt(Context.env,'LIBPATH')
    lastCPPPATH    = GetEnvKeyOpt(Context.env,'CPPPATH')
    lastCPPDEFINES = GetEnvKeyOpt(Context.env,'CPPDEFINES')

    Context.env.Append(CPPPATH=Context.env['GLEW_INC'])
    Context.env.Append(LIBPATH=Context.env['GLEW_LIBPATH'])
    Context.env.Append(LIBS=Context.env['GLEW_LIBS'])

    if Context.env["cross"]:
        Context.env.Append(CPPDEFINES=['GLEW_STATIC'])
        Context.env.Append(LIBS=['opengl32'])

    Ret = Context.TryLink(P3DCheckGLEWUsabilitySrc,".c")

    Context.env.Replace(LIBS=lastLIBS)
    Context.env.Replace(LIBPATH=lastLIBPATH)
    Context.env.Replace(CPPPATH=lastCPPPATH)
    Context.env.Replace(CPPDEFINES=lastCPPDEFINES)

    Context.Result(Ret)

    return Ret

