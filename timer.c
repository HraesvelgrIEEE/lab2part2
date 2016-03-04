/*
 * File:   timer.c
 * Author: dhung
 *
 */

#include <xc.h>
#include "timer.h"

void initTimers() {
    //initTimer1();
    initTimer2();
    initTimer32bit();
}

void initTimer1() {
    TMR1 = 0; //Clear timer
    T1CONbits.TCKPS = 3; //Prescalar 256
    T1CONbits.TCS = 0; //Set oscillator
    PR1 = 78124; //PR for ~10 ms, rounded from 389.625
    IEC0bits.T1IE = 1; //Enable
    IFS0bits.T1IF = 0; //Flag down
    IPC1bits.T1IP = 7; //Default priority
    T1CONbits.TON = 0; //Timer off -- manually turn on when needed
}

void initTimer2() {
    TMR2 = 0;
    T2CONbits.TCKPS = 0; //Prescalar 1
    T2CONbits.TCS = 0; 
    IFS0bits.T2IF = 0; //Flag down
}

void initTimer32bit() {
    //Use timers 4 and 5
    TMR4 = 0;
    TMR5 = 0;
    
    T4CONbits.TCS = 0;
    //T5CONbits.TCS = 0;
    
    T4CONbits.T32 = 1;
    T4CONbits.TCKPS = 7;
    
    IFS0bits.T5IF = 0;
    IEC0bits.T5IE = 1;
    IPC5bits.T5IP = 7;
    
    PR4 = 0b0011000100101100;
    PR5 = 0x01;
    
    T4CONbits.ON = 0;
}

void delayMs(unsigned int delay) {
    int i = 0;
    for (i = 0; i != delay; ++i) {
        delayUs(1000); //delay 1 ms
    }
}

void delayUs(unsigned int delay){
    TMR2 = 0;
    PR2 = 14*delay;
    IFS0bits.T2IF = 0;
    T2CONbits.ON = 1;
    while (IFS0bits.T2IF == 0);
    T2CONbits.ON = 0;
}