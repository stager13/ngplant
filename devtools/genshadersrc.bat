IF NOT EXIST %2\shaders\NUL mkdir %2\shaders
python.exe %1\devtools\text2cdata.py P3DDefaultVertexShaderSrc   < %1\shaders\ngplant_default_vs.glsl > %2\shaders\default_vs.h
python.exe %1\devtools\text2cdata.py P3DDefaultFragmentShaderSrc < %1\shaders\ngplant_default_fs.glsl > %2\shaders\default_fs.h
