#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include "uinput.h"
#include "im.h"

static int fd = 0;

// x11vnc/src/uinput.c
// TODO: proper error handling/cleanup
int initialize_uinput(void)
{
	int i;
	char *s;
	struct uinput_user_dev udev;

	//if (fd >= 0) {
	//	shutdown_uinput();
	//}

	fd = open("/dev/uinput", O_WRONLY | O_NDELAY);

	if (fd < 0) {
		printf("Failed to open uinput dev\n");
		return -1;
	}

	memset(&udev, 0, sizeof(udev));

	strncpy(udev.name, "vramvnc injector", UINPUT_MAX_NAME_SIZE);

	udev.id.bustype = BUS_USB;
	udev.id.version = 4;

	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_RELBIT, REL_X);
	ioctl(fd, UI_SET_RELBIT, REL_Y);

	ioctl(fd, UI_SET_EVBIT, EV_KEY);

	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	for (i=0; i < 256; i++) {
		ioctl(fd, UI_SET_KEYBIT, i);
	}

	ioctl(fd, UI_SET_KEYBIT, BTN_MOUSE);
	ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
	ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
	ioctl(fd, UI_SET_KEYBIT, BTN_FORWARD);
	ioctl(fd, UI_SET_KEYBIT, BTN_BACK);


	//if (uinput_touchscreen) {
		ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH);
	//}
	//if (uinput_touchscreen || uinput_abs) {
		ioctl(fd, UI_SET_EVBIT, EV_ABS);
		ioctl(fd, UI_SET_ABSBIT, ABS_X);
		ioctl(fd, UI_SET_ABSBIT, ABS_Y);
		udev.absmin[ABS_X] = 0;
		udev.absmax[ABS_X] = 1920; // XXX: hardcoded
		udev.absfuzz[ABS_X] = 0;
		udev.absflat[ABS_X] = 0;
		udev.absmin[ABS_Y] = 0;
		udev.absmax[ABS_Y] = 1080; // XXX: hardcoded
		udev.absfuzz[ABS_Y] = 0;
		udev.absflat[ABS_Y] = 0;
		//set_uinput_accel_xy(1.0, 1.0);
	//}


	write(fd, &udev, sizeof(udev));

	if (ioctl(fd, UI_DEV_CREATE) != 0) {
		printf("UI_DEV_CREATE ioctl failed\n");
		return -1;
	}

	if (IM_RequestToCheckInput() < 0) {
		printf("IM_RequestToCheckInput() failed\n");
		return -1;
	}

	return 0;
}

// x11vnc/src/uinput.c
void shutdown_uinput(void) {
	printf("Shutting down uinput...\n");
	if (fd >= 0) {
		ioctl(fd, UI_DEV_DESTROY);
		close(fd);
		fd = -1;
	}
}

// x11vnc/src/uinput.c
void ptr_abs(int x, int y, int p) {
	struct input_event ev;
	struct timeval tval;
	int x0, y0;

	memset(&ev, 0, sizeof(ev));

	gettimeofday(&tval, NULL);
	ev.time.tv_sec = tval.tv_sec;
	ev.time.tv_usec = tval.tv_usec;
	ev.type = EV_ABS;
	ev.code = ABS_Y;
	ev.value = y;
	write(fd, &ev, sizeof(ev));

	ev.type = EV_ABS;
	ev.code = ABS_X;
	ev.value = x;
	write(fd, &ev, sizeof(ev));

	if (p >= 0) {
		ev.type = EV_ABS;
		ev.code = ABS_PRESSURE;
		ev.value = p;
		write(fd, &ev, sizeof(ev));
		
		//if (!btn_touch) {
			ev.type = EV_KEY;
			ev.code = BTN_LEFT;
			ev.value = p ? 1 : 0;
			write(fd, &ev, sizeof(ev));
			
		//	ev.type = EV_KEY;
		//	ev.code = BTN_MOUSE;
		//	ev.value = 0;
		//	write(fd, &ev, sizeof(ev));
		//}
	}

	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	write(fd, &ev, sizeof(ev));
}

