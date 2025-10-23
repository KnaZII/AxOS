#include "kernel.h"
#include "console.h"
#include "keyboard.h"
#include "string.h"
#include "serial.h"
#include "fat.h"
#include "vfs.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "task.h"
#include "paging.h"
#include "syscall.h"
#include "gdt.h"
#include "exec.h"

#define ENABLE_DEMO_TASKS 0

FatFs g_fat;
Vfs g_vfs;

static void vim_editor(const char* name);
void process_command(const char* cmd);

void print_banner() {
    console_clear();

    console_set_color(VGA_COLOR_LIGHT_BLUE);
    console_print("+------------------------------------------------------------------------------+\n");

    console_set_color(VGA_COLOR_LIGHT_GREEN);
    console_print("  ______     __  __     ______     ______    \n");
    console_print(" /\\  __ \\   /\\_\\_\\_\\   /\\  __ \\   /\\  ___\\   \n");
    console_print(" \\ \\  __ \\  \\/_\\/\\_\\/_  \\ \\ \\/\\ \\  \\ \\___  \\  \n");
    console_print("  \\ \\_\\ \\_\\   /\\_\\/\\_\\  \\ \\_____\\  \\/\\_____\\ \n");
    console_print("   \\/_/\\/_/   \\/_/\\/_/   \\/_____/   \\/_____/\n");

    console_set_color(VGA_COLOR_LIGHT_BLUE);
    console_print("+------------------------------------------------------------------------------+\n");

    console_set_color(VGA_COLOR_LIGHT_GREY);
    console_print("Welcome to AxOS!\n");
    console_print("Type 'help' for available commands.\n");
    console_print("Use 'clear' to redraw this start screen.\n\n");
}

void kernel_main() __attribute__((externally_visible));
void kernel_main() {
    console_init();
    keyboard_init();
    serial_init();
    gdt_init();
    idt_init();
    pic_remap();
    pit_init(100);
    paging_init();
    syscall_init();
    syscall_setup_idt();
    task_init();
    task_register_kernel();

#if ENABLE_DEMO_TASKS
    extern void task_demo1();
    extern void task_demo2();
    task_create(task_demo1);
    task_create(task_demo2);
#endif

    __asm__ volatile("sti");

    fat_mount(&g_fat);
    vfs_init_fat(&g_vfs, &g_fat);
    print_banner();
    console_print("> ");
    command_loop();
}

void task_demo1(){
    while(1){
        console_print("[T1]\n");
        task_sleep_ms(500);
    }
}

void task_demo2(){
    while(1){
        console_print("[T2]\n");
        task_sleep_ms(1000);
    }
}

void command_loop() {
    char buffer[128];
    int len = 0;
    while (1) {
        char c = keyboard_getchar();
        if (c == '\r' || c == '\n') {
            console_print("\n");
            buffer[len] = '\0';
            process_command(buffer);
            len = 0;
            console_print("> ");
        } else if (c == '\b') {
            if (len > 0) {
                len--; 
                console_putchar('\b');
            }
        } else {
            if (len < (int)sizeof(buffer) - 1) {
                buffer[len++] = c;
                console_putchar(c);
            }
        }
    }
}

void show_help() {
    console_print("Available commands:\n");
    console_print("  help       - show this help\n");
    console_print("  version    - show kernel version\n");
    console_print("  about      - show info\n");
    console_print("  clear      - clear screen\n");
    console_print("  ping       - test command\n");
    console_print("  ls         - list files in root\n");
    console_print("  cat NAME   - show file contents\n");
    console_print("  touch NAME - create empty file\n");
    console_print("  vim NAME   - edit/create file (I: insert, ESC: save+quit)\n");
    console_print("  exec NAME  - load and run ELF in user mode\n");
}

void show_version() {
    console_print("AxOS v0.1 (demo)\n");
}

void show_about() {
    console_print("AxOS: simple x86 hobby OS demo\n");
}

void ping_command() {
    console_print("PONG\n");
}

void simple_keyboard_test() {
    console_print("Keyboard test: press any key (ESC to exit)\n");
    
    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            
            if (!(scancode & 0x80)) {
                console_print("Key pressed: scancode ");
                console_print_hex(scancode);
                console_print("\n");
                
                if (scancode == 0x01) {
                    console_print("ESC pressed, exiting...\n");
                    break;
                }
            }
        }
        
        for (volatile int i = 0; i < 100000; i++) {
            __asm__("nop");
        }
    }
    
    console_print("Keyboard test finished.\n");
}

void delay(int ms) {
    for (volatile int i = 0; i < ms * 10000; i++) {
        __asm__("nop");
    }
}

