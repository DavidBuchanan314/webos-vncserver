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

#include "uinput.h"

const unsigned int screenwidth = 1920;
const unsigned int screenheight = 1080;
const unsigned int bpp = 4;
#define FBSIZE (screenwidth * screenheight * bpp)
#define DEFAULT_FBADDR 0x3acae000L

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
	ptr_abs(x, y, buttonMask & 1);
}

int main(int argc, char *argv[])
{
	off_t fbaddr = DEFAULT_FBADDR;
	
	if (argc > 1) {
		fbaddr = strtoul(argv[1], NULL, 0); // XXX: assume there will never be a fb at 0
		if (!fbaddr || argc != 2) {
			printf("USAGE: %s fb_addr (default: 0x%08lx)\n", argv[0], DEFAULT_FBADDR);
			return -1;
		}
	}
	
	printf("Framebuffer addr: 0x%08lx\n", fbaddr);
	
	rfbScreenInfoPtr screen = rfbGetScreen(&argc, argv, screenwidth, screenheight, 8, 3, bpp);
	assert(screen != NULL);
	
	assert(initialize_uinput() >= 0);
	
	signal(SIGINT, intHandler);
	
	int devmem = open("/dev/mem", O_RDONLY);
	screen->frameBuffer = mmap(NULL, FBSIZE, PROT_READ, MAP_SHARED, devmem, fbaddr);
	assert(screen->frameBuffer != MAP_FAILED);
	
	// switch red and blue channels
	int tmp = screen->serverFormat.redShift;
	screen->serverFormat.redShift = screen->serverFormat.blueShift;
	screen->serverFormat.blueShift = tmp;

	screen->kbdAddEvent = keyevent;
	screen->ptrAddEvent = ptrevent;
	
	rfbInitServer(screen);
	
	// Run event loop in background thread
	rfbRunEventLoop(screen, -1, TRUE);
	
	while (running) {
		usleep(1000000/30); // update at 30Hz
		// XXX: no synchronisation with GPU - we will get tearing/artefacting
		rfbMarkRectAsModified(screen, 0, 0, screenwidth, screenheight);
	}
	
	printf("\nCleaning up...\n");
	
	rfbShutdownServer(screen, TRUE);
	rfbScreenCleanup(screen);
	munmap(screen->frameBuffer, FBSIZE);
	close(devmem);
	shutdown_uinput();
	
	return 0;
}
