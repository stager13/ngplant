from types import *

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

P3DCheckGLUUsabilitySrc = """
#include <GL/glu.h>

int main (int argc,char *argv[])
 {
  return(gluErrorString(GLU_OUT_OF_MEMORY) != NULL);
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

def GetEnvKeyList(Env,Key):
    try:
        if Env[Key] is not None:
            return Env[Key]
    except:
        pass

    return []

def EnvKeyHasValue(Env,Key):
    try:
        Value = Env[Key]
    except:
        return False

    if Value:
        return True

    return False

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

def ConfigureGLU(Context):
    Context.Message('Checking GLU presence and usability ... ')

    if   not Context.env.has_key('GLU_INC'):
        Context.env.Append(GLU_INC=[])
    elif Context.env['GLU_INC'] != '':
        Context.env.Replace(GLU_INC=Split(Context.env['GLU_INC']))
    else:
        Context.env.Replace(GLU_INC=[])

    if   not Context.env.has_key('GLU_LIBPATH'):
        Context.env.Append(GLU_LIBPATH=[])
    elif Context.env['GLU_LIBPATH'] != '':
        Context.env.Replace(GLU_LIBPATH=Split(Context.env['GLU_LIBPATH']))
    else:
        Context.env.Replace(GLU_LIBPATH=[])

    if Context.env["PLATFORM"] == 'posix' and not Context.env["cross"]:
        DefaultGLULibs=['GLU']
    else:
        DefaultGLULibs=[]

    if  not Context.env.has_key('GLU_LIBS'):
        Context.env.Append(GLU_LIBS=DefaultGLULibs)
    elif Context.env['GLU_LIBS'] != '':
        Context.env.Replace(GLU_LIBS=Split(Context.env['GLU_LIBS']))
    else:
        Context.env.Replace(GLU_LIBS=DefaultGLULibs)

    lastLIBS    = GetEnvKeyOpt(Context.env,'LIBS')
    lastLIBPATH = GetEnvKeyOpt(Context.env,'LIBPATH')
    lastCPPPATH = GetEnvKeyOpt(Context.env,'CPPPATH')

    Context.env.Append(CPPPATH=Context.env['GLU_INC'])
    Context.env.Append(LIBPATH=Context.env['GLU_LIBPATH'])
    Context.env.Append(LIBS=Context.env['GLU_LIBS'])

    Ret = Context.TryLink(P3DCheckGLUUsabilitySrc,".c")

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

def SubtractLists(a,b):
    return filter(lambda i : i not in b,a)

P3DCheckLibPngUsabilitySrc = """
#include <png.h>

int main (int argc,char *argv[])
 {
  png_sig_cmp(0,0,4);

  return(0);
 }
