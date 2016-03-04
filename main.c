/* File: main.c
 * Author: dhung 
 */

#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "interrupt.h"
#include "keypad.h"
#include "lcd.h"
#include "timer.h"

#define MAX_PASSWORDS 10

typedef enum stateTypeEnum {start, setMode, valid, invalid, good, bad, kudos,
            debounce, waitPress, waitRelease, wait, scan, checkPassword, setPassword} stateType;
typedef enum modeTypeEnum {standard, programPrime, program} modeType;

volatile stateType state = start, nextState = waitPress;

int main(void) {
    //Initialize
    initLCD();
    initKeypad();
    initTimers();
    enableInterrupts(); 
    SYSTEMConfigPerformance(10000000);
    
    char keypadChar = 0;
    short numPrinted = 0;
    modeType mode = standard;
    unsigned enteredPassword = 0;
    unsigned passwords[MAX_PASSWORDS];
    short numPasswords = 0;
    
    while (1) {
        switch (state) {
            //Display states
            case start:
                enableNSA();
                mode = standard;
                enteredPassword = 0;
                numPrinted = 0;
                T4CONbits.TON = 0; //Timer off
                
                clearLCD();
                moveCursorLCD(0, 0);
                printStringLCD("Enter");
                moveCursorLCD(0, 1);
                
                state = waitPress;
                break;
            case setMode:
                mode = program;
                clearLCD();
                moveCursorLCD(0,0);
                printStringLCD("Set Mode");
                moveCursorLCD(0,1);
                
                state = waitPress;
                break;
            case valid:
                clearLCD();
                moveCursorLCD(0,0);
                
                if (numPasswords < MAX_PASSWORDS) {
                    int i = 0;
                    for (i = 0; i != numPasswords; ++i) {
                        if (passwords[i] == enteredPassword)
                            break;
                    }
                    if (i == numPasswords)
                        passwords[numPasswords++] = enteredPassword;
                    
                    printStringLCD("Valid");
                }
                
                else {
                    printStringLCD("Max reached");
                    moveCursorLCD(0,1);
                    printStringLCD("(Max says hi)");
                }
                
                //ENABLE TIMER
                TMR1 = 0;
                T4CONbits.TON = 1; //Timer on
                
                nextState = start;
                state = wait;
                break;
            case invalid:
                clearLCD();
                moveCursorLCD(0,0);
                printStringLCD("Invalid");
                
                //ENABLE TIMER
                TMR1 = 0;
                T4CONbits.TON = 1; //Timer on
                
                nextState = start;
                state = wait;
                break;
            case good:
                clearLCD();
                moveCursorLCD(0,0);
                printStringLCD("Good");
                
                //ENABLE TIMER
                TMR1 = 0;
                T4CONbits.TON = 1; //Timer on
                
                nextState = kudos;
                state = wait;
                break;
            case bad:
                clearLCD();
                moveCursorLCD(0,0);
                printStringLCD("Bad");
                
                //ENABLE TIMER
                TMR1 = 0;
                T4CONbits.TON = 1; //Timer on
                
                nextState = start;
                state = wait;
                break;
            case kudos:
                moveCursorLCD(1,1);
                printStringLCD("RIVETING! ");
                printCharLCD(0b11101111);
                printCharLCD(0b01011111);
                printCharLCD(0b11101111);
                
                delayMs(100);
                
                state = start;
                break;
                
            //Wait states
            case debounce:
                delayMs(5);
                state = nextState;
                break;
            case waitPress:
                break;
            case waitRelease:
                break;
            case wait:
                break;
                
            //Operating states
            case scan:
                keypadChar = scanKeypad();
                
                state = waitRelease; //By default, don't do anything until buttons released
                switch (keypadChar) {
                    case -1:
                        nextState = waitPress;
                        break;
                    case '#':
                        printCharLCD(keypadChar);
                        nextState = invalid;
                        break;
                    case '*':
                        printCharLCD(keypadChar);
                        
                        if (mode == program) {
                            nextState = invalid;
                        }
                        else if (mode == programPrime) {
                            state = setMode; //We skip waiting for releasing the button on purpose! We're so cool!
                        }
                        else if (enteredPassword == 0) { //Not already entering password
                            mode = programPrime;
                            nextState = waitPress;
                        }
                        else {
                            nextState = invalid;
                        }
                        break;
                    default:
                        printCharLCD(keypadChar);
                        
                        if (mode == programPrime) {
                            nextState = invalid;
                        }
                        
                        else {
                            enteredPassword = (enteredPassword << 8) + keypadChar;
                            ++numPrinted;

                            if (numPrinted > 3) {
                                if (mode == standard) nextState = checkPassword;
                                else nextState = valid; //program mode
                            }
                            else nextState = waitPress;
                        }
                        break;
                }
                break;
            case checkPassword:
                //TODO: check password
                if (numPasswords == 0) {
                    clearLCD();
                    moveCursorLCD(0,0);
                    printStringLCD("RIP ");
                    printCharLCD(0b00101000);
                    printCharLCD(0b11101011);
                    printCharLCD(0b11011011);
                    printCharLCD(0b11101011);
                    printCharLCD(0b00101001);
                    moveCursorLCD(0,1);
                    printStringLCD("(No saved codes)");
                    
                    TMR1 = 0;
                    T4CONbits.TON = 1;
                    
                    nextState = start;
                    state = wait;
                }
                
                else {
                    nextState = bad; //By default, the glass is completely empty
                    
                    int i = 0;
                    for (i = 0; i != numPasswords; ++i) {
                        if (enteredPassword == passwords[i]) nextState = good;
                    }
                    state = nextState;
                }
                break;
        }
    }
}

__ISR(_CHANGE_NOTICE_VECTOR, IPL7SRS) _CNInterrupt() {
    IFS1bits.CNEIF = 0;
    IFS1bits.CNDIF = 0;
    
    PORTE;
    PORTD;
    
    if (state == waitPress) {
        nextState = scan;
        state = debounce;
    }
    //All buttons released
    else if (state == waitRelease && (KEYPAD_ROW1 + KEYPAD_ROW2 + KEYPAD_ROW3 + KEYPAD_ROW4 == 4)) {
        state = debounce;
    }
}

__ISR(_TIMER_5_VECTOR, IPL7SRS) _T5Interrupt() {
    IFS0bits.T5IF = 0;
    
    state = nextState;
}
