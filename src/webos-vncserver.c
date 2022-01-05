#include <stdio.h>
#include <rfb/rfb.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#include "capture.h"
#include "uinput.h"

unsigned int screenwidth = 1920;
unsigned int screenheight = 1080;
const unsigned int bpp = 4;

unsigned int nativewidth = 1920;
unsigned int nativeheight = 1080;

#define FBSIZE (screenwidth * screenheight * bpp)

int running = 1;
int activeClients = 0;

void intHandler(int dummy) {
	running = 0;
}

static void clientGoneEvent(rfbClientPtr cl) {
	printf("client closed!\n");
	activeClients -= 1;
}

static enum rfbNewClientAction newClientEvent(rfbClientPtr cl) {
	printf("new client!\n");
	activeClients += 1;
	cl->clientGoneHook = &clientGoneEvent;
	return RFB_CLIENT_ACCEPT;
}

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl) {
	uinput_key_command(down, key);
}

static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl) {
	ptr_abs(x * nativewidth / screenwidth, y * nativeheight / screenheight, buttonMask);
}

int main(int argc, char *argv[]) {
	int ret;

	if (argc > 1) {
		screenwidth = strtoul(argv[1], NULL, 0);
		screenheight = strtoul(argv[2], NULL, 0);
		if (!screenwidth || !screenheight || argc != 3) {
			printf("USAGE: %s [width height]\n", argv[0]);
			return -1;
		}
	}

	if ((ret = capture_init(screenwidth, screenheight)) != 0) {
		return -2;
	}

	printf("Native resolution: %dx%d\n", nativewidth, nativeheight);

	if (screenwidth > nativewidth || screenheight > nativeheight) {
		fprintf(stderr, "Requested resolution (%dx%d) too high!\n", screenwidth, screenheight);
		return -1;
	}

	rfbScreenInfoPtr screen = rfbGetScreen(&argc, argv, screenwidth, screenheight, 8, 3, bpp);
	assert(screen != NULL);

	screen->newClientHook = newClientEvent;

	assert(initialize_uinput() >= 0);

	signal(SIGINT, intHandler);

	// switch red and blue channels
	int tmp = screen->serverFormat.redShift;
	screen->serverFormat.redShift = screen->serverFormat.blueShift;
	screen->serverFormat.blueShift = tmp;

	uint8_t framebuffer[FBSIZE];

	screen->kbdAddEvent = keyevent;
	screen->ptrAddEvent = ptrevent;
	screen->frameBuffer = framebuffer;

	static const char* passwords[2]={"secret",0};
	screen->authPasswdData = (void*)passwords;
	screen->passwordCheck = rfbCheckPasswordByList;

	rfbInitServer(screen);

	// Run event loop in background thread
	rfbRunEventLoop(screen, -1, TRUE);

	while (running) {
		if (activeClients > 0) {
			if ((ret = capture_execute(framebuffer, FBSIZE)) != 0) {
				fprintf(stderr, "capture failed: %08x\n", ret);
				return -5;
			}

			rfbMarkRectAsModified(screen, 0, 0, screenwidth, screenheight);
		}

		usleep(1000000/30); // update at 30Hz
	}

	printf("\nCleaning up...\n");

	capture_destroy();
	rfbShutdownServer(screen, TRUE);
	rfbScreenCleanup(screen);
	shutdown_uinput();

	return 0;
}
