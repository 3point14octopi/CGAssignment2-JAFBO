//blurring of lights in the scene for bloom purposes
//heavily based on this shader: 
//https://lettier.github.io/3d-game-shaders-for-beginners/bloom.html 

#version 430

uniform sampler2D colorTexture;


out vec4 fragColor;

void main() {
  int   size       = 1;
  float separation = 3.0;
  float threshold  = 0.6;
  float amount     = 0.6;


  vec2 texSize = textureSize(colorTexture, 0).xy;

  vec4 result = vec4(0.0);
  vec4 color  = vec4(0.0);

  float value = 0.0;
  float count = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      color =
        texture
          ( colorTexture
          ,   (vec2(i, j) * separation + gl_FragCoord.xy)
            / texSize
          );

      value = max(color.r, max(color.g, color.b));
      if (value < threshold) { color = vec4(0.0); }

      result += color;
      count  += 1.0;
    }
  }

  result /= count;

  fragColor = mix(vec4(0.0), result, amount);
}