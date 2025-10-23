#include "vfs.h"
void vfs_init_fat(Vfs* vfs,FatFs* fat){vfs->fat=fat;}
int vfs_open(Vfs* vfs,const char* path,VfsFile* out){return fat_find(vfs->fat,path,&out->file);} 
int vfs_read(Vfs* vfs,VfsFile* f,void* buf,uint32_t size){return fat_read_file(vfs->fat,&f->file,buf,size);} 
int vfs_write(Vfs* vfs,VfsFile* f,const void* buf,uint32_t size){return fat_write_file(vfs->fat,&f->file,buf,size);} 
int vfs_write_file(Vfs* vfs,const char* path,const void* buf,uint32_t size){FatFile f; if(!fat_create_or_overwrite(vfs->fat,path,size,&f)) return 0; return fat_write_file(vfs->fat,&f,buf,size);} 
int vfs_create(Vfs* vfs,const char* path,uint32_t size,VfsFile* out){return fat_create_or_overwrite(vfs->fat,path,size,&out->file);} 
void vfs_close(Vfs* vfs,VfsFile* f){(void)vfs;(void)f;} 
int vfs_ls_root(Vfs* vfs){return fat_list_root(vfs->fat);}