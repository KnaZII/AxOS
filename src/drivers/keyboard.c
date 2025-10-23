#include "keyboard.h"
#include "console.h"

static char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;

void keyboard_init() {
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
}

char keyboard_getchar() {
    uint8_t scancode;
    char ascii;
    
    while (1) {
        while (!(inb(KEYBOARD_STATUS_PORT) & 1)) {
        }
        
        scancode = inb(KEYBOARD_DATA_PORT);
        
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            continue;
        } else if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = 0;
            continue;
        } else if (scancode == 0x1D) {
            ctrl_pressed = 1;
            continue;
        } else if (scancode == 0x9D) {
            ctrl_pressed = 0;
            continue;
        } else if (scancode == 0x38) {
            alt_pressed = 1;
            continue;
        } else if (scancode == 0xB8) {
            alt_pressed = 0;
            continue;
        }
        
        if (scancode & 0x80) {
            continue;
        }
        
        if (scancode < 128) {
            if (shift_pressed) {
                ascii = scancode_to_ascii_shift[scancode];
            } else {
                ascii = scancode_to_ascii[scancode];
            }
            
            if (ascii != 0) {
                return ascii;
            }
        }
    }
}

int keyboard_key_pressed() {
    return (inb(KEYBOARD_STATUS_PORT) & 1);
}

int keyboard_shift_pressed() {
    return shift_pressed;
}

int keyboard_ctrl_pressed() {
    return ctrl_pressed;
}

int keyboard_alt_pressed() {
    return alt_pressed;
}