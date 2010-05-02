# Enable cross-compilation
cross="yes"

# C compiler
CC     = "i586-mingw32msvc-gcc"
# C++ compiler
CXX    = "i586-mingw32msvc-g++"
# Archive indexer
RANLIB = "i586-mingw32msvc-ranlib"
# Archiver
AR     = "i586-mingw32msvc-ar"
# Resource compiler
WINDRES = "i586-mingw32msvc-windres"

# wxWidgets headers paths (space-separated)
WX_INC     = "/home/alice/opt/i586-mingw32/include/wx-2.6 /home/alice/opt/i586-mingw32/lib/wx/include/i586-mingw32-msw-ansi-release-static-2.6"
# wxWidgets libraries (space-separated)
WX_LIB     = "wx_msw-2.6-i586-mingw32 wx_msw_gl-2.6-i586-mingw32 wxtiff-2.6-i586-mingw32 wxjpeg-2.6-i586-mingw32 wxpng-2.6-i586-mingw32 wxzlib-2.6-i586-mingw32"
# wxWidgets libaries paths (space-separated)
WX_LIBPATH = "/home/alice/opt/i586-mingw32/lib"

# Build Lua libraries from sources in extern/lua/src
LUA_INTERNAL= "yes"
# Lua headers path(s) (space-separated)
LUA_INC = ""
# Lua libraries (space-separated)
LUA_LIBS = ""
# Lua libraries path(s) (space-separated)
LUA_LIBPATH = ""

# Build GLEW library from sources in extern/glew
GLEW_INTERNAL= "yes"
# GLEW headers path(s) (space-separated)
GLEW_INC     = ""
# GLEW libraries (space-separated)
GLEW_LIBS    = ""
# GLEW libraries path(s) (space-separated)
GLEW_LIBPATH = ""

# Enable PNG support
WITH_LIBPNG = "yes"

# libpng headers path(s) (space-separated)
LIBPNG_INC     = "/home/alice/opt/i586-mingw32/include/"
# libpng libraries (space-separated)
LIBPNG_LIBS    = "png"
# libpng libraries path(s) (space-separated)
LIBPNG_LIBPATH = "/home/alice/opt/i586-mingw32/lib"
# libpng additional preprocessor definitions (space-separated)
LIBPNG_DEFINES = ""
# libpng pkg-config custom command line
LIBPNG_CONFIG = ""

# Enable JPEG support
WITH_LIBJPEG = "yes"

# libjpeg headers path(s) (space-separated)
LIBJPEG_INC     = "/home/alice/opt/i586-mingw32/include/"
# libjpeg libraries (space-separated)
LIBJPEG_LIBS    = "jpeg"
# libjpeg libraries path(s) (space-separated)
LIBJPEG_LIBPATH = "/home/alice/opt/i586-mingw32/lib"
# libjpeg additional preprocessor definitions (space-separated)
LIBJPEG_DEFINES = ""
# libjpeg pkg-config custom command line
LIBJPEG_CONFIG = ""

# The search path for ngplant plugins ('plugins' if set to None)
PLUGINS_DIR = None

