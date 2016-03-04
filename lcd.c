/*
 * File:   lcd.c
 * Authors:
 *
 * Created on December 31, 2014, 1:39 PM
 */

#include <xc.h>
#include "lcd.h"
#include "timer.h"

#define OUTPUT 0
#define INPUT 1
#define ON 1
#define OFF 0
#define LOWER 1
#define UPPER 0
#define LCD_WRITE 1
#define LCD_COMAND 0

#define LCD_D7 LATEbits.LATE7
#define LCD_D6 LATEbits.LATE6
#define LCD_D5 LATEbits.LATE5
#define LCD_D4 LATEbits.LATE4

#define LCD_RS LATCbits.LATC4
#define LCD_E LATCbits.LATC2

#define TRIS_D7 TRISEbits.TRISE7
#define TRIS_D6 TRISEbits.TRISE6
#define TRIS_D5 TRISEbits.TRISE5
#define TRIS_D4 TRISEbits.TRISE4

#define TRIS_RS TRISCbits.TRISC4
#define TRIS_E  TRISCbits.TRISC2

/* This function should take in a two-byte word and writes either the lower or upper
 * byte to the last four bits of LATE. Additionally, according to the LCD data sheet
 * It should set TRISCbits.TRISC4 and LCD_E with the appropriate values and delays.
 * After these commands are issued, there should be a delay.
 * The command type is a simplification. From the data sheet, the RS is '1'
 * when you are simply writing a character. Otherwise, RS is '0'.
 */
void writeFourBits(unsigned char word, unsigned int commandType, unsigned int delayAfter, unsigned int lower){
    if(lower){ // set least sig bits
        LCD_D4 = word & 0x01;
        LCD_D5 = (word >> 1) & 0x01;
        LCD_D6 = (word >> 2) & 0x01;
        LCD_D7 = (word >> 3) & 0x01;
    }
    else{ // set most sig figs
        LCD_D4 = (word >> 4) & 0x01;
        LCD_D5 = (word >> 5) & 0x01;
        LCD_D6 = (word >> 6) & 0x01;
        LCD_D7 = (word >> 7) & 0x01;
    }
    LCD_RS = commandType & 0x01; // 1 for write 0 for read,  LCD_RW
    LCD_E = 1; // Enable to input data
    delayUs(1);
    LCD_E = 0; // Finishes writing data, LCD_E
    delayUs(1);
    delayUs(delayAfter);
}

/* Using writeFourBits, this function should write the two bytes of a character
 * to the LCD.
 */
void writeLCD(unsigned char word, unsigned int commandType, unsigned int delayAfter){
    writeFourBits(word, commandType, delayAfter, UPPER); // Writes upper 4 bits of work into LCD mem
    writeFourBits(word, commandType, delayAfter, LOWER); // Writes lower 4 bits of work into LCD mem
}

/* Given a character, write it to the LCD. RS should be set to the appropriate value.
 */
void printCharLCD(char c) {
    writeLCD(c, LCD_WRITE, 46); // WRITES a character to LCD
}

/*Initialize the LCD
 */
void initLCD(void) {
    // Setup D, RS, and E to be outputs (0).
    
    TRIS_RS = OUTPUT;
    TRIS_E = OUTPUT;
    TRIS_D4 = OUTPUT;
    TRIS_D5 = OUTPUT;
    TRIS_D6 = OUTPUT;
    TRIS_D7 = OUTPUT;
   
    // Initialization sequence utilizes specific LCD commands before the general configuration
    // commands can be utilized. The first few initialization commands cannot be done using the
    // WriteLCD function. Additionally, the specific sequence and timing is very important.
    
    // Enable 4-bit interface
    // start with 15 ms delay
    delayUs(4000);
    delayUs(4000);
    delayUs(4000);
    delayUs(3000);
    
    // Initialization line/code for setting as 4Bit interface
    //LCD_D4 = 0;    LCD_D5 = 1;    LCD_D6 = 0;    LCD_D7 = 0;
    //LCD_RS = 0;    LCD_E = 1;    delayUs(40);    LCD_E = 0;
    // Delay for    delayUs(40);
    writeFourBits(0x03, LCD_COMAND, 46, 1); // 0000/0011 = 3 // Function set 8 bit
    delayUs(4100); // 4.1 ms delay
    writeFourBits(0x03, LCD_COMAND, 46, 1); // 0000/0011 = 3 // same set 8 bit
    delayUs(100); // 0.1 ms delay
    writeFourBits(0x03, LCD_COMAND, 46, 1); // 0000/0011 = 3 // same 8 bit-ness
    // first line of setting as 4 bit instead of 8 bit
    writeFourBits(0x02, LCD_COMAND, 46, 1); // 0000/0010 = 2
    // Function Set (specifies data width, lines, and font.
   
    // 4-bit mode initialization is complete. We can now configure the various LCD
    // options to control how the LCD will function.
    
    writeLCD(0x2B, LCD_COMAND, 46); // 0010/NF**=0010/1011 = 2B / set function
    
    // Turn Display (D) Off
    writeLCD(0x08, LCD_COMAND, 46); // 0000/1000 = 08 / Display OFF
    

    clearLCD();

    // Set Increment Display, No Shift (i.e. cursor move)
    writeLCD(0x06, LCD_COMAND, 46); // 0000/01,I/D,S=0000/0110 = 06 / Display Cleared
    // input mode set

        // Turn Display (D) On, Cursor (C) Off, and Blink(B) Off
    writeLCD(0x0C, LCD_COMAND, 46); // 0000/1DCB=0000/1100 = 0C / Display ON, Cursor OFF, Blink mode OFF
}

/*
 * You can use printCharLCD here. Note that every time you write a character
 * the cursor increments its position automatically.
 * Since a string is just a character array, try to be clever with your use of pointers.
 */
void printStringLCD(const char* s) {
    //Prints one character at a time from current position until string is terminated
    for (;*s != '\0' ; s++){
        printCharLCD(*s);
    }
}

/*
 * Clear the display.
 */
void clearLCD(){
    writeLCD(0x01, LCD_COMAND, 1640); // 0000/0001 = 01, clears the display
}

/*
 Use the command for changing the DD RAM address to put the cursor somewhere.
 */
void moveCursorLCD(unsigned char x, unsigned char y){
    // Set DD RAM address
    unsigned char DD_address = 0x80; // the base command for DD RAM address
    DD_address += x;
    DD_address += y*0x40;
    writeLCD(DD_address, LCD_COMAND, 40); // 0000/01(S/C)(R/L) = 01, clears the display
}

void testLCD(){
    initLCD();
    int i = 0;
    printCharLCD('c');
    for(i = 0; i < 1000; i++) delayUs(1000);
    clearLCD();
    printStringLCD("Hello!");
    moveCursorLCD(1, 2);
    for(i = 0; i < 1000; i++) delayUs(1000);
    printStringLCD("Hello!");
    for(i = 0; i < 1000; i++) delayUs(1000);
}