static void vim_editor(const char* name){
    static char buf[32768];
    uint32_t len=0;
    VfsFile f;
    console_clear();
    console_set_color(VGA_COLOR_LIGHT_BLUE);
    console_print("+------------------------- AxOS VIM --------------------------+\n");
    console_set_color(VGA_COLOR_LIGHT_GREY);
    if(vfs_open(&g_vfs,name,&f)){
        uint32_t sz=f.file.size; if(sz>sizeof(buf)) sz=sizeof(buf);
        if(vfs_read(&g_vfs,&f,buf,sz)) { len=sz; for(uint32_t i=0;i<len;i++) console_putchar(buf[i]); }
    }
    console_print("\n[Normal] Press I to insert, ESC to save+quit\n");
    int insert=0; int esc_once=0;
    while(1){
        char c = keyboard_getchar();
        if(!insert){
            if(c=='I' || c=='i'){ insert=1; esc_once=0; console_print("-- INSERT --\n"); continue; }
            if(c==27){ if(esc_once){
                    if(!vfs_write_file(&g_vfs,name,buf,len)) console_print("save error\n");
                    console_set_color(VGA_COLOR_LIGHT_BLUE);
                    console_print("\n+--------------------------- Saved ----------------------------+\n");
                    return;
                } else { esc_once=1; console_print("(ESC)\n"); }
                continue;
            }
            esc_once=0;
        } else {
            if(c==27){ insert=0; console_print("\n-- NORMAL --\n"); continue; }
            if(c=='\b'){ if(len>0){ len--; console_putchar('\b'); } continue; }
            if(len<sizeof(buf)-1){ buf[len++]=c; console_putchar(c);} 
        }
    }
}

void process_command(const char* cmd) {
    if (strlen(cmd) == 0) {
        return;
    }
    if (strcmp(cmd, "help") == 0) {
        show_help();
    } else if (strcmp(cmd, "version") == 0) {
        show_version();
    } else if (strcmp(cmd, "about") == 0) {
        show_about();
    } else if (strcmp(cmd, "clear") == 0) {
        console_clear();
        print_banner();
    } else if (strcmp(cmd, "ping") == 0) {
        ping_command();
    } else if (strcmp(cmd, "ls") == 0) {
        vfs_ls_root(&g_vfs);
    } else if ((strlen(cmd) > 3) && cmd[0]=='c' && cmd[1]=='a' && cmd[2]=='t' && (cmd[3]==' ' || cmd[3]=='\t')) {
        const char* name = cmd + 4;
        while (*name==' ' || *name=='\t') name++;
        char fname[64];
        int i = 0;
        while (name[i] && name[i] != ' ' && name[i] != '\t') { if (i<63) { fname[i] = name[i]; i++; } else { break; } }
        fname[i] = '\0';
        if (i == 0) {
            console_print("usage: cat NAME\n");
        } else {
            VfsFile f;
            if (vfs_open(&g_vfs, fname, &f)) {
                static char buf[32768];
                uint32_t sz = f.file.size;
                if (sz > sizeof(buf)) sz = sizeof(buf);
                if (vfs_read(&g_vfs, &f, buf, sz)) {
                    for (uint32_t j = 0; j < sz; j++) console_putchar(buf[j]);
                    console_print("\n");
                } else {
                    console_print("read error\n");
                }
            } else {
                console_print("not found\n");
            }
        }
    } else if ((strlen(cmd) > 3) && cmd[0]=='v' && cmd[1]=='i' && cmd[2]=='m' && (cmd[3]==' ' || cmd[3]=='\t')) {
        const char* name = cmd + 4; while (*name==' '||*name=='\t') name++;
        char fname[64]; int i=0; while(name[i] && name[i]!=' ' && name[i]!='\t'){ if(i<63){fname[i]=name[i]; i++;} else break; } fname[i]='\0';
        if(i==0){ console_print("usage: vim NAME\n"); }
        else { vim_editor(fname); }
    } else if ((strlen(cmd) > 5) && cmd[0]=='t' && cmd[1]=='o' && cmd[2]=='u' && cmd[3]=='c' && cmd[4]=='h' && (cmd[5]==' ' || cmd[5]=='\t')) {
        const char* name = cmd + 6; while (*name==' '||*name=='\t') name++;
        char fname[64]; int i=0; while(name[i] && name[i]!=' ' && name[i]!='\t'){ if(i<63){fname[i]=name[i]; i++;} else break; } fname[i]='\0';
        if(i==0){ console_print("usage: touch NAME\n"); }
        else { VfsFile nf; if(vfs_create(&g_vfs,fname,0,&nf)) console_print("created\n"); else console_print("create failed\n"); }
    } else if ((strlen(cmd) > 4) && cmd[0]=='e' && cmd[1]=='x' && cmd[2]=='e' && cmd[3]=='c' && (cmd[4]==' ' || cmd[4]=='\t')) {
        const char* name = cmd + 5; while(*name==' '||*name=='\t') name++;
        char fname[64]; int i=0; while(name[i] && name[i]!=' ' && name[i]!='\t'){ if(i<63){fname[i]=name[i]; i++;} else break; } fname[i]='\0';
        if(i==0){ console_print("usage: exec NAME\n"); }
        else { if(!exec(fname)) console_print("exec failed\n"); }
    } else {
        console_print("Unknown command. Type 'help'\n");
    }
}