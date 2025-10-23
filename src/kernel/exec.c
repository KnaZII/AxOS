#include "exec.h"
#include "vfs.h"
#include "user.h"
#include "paging.h"
#include "string.h"
#include "console.h"
#include "task.h"

typedef struct {unsigned char e_ident[16];uint16_t e_type;uint16_t e_machine;uint32_t e_version;uint32_t e_entry;uint32_t e_phoff;uint32_t e_shoff;uint32_t e_flags;uint16_t e_ehsize;uint16_t e_phentsize;uint16_t e_phnum;uint16_t e_shentsize;uint16_t e_shnum;uint16_t e_shstrndx;} Elf32_Ehdr;
typedef struct {uint32_t p_type;uint32_t p_offset;uint32_t p_vaddr;uint32_t p_paddr;uint32_t p_filesz;uint32_t p_memsz;uint32_t p_flags;uint32_t p_align;} Elf32_Phdr;

extern Vfs g_vfs;
static unsigned char buf[262144];

static int load_elf_to_memory(const unsigned char* img,uint32_t size,uint32_t* out_entry,uint32_t* out_stack,uint32_t* out_pd){if(size<sizeof(Elf32_Ehdr))return 0;const Elf32_Ehdr* eh=(const Elf32_Ehdr*)img;if(!(eh->e_ident[0]==0x7F&&eh->e_ident[1]=='E'&&eh->e_ident[2]=='L'&&eh->e_ident[3]=='F'))return 0;if(eh->e_ident[4]!=1)return 0;if(eh->e_phnum==0)return 0;uint32_t* pd=paging_new_pd();uint32_t stack_top=0x00400000-0x1000;uint32_t stack_size=0x00010000;paging_mark_user(pd,0,0x00400000);paging_mark_user(pd,stack_top-stack_size,stack_size);for(uint16_t i=0;i<eh->e_phnum;i++){uint32_t off=eh->e_phoff+i*eh->e_phentsize;if(off+sizeof(Elf32_Phdr)>size)return 0;const Elf32_Phdr* ph=(const Elf32_Phdr*)(img+off);if(ph->p_type!=1)continue;uint32_t v=ph->p_vaddr;uint32_t fsz=ph->p_filesz;uint32_t msz=ph->p_memsz;uint32_t src_off=ph->p_offset;if(src_off+fsz>size)return 0;if(v+msz>0x00400000)return 0;memcpy((void*)v,img+src_off,fsz);if(msz>fsz){memset((void*)(v+fsz),0,msz-fsz);} }*out_entry=eh->e_entry;*out_stack=stack_top;*out_pd=(uint32_t)pd;return 1;}

int exec(const char* path){VfsFile f;if(!vfs_open(&g_vfs,path,&f))return 0;if(f.file.size>sizeof(buf))return 0;if(!vfs_read(&g_vfs,&f,buf,f.file.size))return 0;uint32_t entry=0,stack=0,pd_ptr=0;int ok=load_elf_to_memory(buf,f.file.size,&entry,&stack,&pd_ptr);if(!ok)return 0;task_set_current_pd((uint32_t*)pd_ptr);paging_switch((uint32_t*)pd_ptr);enter_user_mode(entry,stack);return 1;}