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

# print('Reading variables from custom configuration file:',CustomConfFileName)

opts = Variables(CustomConfFileName,ARGUMENTS)

opts.Add(BoolVariable('cross',help='Set to build in cross-compile mode',default='no'))
opts.Add('CC',help='The C compiler')
opts.Add('CXX',help='The C++ compiler')
opts.Add('RANLIB',help='The archive indexer')
opts.Add('AR',help='The static library archiver')
opts.Add('WINDRES',help='The resource compiler')

opts.Add('WX_INC',help='wxWidgets headers path(s)')
opts.Add('WX_LIB',help='wxWidgets libraries')
opts.Add('WX_LIBPATH',help='wxWidgets libraries path(s)')
opts.Add('WX_CONFIG',help='wx-config command',default='wx-config')
opts.Add('WX_LIB_STATIC',help='wxWidgets static libraries',default='')

opts.Add(BoolVariable('WITH_LUA',help='Enable Lua plug-ins support',default='yes'))
opts.Add(BoolVariable('LUA_INTERNAL',help='Compile Lua libraries from sources in extern/lua',default='no'))
opts.Add('LUA_INC',help='Lua headers path(s)',default='')
opts.Add('LUA_LIBS',help='Lua libraries',default='')
opts.Add('LUA_LIBPATH',help='Lua libraries path(s)',default='')

opts.Add('GLU_INC',help='GLU headers path(s)',default='')
opts.Add('GLU_LIBS',help='GLU libraries',default='')
opts.Add('GLU_LIBPATH',help='GLU libraries path(s)',default='')

opts.Add(BoolVariable('GLEW_INTERNAL',help='Compile GLEW library from sources in extern/glew',default='no'))
opts.Add('GLEW_INC',help='GLEW headers path(s)',default='')
opts.Add('GLEW_LIBS',help='GLEW libraries',default='')
opts.Add('GLEW_LIBPATH',help='GLEW libraries path(s)',default='')

opts.Add(BoolVariable('WITH_LIBPNG',help='Enable PNG support',default='yes'))
opts.Add('LIBPNG_INC',help='libpng headers path(s)',default='')
opts.Add('LIBPNG_LIBS',help='libpng library',default='')
opts.Add('LIBPNG_LIBPATH',help='libpng library path',default='')
opts.Add('LIBPNG_DEFINES',help='libpng additional preprocessor definitions',default='')
opts.Add('LIBPNG_CONFIG',help='libpng pkg-config custom command line',default='')

opts.Add(BoolVariable('WITH_LIBJPEG',help='Enable JPEG support',default='yes'))
opts.Add('LIBJPEG_INC',help='libjpeg headers path(s)',default='')
opts.Add('LIBJPEG_LIBS',help='libjpeg library',default='')
opts.Add('LIBJPEG_LIBPATH',help='libjpeg library path',default='')
opts.Add('LIBJPEG_DEFINES',help='libjpeg additional preprocessor definitions',default='')
opts.Add('LIBJPEG_CONFIG',help='libjpeg pkg-config custom command line',default='')

opts.Add('PLUGINS_DIR',help='The search path for ngplant plugins',default=None)

opts.Add(BoolVariable('enable_timings',help='Set to enable debug timings dump on ngplant',default='no'))
opts.Add(BoolVariable('enable_profiling',help='Set to pass profiling options to compiler and linker ',default='no'))

opts.Add('EXTRA_VERSION',help='ngplant version suffix',default=None)

# Scons sets TARGET_ARCH depending on which version of Windows you're running,
# in order to force x86 (not x64) arch we create dummy environment and check
# if we're using msvc, in this case we create BaseEnv with TARGET_ARCH='x86'
# option. Unfortunately we cannot change TARGET_ARCH after constructor has
# been called, so have to use two different constructor calls...

DummyEnv = Environment(ENV = os.environ,options=opts)

if 'msvc' in DummyEnv['TOOLS']:
    BaseEnv = Environment(ENV = os.environ,options=opts,TARGET_ARCH='x86')
else:
    BaseEnv = Environment(ENV = os.environ,options=opts)

Help(opts.GenerateHelpText(BaseEnv))

# Testing to make sure that all variables we got so far are known
# (gwyneth 20210610)
#unknown = opts.UnknownVariables()
#if unknown:
#    print("Unknown variables: %s" % " ".join(unknown.keys()))

# Temporarily check environment
#if not BaseEnv:
#    print('WTF?! BaseEnv MUST not be empty!')
#else:
#    print(BaseEnv.Dump())

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

# Before configuring everything, test if we still have our settings for PNG and JPEG
# (gwyneth 20210610)
PNGEnabled = BaseEnv['WITH_LIBPNG']
JPEGEnabled = BaseEnv['WITH_LIBJPEG']
#if PNGEnabled:
#    print('PNG enabled before Configure')
#else:
#    print('PNG not enabled before Configure')
#if JPEGEnabled:
#    print('JPEG enabled before Configure')
#else:
#    print('JPEG not enabled before Configure')

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
    print ('error: c++ compiler not found.')
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
        print ('Python.h not found... _ngp (Python bindings) will not be built')

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
    print ('glut library not found - ngpview application will not be built')

GLEWInternal = BaseEnv['GLEW_INTERNAL']
#print('GLEW_INTERNAL:',GLEWInternal)

if not GLEWInternal:
    if BaseConf.ConfigureGLEW():
        BaseEnv.Append(GLEXT_INC=BaseEnv['GLEW_INC'])
        BaseEnv.Append(GLEXT_LIBPATH=BaseEnv['GLEW_LIBPATH'])
        BaseEnv.Append(GLEXT_LIBS=BaseEnv['GLEW_LIBS'])
        # it works up to this point but then GLEXT_* gets 'forgotten' by BaseEnv and so
        # everything will fail...
        print('GLEW_INC:',BaseEnv['GLEW_INC'],'GLEXT_INC:',BaseEnv['GLEXT_INC'])
    else:
        GLEWInternal = True
        print ('No installed glew library found - using internal GLEW sources...')

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
                print ('Lua installation seems to be broken. Using internal Lua sources...')
                LuaInternal = True
        else:
            # Fall to internal Lua
            print ('No installed Lua libraries found. Using internal Lua sources...')
            LuaInternal = True

    if LuaInternal:
        BaseEnv.Replace(LUA_INC=['#/extern/lua/src'])
        BaseEnv.Replace(LUA_LIBPATH=['#/extern/libs'])
        BaseEnv.Replace(LUA_LIBS=['ext_lua','ext_lualib'])

if LuaEnabled:
    if LuaInternal:
        BaseEnv.Append(LUA_VERSION='51')
    elif BaseConf.CheckLuaFunc('lua_newstate'):
        BaseEnv.Append(LUA_VERSION='51')
    else:
        BaseEnv.Append(LUA_VERSION='50')

#if BaseEnv['WITH_LIBPNG']:
if PNGEnabled:
    BaseConf.ConfigureLibPng()

#if BaseEnv['WITH_LIBJPEG']:
if JPEGEnabled:
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
    if BaseEnv['PLATFORM'] == 'darwin':
        CC_OPT_FLAGS='-O3 -ffast-math -stdlib=libc++'
    else:
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
    BaseEnv.Append(CFLAGS=['-mmacosx-version-min=10.7'])
    BaseEnv.Append(CXXFLAGS=['-mmacosx-version-min=10.7'])
    BaseEnv.Append(LINKFLAGS=['-mmacosx-version-min=10.7'])

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

