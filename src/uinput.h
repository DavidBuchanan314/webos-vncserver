#pragma once

int initialize_uinput(void);
void shutdown_uinput(void);
void ptr_abs(int x, int y, int p);
void uinput_key_command(int down, int keysym);
int lookup_code(int keysym);
