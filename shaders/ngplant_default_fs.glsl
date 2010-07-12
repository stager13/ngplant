#ifdef HAVE_DIFFUSE_TEX
uniform sampler2D  DiffuseTexSampler;
#endif

#ifdef HAVE_NORMAL_MAP
uniform sampler2D  NormalMapSampler;
#endif

#ifdef HAVE_NORMAL_MAP

varying vec3 LightDir;
varying vec4 DiffuseProduct;

void     main     ()
 {
  float  DiffuseFactor;
  vec3   FragmentNormal;

  FragmentNormal = texture2D(NormalMapSampler,gl_TexCoord[0].st).rgb;
  FragmentNormal *= 2.0;
  FragmentNormal -= 1.0;

  #ifdef TWO_SIDED
  if (gl_FrontFacing)
   {
    DiffuseFactor = max(0.0,dot(normalize(LightDir),FragmentNormal));
   }
  else
   {
    vec3      RefLightDir;

    RefLightDir = vec3(LightDir.x,LightDir.y,-LightDir.z);

    DiffuseFactor = max(0.0,dot(normalize(RefLightDir),FragmentNormal));
   }
  #else
  DiffuseFactor  = max(0.0,dot(normalize(LightDir),FragmentNormal));
  #endif

  gl_FragColor = gl_Color + DiffuseFactor * DiffuseProduct;

  #ifdef HAVE_DIFFUSE_TEX
  gl_FragColor *= texture2D(DiffuseTexSampler,gl_TexCoord[0].st);
  #endif
 }

#else

void     main     ()
 {
  gl_FragColor = gl_Color;

  #ifdef HAVE_DIFFUSE_TEX
  gl_FragColor *= texture2D(DiffuseTexSampler,gl_TexCoord[0].st);
  #endif
 }

#endif

