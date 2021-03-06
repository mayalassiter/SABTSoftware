#ifndef _DIGITALIO_H_
#define _DIGITALIO_H


//User inputs on the PORTB
#define UI_MODE1 1
#define UI_MODE2 0
#define UI_ENTER1 7
#define UI_ENTER2 6

//User inputs in the PORTD
#define UI_VOLUP 3
#define UI_VOLDOWN 2

//User outputs for PORTD
#define UI_STAT1 6
#define UI_STAT2 7

//Digital IO button states
/*
0-Idle
1-Button Pressed
2-Button command used
*/
volatile char Enter1State;
volatile char Enter2State;
volatile char Mode1State;
volatile char Mode2State;
volatile char VolUpState;
volatile char VolDownState;

void Initialize_Digital_IO(void);
void CheckCommandButtons(void);
void RunCommandTasks(void);

void SetStatLED1(bool bState);
void SetStatLED2(bool bState);
void Enter1Task(void);
void Enter2Task(void);
void Mode1Task(void);
void Mode2Task(void);
void VolUpTask(void);
void VolDownTask(void);

#endif

