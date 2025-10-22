#ifndef VFS_H
#define VFS_H
#include "kernel.h"
#include "fat.h"
typedef struct {FatFs* fat;} Vfs;
typedef struct {FatFile file;} VfsFile;
void vfs_init_fat(Vfs* vfs,FatFs* fat);
int vfs_open(Vfs* vfs,const char* path,VfsFile* out);
int vfs_read(Vfs* vfs,VfsFile* f,void* buf,uint32_t size);
int vfs_write(Vfs* vfs,VfsFile* f,const void* buf,uint32_t size);
int vfs_write_file(Vfs* vfs,const char* path,const void* buf,uint32_t size);
int vfs_create(Vfs* vfs,const char* path,uint32_t size,VfsFile* out);
void vfs_close(Vfs* vfs,VfsFile* f);
int vfs_ls_root(Vfs* vfs);
#endif