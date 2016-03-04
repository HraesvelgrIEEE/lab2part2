/* 
 * File:   keypad.h
 * Author: user
 *
 * Created on February 5, 2015, 11:42 AM
 */

#ifndef KEYPAD_H
#define	KEYPAD_H

#define KEYPAD_COL1 LATDbits.LATD10
#define KEYPAD_COL2 LATDbits.LATD4
#define KEYPAD_COL3 LATCbits.LATC1

#define KEYPAD_ROW1 PORTEbits.RE3
#define KEYPAD_ROW2 PORTEbits.RE0
#define KEYPAD_ROW3 PORTEbits.RE1
#define KEYPAD_ROW4 PORTDbits.RD5

void initKeypad(void);
char scanKeypad(void);
void enableNSA();

#endif	/* KEYPAD_H */

