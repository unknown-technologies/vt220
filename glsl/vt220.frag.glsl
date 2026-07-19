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

#define	COLOR_NORMAL			 0u
#define	COLOR_REVERSE			 1u
#define	COLOR_BLINK_OFF			 2u
#define	COLOR_BLINK_ON			 3u
#define	COLOR_REVERSE_BLINK_OFF		 4u
#define	COLOR_REVERSE_BLINK_ON		 5u
#define	COLOR_BOLD			 6u
#define	COLOR_BOLD_BLINK_OFF		 7u
#define	COLOR_BOLD_BLINK_ON		 8u
#define	COLOR_REVERSE_BOLD		 9u
#define	COLOR_REVERSE_BOLD_BLINK_OFF	10u
#define	COLOR_REVERSE_BOLD_BLINK_ON	11u

#define	COLOR_BG			0u
#define	COLOR_FG			1u

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

/* monochrome: normal */
const uvec2 vt220_colors_normal[12] = uvec2[](
	/* normal */			uvec2(0, 2),
	/* reverse */			uvec2(1, 0),
	/* blink off */			uvec2(0, 0),
	/* blink on */			uvec2(0, 2),
	/* reverse blink off */		uvec2(0, 0),
	/* reverse blink on */		uvec2(1, 0),
	/* bold */			uvec2(0, 3),
	/* bold blink off */		uvec2(0, 2),
	/* bold blink on */		uvec2(0, 3),
	/* reverse bold */		uvec2(3, 0),
	/* reverse bold blink off */	uvec2(1, 0),
	/* reverse bold blink on */	uvec2(3, 0)
);

/* monochrome: reverse */
const uvec2 vt220_colors_reverse[12] = uvec2[](
	/* normal */			uvec2(1, 0),
	/* reverse */			uvec2(0, 2),
	/* blink off */			uvec2(0, 0),
	/* blink on */			uvec2(1, 0),
	/* reverse blink off */		uvec2(0, 0),
	/* reverse blink on */		uvec2(0, 2),
	/* bold */			uvec2(3, 0),
	/* bold blink off */		uvec2(1, 0),
	/* bold blink on */		uvec2(3, 0),
	/* reverse bold */		uvec2(0, 3),
	/* reverse bold blink off */	uvec2(0, 1),
	/* reverse bold blink on */	uvec2(0, 3)
);

uniform uvec2 text_size;

uniform usampler2D font;
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
uniform vec3 colorscheme[4];

in  vec2 pos;
out vec4 color;

uint VT220GetColorID(uint sgr, bool blink_on)
{
	if((sgr & SGR_REVERSE) != 0u) {
		if((sgr & SGR_BOLD) != 0u) {
			if((sgr & SGR_BLINKING) != 0u) {
				if(blink_on) {
					return COLOR_REVERSE_BOLD_BLINK_ON;
				} else {
					return COLOR_REVERSE_BOLD_BLINK_OFF;
				}
			} else {
				return COLOR_REVERSE_BOLD;
			}
		} else {
			if((sgr & SGR_BLINKING) != 0u) {
				if(blink_on) {
					return COLOR_REVERSE_BLINK_ON;
				} else {
					return COLOR_REVERSE_BLINK_OFF;
				}
			} else {
				return COLOR_REVERSE;
			}
		}
	} else {
		if((sgr & SGR_BOLD) != 0u) {
			if((sgr & SGR_BLINKING) != 0u) {
				if(blink_on) {
					return COLOR_BOLD_BLINK_ON;
				} else {
					return COLOR_BOLD_BLINK_OFF;
				}
			} else {
				return COLOR_BOLD;
			}
		} else {
			if((sgr & SGR_BLINKING) != 0u) {
				if(blink_on) {
					return COLOR_BLINK_ON;
				} else {
					return COLOR_BLINK_OFF;
				}
			} else {
				return COLOR_NORMAL;
			}
		}
	}
}

uvec2 VT220GetColor(uint sgr, bool blink_on)
{
	uint id = VT220GetColorID(sgr, blink_on);

	if((mode & DECSCNM) != 0u) {
		// reverse mode
		return vt220_colors_reverse[id];
	} else {
		return vt220_colors_normal[id];
	}
}

bool get_font_pixel(uint glyph, uvec2 pos)
{
	ivec2 fontpos = ivec2(int(pos.y), int(glyph));

	if(fontpos.x >= 10 || fontpos.y >= 288) {
		return false;
	}

	uint bits = texelFetch(font, fontpos, 0).r;

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

void main(void)
{
	// is this VT220 in 80 or in 132 column mode?
	bool is_132col = (mode & DECCOLM) != 0u;
	int cell_width = is_132col ? cell_width_132 : cell_width_80;
	int width = is_132col ? width_132 : width_80;

	// pixel position on the screen
	vec2 position = pos * vec2(float(width), float(height));

	// compute text cell and relative glyph coordinates
	vec2 textpos = position / vec2(float(cell_width), float(cell_height));
	uvec2 cell = uvec2(textpos);
	uvec2 cell_pos = uvec2((textpos - vec2(cell)) * vec2(float(cell_width), float(cell_height)));

	ivec2 cell_offset = ivec2(0);
	if(in_setup) {
		cell_offset += ivec2(0, 8);
	}

	// retrieve glyph and attributes
	ivec2 cellcoord = ivec2(cell) + cell_offset;
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
	uint attr = textcell.g;

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
	bool bit = get_font_pixel(glyph, cell_pos);

////////////////////////////////////////////////////////////////////////////////
	bool blink_on = blink_time < BLINK_ON_TIME;
	bool cursor_on = cursor_time < CURSOR_ON_TIME;

	uvec2 cursor_cell = cursor;
	uint line_length = is_132col ? 132u : 80u;
	if(cursor_cell.x >= line_length) {
		cursor_cell.x = line_length - 1u;
	}
	if((mode & DECTCEM) != 0u && cursor_on && !in_setup && cell == cursor_cell) {
		if(block_cursor) {
			attr ^= SGR_REVERSE;
		} else {
			attr ^= SGR_UNDERSCORE;
		}
	}

	if((attr & SGR_UNDERSCORE) != 0u && cell_pos.y == 8u) {
		bit = true;
	}

	// select color according to SGR and colorscheme
	uvec2 colors = VT220GetColor(attr, blink_on);
	vec3 text_color = colorscheme[bit ? colors.y : colors.x];

	color = vec4(text_color * intensity, bit ? 1.0 : 0.0);
}
