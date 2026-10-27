#include <stdlib.h>
#include <string.h>

#include "spf.h"
#include "cm.h"

typedef struct {
	const char*	title;
	const char*	message;
	void*		userdata;
	void		(*callback)(void* userdata, BOOL result);

	SPFLabel	question;
	SPFLabel	bracket_1;
	SPFLabel	bracket_2;
	SPFInputChar	answer;
	SPFWidget*	widgets[5];
} CMMessageBox;

void CMMessageBoxCreate(SPFPanel* self)
{
	CMMessageBox* this = (CMMessageBox*) malloc(sizeof(CMMessageBox));
	ASSERT(this);
	memset(this, 0, sizeof(CMMessageBox));
	SPFSetPanelUserData(self, (void*) this);

	SPFSetDialog(self);
}

void CMMessageBoxDestroy(SPFPanel* self)
{
	CMMessageBox* this = (CMMessageBox*) SPFGetPanelUserData(self);

	free(this);
}

void CMMessageBoxInitWidgets(SPFPanel* self)
{
	CMMessageBox* this = (CMMessageBox*) SPFGetPanelUserData(self);

	size_t msglen = strlen(this->message);
	size_t textlen = msglen + 4;

	if(!this->callback) {
		textlen = msglen;
	}

	size_t lines = 3;
	size_t columns = textlen + 4;
	SPFSetSize(self, lines, columns);

	SPFCreateLabel(&this->question, 2, 3, this->message, 0);
	SPFCreateLabel(&this->bracket_1, 2, msglen + 4, "[", 0);
	SPFCreateLabel(&this->bracket_2, 2, msglen + 6, "]", 0);
	SPFCreateInputChar(&this->answer, 0, 2, msglen + 5, "NY", TRUE, NULL);

	SPFSetInputCharValue(&this->answer, 'N');

	if(this->callback) {
		this->widgets[0] = &this->question.widget;
		this->widgets[1] = &this->bracket_1.widget;
		this->widgets[2] = &this->answer.widget;
		this->widgets[3] = &this->bracket_2.widget;
		this->widgets[4] = NULL;
		SPFShowCursor(self);
	} else {
		this->widgets[0] = &this->question.widget;
		this->widgets[1] = NULL;
		SPFHideCursor(self);
	}

	SPFSetWidgets(self, this->widgets);

	SPFSetPanelTitle(self, this->title);
}

void CMMessageBoxProcess(SPFPanel* self, u16 key)
{
	CMMessageBox* this = (CMMessageBox*) SPFGetPanelUserData(self);

	switch(key) {
		case CR:
		case LF:
			if(this->callback) {
				char c = SPFGetInputCharValue(&this->answer);
				void* userdata = this->userdata;
				void (*callback)(void*, BOOL) = this->callback;
				SPFDestroyPanel(self);
				callback(userdata, c == 'Y');
			} else {
				SPFDestroyPanel(self);
			}
			break;
		case 0xFFFF: /* this is the no-op value */
			break;
		default:
			if(!this->callback) {
				SPFDestroyPanel(self);
			}
			break;
	}
}

static SPFPanelDefinition message_box = {
	.init = CMMessageBoxCreate,
	.draw = NULL,
	.resize = NULL,
	.process = CMMessageBoxProcess,
	.destroy = CMMessageBoxDestroy
};

void CMiCreateMessageBox(CM* self, const char* title, const char* message, void (*callback)(void*, BOOL), void* userdata)
{
	SPFPanel* panel = SPFCreatePanel(&self->spf, &message_box);
	CMMessageBox* msgbox = (CMMessageBox*) SPFGetPanelUserData(panel);
	msgbox->title = title;
	msgbox->message = message;
	msgbox->callback = callback;
	msgbox->userdata = userdata;

	CMMessageBoxInitWidgets(panel);
}
