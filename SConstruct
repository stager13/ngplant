import os
import os.path
from sctool.SConcheck import *
from sctool.SConcompat import *

def GetPythonIncPath():
    try:
        import distutils.sysconfig

        return distutils.sysconfig.get_python_inc()
    except:
        return None

CustomConfFileName = ARGUMENTS.get('config',None)

if CustomConfFileName is not None:
    CustomConfFileName = os.path.join('config',CustomConfFileName + ".py")

opts = Variables(CustomConfFileName,ARGUMENTS)
opts.Add(BoolVariable('cross','Set to build in cross-compile mode','no'))
opts.Add('CC','The C compiler')
opts.Add('CXX','The C++ compiler')
opts.Add('RANLIB','The archive indexer')
opts.Add('AR','The static library archiver')
opts.Add('WINDRES','The resource compiler')

opts.Add('WX_INC','wxWidgets headers path(s)')
opts.Add('WX_LIB','wxWidgets libraries')
opts.Add('WX_LIBPATH','wxWidgets libraries path(s)')
opts.Add('WX_CONFIG','wx-config command','wx-config')
opts.Add('WX_LIB_STATIC','wxWidgets static libraries','')

opts.Add(BoolVariable('WITH_LUA','Enable Lua plug-ins support','yes'))
opts.Add(BoolVariable('LUA_INTERNAL','Compile Lua libraries from sources in extern/lua','no'))
opts.Add('LUA_INC','Lua headers path(s)','')
opts.Add('LUA_LIBS','Lua libraries','')
opts.Add('LUA_LIBPATH','Lua libraries path(s)','')

opts.Add('GLU_INC','GLU headers path(s)','')
opts.Add('GLU_LIBS','GLU libraries','')
opts.Add('GLU_LIBPATH','GLU libraries path(s)','')

opts.Add(BoolVariable('GLEW_INTERNAL','Compile GLEW library from sources in extern/glew','no'))
opts.Add('GLEW_INC','GLEW headers path(s)','')
opts.Add('GLEW_LIBS','GLEW libraries','')
opts.Add('GLEW_LIBPATH','GLEW libraries path(s)','')

opts.Add(BoolVariable('WITH_LIBPNG','Enable PNG support','yes'))
opts.Add('LIBPNG_INC','libpng headers path(s)','')
opts.Add('LIBPNG_LIBS','libpng library','')
opts.Add('LIBPNG_LIBPATH','libpng library path','')
opts.Add('LIBPNG_DEFINES','libpng additional preprocessor definitions','')
opts.Add('LIBPNG_CONFIG','libpng pkg-config custom command line','')

opts.Add(BoolVariable('WITH_LIBJPEG','Enable JPEG support','yes'))
opts.Add('LIBJPEG_INC','libjpeg headers path(s)','')
opts.Add('LIBJPEG_LIBS','libjpeg library','')
opts.Add('LIBJPEG_LIBPATH','libjpeg library path','')
opts.Add('LIBJPEG_DEFINES','libjpeg additional preprocessor definitions','')
opts.Add('LIBJPEG_CONFIG','libjpeg pkg-config custom command line','')

opts.Add('PLUGINS_DIR','The search path for ngplant plugins',None)

opts.Add(BoolVariable('enable_timings','Set to enable debug timings dump on ngplant','no'))
opts.Add(BoolVariable('enable_profiling','Set to pass profiling options to compiler and linker ','no'))

opts.Add('EXTRA_VERSION','ngplant version suffix',None)

BaseEnv = Environment(ENV = os.environ,options=opts)

Help(opts.GenerateHelpText(BaseEnv))

if BaseEnv["cross"]:
    CrossCompileMode = 1
else:
    CrossCompileMode = 0

if BaseEnv["enable_timings"]:
    TimingsEnabled = True
else:
    TimingsEnabled = False

if BaseEnv["enable_profiling"]:
    ProfilingEnabled = True
else:
    ProfilingEnabled = False

HavePythonDev = False
HaveGLUTDev   = False
HaveXSLTProc  = False

BaseConf = Configure(BaseEnv,
 custom_tests={ 'CheckCXXPresence' : CheckCXXPresence,
                'CheckGXXSupportArgAttrUnused' : CheckGXXSupportArgAttrUnused,
                'CheckTypeSizeOf' : CheckTypeSizeOf,
                'CheckEndianess' : CheckEndianess,
                'CheckCommand' : CheckCommand,
                'CheckLuaFunc' : CheckLuaFunc,
                'ConfigureGLU' : ConfigureGLU,
                'ConfigureGLEW' : ConfigureGLEW,
                'ConfigureLibPng' : ConfigureLibPng,
                'ConfigureLibJpeg' : ConfigureLibJpeg})

