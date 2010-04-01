#ifdef HAVE_NORMAL_MAP

attribute vec3 ngp_BiNormal;

varying vec3 LightDir;
varying vec4 DiffuseProduct;

void     main     ()
 {
  vec3   Normal;
  vec3   BiNormal;
  vec3   Tangent;

  gl_Position    = ftransform();
  gl_TexCoord[0] = gl_MultiTexCoord0;

  Normal   = gl_NormalMatrix * gl_Normal;
  BiNormal = gl_NormalMatrix * ngp_BiNormal;
  Tangent  = cross(BiNormal,Normal);

  LightDir.x = dot(Tangent,vec3(gl_LightSource[0].position));
  LightDir.y = dot(BiNormal,vec3(gl_LightSource[0].position));
  LightDir.z = dot(Normal,vec3(gl_LightSource[0].position));

  #ifdef TWO_SIDED
  gl_BackColor = gl_Color * gl_LightModel.ambient +
                  gl_Color * gl_LightSource[0].ambient;
  #endif

  gl_FrontColor = gl_Color * gl_LightModel.ambient +
                   gl_Color * gl_LightSource[0].ambient;

  DiffuseProduct = gl_Color * gl_LightSource[0].diffuse;
 }

#else /* do not HAVE_NORMAL_MAP */

void     main     ()
 {
  vec3   NormalES;
  float  LdotN;

  gl_Position = ftransform();

  #ifdef HAVE_DIFFUSE_TEX
  gl_TexCoord[0] = gl_MultiTexCoord0;
  #endif

  NormalES = gl_NormalMatrix * gl_Normal;

  LdotN = dot(vec3(gl_LightSource[0].position),NormalES);

  gl_FrontColor = gl_Color * gl_LightModel.ambient +
                   gl_Color * gl_LightSource[0].ambient +
                    max(0.0,LdotN) * gl_Color * gl_LightSource[0].diffuse;
  #ifdef TWO_SIDED
  gl_BackColor = gl_Color * gl_LightModel.ambient +
                  gl_Color * gl_LightSource[0].ambient +
                   max(0.0,-LdotN) * gl_Color * gl_LightSource[0].diffuse;
  #endif
 }

#endif

