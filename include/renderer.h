#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "vt.h"

typedef struct {
	VT220*		vt;

	GLuint		quad_vbo;
	GLuint		quad_vao;

	GLuint		vt_fb;
	GLuint		vt_tex;

	GLuint		font_tex;
	GLuint		text_tex;
	GLuint		line_attrib_tex;
	GLuint		setup_text_tex;
	GLuint		setup_line_attrib_tex;

	GLuint		vt_shader;
	GLuint		vt_shader_font;
	GLuint		vt_shader_text;
	GLuint		vt_shader_line_attributes;
	GLuint		vt_shader_setup_text;
	GLuint		vt_shader_setup_line_attributes;
	GLuint		vt_shader_text_size;
	GLuint		vt_shader_cursor;
	GLuint		vt_shader_colorscheme;
	GLuint		vt_shader_cursor_time;
	GLuint		vt_shader_blink_time;
	GLuint		vt_shader_mode;
	GLuint		vt_shader_in_setup;
	GLuint		vt_shader_block_cursor;
	GLuint		vt_shader_intensity;

	GLuint		blur_fb[2];
	GLuint		blur_tex[2];

	GLuint		blur_shader;
	GLuint		blur_shader_tex;
	GLuint		blur_shader_dir;

	GLuint		post_shader;
	GLuint		post_shader_fbtex;
	GLuint		post_shader_blurtex;
	GLuint		post_shader_enableglow;
	GLuint		post_shader_is132col;
	GLuint		post_shader_raw;
	GLuint		post_shader_focus;

	unsigned long	blink_time;

	bool		enable_glow;
	bool		raw;
	float		focus;
	float		intensity;

	bool		was_132col;
} VTRenderer;

void	VTInitRenderer(VTRenderer* renderer, VT220* vt);
void	VTEnableGlow(VTRenderer* self, bool enabled);
void	VTSetRaw(VTRenderer* vt, bool raw);
void	VTSetFocus(VTRenderer* vt, float color);
void	VTSetIntensity(VTRenderer* vt, float intensity);
void	VTProcess(VTRenderer* self, unsigned long dt);
void	VTRender(VTRenderer* self, unsigned int width, unsigned int height);

void	VTCreateBuffers(VTRenderer* self);
void	VTCreateFrameBuffer(VTRenderer* self);
void	VTCreateBlurFrameBuffer(VTRenderer* self);
void	VTCreateGraphicsTexture(VTRenderer* self);
void	VTCreateFontTexture(VTRenderer* self);
void	VTCreateTextTexture(VTRenderer* self);
GLuint	VTCompileShader(GLuint type, const char* src);
GLuint	VTCreateShader(const char* vs_src, const char* fs_src);

void	VTRenderTerminal(VTRenderer* self);
void	VTRenderBlur(VTRenderer* self);

#endif