if BaseConf.CheckCXXPresence() is None:
    print 'error: c++ compiler not found.'
    sys.exit(1)

if BaseConf.CheckGXXSupportArgAttrUnused():
    BaseEnv.Append(CPPDEFINES=['HAVE_GXX_ARG_ATTR_UNUSED'])

if BaseConf.CheckCXXHeader('stdint.h'):
    BaseEnv.Append(CPPDEFINES=['HAVE_STDINT_H'])

if CrossCompileMode:
    BaseEnv["PROGSUFFIX"] = ".exe"
    UnsignedIntSizeOf    = 4
    UnsignedIntPtrSizeOf = 4
else:
    PythonIncPath = GetPythonIncPath()

    if PythonIncPath is not None:
        if BaseConf.env.has_key('CPPPATH'):
            lastCPPPATH = BaseConf.env['CPPPATH']
        else:
            lastCPPPATH = None

        BaseConf.env.Append(CPPPATH=[PythonIncPath])

        if BaseConf.CheckCHeader('Python.h'):
            HavePythonDev = True

        if lastCPPPATH is not None:
            BaseConf.env.Replace(CPPPATH=lastCPPPATH)
        else:
            del BaseConf.env['CPPPATH']

    if not HavePythonDev:
        print 'Python.h not found... _ngp (Python bindings) will not be built'

    Endianess = BaseConf.CheckEndianess()

    if Endianess == 'big':
        BaseEnv.Append(CPPDEFINES=[('P3D_BIG_ENDIAN',1)])

# It looks like CheckFunc method have been changed in 0.96... version
# To workaround this, I'm trying to detect functions presence using
# both prototypes.

if BaseConf.CheckFunc('sincosf','C++'):
    BaseEnv.Append(CPPDEFINES=['HAVE_SINCOSF'])
else:
    try:
        if BaseConf.CheckFunc('sincosf',None,'C++'):
            BaseEnv.Append(CPPDEFINES=['HAVE_SINCOSF'])
    except:
        pass

if BaseConf.CheckFunc('roundf','C++'):
    BaseEnv.Append(CPPDEFINES=['HAVE_ROUNDF'])
else:
    try:
        if BaseConf.CheckFunc('roundf',None,'C++'):
            BaseEnv.Append(CPPDEFINES=['HAVE_ROUNDF'])
    except:
        pass

BaseConf.ConfigureGLU()

if BaseConf.CheckCXXHeader('GL/glut.h'):
    HaveGLUTDev = True
else:
    print 'glut library not found - ngpview application will not be built'

GLEWInternal = BaseEnv['GLEW_INTERNAL']

if not GLEWInternal:
    if BaseConf.ConfigureGLEW():
        BaseEnv.Append(GLEXT_INC=BaseEnv['GLEW_INC'])
        BaseEnv.Append(GLEXT_LIBPATH=BaseEnv['GLEW_LIBPATH'])
        BaseEnv.Append(GLEXT_LIBS=BaseEnv['GLEW_LIBS'])
    else:
        GLEWInternal = True
        print 'No installed glew library found - using internal GLEW sources...'

if GLEWInternal:
    BaseEnv.Append(GLEXT_INC=['#/extern/glew/include'])
    BaseEnv.Append(GLEXT_LIBPATH=['#/extern/libs'])
    BaseEnv.Append(GLEXT_LIBS=['GLEW'])
    BaseEnv.Append(CPPDEFINES=[('GLEW_STATIC','1')])

if BaseConf.CheckCommand('xsltproc'):
    HaveXSLTProc = True

# Lua auto-configuration

LuaEnabled  = BaseEnv['WITH_LUA']

if LuaEnabled:
    LuaInternal = BaseEnv['LUA_INTERNAL']
else:
    LuaInternal = False

