/*****************************************************************************************
* Key.h - The header file for the keypad module, Key.c
* 02/20/2001 Todd Morton
* 11/06/2017 Todd Morton
******************************************************************************************
* Public Resources
*****************************************************************************************/
#ifndef KEY_INC
#define KEY_INC

/*****************************************************************************************
* Defines for the alpha keys: A, B, C, D
* These are ASCII control characters
*****************************************************************************************/
#define DC1 (INT8C)0x11     /*ASCII control code for the A button */
#define DC2 (INT8C)0x12     /*ASCII control code for the B button */
#define DC3 (INT8C)0x13     /*ASCII control code for the C button */
#define DC4 (INT8C)0x14     /*ASCII control code for the D button */


/*****************************************************************************************
* GetKey() - Returns current value of the keypad buffer then clears the buffer.
*            The value returned is the ASCII code for the key pressed or zero if no key
*            was pressed.
*****************************************************************************************/
INT8C GetKey(void);
                              
/*****************************************************************************************
* KeyInit() - Keypad Initialization. Must run before calling KeyTask.
*****************************************************************************************/
void KeyInit(void);

/*****************************************************************************************
* KeyTask() - The main keypad scanning task. It scans the keypad and updates the keypad
*             buffer if a keypress was verified. This is a cooperative task that must be
*             called with a period between: Tb/2 < Tp < (Tact-Tb)/2
*****************************************************************************************/
void KeyTask(void);

#endif
