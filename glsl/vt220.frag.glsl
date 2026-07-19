#version 330

#define _BV(x)		(1u << (x))
#define	KAM		_BV(0)
#define	IRM		_BV(1)
#define	SRM		_BV(2)
#define	LNM		_BV(3)
#define	DECCKM		_BV(4)
#define	DECANM		_BV(5)
#define	DECCOLM		_BV(6)
#define	DECSCLM		_BV(7)
#define	DECSCNM		_BV(8)
#define	DECOM		_BV(9)
#define	DECAWM		_BV(10)
#define	DECARM		_BV(11)
#define	DECPFF		_BV(11)
#define	DECPEX		_BV(12)
#define	DECTCEM		_BV(13)
#define	DECKPAM		_BV(14)
#define	DECTEK		_BV(15)
#define	DECNRCM		_BV(16)
#define	DECGEPM		_BV(17)
#define	DECGPCM		_BV(18)
#define	DECGPCS		_BV(19)
#define	DECGPBM		_BV(20)
#define	DECGRPM		_BV(21)

#define	SGR_BOLD	_BV(0)
#define	SGR_UNDERSCORE	_BV(1)
#define	SGR_BLINKING	_BV(2)
#define	SGR_REVERSE	_BV(3)
#define	SCA_ON		_BV(4)

#define	DECSWL		0u
#define	DECDHL_TOP	1u
#define	DECDHL_BOTTOM	2u
#define	DECDWL		3u

#define	CURSOR_OFF_TIME			(2.0f / 3.0f)
#define	CURSOR_ON_TIME			(4.0f / 3.0f)

#define	BLINK_OFF_TIME			1.8f
#define	BLINK_ON_TIME			1.8f

const int width_80  = 80 * 10;
const int width_132  = 132 * 9;
const int height = 240;
const int cell_width_80 = 10;
const int cell_width_132 = 9;
const int cell_height = 10;

uniform uvec2 text_size;

uniform usampler2D font;
uniform usampler2D drcs;
uniform usampler2D text;
uniform usampler2D line_attributes;
uniform usampler2D setup_text;
uniform usampler2D setup_line_attributes;

uniform uvec2 cursor;
uniform float cursor_time;
uniform float blink_time;
uniform uint mode;
uniform bool in_setup;
uniform bool block_cursor;

uniform float intensity = 1.0;

in  vec2 pos;
out vec4 color;

bool get_font_pixel(uint glyph, uvec2 pos)
{
	int glyphid = glyph < 288u ? int(glyph) : (int(glyph) - 288);
	ivec2 fontpos = ivec2(int(pos.y), glyphid);

	if(fontpos.x >= 10 || glyph >= (288u + 94u)) {
		return false;
	}

	uint bits;
	if(glyph < 288u) {
		bits = texelFetch(font, fontpos, 0).r;
	} else {
		bits = texelFetch(drcs, fontpos, 0).r;
	}

	bool bit;
	if(pos.x > 0u && pos.x < 9u) {
		bit = ((bits << (pos.x - 1u)) & 0x80u) != 0u;
	} else if(pos.x == 9u) {
		bit = (bits & 1u) != 0u;
	} else {
		bit = false;
	}

	return bit;
}

