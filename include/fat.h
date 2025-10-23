#ifndef FAT_H
#define FAT_H
#include "kernel.h"
#include "disk.h"
typedef struct {uint16_t bytes_per_sector;uint8_t sectors_per_cluster;uint16_t reserved_sectors;uint8_t num_fats;uint16_t root_entries;uint32_t total_sectors;uint32_t sectors_per_fat;uint32_t fat_start;uint32_t root_start;uint32_t data_start;uint32_t root_dir_sectors;uint8_t fat_type;} FatFs;
typedef struct {uint32_t start_cluster;uint32_t size;} FatFile;
int fat_mount(FatFs* fs);
int fat_find(FatFs* fs,const char* name,FatFile* file);
int fat_read_file(FatFs* fs,const FatFile* file,void* buf,uint32_t size);
int fat_write_file(FatFs* fs,const FatFile* file,const void* buf,uint32_t size);
int fat_create_or_overwrite(FatFs* fs,const char* name,uint32_t size,FatFile* file);
int fat_list_root(FatFs* fs);
#endif