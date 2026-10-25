#include <string.h>

#include "spf.h"

void SPFCreateWidget(SPFWidget* widget, unsigned int type, unsigned int id)
{
	ASSERT(widget);

	memset(widget, 0, sizeof(SPFWidget));
	widget->type = type;
	widget->id = id;
}

void SPFDrawWidget(SPFWidget* widget)
{
	ASSERT(widget);

	switch(widget->type) {
		case SPF_WIDGET_CHECKBOX:
			SPFDrawCheckbox((SPFCheckbox*) widget);
			break;
		case SPF_WIDGET_INPUT_STR:
			SPFDrawInputStr((SPFInputStr*) widget);
			break;
		case SPF_WIDGET_INPUT_DEC:
			SPFDrawInputDec((SPFInputDec*) widget);
			break;
		case SPF_WIDGET_INPUT_CHR:
			SPFDrawInputChar((SPFInputChar*) widget);
			break;
		case SPF_WIDGET_LABEL:
			SPFDrawLabel((SPFLabel*) widget);
			break;
		case SPF_WIDGET_DYNAMIC_LBL:
			break;
	}
}

/* TODO: improve this. For example, right now it only selects some widget on
 * the next/previous line, while completely ignoring if this widget is close to
 * the cursor position in case there are multiple widgets on the same line */
BOOL SPFiWidgetKeyUp(SPFWidget* widget)
{
	SPFPanel* panel = widget->panel;
	SPFWidget* focus = NULL;
	int line = widget->pos_y;
	int focus_line = 0;
	int focus_dst = line - focus_line;
	ASSERT(focus_dst >= 0);

	for(SPFWidget** w = panel->widgets; *w; w++) {
		if(*w != widget && SPFCanFocusWidget(*w) && (*w)->pos_y < line) {
			int dst = line - (*w)->pos_y;
			ASSERT(dst >= 0);
			if(dst < focus_dst) {
				focus = *w;
				focus_dst = dst;
			}
			if(dst == 0) {
				break;
			}
		}
	}

	if(focus) {
		SPFSetFocus(panel, focus);
		SPFFocusWidget(panel->focus);
		VT220ResetCursorTime(SPFGetVT(SPFGet(panel)));
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL SPFiWidgetKeyDown(SPFWidget* widget)
{
	SPFPanel* panel = widget->panel;
	SPFWidget* focus = NULL;
	int line = widget->pos_y;
	int focus_line = SPFGetHeight(panel);
	int focus_dst = focus_line - line;
	ASSERT(focus_dst >= 0);

	for(SPFWidget** w = panel->widgets; *w; w++) {
		if(*w != widget && SPFCanFocusWidget(*w) && (*w)->pos_y > line) {
			int dst = (*w)->pos_y - line;
			ASSERT(dst >= 0);
			if(dst < focus_dst) {
				focus = *w;
				focus_dst = dst;
			}
			if(dst == 0) {
				break;
			}
		}
	}

	if(focus) {
		SPFSetFocus(panel, focus);
		SPFFocusWidget(panel->focus);
		VT220ResetCursorTime(SPFGetVT(SPFGet(panel)));
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL SPFProcessWidget(SPFWidget* widget, u16 key)
{
	ASSERT(widget);

	switch(key) {
		case HT: {
			/* find next widget */
			SPFPanel* panel = widget->panel;
			BOOL found = FALSE;
			SPFWidget* first = NULL;
			SPFWidget* focus = NULL;
			for(SPFWidget** w = panel->widgets; *w; w++) {
				if(!first && SPFCanFocusWidget(*w)) {
					first = *w;
				}

				if(widget == *w) {
					found = TRUE;
				} else if(found && SPFCanFocusWidget(*w)) {
					focus = *w;
					break;
				}
			}

			if(focus) {
				SPFSetFocus(panel, focus);
			} else {
				SPFSetFocus(panel, first);
			}

			/* focus can never be NULL here */
			SPFFocusWidget(panel->focus);

			return TRUE;
		}
		case VT220_KEY_UP:
			 if(SPFiWidgetKeyUp(widget)) {
				 return TRUE;
			 }
			 break;
		case VT220_KEY_DOWN:
			 if(SPFiWidgetKeyDown(widget)) {
				 return TRUE;
			 }
			 break;
	}

	switch(widget->type) {
		case SPF_WIDGET_CHECKBOX:
			return SPFProcessCheckbox((SPFCheckbox*) widget, key);
		case SPF_WIDGET_INPUT_STR:
			return SPFProcessInputStr((SPFInputStr*) widget, key);
		case SPF_WIDGET_INPUT_DEC:
			return SPFProcessInputDec((SPFInputDec*) widget, key);
		case SPF_WIDGET_INPUT_CHR:
			return SPFProcessInputChar((SPFInputChar*) widget, key);
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
		default:
			return FALSE;
	}
}

void SPFFocusWidget(SPFWidget* widget)
{
	ASSERT(widget);

	switch(widget->type) {
		case SPF_WIDGET_CHECKBOX:
			SPFFocusCheckbox((SPFCheckbox*) widget);
			break;
		case SPF_WIDGET_INPUT_STR:
			SPFFocusInputStr((SPFInputStr*) widget);
			break;
		case SPF_WIDGET_INPUT_DEC:
			SPFFocusInputDec((SPFInputDec*) widget);
			break;
		case SPF_WIDGET_INPUT_CHR:
			SPFFocusInputChar((SPFInputChar*) widget);
			break;
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
			break;
	}
}

BOOL SPFCanFocusWidget(SPFWidget* widget)
{
	ASSERT(widget);

	switch(widget->type) {
		case SPF_WIDGET_CHECKBOX:
		case SPF_WIDGET_INPUT_STR:
		case SPF_WIDGET_INPUT_DEC:
		case SPF_WIDGET_INPUT_CHR:
			return TRUE;
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
		default:
			return FALSE;
	}
}
