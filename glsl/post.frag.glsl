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
uniform bool use_simple_phosphor;

uniform vec3 colorscheme;

in  vec2 pos;
out vec4 color;

float phosphor(const vec4 history, const float position)
{
	// f(-3) = history.w
	// f(-2) = history.z
	// f(-1) = history.y
	// f(0)  = history.x
	// position defines the position between f(-1) and f(0)

	// essentially this function is a temporal LPF
	vec4 vin = history.wzyx;
	float omega = 1.2;
	float beta = exp(-omega);
	float y = vin[0];

	for(int i = 1; i < 3; i++) {
		y = mix(vin[i], y, beta);
	}

	// last round
	float beta_last = exp(-omega * position);
	float result = mix(vin[3], y, beta_last);
	float next = mix(vin[3], y, beta);
	// TODO: is there a way to make this more smooth?
	float fall = mix(y, next, position);

	// use different function for high to low transition to make it
	// faster = "shorter" on the screen
	if(vin[3] < vin[2]) {
		return fall;
	} else {
		return result;
	}
}

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
	} else if(lineoffset > 0.75 && (texline + 1) >= height) {
		// blank last line / avoid 25% of incorrect color
		intensity = 0.0;
	}

	// retrieve 4 samples for phosphor simulation
	// first sample: current pixel
	ivec2 texcoord0 = ivec2(int(colbase), texline);
	float fb0 = texelFetch(vt220_screen, texcoord0, 0).r;

	// second sample: current - 1
	ivec2 texcoord1 = texcoord0 - ivec2(1, 0);
	bool blank0 = texcoord1.x < 0;
	if(blank0) {
		texcoord1.x = 0;
	}

	float fb1 = texelFetch(vt220_screen, texcoord1, 0).r;
	if(blank0) {
		// blank it if we are outside of the screen
		fb1 = 0.0;
	}

	// third sample: current - 2
	ivec2 texcoord2 = texcoord1 - ivec2(1, 0);
	bool blank1 = texcoord2.x < 0;
	if(blank1) {
		texcoord2.x = 0;
	}

	float fb2 = texelFetch(vt220_screen, texcoord2, 0).r;
	if(blank1) {
		// blank it if we are outside of the screen
		fb2 = 0.0;
	}

	// fourth sample: current - 3
	ivec2 texcoord3 = texcoord2 - ivec2(1, 0);
	bool blank2 = texcoord3.x < 0;
	if(blank2) {
		texcoord3.x = 0;
	}

	float fb3 = texelFetch(vt220_screen, texcoord3, 0).r;
	if(blank2) {
		// blank it if we are outside of the screen
		fb3 = 0.0;
	}

	float result;
	if(use_simple_phosphor) {
		result = mix(fb1, fb0, xblend);
	} else {
		// now combine the 4 samples using the phosphor function
		vec4 history = vec4(fb0, fb1, fb2, fb3);
		result = phosphor(history, xblend);
	}

	// get glow
	float glow = pow(texture(blur_texture, pos).r, glow_control);

	if(!enable_glow) {
		glow = 0.0;
	}

	// combine VT220 screen with scanlines and a bit of glow
	float vt220 = result * intensity + pow(glow, 2.0) * 0.2;

	float alpha = 1.2;
	float beta =  0.0;
	float gamma = 1.8;

	if(raw_mode) {
		// always use the gamma here to get the same intensity
		// curve as in non-raw mode
		float Y = pow(fb0, gamma);
		color = vec4(vec3(Y) * colorscheme, 1.0);
	} else {
		// combine with glow for the final result
		float beam = vt220;
		float Y = pow(beam * alpha + beta, gamma);

		// TODO: should we clamp here? This prevents strange
		// colors if the intensity is too high
		//Y = clamp(Y, 0.0, 1.0);

		// now add the glow, with less gamma
		Y += pow(glow * glow_intensity, 1.3);

		color = vec4(vec3(Y) * colorscheme, 1.0);
	}
}
