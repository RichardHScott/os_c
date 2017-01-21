#include "keyboard.h"

char ascii_key[0x90] = {0};

enum keyboard_port {
    keyboard_controller_port = 0x64,
    keyboard_encoder_port = 0x60
};

void keyboard_init(void) {
ascii_key[0x15] = 'Q';
ascii_key[0x16] = '1'; 
ascii_key[0x21] = 'C'; 
ascii_key[0x22] = 'X'; 
ascii_key[0x23] = 'D'; 
ascii_key[0x24] = 'E'; 
ascii_key[0x25] = '4'; 
ascii_key[0x26] = '3'; 
ascii_key[0x29] = ' ';
ascii_key[0x31] = 'N';
ascii_key[0x32] = 'B';
ascii_key[0x33] = 'H';
ascii_key[0x34] = 'G';
ascii_key[0x35] = 'Y';
ascii_key[0x36] = '6';
ascii_key[0x41] = ',';
ascii_key[0x42] = 'K';
ascii_key[0x43] = 'I';
ascii_key[0x44] = 'O';
ascii_key[0x45] = '0';
ascii_key[0x46] = '9';
ascii_key[0x49] = '.';
ascii_key[0x52] = '\'';
ascii_key[0x54] = '[';
ascii_key[0x55] = '=';
ascii_key[0x69] = '1'; 
ascii_key[0x70] = '0'; 
ascii_key[0x71] = '.'; 
ascii_key[0x72] = '2'; 
ascii_key[0x73] = '5'; 
ascii_key[0x74] = '6'; 
ascii_key[0x75] = '8'; 
ascii_key[0x79] = '\n';
ascii_key[0x0D] = '\t';
ascii_key[0x0E] = '`'; 
ascii_key[0x1A] = 'Z'; 
ascii_key[0x1B] = 'S'; 
ascii_key[0x1C] = 'A'; 
ascii_key[0x1D] = 'W'; 
ascii_key[0x1E] = '2'; 
ascii_key[0x2A] = 'V'; 
ascii_key[0x2B] = 'F'; 
ascii_key[0x2C] = 'T'; 
ascii_key[0x2D] = 'R'; 
ascii_key[0x2E] = '5'; 
ascii_key[0x3A] = 'M'; 
ascii_key[0x3B] = 'J'; 
ascii_key[0x3C] = 'U'; 
ascii_key[0x3D] = '7'; 
ascii_key[0x3E] = '8'; 
ascii_key[0x4A] = '/';
ascii_key[0x4A] = '/';
ascii_key[0x4B] = 'L';
ascii_key[0x4C] = ';';
ascii_key[0x4D] = 'P';
ascii_key[0x4E] = '-';
ascii_key[0x4E] = '-';
ascii_key[0x5A] = '\n';
ascii_key[0x5B] = ']';
ascii_key[0x5C] = '\\';
ascii_key[0x6B] = '4';
ascii_key[0x6C] = '7';
ascii_key[0x7A] = '3';
ascii_key[0x7C] = '+';
ascii_key[0x7D] = '9';
ascii_key[0x7E] = '*';
}

const uint8_t key_relased_code = 0xf0;

bool keyboard_capslock = false;
bool keyboard_shift = false;
bool alt = false;
bool scroll_lock = false;
bool num_lock = false;

bool keyboard_key_released = false;

void keyboard_interrupt(void) {
    uint8_t code = inb(keyboard_encoder_port);

    if(code == key_relased_code) {
        //key released
        keyboard_key_released = true;
        return;
    } else if(keyboard_key_released) {
        keyboard_key_released = false;
        return;
    }

    char ascii = ascii_key[code];
    if(ascii != 0) {
        print_char(ascii);
    }
}