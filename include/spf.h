#ifndef __SPF_H__
#define __SPF_H__

#include <stddef.h>

#include "types.h"
#include "vt.h"
#include "renderer.h"

#define	VTCTRL(x)	((x) - 0x40)

#define	SPF_MAX_PANELS	8

#define	SPF_FLAG_ACTIVE	_BV(0)
#define	SPF_FLAG_CURSOR	_BV(1)
#define	SPF_FLAG_OPAQUE	_BV(2)
#define	SPF_FLAG_DIALOG	_BV(3)
#define	SPF_FLAG_DESTROY _BV(4)

#define SPF_WIDGET_CHECKBOX	0
#define SPF_WIDGET_INPUT_STR	1
#define SPF_WIDGET_INPUT_DEC	2
#define SPF_WIDGET_INPUT_CHR	3
#define SPF_WIDGET_LABEL	4
#define SPF_WIDGET_DYNAMIC_LBL	5

#define SPF_LABEL_NORMAL	0
#define SPF_LABEL_BOLD		_BV(0)
#define SPF_LABEL_UNDERLINE	_BV(1)
#define SPF_LABEL_REVERSE	_BV(2)
#define SPF_LABEL_BLINK		_BV(3)

typedef struct SPF SPF;
typedef struct SPFPanel SPFPanel;
typedef struct SPFPanelDefinition SPFPanelDefinition;

struct SPFPanelDefinition {
	void		(*init)(SPFPanel* self);
	void		(*draw)(SPFPanel* self);
	void		(*resize)(SPFPanel* self, int width, int height);
	void		(*process)(SPFPanel* self, u16 key);
	void		(*destroy)(SPFPanel* self);
};

typedef struct {
	u8		type;
	u8		id;
	u8		pos_x;
	u8		pos_y;
	SPFPanel*	panel;
} SPFWidget;

typedef struct SPFInputStr SPFInputStr;
struct SPFInputStr {
	SPFWidget	widget;
	char*		string;
	void		(*callback)(SPFInputStr* self);
	int		max_len;
	int		len;
	int		cursor;
};

typedef struct SPFInputDec SPFInputDec;
struct SPFInputDec {
	SPFWidget	widget;
	void		(*callback)(SPFInputDec* self);
	int		min;
	int		max;
	int		max_len;
	char		input[16];
	int		cursor;
	int		len;
};

typedef struct SPFInputChar SPFInputChar;
struct SPFInputChar {
	SPFWidget	widget;
	const char*	allowed_chars;
	void		(*callback)(SPFInputChar* self);
	char		value;
	char		uppercase;
};

typedef struct {
	SPFWidget	widget;
	const char*	text;
	unsigned int	flags;
} SPFLabel;

struct SPFCheckbox;
typedef struct SPFCheckbox SPFCheckbox;
struct SPFCheckbox {
	SPFWidget	widget;
	void		(*callback)(SPFCheckbox* self);
	BOOL		state;
};

struct SPFPanel {
	const SPFPanelDefinition* proto;
	const char*	title;
	SPF*		spf;
	void*		userdata;
	unsigned int	flags;
	int		width;
	int		height;
	int		cursor_x;
	int		cursor_y;
	SPFWidget**	widgets;
	SPFWidget*	focus;
};

struct SPF {
	VT220*		vt;
	VTScreen	screen;

	int		write_x;
	int		write_y;
	int		offset_x;
	int		offset_y;

	void*		userdata;
	SPFPanel	panels[SPF_MAX_PANELS];
	int		current_panel;

	BOOL		dirty;
};

/******************************************************************************/
/* SPF Routines                                                               */
/******************************************************************************/
void	SPFInit(SPF* self, VT220* vt);
void	SPFDestroy(SPF* self);
void	SPFDraw(SPF* self);
void	SPFProcess(SPF* self);
void	SPFProcessKey(SPF* self, u16 key);
void	SPFResize(SPF* self, int width, int height);
void	SPFRepaint(SPF* self);

