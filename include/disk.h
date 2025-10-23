#ifndef DISK_H
#define DISK_H
#include "kernel.h"
int disk_read_lba(uint32_t lba, uint16_t count, void* buffer);
int disk_write_lba(uint32_t lba, uint16_t count, const void* buffer);
#endif