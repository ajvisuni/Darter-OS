void outb(unsigned short port, unsigned char val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static unsigned char* const VGA_BUFFER = (unsigned char*) 0xB8000;
int terminal_row = 0;
int terminal_column = 0;
unsigned char terminal_color = 0x0A;

void update_cursor() {
    unsigned short pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F); outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void terminal_scroll() {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
        VGA_BUFFER[i] = VGA_BUFFER[i + VGA_WIDTH * 2];
    }
    int start_of_last_line = (VGA_HEIGHT - 1) * VGA_WIDTH * 2;
    for (int i = 0; i < VGA_WIDTH * 2; i += 2) {
        VGA_BUFFER[start_of_last_line + i] = ' ';
        VGA_BUFFER[start_of_last_line + i + 1] = terminal_color;
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0; terminal_row++;
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            VGA_BUFFER[(terminal_row * VGA_WIDTH + terminal_column) * 2] = ' ';
        }
    } else {
        int index = (terminal_row * VGA_WIDTH + terminal_column) * 2;
        VGA_BUFFER[index] = c;
        VGA_BUFFER[index + 1] = terminal_color;
        terminal_column++;
    }
    if (terminal_column >= VGA_WIDTH) { terminal_column = 0; terminal_row++; }
    if (terminal_row >= VGA_HEIGHT) { terminal_scroll(); }
    update_cursor();
}

void kprint(char* str) {
    for (int i = 0; str[i] != '\0'; i++) terminal_putchar(str[i]);
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        VGA_BUFFER[i] = ' '; VGA_BUFFER[i+1] = terminal_color;
    }
    terminal_row = 0; terminal_column = 0;
    update_cursor();
}

int strcmp(char *s1, char *s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return 1;
}

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

char get_input_keycode() {
    while (!(inb(0x64) & 1));
    return inb(0x60);
}

char cmd_buffer[256];
int cmd_len = 0;

void print_prompt() {
    terminal_color = 0x0B;
    kprint("SYS_OP@DARTER-OS:~$ ");
    terminal_color = 0x0A;
}

void execute_command() {
    kprint("\n");
    terminal_color = 0x0F;

    if (cmd_len == 0) {
    } else if (strcmp(cmd_buffer, "help") == 0) {
        kprint("AVAILABLE COMMANDS:\n");
        kprint(" > help    - Shows this menu\n");
        kprint(" > clear   - Wipes the visual matrix\n");
        kprint(" > sysinfo - Displays kernel telemetry\n");
	kprint(" > cmd     - Command prompt\n");
    } else if (strcmp(cmd_buffer, "clear") == 0) {
        clear_screen();
    } else if (strcmp(cmd_buffer, "sysinfo") == 0) {
        terminal_color = 0x0C;
        kprint("[SYSTEM TELEMETRY]\n");
        kprint("OS Core      : Darter OS v0.4 'Sci-Fi'\n");
        kprint("Architecture : x86 32-bit Protected Mode\n");
        kprint("Memory Size  : 640K Base / Extended Unmapped\n");
        kprint("Network      : OFFLINE\n");
    } else {
        terminal_color = 0x0C;
        kprint("ERR: Command '");
        kprint(cmd_buffer);
        kprint("' unrecognized by system.\n");
    }

    cmd_len = 0;
    print_prompt();
}

void run_shell() {
    print_prompt();
    while(1) {
        unsigned char keycode = get_input_keycode();
        
        if (!(keycode & 0x80)) {
            char c = keyboard_map[keycode];
            if (c != 0) {
                if (c == '\n') {
                    cmd_buffer[cmd_len] = '\0';
                    execute_command();
                } else if (c == '\b') {
                    if (cmd_len > 0) {
                        cmd_len--;
                        terminal_putchar('\b');
                    }
                } else if (cmd_len < 255) {
                    cmd_buffer[cmd_len++] = c;
                    terminal_putchar(c);
                }
            }
        }
    }
}

void main() {
    clear_screen();
    
    terminal_color = 0x0B;
    kprint("======================================================================\n");
    
    terminal_color = 0x0A;
    kprint("________               __                ________  _________\n");
    kprint("\\______ \\ _____ _______/  |_  ___________ \\_____  \\/   _____/\n");
    kprint(" |    |  \\\\__  \\\\_  __ \\   __\\/ __ \\_  __ \\ /   |   \\_____  \\ \n");
    kprint(" |    `   \\/ __ \\|  | \\/|  | \\  ___/|  | \\//    |    \\/        \\\n");
    kprint("/_______  (____  /__|   |__|  \\___  >__|   \\_______  /_______  /\n");
    kprint("        \\/     \\/                 \\/               \\/        \\/ \n");
    
    terminal_color = 0x0B;
    kprint("======================================================================\n\n");
    
    terminal_color = 0x08;
    kprint("[+] SYNCING HARDWARE MATRIX... ");
    terminal_color = 0x0A; kprint("OK\n"); terminal_color = 0x08;
    kprint("[+] MOUNTING VGA VIRTUAL BUFFER... ");
    terminal_color = 0x0A; kprint("OK\n"); terminal_color = 0x08;
    kprint("[+] ESTABLISHING KEYBOARD UPLINK... ");
    terminal_color = 0x0A; kprint("OK\n\n");
    
    terminal_color = 0x0F;
    kprint("SYSTEM INITIALIZATION COMPLETE. WAITING FOR OPERATOR INPUT.\n\n");
    
    run_shell();
}