#include <stdio.h>
#include <gm.h>

#include "capture.h"

GM_SURFACE surface_info;
uint32_t w;
uint32_t h;

int capture_init(uint32_t width, uint32_t height) {
	w = width;
	h = height;
	return GM_CreateSurface(width, height, 0, &surface_info);
}

int capture_execute(uint8_t* target, uint32_t size) {
	int ret;

	if ((ret = GM_CaptureGraphicScreen(surface_info.surfaceID, &w, &h)) != 0) {
		return ret;
	}

	memcpy(target, surface_info.framebuffer, size);

	return 0;
}

int capture_destroy() {
	return GM_DestroySurface(surface_info.surfaceID);
}