// x11vnc/src/uinput.c
void uinput_key_command(int down, int keysym) {
	struct input_event ev;
	struct timeval tval;
	int scancode;

	scancode = lookup_code(keysym);

	if (scancode < 0) {
		return;
	}

	memset(&ev, 0, sizeof(ev));
	gettimeofday(&tval, NULL);
	ev.time.tv_sec = tval.tv_sec;
	ev.time.tv_usec = tval.tv_usec;
	ev.type = EV_KEY;
	ev.code = (unsigned char) scancode;
	ev.value = down;

	write(fd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	write(fd, &ev, sizeof(ev));
}

// x11vnc/src/uinput.c
int lookup_code(int keysym) {

	if (keysym == NoSymbol) {
		return -1;
	}

	switch(keysym) {
	case XK_Escape:	return KEY_ESC;
	case XK_1:		return KEY_1;
	case XK_2:		return KEY_2;
	case XK_3:		return KEY_3;
	case XK_4:		return KEY_4;
	case XK_5:		return KEY_5;
	case XK_6:		return KEY_6;
	case XK_7:		return KEY_7;
	case XK_8:		return KEY_8;
	case XK_9:		return KEY_9;
	case XK_0:		return KEY_0;
	case XK_exclam:	return KEY_1;
	case XK_at:		return KEY_2;
	case XK_numbersign:	return KEY_3;
	case XK_dollar:	return KEY_4;
	case XK_percent:	return KEY_5;
	case XK_asciicircum:	return KEY_6;
	case XK_ampersand:	return KEY_7;
	case XK_asterisk:	return KEY_8;
	case XK_parenleft:	return KEY_9;
	case XK_parenright:	return KEY_0;
	case XK_minus:	return KEY_MINUS;
	case XK_underscore:	return KEY_MINUS;
	case XK_equal:	return KEY_EQUAL;
	case XK_plus:	return KEY_EQUAL;
	case XK_BackSpace:	return KEY_BACKSPACE;
	case XK_Tab:		return KEY_TAB;
	case XK_q:		return KEY_Q;
	case XK_Q:		return KEY_Q;
	case XK_w:		return KEY_W;
	case XK_W:		return KEY_W;
	case XK_e:		return KEY_E;
	case XK_E:		return KEY_E;
	case XK_r:		return KEY_R;
	case XK_R:		return KEY_R;
	case XK_t:		return KEY_T;
	case XK_T:		return KEY_T;
	case XK_y:		return KEY_Y;
	case XK_Y:		return KEY_Y;
	case XK_u:		return KEY_U;
	case XK_U:		return KEY_U;
	case XK_i:		return KEY_I;
	case XK_I:		return KEY_I;
	case XK_o:		return KEY_O;
	case XK_O:		return KEY_O;
	case XK_p:		return KEY_P;
	case XK_P:		return KEY_P;
	case XK_braceleft:	return KEY_LEFTBRACE;
	case XK_braceright:	return KEY_RIGHTBRACE;
	case XK_bracketleft:	return KEY_LEFTBRACE;
	case XK_bracketright:	return KEY_RIGHTBRACE;
	case XK_Return:	return KEY_ENTER;
	case XK_Control_L:	return KEY_LEFTCTRL;
	case XK_a:		return KEY_A;
	case XK_A:		return KEY_A;
	case XK_s:		return KEY_S;
	case XK_S:		return KEY_S;
	case XK_d:		return KEY_D;
	case XK_D:		return KEY_D;
	case XK_f:		return KEY_F;
	case XK_F:		return KEY_F;
	case XK_g:		return KEY_G;
	case XK_G:		return KEY_G;
	case XK_h:		return KEY_H;
	case XK_H:		return KEY_H;
	case XK_j:		return KEY_J;
	case XK_J:		return KEY_J;
	case XK_k:		return KEY_K;
	case XK_K:		return KEY_K;
	case XK_l:		return KEY_L;
	case XK_L:		return KEY_L;
	case XK_semicolon:	return KEY_SEMICOLON;
	case XK_colon:	return KEY_SEMICOLON;
	case XK_apostrophe:	return KEY_APOSTROPHE;
	case XK_quotedbl:	return KEY_APOSTROPHE;
	case XK_grave:	return KEY_GRAVE;
	case XK_asciitilde:	return KEY_GRAVE;
	case XK_Shift_L:	return KEY_LEFTSHIFT;
	case XK_backslash:	return KEY_BACKSLASH;
	case XK_bar:		return KEY_BACKSLASH;
	case XK_z:		return KEY_Z;
	case XK_Z:		return KEY_Z;
	case XK_x:		return KEY_X;
	case XK_X:		return KEY_X;
	case XK_c:		return KEY_C;
	case XK_C:		return KEY_C;
	case XK_v:		return KEY_V;
	case XK_V:		return KEY_V;
	case XK_b:		return KEY_B;
	case XK_B:		return KEY_B;
	case XK_n:		return KEY_N;
	case XK_N:		return KEY_N;
	case XK_m:		return KEY_M;
	case XK_M:		return KEY_M;
	case XK_comma:	return KEY_COMMA;
	case XK_less:	return KEY_COMMA;
	case XK_period:	return KEY_DOT;
	case XK_greater:	return KEY_DOT;
	case XK_slash:	return KEY_SLASH;
	case XK_question:	return KEY_SLASH;
	case XK_Shift_R:	return KEY_RIGHTSHIFT;
	case XK_KP_Multiply:	return KEY_KPASTERISK;
	case XK_Alt_L:	return KEY_LEFTALT;
	case XK_space:	return KEY_SPACE;
	case XK_Caps_Lock:	return KEY_CAPSLOCK;
	case XK_F1:		return KEY_F1;
	case XK_F2:		return KEY_F2;
	case XK_F3:		return KEY_F3;
	case XK_F4:		return KEY_F4;
	case XK_F5:		return KEY_F5;
	case XK_F6:		return KEY_F6;
	case XK_F7:		return KEY_F7;
	case XK_F8:		return KEY_F8;
	case XK_F9:		return KEY_F9;
	case XK_F10:		return KEY_F10;
	case XK_Num_Lock:	return KEY_NUMLOCK;
	case XK_Scroll_Lock:	return KEY_SCROLLLOCK;
	case XK_KP_7:		return KEY_KP7;
	case XK_KP_8:		return KEY_KP8;
	case XK_KP_9:		return KEY_KP9;
	case XK_KP_Subtract:	return KEY_KPMINUS;
	case XK_KP_4:		return KEY_KP4;
	case XK_KP_5:		return KEY_KP5;
	case XK_KP_6:		return KEY_KP6;
	case XK_KP_Add:	return KEY_KPPLUS;
	case XK_KP_1:		return KEY_KP1;
	case XK_KP_2:		return KEY_KP2;
	case XK_KP_3:		return KEY_KP3;
	case XK_KP_0:		return KEY_KP0;
	case XK_KP_Decimal:	return KEY_KPDOT;
	case XK_F13:		return KEY_F13;
	case XK_F11:		return KEY_F11;
	case XK_F12:		return KEY_F12;
	case XK_F14:		return KEY_F14;
	case XK_F15:		return KEY_F15;
	case XK_F16:		return KEY_F16;
	case XK_F17:		return KEY_F17;
	case XK_F18:		return KEY_F18;
	case XK_F19:		return KEY_F19;
	case XK_F20:		return KEY_F20;
	case XK_KP_Enter:	return KEY_KPENTER;
	case XK_Control_R:	return KEY_RIGHTCTRL;
	case XK_KP_Divide:	return KEY_KPSLASH;
	case XK_Sys_Req:	return KEY_SYSRQ;
	case XK_Alt_R:	return KEY_RIGHTALT;
	case XK_Linefeed:	return KEY_LINEFEED;
	case XK_Home:		return KEY_HOME;
	case XK_Up:		return KEY_UP;
	case XK_Page_Up:	return KEY_PAGEUP;
	case XK_Left:		return KEY_LEFT;
	case XK_Right:	return KEY_RIGHT;
	case XK_End:		return KEY_END;
	case XK_Down:		return KEY_DOWN;
	case XK_Page_Down:	return KEY_PAGEDOWN;
	case XK_Insert:	return KEY_INSERT;
	case XK_Delete:	return KEY_DELETE;
	case XK_KP_Equal:	return KEY_KPEQUAL;
	case XK_Pause:	return KEY_PAUSE;
	case XK_F21:		return KEY_F21;
	case XK_F22:		return KEY_F22;
	case XK_F23:		return KEY_F23;
	case XK_F24:		return KEY_F24;
	case XK_KP_Separator:	return KEY_KPCOMMA;
	case XK_Meta_L:	return KEY_LEFTMETA;
	case XK_Meta_R:	return KEY_RIGHTMETA;
	case XK_Multi_key:	return KEY_COMPOSE;
	case XK_Super_L:	return KEY_LEFTMETA; // win key
	default:		return -1;
	}
}