if LuaEnabled:
    if not LuaInternal:
        # Check if Lua configuration have been done in LUA_... vars
        # (at least LUA_LIBS must be set)
        if BaseEnv['LUA_LIBS'] != '':
            if BaseEnv['LUA_INC'] != '':
                BaseEnv.Replace(LUA_INC=Split(BaseEnv['LUA_INC']))
            else:
                BaseEnv.Replace(LUA_INC=[])
            if BaseEnv['LUA_LIBPATH'] != '':
                BaseEnv.Replace(LUA_LIBPATH=Split(BaseEnv['LUA_LIBPATH']))
            else:
                BaseEnv.Replace(LUA_LIBPATH=[])
            BaseEnv.Replace(LUA_LIBS=Split(BaseEnv['LUA_LIBS']))
        elif (not CrossCompileMode) and (BaseConf.CheckCommand('lua-config')):
            # Try to use lua-config (at least Debian Sarge has it)
            LuaConfEnv = EnvClone(BaseEnv)
            LuaConfEnv.ParseConfig('lua-config --include --libs')
            BaseEnv.Replace(LUA_INC=LuaConfEnv['CPPPATH'])
            BaseEnv.Replace(LUA_LIBPATH=LuaConfEnv['LIBPATH'])
            BaseEnv.Replace(LUA_LIBS=LuaConfEnv['LIBS'])
            del LuaConfEnv

            if not BaseConf.CheckLuaFunc('luaL_newmetatable'):
                print 'Lua installation seems to be broken. Using internal Lua sources...'
                LuaInternal = True
        else:
            # Fall to internal Lua
            print 'No installed Lua libraries found. Using internal Lua sources...'
            LuaInternal = True

    if LuaInternal:
        BaseEnv.Replace(LUA_INC=['#/extern/lua/src'])
        BaseEnv.Replace(LUA_LIBPATH=['#/extern/libs'])
        BaseEnv.Replace(LUA_LIBS=['ext_lua','ext_lualib'])

if LuaEnabled:
    if   LuaInternal:
        BaseEnv.Append(LUA_VERSION='51')
    elif BaseConf.CheckLuaFunc('lua_newstate'):
        BaseEnv.Append(LUA_VERSION='51')
    else:
        BaseEnv.Append(LUA_VERSION='50')

if BaseEnv['WITH_LIBPNG']:
    BaseConf.ConfigureLibPng()

if BaseEnv['WITH_LIBJPEG']:
    BaseConf.ConfigureLibJpeg()

BaseEnv = BaseConf.Finish()

if 'msvc' in BaseEnv['TOOLS']:
    BaseEnv.Append(CXXFLAGS=['/MD','/GR','/O2','/EHsc'])
    BaseEnv.Append(CCFLAGS=['/MD','/GR','/O2','/EHsc'])

if 'gcc' in BaseEnv['TOOLS']:
    CC_WARN_FLAGS='-W -Wall'
else:
    CC_WARN_FLAGS=''

if 'gcc' in BaseEnv['TOOLS']:
    CC_OPT_FLAGS='-O3 --fast-math'
else:
    CC_OPT_FLAGS=''

if ProfilingEnabled:
    if 'gcc' in BaseEnv['TOOLS']:
        BaseEnv.Append(CXXFLAGS=['-pg'])
        BaseEnv.Append(CFLAGS=['-pg'])
        BaseEnv.Append(LINKFLAGS=['-pg'])

if BaseEnv['PLATFORM'] == 'darwin':
    BaseEnv.Append(CPPPATH=['/usr/X11/include'])
    BaseEnv.Append(CFLAGS=['-mmacosx-version-min=10.5'])
    BaseEnv.Append(CXXFLAGS=['-mmacosx-version-min=10.5'])
    BaseEnv.Append(LINKFLAGS=['-mmacosx-version-min=10.5'])

Export('BaseEnv',
       'CC_WARN_FLAGS',
       'CC_OPT_FLAGS',
       'CrossCompileMode',
       'LuaEnabled',
       'TimingsEnabled',
       'ProfilingEnabled')

SubPrjList = ['ngpcore/SConscript',
              'ngput/SConscript',
              'ngplant/SConscript',
              'ngpshot/SConscript',
              'devtools/SConscript']

if HavePythonDev:
    SubPrjList.append('pywrapper/SConscript')

if HaveGLUTDev:
    SubPrjList.append('ngpview/SConscript')

if HaveXSLTProc:
    SubPrjList.append('docapi/SConscript')

if LuaInternal:
    SubPrjList.append('extern/SConscript')

if GLEWInternal:
    SubPrjList.append('extern/glew/SConscript')

SConscript(SubPrjList)

if CrossCompileMode or (BaseEnv['PLATFORM'] == 'win32') or (BaseEnv['PLATFORM'] == 'cygwin'):
    Default('ngplant/ngplant.exe')
    Clean('ngplant/ngplant.exe',['config.log','.sconf_temp','sctool/__init__.pyc',
                             'sctool/SConcheck.pyc'])
else:
    Default('ngplant/ngplant')
    Clean('ngplant/ngplant',['config.log','.sconf_temp','sctool/__init__.pyc',
                             'sctool/SConcheck.pyc',
                             'sctool/SConcompat.pyc',
                             'extern/libs/.sconsign',
                             'extern/lua/src/.sconsign'])