"""

def EnvGetValAsList(Env,Key):
    try:
        v = Env[Key]

        if type(v) is StringType:
            v = Split(v)
    except:
        v = []

    return v

def ConfigureLibPng(Context):
    Context.Message('Checking libpng presence and usability ... ')

    lastLIBS       = GetEnvKeyList(Context.env,'LIBS')
    lastLIBPATH    = GetEnvKeyList(Context.env,'LIBPATH')
    lastCPPPATH    = GetEnvKeyList(Context.env,'CPPPATH')
    lastCPPDEFINES = GetEnvKeyList(Context.env,'CPPDEFINES')

    if EnvKeyHasValue(Context.env,'LIBPNG_INC')     or\
       EnvKeyHasValue(Context.env,'LIBPNG_DEFINES') or\
       EnvKeyHasValue(Context.env,'LIBPNG_LIBPATH') or\
       EnvKeyHasValue(Context.env,'LIBPNG_LIBS'):
        LIBPNG_INC     = EnvGetValAsList(Context.env,'LIBPNG_INC')
        LIBPNG_CPPDEFS = EnvGetValAsList(Context.env,'LIBPNG_DEFINES')
        LIBPNG_LIBPATH = EnvGetValAsList(Context.env,'LIBPNG_LIBPATH')
        LIBPNG_LIBS    = EnvGetValAsList(Context.env,'LIBPNG_LIBS')

        Context.env.Append(CPPPATH=LIBPNG_INC)
        Context.env.Append(CPPDEFINES=LIBPNG_CPPDEFS)
        Context.env.Append(LIBPATH=LIBPNG_LIBPATH)
        Context.env.Append(LIBS=LIBPNG_LIBS)
    else:
        try:
            if EnvKeyHasValue(Context.env,'LIBPNG_CONFIG'):
                Context.env.ParseConfig(Context.env['LIBPNG_CONFIG'])
            else:
                Context.env.ParseConfig('pkg-config libpng --cflags --libs')

            LIBPNG_INC = SubtractLists(GetEnvKeyList(Context.env,'CPPPATH'),lastCPPPATH)
            LIBPNG_CPPDEFS = SubtractLists(GetEnvKeyList(Context.env,'CPPDEFINES'),lastCPPDEFINES)
            LIBPNG_LIBS = SubtractLists(GetEnvKeyList(Context.env,'LIBS'),lastLIBS)
            LIBPNG_LIBPATH = SubtractLists(GetEnvKeyList(Context.env,'LIBPATH'),lastLIBPATH)
        except:
            LIBPNG_INC     = []
            LIBPNG_CPPDEFS = []
            LIBPNG_LIBS    = []
            LIBPNG_LIBPATH = []

    Ret = Context.TryLink(P3DCheckLibPngUsabilitySrc,".c")

    Context.Result(Ret)

    Context.env.Replace(LIBS=lastLIBS)
    Context.env.Replace(LIBPATH=lastLIBPATH)
    Context.env.Replace(CPPPATH=lastCPPPATH)
    Context.env.Replace(CPPDEFINES=lastCPPDEFINES)

    if Ret:
        Context.env.Append(HAVE_LIBPNG=True)
        Context.env.Replace(WITH_LIBPNG=True)
        Context.env['LIBPNG_CPPPATH'] = LIBPNG_INC
        Context.env['LIBPNG_CPPDEFINES'] = LIBPNG_CPPDEFS
        Context.env['LIBPNG_LIBS'] = LIBPNG_LIBS
        Context.env['LIBPNG_LIBPATH'] = LIBPNG_LIBPATH
    else:
        Context.env.Append(HAVE_LIBPNG=False)
        Context.env.Replace(WITH_LIBPNG=False)

    return Ret

def AppendLibPngConf(Env):
    if Env['WITH_LIBPNG']:
        Env.Append(CPPPATH=Env['LIBPNG_CPPPATH'])
        Env.Append(CPPDEFINES=Env['LIBPNG_CPPDEFINES'])
        Env.Append(LIBS=Env['LIBPNG_LIBS'])
        Env.Append(LIBPATH=Env['LIBPNG_LIBPATH'])
        Env.Append(CPPDEFINES=[('WITH_LIBPNG',1)])

#FIXME: Very similar to libpng's one - need to investigate further and create
# unified libraries configuration framework

P3DCheckLibJpegUsabilitySrc = """
#include <stdio.h>
#include <jpeglib.h>

int main (int argc,char *argv[])
 {
  struct jpeg_decompress_struct        cinfo;

  jpeg_create_decompress(&cinfo);

  return(0);
 }
