#ifdef HAVE_NORMAL_MAP

uniform sampler2D  NormalMapSampler;

varying vec3 NormalES;
varying vec3 BiNormalES;
varying vec3 TangentES;

void     main     ()
 {
  vec3             Normal;
  vec3             BiNormal;
  vec3             Tangent;
  vec3             FragmentNormal;

  Normal   = normalize(NormalES);
  BiNormal = normalize(BiNormalES);
  Tangent  = normalize(TangentES);

  FragmentNormal = texture2D(NormalMapSampler,gl_TexCoord[0].st).rgb;
  FragmentNormal -= 0.5;
  FragmentNormal *= 2.0;

  gl_FragColor = vec4(Tangent.x  * FragmentNormal.x +
                      BiNormal.x * FragmentNormal.y +
                      Normal.x   * FragmentNormal.z,
                      Tangent.y  * FragmentNormal.x +
                      BiNormal.y * FragmentNormal.y +
                      Normal.y   * FragmentNormal.z,
                      Tangent.z  * FragmentNormal.x +
                      BiNormal.z * FragmentNormal.y +
                      Normal.z   * FragmentNormal.z,
                      1.0);

  gl_FragColor += 1.0;
  gl_FragColor *= 0.5;
 }

#else /* do not HAVE_NORMAL_MAP */

varying vec3 NormalES;

void     main     ()
 {
  vec3             FragmentNormal;

  FragmentNormal = NormalES;

  #ifdef TWO_SIDED
  if (!gl_FrontFacing)
   {
    FragmentNormal *= -1.0;
   }
  #endif

  FragmentNormal += 1.0;
  FragmentNormal *= 0.5;

  gl_FragColor  = vec4(FragmentNormal,1.0);
 }

#endif

