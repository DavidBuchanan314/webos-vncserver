#include <stdio.h>
#include "rfb/rfb.h"
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <gm.h>

#include "uinput.h"

GM_SURFACE surfaceinfo;
unsigned int screenwidth = 1920;
unsigned int screenheight = 1080;
const unsigned int bpp = 4;

unsigned int nativewidth = 0;
unsigned int nativeheight = 0;

#define FBSIZE (screenwidth * screenheight * bpp)

int running = 1;

void intHandler(int dummy) {
	running = 0;
}

static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
	printf("keyevent 0x%08x 0x%08x\n", down, key);
	uinput_key_command(down, key);
}

static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl)
{
	printf("ptrevent 0x%08x %d %d\n", buttonMask, x, y);
	ptr_abs(x * nativewidth / screenwidth, y * nativeheight / screenheight, buttonMask & 1);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		screenwidth = strtoul(argv[1], NULL, 0);
		screenheight = strtoul(argv[2], NULL, 0);
		if (!screenwidth || !screenheight || argc != 3) {
			printf("USAGE: %s [width height]\n", argv[0]);
			return -1;
		}
	}

	if (GM_GetGraphicResolution(&nativewidth, &nativeheight) != 0) {
		fprintf(stderr, "Unable to get native screen resolution\n");
		return -1;
	}

	printf("Native resolution: %dx%d\n", nativewidth, nativeheight);

	if (screenwidth > nativewidth || screenheight > nativeheight) {
		fprintf(stderr, "Requested resolution (%dx%d) too high!\n", screenwidth, screenheight);
		return -1;
	}

	rfbScreenInfoPtr screen = rfbGetScreen(&argc, argv, screenwidth, screenheight, 8, 3, bpp);
	assert(screen != NULL);

	assert(initialize_uinput() >= 0);
	assert(GM_CreateSurface(screenwidth, screenheight, 0, &surfaceinfo) == 0);

	signal(SIGINT, intHandler);

	// switch red and blue channels
	int tmp = screen->serverFormat.redShift;
	screen->serverFormat.redShift = screen->serverFormat.blueShift;
	screen->serverFormat.blueShift = tmp;

	uint8_t framebuffer[FBSIZE];

	screen->kbdAddEvent = keyevent;
	screen->ptrAddEvent = ptrevent;
	screen->frameBuffer = framebuffer;

	rfbInitServer(screen);

	// Run event loop in background thread
	rfbRunEventLoop(screen, -1, TRUE);

	while (running) {
		assert(GM_CaptureGraphicScreen(surfaceinfo.surfaceID, &screenwidth, &screenheight) == 0);
		memcpy(framebuffer, surfaceinfo.framebuffer, FBSIZE);
		rfbMarkRectAsModified(screen, 0, 0, screenwidth, screenheight);
		usleep(1000000/30); // update at 30Hz
	}

	printf("\nCleaning up...\n");

	GM_DestroySurface(surfaceinfo.surfaceID);
	rfbShutdownServer(screen, TRUE);
	rfbScreenCleanup(screen);
	shutdown_uinput();

	return 0;
}
