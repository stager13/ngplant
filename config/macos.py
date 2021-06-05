# Enable cross-compilation
cross="no"

# C compiler
CC     = "gcc"
# C++ compiler
CXX    = "g++"
# Archive indexer
RANLIB = "ranlib"
# Archiver
AR     = "ar"
# Resource compiler
#WINDRES = "windres"

# wxWidgets headers paths (space-separated)
WX_INC     = "/usr/local/include/wx-3.0/wx /usr/local/lib/wx/include/osx_cocoa-unicode-3.0/wx/"
# wxWidgets libraries (space-separated)
WX_LIB		= "wx wx_osx_cocoau_core wx_osx_cocoau_gl"
# wxWidgets libaries paths (space-separated)
WX_LIBPATH = "/usr/local/lib/"

# Build Lua libraries from sources in extern/lua/src
LUA_INTERNAL= "no"
# Lua headers path(s) (space-separated)
LUA_INC = "/usr/local/include/lua/"
# Lua libraries (space-separated)
LUA_LIBS = "lua"
# Lua libraries path(s) (space-separated)
LUA_LIBPATH = "/usr/local/lib/"

# GLU headers path(s) (space-separated)
GLU_INC     = "/usr/local/include/GL/"
# GLU libraries (space-separated)
GLU_LIBS    = "glut"
# GLU libraries path(s) (space-separated)
GLU_LIBPATH = "/usr/local/lib/"

# Build GLEW library from sources in extern/glew
GLEW_INTERNAL= "no"
# GLEW headers path(s) (space-separated)
GLEW_INC     = "/usr/local/include/GL/"
# GLEW libraries (space-separated)
GLEW_LIBS    = "GLEW"
# GLEW libraries path(s) (space-separated)
GLEW_LIBPATH = "/usr/local/lib/"

# Enable PNG support
WITH_LIBPNG = "yes"

# libpng headers path(s) (space-separated)
LIBPNG_INC     = "/usr/local/include/"
# libpng libraries (space-separated)
LIBPNG_LIBS    = "png"
# libpng libraries path(s) (space-separated)
LIBPNG_LIBPATH = "/usr/local/lib/"
# libpng additional preprocessor definitions (space-separated)
LIBPNG_DEFINES = ""
# libpng pkg-config custom command line
LIBPNG_CONFIG = ""

# Enable JPEG support
WITH_LIBJPEG = "yes"

# libjpeg headers path(s) (space-separated)
LIBJPEG_INC     = "/usr/local/include/"
# libjpeg libraries (space-separated)
LIBJPEG_LIBS    = "jpeg"
# libjpeg libraries path(s) (space-separated)
LIBJPEG_LIBPATH = "/usr/local/lib/"
# libjpeg additional preprocessor definitions (space-separated)
LIBJPEG_DEFINES = ""
# libjpeg pkg-config custom command line
LIBJPEG_CONFIG = ""

# The search path for ngplant plugins ('plugins' if set to None)
PLUGINS_DIR = None

