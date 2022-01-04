#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <halgal.h>

#include "capture.h"

HAL_GAL_SURFACE surface_info;
int gfx_fd;
uint8_t* framebuffer_mem;

int capture_init(uint32_t width, uint32_t height) {
	int ret;

	if ((ret = HAL_GAL_Init()) != 0) {
		fprintf(stderr, "Unable to initialize HAL_GAL: %08x\n", ret);
		return -2;
	}

	if ((ret = HAL_GAL_CreateSurface(width, height, 0, &surface_info)) != 0) {
		fprintf(stderr, "HAL_GAL_CreateSurface failed: %08x", ret);
		return -3;
	}

	// Attempt one frame capture to verify this backend works properly
	if ((ret = HAL_GAL_CaptureFrameBuffer(&surface_info)) != 0) {
		fprintf(stderr, "HAL_GAL_CaptureFrameBuffer failed: %d\n", ret);
		return -5;
	}

	gfx_fd = open("/dev/gfx",2);
	if (gfx_fd < 0) {
		perror("/dev/gfx open failed");
		return -4;
	}

	int len = surface_info.property;

	if (len == 0){
		len = surface_info.height * surface_info.pitch;
	}

	framebuffer_mem = (uint8_t*) mmap(0, len, PROT_READ, MAP_SHARED, gfx_fd, surface_info.offset);

	return 0;
}

int capture_execute(uint8_t* target, uint32_t size) {
	int ret;
	if ((ret = HAL_GAL_CaptureFrameBuffer(&surface_info)) != 0) {
		fprintf(stderr, "HAL_GAL_CaptureFrameBuffer failed: %d\n", ret);
		return -5;
	}

	memcpy(target, framebuffer_mem, size);

	return 0;
}

int capture_destroy() {
	return HAL_GAL_DestroySurface(&surface_info);
}
