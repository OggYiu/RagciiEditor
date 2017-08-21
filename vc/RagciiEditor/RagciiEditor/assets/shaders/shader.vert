#version 130
attribute vec4 aPosition;
varying vec2 vTexpos;

void main(void) {
  gl_Position = vec4(aPosition.xy, 0, 1);
  vTexpos = aPosition.zw;
}