/******************************************************************************/
/* Panel Routines                                                             */
/******************************************************************************/
SPFPanel* SPFCreatePanel(SPF* self, const SPFPanelDefinition* panel);
void	SPFDestroyPanel(SPFPanel* self);
void	SPFDrawPanel(SPFPanel* self);
void	SPFSetWidgets(SPFPanel* self, SPFWidget** widgets);


/******************************************************************************/
/* SPF Screen Prototypes                                                      */
/******************************************************************************/
void	SPFScreenShowCursor(SPF* self);
void	SPFScreenHideCursor(SPF* self);
void	SPFScreenSetCursor(SPF* self, const int line, const int column);
void	SPFScreenPutc(SPF* self, const u16 c, const int sgr, BOOL display_controls);
void	SPFScreenPrintNumber(SPF* self, int val, const int width, const int align, const int sgr);
void	SPFScreenPrintCenter(SPF* self, const int line, const char* s, const int sgr);
void	SPFScreenSetLineAttribute(SPF* self, int attr);
void	SPFScreenGoto(SPF* self, const int line, const int column);
void	SPFScreenCursorRight(SPF* self);
void	SPFScreenCursorRightN(SPF* self, int n);
void	SPFScreenPrint(SPF* self, const char* s, const int sgr);
void	SPFScreenFill(SPF* self, const u16 c, unsigned int count, const int sgr);
void	SPFScreenSetOffset(SPF* self, int line, int column);

/******************************************************************************/
/* Helper routines                                                            */
/******************************************************************************/
static inline VTScreen* SPFGetScreen(SPF* self)
{
	ASSERT(self);

	return &self->screen;
}

static inline SPF* SPFGet(SPFPanel* self)
{
	ASSERT(self);

	return self->spf;
}

static inline VT220* SPFGetVT(SPF* self)
{
	ASSERT(self);

	return self->vt;
}

static inline void* SPFGetUserData(SPF* self)
{
	ASSERT(self);

	return self->userdata;
}

static inline void SPFSetUserData(SPF* self, void* userdata)
{
	ASSERT(self);

	self->userdata = userdata;
}

static inline SPFPanel* SPFGetCurrentPanel(SPF* self)
{
	ASSERT(self);

	if(self->current_panel == -1) {
		return NULL;
	} else {
		return &self->panels[self->current_panel];
	}
}

static inline const char* SPFGetCurrentPanelTitle(SPF* self)
{
	ASSERT(self);

	SPFPanel* panel = SPFGetCurrentPanel(self);
	if(panel) {
		return panel->title;
	} else {
		return NULL;
	}
}

static inline void SPFSetPanelTitle(SPFPanel* self, const char* title)
{
	ASSERT(self);

	self->title = title;
}

static inline void* SPFGetPanelUserData(SPFPanel* self)
{
	ASSERT(self);

	return self->userdata;
}

static inline void SPFSetPanelUserData(SPFPanel* self, void* data)
{
	ASSERT(self);

	self->userdata = data;
}

static inline SPFWidget* SPFGetFocus(SPFPanel* self)
{
	ASSERT(self);

	return self->focus;
}

static inline void SPFSetFocus(SPFPanel* self, SPFWidget* widget)
{
	ASSERT(self);

	self->focus = widget;
}

static inline void SPFShowCursor(SPFPanel* self)
{
	ASSERT(self);

	self->flags |= SPF_FLAG_CURSOR;
}

static inline void SPFHideCursor(SPFPanel* self)
{
	ASSERT(self);

	self->flags &= ~SPF_FLAG_CURSOR;
}

static inline BOOL SPFIsCursorVisible(SPFPanel* self)
{
	ASSERT(self);

	return self->flags & SPF_FLAG_CURSOR;
}

static inline void SPFSetOpaque(SPFPanel* self)
{
	ASSERT(self);

	self->flags |= SPF_FLAG_OPAQUE;
}

static inline void SPFClearOpaque(SPFPanel* self)
{
	ASSERT(self);

	self->flags &= ~SPF_FLAG_OPAQUE;
}

static inline BOOL SPFIsOpaque(SPFPanel* self)
{
	ASSERT(self);

	return self->flags & SPF_FLAG_OPAQUE;
}

