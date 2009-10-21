#ifdef HAVE_NORMAL_MAP

attribute vec3 ngp_BiNormal;

varying vec3 NormalES;
varying vec3 BiNormalES;
varying vec3 TangentES;

void     main     ()
 {
  gl_Position = ftransform();

  NormalES   = gl_NormalMatrix * gl_Normal;
  BiNormalES = gl_NormalMatrix * ngp_BiNormal;
  TangentES  = cross(BiNormalES,NormalES);

  gl_TexCoord[0] = gl_MultiTexCoord0;
 }

#else /* do not HAVE_NORMAL_MAP */

varying vec3 NormalES;

void     main     ()
 {
  gl_Position = ftransform();

  NormalES = gl_NormalMatrix * gl_Normal;
 }

#endif