void get_cell(const in bool is_132col, const in int cell_width,
		const in int width, const in vec2 position,
		out uvec2 cell, out uvec2 cell_pos, out uint attr,
		out bool bit)
{
	// compute text cell and relative glyph coordinates
	vec2 textpos = position / vec2(float(cell_width), float(cell_height));
	cell = uvec2(textpos);
	cell_pos = uvec2((textpos - vec2(cell)) * vec2(float(cell_width), float(cell_height)));

	// retrieve glyph and attributes
	ivec2 cellcoord = ivec2(cell);
	uint lineattr = texelFetch(line_attributes, ivec2(cellcoord.y, 0), 0).r;

	if(lineattr != DECSWL) {
		cellcoord.x /= 2;
	}

	uvec2 textcell = texelFetch(text, cellcoord, 0).rg;

	// maybe this is overwritten by the setup?
	int setupline = int(cell.y) - (int(text_size.y) - 8);
	ivec2 setupcell_pos = ivec2(int(cell.x), setupline); 
	if(setupline < 0) {
		setupcell_pos.y = 0;
	}

	uint setuplineattr = texelFetch(setup_line_attributes, ivec2(setupcell_pos.y, 0), 0).r;
	if(setuplineattr != DECSWL) {
		setupcell_pos.x /= 2;
	}
	uvec2 setupcell = texelFetch(setup_text, setupcell_pos, 0).rg;

	if(in_setup) {
		if(setupline >= 0) {
			textcell = setupcell;
			lineattr = setuplineattr;
		} else {
			// top part of screen is empty in setup
			textcell = uvec2(0u);
			lineattr = 0u;
		}
	}

	uint glyph = textcell.r;
	attr = textcell.g;

	if(lineattr != DECSWL) {
		// is this the left or the right part of a double width cell?
		bool cell1 = (cell.x % 2u) == 1u;

		uint x = cell_pos.x;
		if(cell1) {
			x += uint(cell_width);
		}

		cell.x /= 2u;
		cell_pos.x = x / 2u;

		// in case of double wide lines, there would be a 2px gap at the
		// beginning of the cell; fix it by moving everything left by
		// one pixel = half a font pixel

		if((x % 2u) == 1u && cell_pos.x < 9u) {
			cell_pos.x += 1u;
		}

		// now do the upper/lower stuff too
		if(lineattr == DECDHL_TOP) {
			cell_pos.y /= 2u;
		} else if(lineattr == DECDHL_BOTTOM) {
			cell_pos.y /= 2u;
			cell_pos.y += 5u;
		}
	}

	// get glyph bit
	bit = get_font_pixel(glyph, cell_pos);
}

void main(void)
{
	// is this VT220 in 80 or in 132 column mode?
	bool is_132col = (mode & DECCOLM) != 0u;
	int cell_width = is_132col ? cell_width_132 : cell_width_80;
	int width = is_132col ? width_132 : width_80;

	// pixel position on the screen
	vec2 position = pos * vec2(float(width), float(height));

	// get current cell/attribute/bit
	uvec2 cell;
	uvec2 cell_pos;
	uint attr;
	bool bit;

	get_cell(is_132col, cell_width, width, position, cell, cell_pos,
			attr, bit);

	// get previous cell/attribute/bit
	vec2 last_pos = vec2(max(position.x - 1, 0), position.y);

	uvec2 last_cell;
	uvec2 last_cell_pos;
	uint last_attr;
	bool last_bit;

	get_cell(is_132col, cell_width, width, last_pos, last_cell,
			last_cell_pos, last_attr, last_bit);

	// get previous - 1 cell/attribute/bit
	vec2 last_2_pos = vec2(max(position.x - 2, 0), position.y);

	uvec2 last_2_cell;
	uvec2 last_2_cell_pos;
	uint last_2_attr;
	bool last_2_bit;

	get_cell(is_132col, cell_width, width, last_2_pos, last_2_cell,
			last_2_cell_pos, last_2_attr, last_2_bit);

	// perform dot stretching
	bit = bit || last_bit;

	// the real VT220 is really weird
	bit = bit || ((!is_132col && cell_pos.x == 1u) && last_2_bit);

////////////////////////////////////////////////////////////////////////////////
	bool blink_on = blink_time < BLINK_ON_TIME;
	bool cursor_on = cursor_time < CURSOR_ON_TIME;

	uvec2 cursor_cell = cursor;
	uint line_length = is_132col ? 132u : 80u;
	if(cursor_cell.x >= line_length) {
		cursor_cell.x = line_length - 1u;
	}

	if((mode & DECTCEM) != 0u && cursor_on && !in_setup && cell == cursor_cell) {
		uint xor = block_cursor ? SGR_REVERSE : SGR_UNDERSCORE;
		attr ^= xor;
	}

	bit = bit || ((attr & SGR_UNDERSCORE) != 0u && cell_pos.y == 8u);

	// select intensity according to SGR
	bool decscnm = (mode & DECSCNM) != 0u;
	bool sgr_rev = (attr & SGR_REVERSE) != 0u;
	bool reverse = sgr_rev != decscnm;
	bool pixel = bit != reverse;

	bool sgr_bold = (attr & SGR_BOLD) != 0u;
	bool sgr_blink = (attr & SGR_BLINKING) != 0u;

	float beam_base = pixel ? 0.5 : 0.0;
	float beam_bold = pixel && sgr_bold ? 0.5 : 0.0;
	float beam_blink = pixel && sgr_blink && blink_on ? -0.5 : 0.0;
	float beam = (beam_base + beam_bold + beam_blink) * intensity;

	// multiply by 1.2 to compress intensity range; this compensates
	// for the gamma function used in the CRT rendering step
	color = vec4(vec3(clamp(beam * 1.2, 0.0, 1.0)), 1.0);
}
