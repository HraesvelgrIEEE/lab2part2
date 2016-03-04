/*
 * File:   lcd.h
 * Authors:
 *
 */

void writeLCD(unsigned char word, unsigned int commandType, unsigned int delay);
void writeFourBits(unsigned char word, unsigned int commandType, unsigned int delayAfter, unsigned int lower);
void initLCD(void);
void clearLCD(void);
void moveCursorLCD(unsigned char x, unsigned char y); //column, row, 0-indexed
void printCharLCD(char c);
void printStringLCD(const char* s);
void testLCD();