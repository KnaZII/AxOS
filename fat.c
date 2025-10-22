#include "fat.h"
#include "string.h"
#include "console.h"
static uint8_t bsec[512];
static uint8_t fatbuf[8192];
static uint8_t rootbuf[8192];
static uint32_t fat_size;
static uint32_t read_root(FatFs* fs,uint8_t* dst){uint32_t l=fs->root_start;uint32_t n=fs->root_dir_sectors;uint32_t i=0;while(n){uint16_t c=n>127?127:n; if(!disk_read_lba(l,c,dst+i))return 0;l+=c;n-=c;i+=c*fs->bytes_per_sector;}return 1;}
static uint32_t write_root(FatFs* fs,const uint8_t* src){uint32_t l=fs->root_start;uint32_t n=fs->root_dir_sectors;uint32_t i=0;while(n){uint16_t c=n>127?127:n; if(!disk_write_lba(l,c,src+i))return 0;l+=c;n-=c;i+=c*fs->bytes_per_sector;}return 1;}
static void upper(char* s){for(;*s;s++){if(*s>='a'&&*s<='z')*s-=32;}}
static void name_to_83(const char* in,char* out){
    char base[9];char ext[4];int bi=0;int ei=0;int i=0;
    while(in[i]==' '||in[i]=='\t') i++;
    while(in[i] && in[i] != '.' && in[i] != ' ' && in[i] != '\t') { if (bi<8) base[bi++] = in[i]; i++; }
    if(in[i]=='.') i++;
    while(in[i] && in[i] != ' ' && in[i] != '\t') { if (ei<3) ext[ei++] = in[i]; i++; }
    base[bi]=0; ext[ei]=0; upper(base); upper(ext);
    int p=0; for(int k=0;k<8;k++){ out[p++] = (k<bi)?base[k]:' '; }
    for(int k=0;k<3;k++){ out[p++] = (k<ei)?ext[k]:' '; }
}
static uint32_t le16(const uint8_t* p){return p[0]|(p[1]<<8);} 
static uint32_t le32(const uint8_t* p){return p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24);} 
int fat_mount(FatFs* fs){if(!disk_read_lba(0,1,bsec))return 0;fs->bytes_per_sector=le16(&bsec[11]);fs->sectors_per_cluster=bsec[13];fs->reserved_sectors=le16(&bsec[14]);fs->num_fats=bsec[16];fs->root_entries=le16(&bsec[17]);uint32_t ts16=le16(&bsec[19]);uint32_t ts32=le32(&bsec[32]);fs->total_sectors=ts16?ts16:ts32;uint32_t spf16=le16(&bsec[22]);uint32_t spf32=le32(&bsec[36]);fs->sectors_per_fat=spf16?spf16:spf32;fs->root_dir_sectors=((fs->root_entries*32)+(fs->bytes_per_sector-1))/fs->bytes_per_sector;fs->fat_start=fs->reserved_sectors;fs->root_start=fs->fat_start+fs->num_fats*fs->sectors_per_fat;fs->data_start=fs->root_start+fs->root_dir_sectors;uint32_t data_secs=fs->total_sectors-(fs->reserved_sectors+fs->num_fats*fs->sectors_per_fat+fs->root_dir_sectors);uint32_t clusters=data_secs/fs->sectors_per_cluster;fs->fat_type=clusters<4085?12: (clusters<65525?16:32);fat_size=fs->sectors_per_fat*fs->bytes_per_sector;if(fat_size>sizeof(fatbuf))fat_size=sizeof(fatbuf);if(!disk_read_lba(fs->fat_start,(uint16_t)fs->sectors_per_fat,fatbuf))return 0;return 1;}
static uint32_t fat12_next(uint32_t c){uint32_t i=c+(c>>1);uint16_t v=fatbuf[i]|(fatbuf[i+1]<<8);return (c&1)?(v>>4):(v&0x0FFF);} 
static uint32_t fat16_next(uint32_t c){uint32_t i=c*2;return fatbuf[i]|(fatbuf[i+1]<<8);} 
static void fat12_set(uint32_t c,uint16_t val){uint32_t i=c+(c>>1);uint16_t v=fatbuf[i]|(fatbuf[i+1]<<8);if(c&1){v=(v&0x000F)|((val&0x0FFF)<<4);}else{v=(v&0xF000)|(val&0x0FFF);}fatbuf[i]=v&0xFF;fatbuf[i+1]=(v>>8)&0xFF;}
static void fat16_set(uint32_t c,uint16_t val){uint32_t i=c*2;fatbuf[i]=val&0xFF;fatbuf[i+1]=(val>>8)&0xFF;}
static uint16_t fat12_get(uint32_t c){uint32_t i=c+(c>>1);uint16_t v=fatbuf[i]|(fatbuf[i+1]<<8);return (c&1)?(v>>4):(v&0x0FFF);} 
static uint16_t fat16_get(uint32_t c){uint32_t i=c*2;return fatbuf[i]|(fatbuf[i+1]<<8);} 
static int fat_is_free(FatFs* fs,uint32_t c){return (fs->fat_type==12)?(fat12_get(c)==0):(fat16_get(c)==0);} 
static int fat_flush(FatFs* fs){if(!disk_write_lba(fs->fat_start,(uint16_t)fs->sectors_per_fat,fatbuf))return 0;for(uint8_t i=1;i<fs->num_fats;i++){uint32_t l=fs->fat_start+i*fs->sectors_per_fat;if(!disk_write_lba(l,(uint16_t)fs->sectors_per_fat,fatbuf))return 0;}return 1;}
static int fat_alloc_chain(FatFs* fs,uint32_t clusters,uint32_t* first_out,uint32_t* last_out){uint32_t data_secs=fs->total_sectors-(fs->reserved_sectors+fs->num_fats*fs->sectors_per_fat+fs->root_dir_sectors);uint32_t maxc=2+data_secs/fs->sectors_per_cluster;uint32_t first=0,last=0;for(uint32_t c=2;c<maxc && clusters; c++){if(fat_is_free(fs,c)){if(!first) first=c; if(last){ if(fs->fat_type==12) fat12_set(last,(uint16_t)c); else fat16_set(last,(uint16_t)c);} last=c; clusters--;}}
 if(clusters>0) return 0; if(fs->fat_type==12) fat12_set(last,0xFFF); else fat16_set(last,0xFFFF); *first_out=first; *last_out=last; return fat_flush(fs);} 
