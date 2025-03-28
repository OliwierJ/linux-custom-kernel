/*
 *	kernel.c
 */

#include "keyboard_map.h"
#include "keyboard_map_shift.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

// array and pointer for line feeds
int lf_stack[LINES];
int* lf_ptr = lf_stack;

// bits for shift and caps
int shift_pressed = 0; 
int caps_locked = 0;

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate idt entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = 0x08; // kernel_code_segement_offset
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e; // interrupt_gate
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;
	
	/* 		Ports
			PIC1	PIC2
	Command 0x20	0xA0
	Data	0x21	0xA1
	*/

	/* icw1 - begin initialization */
	write_port(0x20, 0x11);
	write_port(0xa0, 0x11);

	/* icw2 - remap offset address of idt
	 *
	 * in x86 protected mode, we have to remap the pics beyond 0x21 because
	 * intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	 */

	write_port(0x21, 0x20);
	write_port(0xa1, 0x28);

	// icw3 - setup cascading
	write_port(0x21, 0x00);
	write_port(0xa1, 0x00);

	// icw4 - environment info
	write_port(0x21, 0x01);
	write_port(0xa1, 0x01);
	// initialization finished
	
	// mask interrupts
	write_port(0x21, 0xff);
	write_port(0xa1, 0xff);

	// fill the idt descriptor
	idt_address = (unsigned long)IDT;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;

	load_idt(idt_ptr);
}

void kb_init(void) 
{
	// 0xFD is 11111101 - enables only IRQ1 (keyboard)
	write_port(0x21, 0xFD);

}

// function to update the hardware cursor
void updateCaret(int position) {
	write_port(0x3d4, 0x0f);					// Select low byte of cursor position
	write_port(0x3d5, position & 0xff);			// Send low byte 
	write_port(0x3d4, 0x0e);					// Select high byte of cursor position
	write_port(0x3d5, (position >> 8) & 0xFF);	// Send high byte

}

// carriage return
void kprint_newline(void)
{
	// push the current loc onto lf stack
	*lf_ptr++ = current_loc;
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
	updateCaret(current_loc/2);
}	

// backspace function
void kprint_backspace(void) {
	if (!current_loc) {return;} //return early if the location is 0
	
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	if (current_loc % line_size == 0) {
		// pop the current loc off the stack
		current_loc = *--lf_ptr;
		updateCaret(current_loc/2);
		return;
	}
	current_loc -= 2;
	vidptr[current_loc] = ' ';

	updateCaret(current_loc/2);
}

// prints a string at a location on screen 
void kprint(const char *str, unsigned int loc)
{
	unsigned int temp_loc = current_loc;
	// index
	current_loc = loc;
	unsigned int i = 0;
	// print each char until null terminator
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x03;
	}
	current_loc = temp_loc;
	updateCaret(current_loc/2);
}


// check is a char is a letter
int isAlpha(unsigned letter) {
	return letter >= 'a' && letter <= 'z';
}
void keyboard_handler_main(void) {
	unsigned char status;	// status that checks if a key was pressed
	unsigned char keycode;			// this is the scancode recieved from the keyboard buffer
	unsigned char key_pressed;


	//write EOI
	write_port(0x20,0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	// lowest bit of status will be set if buffer is not empty
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if (keycode < 0) {
			return;
		} 
		if( keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}
		if ( keycode == BACKSPACE_KEY_CODE) {
			kprint_backspace();
			return;
		}
		if ( keycode == 0x3a) {
			caps_locked = !caps_locked;
			//const char *prsdshft= "CAPS " + caps_locked;
			//kprint(prsdshft,2800);
			return;
		}
		if (keycode == 0x2A || keycode == 0x36) {
			shift_pressed = 1;
			//const char *prsdshft= "shift pressed  ";
			//kprint(prsdshft,2000);
			return;
		} 
		if (keycode & 0x80) {
			if (keycode == 0xAA) {
				shift_pressed = 0;
				//const char *notprsdshft= "shift unpressed";
				//kprint(notprsdshft,2000);
			}
			return;
		}

		// if shift is pressed then use the shifted key map
		if (shift_pressed) {
			key_pressed = keyboard_map_shift[keycode];
		} // else use the regular one 
		else { 
			key_pressed = keyboard_map[keycode];

		}

		// if caps lock is on and shift is NOT pressed
		if (caps_locked && !shift_pressed && isAlpha(key_pressed)) {
			key_pressed -= 32;
		}
		// if caps lock is on and shift is pressed
		if (caps_locked && shift_pressed && isAlpha(key_pressed)) {
			key_pressed += 32;
		}

		// set the current location to the character and set the formatting
		vidptr[current_loc++] = key_pressed;
		vidptr[current_loc++] = 0x03;

		updateCaret(current_loc/2);
	}
}





void clear_screen(void) 
{
	unsigned int i = 0;
	/* this loop clears the screen
	 * there are 25 lines each of 80 columns; each element takes 2 bytes */
	while(i < SCREENSIZE) {
		// blank character
		vidptr[i++] = ' ';
		// attribute-byte - light grey on black screen
		vidptr[i++] = 0x03;
	}
}
void kmain(void) 
{
	const char *str = "my first kernel";
	//char *vidptr = (char*)0xb8000;  video mem begins here. 0xb80000 is the start of video memory in protected mode
	clear_screen();
	kprint(str,0);
	kprint_newline();
	kprint_newline();

	idt_init();
	kb_init();
	// end kmain
	while(1);
}