static inline void SPFSetDialog(SPFPanel* self)
{
	ASSERT(self);

	self->flags |= SPF_FLAG_DIALOG;
}

static inline void SPFClearDialog(SPFPanel* self)
{
	ASSERT(self);

	self->flags &= ~SPF_FLAG_DIALOG;
}

static inline BOOL SPFIsDialog(SPFPanel* self)
{
	ASSERT(self);

	return self->flags & SPF_FLAG_DIALOG;
}

static inline void SPFSetSize(SPFPanel* self, const int lines, const int columns)
{
	ASSERT(self);
	ASSERT(lines > 0 && columns > 0);

	self->width = columns;
	self->height = lines;
}

static inline void SPFGetSize(SPFPanel* self, int* lines, int* columns)
{
	ASSERT(self);
	ASSERT(lines && columns);

	*lines = self->height;
	*columns = self->width;
}

static inline int SPFGetWidth(SPFPanel* self)
{
	ASSERT(self);

	return self->width;
}

static inline int SPFGetHeight(SPFPanel* self)
{
	ASSERT(self);

	return self->height;
}

static inline void SPFSetCursor(SPFPanel* self, const int line, const int column)
{
	ASSERT(self);
	ASSERT(line >= 0 && column >= 0);

	self->cursor_x = column;
	self->cursor_y = line;
}

static inline void SPFGoto(SPFPanel* self, const int line, const int column)
{
	ASSERT(self);
	ASSERT(line > 0 && line <= self->height);
	ASSERT(column > 0 && column <= self->width);

	SPFScreenGoto(SPFGet(self), line, column);
}

static inline void SPFPutc(SPFPanel* self, const char c, const int sgr)
{
	ASSERT(self);

	SPFScreenPutc(SPFGet(self), c, sgr, 0);
}

static inline void SPFPrint(SPFPanel* self, const char* s, const int sgr)
{
	ASSERT(self);
	ASSERT(s);

	SPFScreenPrint(SPFGet(self), s, sgr);
}

static inline void SPFPrintAt(SPFPanel* self, const int line, const int column, const char* s, const int sgr)
{
	ASSERT(self);
	ASSERT(s);
	ASSERT(line > 0 && line <= self->height);
	ASSERT(column > 0 && column <= self->width);

	SPFGoto(self, line, column);
	SPFPrint(self, s, sgr);
}

static inline void SPFPrintCenter(SPFPanel* self, const int line, const char* s, const int sgr)
{
	ASSERT(self);
	ASSERT(line > 0 && line <= self->height);

	SPFScreenPrintCenter(SPFGet(self), line, s, sgr);
}

static inline void SPFPutcAt(SPFPanel* self, const int line, const int column, const char c, const int sgr)
{
	ASSERT(self);
	ASSERT(line > 0 && line <= self->height);
	ASSERT(column > 0 && column <= self->width);

	SPFGoto(self, line, column);
	SPFPutc(self, c, sgr);
}

static inline void SPFPrintNumber(SPFPanel* self, int val, const int width, const int align, const int sgr)
{
	ASSERT(self);

	SPFScreenPrintNumber(SPFGet(self), val, width, align, sgr);
}

static inline void SPFMoveRight(SPFPanel* self)
{
	ASSERT(self);

	SPFScreenCursorRight(SPFGet(self));
}

static inline void SPFMoveRightN(SPFPanel* self, int n)
{
	ASSERT(self);

	SPFScreenCursorRightN(SPFGet(self), n);
}

static inline void SPFFieldAt(SPFPanel* self, int line, int column, const char* text, int width, int sgr)
{
	ASSERT(self);

	SPFGoto(self, line, column);
	SPFScreenFill(SPFGet(self), 0x20, width, sgr);
	SPFPrintAt(self, line, column, text, sgr);
}

/******************************************************************************/
void	SPFCreateWidget(SPFWidget* widget, unsigned int type, unsigned int id);
void	SPFDrawWidget(SPFWidget* widget);
BOOL	SPFProcessWidget(SPFWidget* widget, u16 key);
void	SPFFocusWidget(SPFWidget* widget);
BOOL	SPFCanFocusWidget(SPFWidget* widget);