static void format_name(const uint8_t* e,char* out){int p=0;for(int i=0;i<8;i++){if(e[i]!=' ')out[p++]=e[i];}if(e[8]!=' '||e[9]!=' '||e[10]!=' '){out[p++]='.';for(int i=8;i<11;i++){if(e[i]!=' ')out[p++]=e[i];}}out[p]=0;} 
int fat_find(FatFs* fs,const char* name,FatFile* file){char n83[11];name_to_83(name,n83);uint32_t total=fs->root_dir_sectors*fs->bytes_per_sector;uint8_t* dir=rootbuf; if(!read_root(fs,dir))return 0;for(uint32_t off=0;off<total;off+=32){uint8_t* e=dir+off;uint8_t first=e[0];if(first==0x00)break;if(first==0xE5)continue;uint8_t attr=e[11];if(attr==0x0F)continue;int match=1;for(int i=0;i<11;i++){if(e[i]!=n83[i]){match=0;break;}}if(match){uint16_t cl=le16(&e[26]);uint32_t sz=le32(&e[28]);file->start_cluster=cl;file->size=sz;return 1;}}return 0;}
int fat_read_file(FatFs* fs,const FatFile* file,void* buf,uint32_t size){uint32_t c=file->start_cluster;uint32_t left=size;uint8_t* p=(uint8_t*)buf;while(left){uint32_t lba=fs->data_start+(c-2)*fs->sectors_per_cluster;uint32_t bytes=fs->sectors_per_cluster*fs->bytes_per_sector;uint32_t to=left<bytes?left:bytes;if(!disk_read_lba(lba,(uint16_t)fs->sectors_per_cluster,p))return 0;p+=to;left-=to;uint32_t nxt= fs->fat_type==12?fat12_next(c):fat16_next(c);if(fs->fat_type==12){if(nxt>=0xFF8)break;}else{if(nxt>=0xFFF8)break;}c=nxt;}return 1;}
static int update_dir_entry(FatFs* fs,const char* name83,uint16_t start,uint32_t size,int create){uint32_t total=fs->root_dir_sectors*fs->bytes_per_sector;uint8_t* dir=rootbuf; if(!read_root(fs,dir))return 0;uint8_t* slot=NULL;for(uint32_t off=0;off<total;off+=32){uint8_t* e=dir+off;uint8_t first=e[0];if(first==0x00){ if(slot==NULL) slot=e; break;} if(first==0xE5){ if(slot==NULL) slot=e; continue;} uint8_t attr=e[11];if(attr==0x0F) continue; int match=1; for(int i=0;i<11;i++){ if(e[i]!=name83[i]){ match=0; break; } } if(match && !create){ slot=e; break; } }
 if(slot==NULL) return 0; for(int i=0;i<11;i++) slot[i]=name83[i]; slot[11]=0x20; for(int i=12;i<26;i++) slot[i]=0; slot[26]= (uint8_t)(start & 0xFF); slot[27]=(uint8_t)((start>>8)&0xFF); slot[28]=(uint8_t)(size & 0xFF); slot[29]=(uint8_t)((size>>8)&0xFF); slot[30]=(uint8_t)((size>>16)&0xFF); slot[31]=(uint8_t)((size>>24)&0xFF);
 return write_root(fs,dir);
}
static int fat_free_chain(FatFs* fs,uint32_t first){ if(first<2) return 1; uint32_t c=first; while(1){ uint32_t nxt = (fs->fat_type==12)?fat12_next(c):fat16_next(c); if(fs->fat_type==12) fat12_set(c,0); else fat16_set(c,0); if( (fs->fat_type==12 && nxt>=0xFF8) || (fs->fat_type==16 && nxt>=0xFFF8) ) break; c=nxt; } return fat_flush(fs); }
int fat_create_or_overwrite(FatFs* fs,const char* name,uint32_t size,FatFile* file){char n83[11];name_to_83(name,n83);uint32_t clust_bytes=fs->bytes_per_sector*fs->sectors_per_cluster;uint32_t need=size? ( (size + clust_bytes -1)/clust_bytes ):1; uint32_t first=0,last=0; FatFile existing; if(fat_find(fs,name,&existing)){ if(existing.start_cluster>=2) fat_free_chain(fs,existing.start_cluster); if(!fat_alloc_chain(fs,need,&first,&last)) return 0; file->start_cluster=(uint16_t)first; file->size=size; if(!update_dir_entry(fs,n83,(uint16_t)first,size,0)) return 0; return 1; } else { if(!fat_alloc_chain(fs,need,&first,&last)) return 0; file->start_cluster=(uint16_t)first; file->size=size; if(!update_dir_entry(fs,n83,(uint16_t)first,size,1)) return 0; return 1; } }
int fat_write_file(FatFs* fs,const FatFile* file,const void* buf,uint32_t size){uint32_t c=file->start_cluster;const uint8_t* p=(const uint8_t*)buf;uint32_t left=size;uint32_t clust_bytes=fs->bytes_per_sector*fs->sectors_per_cluster;while(left){uint32_t lba=fs->data_start+(c-2)*fs->sectors_per_cluster;uint32_t to=left<clust_bytes?left:clust_bytes;uint32_t count=fs->sectors_per_cluster; if(!disk_write_lba(lba,(uint16_t)count,p)) return 0; p+=to; left-=to; uint32_t nxt= fs->fat_type==12?fat12_next(c):fat16_next(c); if(fs->fat_type==12){ if(nxt>=0xFF8) break; } else { if(nxt>=0xFFF8) break; } c=nxt; }
 // update size in dir entry
 char n83[11]; name_to_83("", n83); // not used here
 // we don't know name here; caller should update via create_or_overwrite already
 return 1; }
int fat_list_root(FatFs* fs){uint32_t total=fs->root_dir_sectors*fs->bytes_per_sector;uint8_t* dir=rootbuf;if(!read_root(fs,dir))return 0;for(uint32_t off=0;off<total;off+=32){uint8_t* e=dir+off;uint8_t first=e[0];if(first==0x00)break;if(first==0xE5)continue;uint8_t attr=e[11];if(attr==0x0F)continue;char name[14];format_name(e,name);console_print(name);console_print(" ");console_print_dec(le32(&e[28]));console_print("\n");}return 1;}