#pragma once
#include <stdint.h>

int capture_init(uint32_t width, uint32_t height);
int capture_execute(uint8_t* target, uint32_t size);
int capture_destroy();