static inline unsigned int SPFGetWidgetID(SPFWidget* self)
{
	ASSERT(self);

	return self->id;
}

static inline void SPFSetWidgetPanel(SPFWidget* self, SPFPanel* panel)
{
	ASSERT(self);

	self->panel = panel;
}

static inline SPFPanel* SPFGetWidgetPanel(SPFWidget* self)
{
	ASSERT(self);

	return self->panel;
}

/******************************************************************************/
/* Label routines                                                             */
/******************************************************************************/
void	SPFCreateLabel(SPFLabel* label, unsigned int line, unsigned int col,
		const char* text, unsigned int flags);
void	SPFDrawLabel(SPFLabel* label);

/******************************************************************************/
/* Checkbox routines                                                          */
/******************************************************************************/
void	SPFCreateCheckbox(SPFCheckbox* self, unsigned int id,
		unsigned int line, unsigned int col,
		void (*callback)(SPFCheckbox*));
void	SPFDrawCheckbox(SPFCheckbox* self);
void	SPFFocusCheckbox(SPFCheckbox* self);
BOOL	SPFProcessCheckbox(SPFCheckbox* self, u16 key);

static inline BOOL SPFGetCheckboxState(SPFCheckbox* self)
{
	ASSERT(self);

	return self->state;
}

static inline void SPFSetCheckboxState(SPFCheckbox* self, BOOL state)
{
	ASSERT(self);

	self->state = state;

	/* panel can be NULL if the widget is not yet on a panel */
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	if(panel) {
		SPFRepaint(SPFGet(panel));
	}
}

/******************************************************************************/
/* Input (string) routines                                                    */
/******************************************************************************/
void	SPFCreateInputStr(SPFInputStr* self, unsigned int id, unsigned int line,
		unsigned int col, char* string, int len,
		void (*callback)(SPFInputStr*));
void	SPFDrawInputStr(SPFInputStr* self);
void	SPFFocusInputStr(SPFInputStr* self);
BOOL	SPFProcessInputStr(SPFInputStr* self, u16 key);

static inline char* SPFGetInputStrString(SPFInputStr* self)
{
	ASSERT(self);

	return self->string;
}

static inline void SPFSetInputStrString(SPFInputStr* self, char* string)
{
	ASSERT(self);

	self->string = string;

	/* panel can be NULL if the widget is not yet on a panel */
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	if(panel) {
		SPFRepaint(SPFGet(panel));
	}
}

/******************************************************************************/
/* Input (single character) routines                                          */
/******************************************************************************/
void	SPFCreateInputChar(SPFInputChar* self, unsigned int id, unsigned int line,
		unsigned int col, const char* allowed_chars, BOOL uppercase,
		void (*callback)(SPFInputChar*));
void	SPFDrawInputChar(SPFInputChar* self);
void	SPFFocusInputChar(SPFInputChar* self);
BOOL	SPFProcessInputChar(SPFInputChar* self, u16 key);

static inline char SPFGetInputCharValue(SPFInputChar* self)
{
	ASSERT(self);

	return self->value;
}

static inline void SPFSetInputCharValue(SPFInputChar* self, char value)
{
	ASSERT(self);

	self->value = value;

	/* panel can be NULL if the widget is not yet on a panel */
	SPFPanel* panel = SPFGetWidgetPanel(&self->widget);
	if(panel) {
		SPFRepaint(SPFGet(panel));
	}
}

/******************************************************************************/
/* Input (decimal) routines                                                   */
/******************************************************************************/
void	SPFCreateInputDec(SPFInputDec* self, unsigned int id, unsigned int line,
		unsigned int col, int len, int min, int max, int value,
		void (*callback)(SPFInputDec*));
void	SPFDrawInputDec(SPFInputDec* self);
void	SPFFocusInputDec(SPFInputDec* self);
BOOL	SPFProcessInputDec(SPFInputDec* self, u16 key);
int	SPFGetInputDecValue(SPFInputDec* self);
void	SPFSetInputDecValue(SPFInputDec* self, int value);
#endif
