#version 130
varying vec2 vTexpos;
uniform sampler2D uTex;
//uniform vec4 uColor;

void main(void) {
  //gl_FragColor = vec4(1, 1, 1, texture2D(uTex, vTexpos).a) * uColor;
  //gl_FragColor = texture2D(uTex, vTexpos);
  gl_FragColor = texture2D(uTex, vec2(vTexpos.s, vTexpos.t));

}