"""

def ConfigureLibJpeg(Context):
    Context.Message('Checking libjpeg presence and usability ... ')

    lastLIBS       = GetEnvKeyList(Context.env,'LIBS')
    lastLIBPATH    = GetEnvKeyList(Context.env,'LIBPATH')
    lastCPPPATH    = GetEnvKeyList(Context.env,'CPPPATH')
    lastCPPDEFINES = GetEnvKeyList(Context.env,'CPPDEFINES')

    if EnvKeyHasValue(Context.env,'LIBJPEG_INC')     or\
       EnvKeyHasValue(Context.env,'LIBJPEG_DEFINES') or\
       EnvKeyHasValue(Context.env,'LIBJPEG_LIBPATH') or\
       EnvKeyHasValue(Context.env,'LIBJPEG_LIBS'):
        LIBJPEG_INC     = EnvGetValAsList(Context.env,'LIBJPEG_INC')
        LIBJPEG_CPPDEFS = EnvGetValAsList(Context.env,'LIBJPEG_DEFINES')
        LIBJPEG_LIBPATH = EnvGetValAsList(Context.env,'LIBJPEG_LIBPATH')
        LIBJPEG_LIBS    = EnvGetValAsList(Context.env,'LIBJPEG_LIBS')

        Context.env.Append(CPPPATH=LIBJPEG_INC)
        Context.env.Append(CPPDEFINES=LIBJPEG_CPPDEFS)
        Context.env.Append(LIBPATH=LIBJPEG_LIBPATH)
        Context.env.Append(LIBS=LIBJPEG_LIBS)
    else:
        try:
            if EnvKeyHasValue(Context.env,'LIBJPEG_CONFIG'):
                Context.env.ParseConfig(Context.env['LIBJPEG_CONFIG'])
            else:
                Context.env.ParseConfig('pkg-config libjpeg --cflags --libs 2>/dev/null')

            LIBJPEG_INC = SubtractLists(GetEnvKeyList(Context.env,'CPPPATH'),lastCPPPATH)
            LIBJPEG_CPPDEFS = SubtractLists(GetEnvKeyList(Context.env,'CPPDEFINES'),lastCPPDEFINES)
            LIBJPEG_LIBS = SubtractLists(GetEnvKeyList(Context.env,'LIBS'),lastLIBS)
            LIBJPEG_LIBPATH = SubtractLists(GetEnvKeyList(Context.env,'LIBPATH'),lastLIBPATH)
        except:
            LIBJPEG_INC     = []
            LIBJPEG_CPPDEFS = []
            LIBJPEG_LIBS    = ['jpeg']
            LIBJPEG_LIBPATH = []

            Context.env.Append(LIBS=LIBJPEG_LIBS)

    Ret = Context.TryLink(P3DCheckLibJpegUsabilitySrc,".c")

    Context.Result(Ret)

    if Ret:
        Context.Message('Checking if extern "C" required for libjpeg... ')

        if Context.TryLink(P3DCheckLibJpegUsabilitySrc,".cpp"):
            Context.Result('no')
        else:
            Context.Result('yes')

            LIBJPEG_CPPDEFS.append(('LIBJPEG_NO_EXTERN_C',1))

    Context.env.Replace(LIBS=lastLIBS)
    Context.env.Replace(LIBPATH=lastLIBPATH)
    Context.env.Replace(CPPPATH=lastCPPPATH)
    Context.env.Replace(CPPDEFINES=lastCPPDEFINES)

    if Ret:
        Context.env.Append(HAVE_LIBJPEG=True)
        Context.env.Replace(WITH_LIBJPEG=True)
        Context.env['LIBJPEG_CPPPATH'] = LIBJPEG_INC
        Context.env['LIBJPEG_CPPDEFINES'] = LIBJPEG_CPPDEFS
        Context.env['LIBJPEG_LIBS'] = LIBJPEG_LIBS
        Context.env['LIBJPEG_LIBPATH'] = LIBJPEG_LIBPATH
    else:
        Context.env.Append(HAVE_LIBJPEG=False)
        Context.env.Replace(WITH_LIBJPEG=False)

    return Ret

def AppendLibJpegConf(Env):
    if Env['WITH_LIBJPEG']:
        Env.Append(CPPPATH=Env['LIBJPEG_CPPPATH'])
        Env.Append(CPPDEFINES=Env['LIBJPEG_CPPDEFINES'])
        Env.Append(LIBS=Env['LIBJPEG_LIBS'])
        Env.Append(LIBPATH=Env['LIBJPEG_LIBPATH'])
        Env.Append(CPPDEFINES=[('WITH_LIBJPEG',1)])

