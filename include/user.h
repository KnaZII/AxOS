#ifndef USER_H
#define USER_H
#include "kernel.h"
void enter_user_mode(uint32_t entry, uint32_t user_stack);
#endif