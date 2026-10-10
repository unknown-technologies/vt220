#version 330

layout(location = 0) in vec3 position;

uniform bool flipped;

out vec2 pos;

void main(void)
{
	gl_Position = vec4(position.xyz, 1.0);

	vec2 screen = (position.xy + vec2(1.0, 1.0)) / 2.0;

	if(flipped) {
		pos = vec2(1.0 - screen.x, screen.y);
	} else {
		pos = vec2(screen.x, 1.0 - screen.y);
	}
}
