#version 330

#define M_PI	3.14159265358979323846

const int width_80  = 80 * 10;
const int width_132  = 132 * 9;
const int height = 240;

uniform float focus = 0.75;
const float glow_control = 0.9;
const float glow_intensity = 0.5;

uniform sampler2D vt220_screen;
uniform sampler2D blur_texture;

uniform bool is_132col;
uniform bool enable_glow;
uniform bool raw_mode;

in  vec2 pos;
out vec4 color;

void main(void)
{
	// the VT220 has 240 lines
	float line = pos.y * float(height);
	float linebase = float(int(line));

	float lineoffset = line - linebase;
	float phi = 2.0 * M_PI * lineoffset;
	float intensity = pow((sin(phi) + 1.0) / 2.0, focus);

	// ... and 800 columns (unless in 132col mode)
	int width = is_132col ? width_132 : width_80;
	float column = pos.x * float(width);
	float colbase = float(int(column));

	float xblend = raw_mode ? 0.0 : (column - colbase);

	// jump to next line if we are over 75% of the line
	int texline = int(linebase);
	if(lineoffset > 0.75 && (texline + 1) < height) {
		texline++;
	}

	// blank last line / avoid 25% of incorrect color
	if(lineoffset > 0.75 && (texline + 1) == height) {
		intensity = 0.0;
	}

	// retrieve 3 samples for phosphor simulation with bit extension
	// first sample: current pixel
	ivec2 texcoord0 = ivec2(int(colbase), texline);
	vec4 fb0 = texelFetch(vt220_screen, texcoord0, 0);

	// second sample: current - 1
	ivec2 texcoord1 = texcoord0 - ivec2(1, 0);
	bool blank0 = texcoord1.x < 0;
	if(blank0) {
		texcoord1.x = 0;
	}

	vec4 fb1 = texelFetch(vt220_screen, texcoord1, 0);
	if(blank0) {
		// blank it if we are outside of the screen
		fb1 = vec4(0.0);
	}

	// third sample: current - 2
	ivec2 texcoord2 = texcoord1 - ivec2(1, 0);
	bool blank1 = texcoord2.x < 0;
	if(blank1) {
		texcoord2.x = 0;
	}

	vec4 fb2 = texelFetch(vt220_screen, texcoord2, 0);
	if(blank1) {
		// blank it if we are outside of the screen
		fb2 = vec4(0.0);
	}

	// combine the samples with phosphor simulation
	vec4 result;
	if(fb1.a > fb0.a) {
		// sequence 'X10' => '1'
		result = fb1;
	} else if(fb2.a > fb0.a) {
		// sequence '100' => '0'
		result = fb0 * vec4(xblend) + fb2 * vec4(1.0 - xblend);
	} else if(fb2.a > fb1.a) {
		// sequence '101' => '1'
		result = fb0 * vec4(xblend) + fb2 * vec4(1.0 - xblend);
	} else {
		// sequence 'X01' => '1'
		result = fb0 * vec4(xblend) + fb1 * vec4(1.0 - xblend);
	}

	result = vec4(result.rgb, 1.0);

	// get glow
	vec4 glow = pow(texture(blur_texture, pos), vec4(glow_control));

	if(!enable_glow) {
		glow = vec4(0.0);
	}

	// combine VT220 screen with scanlines and a bit of glow
	vec4 vt220 = result * vec4(intensity) + pow(glow, vec4(2.0)) * 0.2;

	if(raw_mode) {
		color = vec4(result.rgb, 1.0);
	} else {
		// combine with glow for the final result
		color = vec4((vt220 + glow * glow_intensity).rgb, 1.0);
	}
}
