/*****************************************************************************************
* Lcd.c
* 
* A set of general purpose LCD utilities. This module should not be used with a preemptive
* kernel without protection of the shared LCD.
*  
* Originally the 9S12 LCD module from Andrew Pace, 2/6/99, ET 454 
* MOdified for the K70. Todd Morton, 2/24/2013 
* Modified for the K65 Todd Morton, 11/03/2015
******************************************************************************************
* Master Include File  
*****************************************************************************************/
#include "MCUType.h"
#include "LCD.h"

/*****************************************************************************************
* LCD Port Defines 
*****************************************************************************************/
#define LCD_RS_BIT     0x2U
#define LCD_E_BIT      0x4U
#define LCD_DB_MASK    0x78U
#define LCD_PORT       GPIOD_PDOR
#define LCD_PORT_DIR   GPIOD_PDDR
#define INIT_BIT_DIR() (LCD_PORT_DIR |= (LCD_RS_BIT|LCD_E_BIT|LCD_DB_MASK))
#define LCD_SET_RS()   GPIOD_PSOR = LCD_RS_BIT
#define LCD_CLR_RS()   GPIOD_PCOR = LCD_RS_BIT
#define LCD_SET_E()    GPIOD_PSOR = LCD_E_BIT
#define LCD_CLR_E()    GPIOD_PCOR = LCD_E_BIT

/*****************************************************************************************
* LCD Defines
*****************************************************************************************/
#define NUM_CHARS     16      /* 16 character display */
#define LCD_DAT_INIT  0x28    /*Data length: 4 bit. Lines: 2. Font: 5x7 dots.*/
#define LCD_SHIFT_CUR 0x06    /*Increments cursor addr after write.*/
#define LCD_DIS_INIT  0x0C    /*Display: on. Cursor: off. Blink: off */
#define LCD_CLR_CMD   0x01    /*Clear display and move cursor home */

#define LCD_LINE1_ADDR 0x80   /* Display address for line1 column1 */
#define LCD_LINE2_ADDR 0xC0   /* Display address for line2 column1 */
#define LCD_BS_CMD     0x10   /* Move cursor left one space */
#define LCD_FS_CMD     0x14   /* Move cursor right one space */

/*****************************************************************************************
* Private Function prototypes
*****************************************************************************************/
static void lcdWrCmd(const INT8U cmd);
static void lcdDly500ns(void);
static void lcdDly40us(void);
static void lcdDlyms(const INT8U ms);
static void lcdWrNib(INT8U nib);

/*****************************************************************************************
* Function Definitions
******************************************************************************************
* lcdWrCmd(INT8U cmd) - Private
*  PARAMETERS: cmd - Command to be sent to the LCD
*  DESCRIPTION: Sends a command write sequence to the LCD
*****************************************************************************************/
static void lcdWrCmd(const INT8U cmd) {
      LCD_CLR_RS();                 //Select command
      lcdWrNib(cmd>>4);             //Out most sig nibble
      LCD_SET_E();                  //Pulse E. 230ns min per Seiko doc
      lcdDly500ns();
      LCD_CLR_E();
      lcdDly500ns();                //Wait >1us per Seiko doc
      lcdDly500ns();
      lcdWrNib((cmd & 0x0fu));      //Out least sig nibble
      LCD_SET_E();                  //Pulse E
      lcdDly500ns();
      LCD_CLR_E();
      lcdDly40us();                 //Wait 40us per Seiko doc
      LCD_SET_RS();                 //Set back to data
}

/*****************************************************************************************
* lcdWrNib(INT8U nib) - Private
*  PARAMETERS: nib - 4-bit nibble (upper four bits zero) sent to the LCD
*  DESCRIPTION: Sends a nibble to the LCD
*****************************************************************************************/
static void lcdWrNib(INT8U nib){
    INT8U lnib = nib & 0x0fu;
    GPIOD_PDOR = (GPIOD_PDOR & (INT32U)~LCD_DB_MASK)|((INT32U)lnib << 3);
}

/*****************************************************************************************
* LcdInit()
*  PARAMETERS: None
*  DESCRIPTION: Initialises LCD ports to outputs and sends LCD reset sequence per Seiko
*               Data sheet. In this case, 4-bit mode.
*****************************************************************************************/
void LcdInit(void) {
    SIM_SCGC5 |= SIM_SCGC5_PORTD(1);
	PORTD_PCR1 = PORT_PCR_MUX(1);
	PORTD_PCR2 = PORT_PCR_MUX(1);
	PORTD_PCR3 = PORT_PCR_MUX(1);
	PORTD_PCR4 = PORT_PCR_MUX(1);
	PORTD_PCR5 = PORT_PCR_MUX(1);
	PORTD_PCR6 = PORT_PCR_MUX(1);
	INIT_BIT_DIR();
    LCD_CLR_E(); 
    LCD_SET_RS();               /*Data select unless in lcdWrCmd()  */
    lcdDlyms(15);               /* LCD requires 15ms delay at powerup */
   
    LCD_CLR_RS();               /*Send first command for RESET sequence*/
    lcdWrNib(0x3u);
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDlyms(5);                /*Wait >4.1ms */
  
    lcdWrNib(0x3u);             /*Repeat */
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDlyms(1);                /*Wait >100us */
  
    lcdWrNib(0x3u);             /* Repeat */
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();               /*Wait >40us*/
  
    lcdWrNib(0x2u);             /*Send last command for RESET sequence*/
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();
  
    lcdWrCmd(LCD_DAT_INIT);     /*Send command for 4-bit mode */
    lcdWrCmd(LCD_SHIFT_CUR);
    lcdWrCmd(LCD_DIS_INIT);
    LcdClrDisp();
} 

/*****************************************************************************************
** LcdDispChar() - Public
*
*  PARAMETERS: c - ASCII character to be sent to the LCD
*
*  DESCRIPTION: Displays a character at current LCD address. Assumes that the LCD port is
*               configured for a data write.
*****************************************************************************************/
void LcdDispChar(const INT8C c) {
    lcdWrNib(((INT8U)c >> 4));
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly500ns();
    lcdDly500ns();
    lcdWrNib(((INT8U)c & 0x0fu));
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();
}

/*****************************************************************************************
** LcdClrDisp
*  PARAMETERS: None
*  DESCRIPTION: Clears the LCD display and returns the cursor to row1, col1.
*****************************************************************************************/
void LcdClrDisp(void) {

    lcdWrCmd(LCD_CLR_CMD);
    lcdDlyms(2);
}

/*****************************************************************************************
** LcdClrLine
*  PARAMETERS: line - Line to be cleared (1 or 2).
*  DESCRIPTION: Writes spaces to every location in a line and then returns the cursor to
*               column 1 of that line.
*****************************************************************************************/
void LcdClrLine(const INT8U line) {
  
   INT8U start_addr; 
   INT8U i;

   if(line == 1){ 
      start_addr = LCD_LINE1_ADDR;  
      lcdWrCmd(start_addr);
      for(i = 0x0; i < (NUM_CHARS); i++) {
         LcdDispChar(' ');
      }
      lcdWrCmd(start_addr);
   }else if(line == 2){ 
      start_addr = LCD_LINE2_ADDR;
      lcdWrCmd(start_addr);
      for(i = 0x0; i < (NUM_CHARS); i++) {
         LcdDispChar(' ');
      }
      lcdWrCmd(start_addr);
   }else{
      /* Input error, do nothing */
   }
}
 
/*****************************************************************************************
* LcdDispByte()
*  PARAMETERS: b - byte to be displayed.
*  DESCRIPTION: Displays the byte, b, in hex.
*****************************************************************************************/
void LcdDispByte(const INT8U b) {

    INT8U nib;
    INT8C nib_c;

    nib = (b & 0xF0U) >> 4;
    if(nib > 9){
        nib_c = (INT8C)(nib + 0x37U);
    }else{
        nib_c = (INT8C)(nib + 0x30U);
    }
    LcdDispChar(nib_c);
    nib = b & 0x0F;
    if(nib > 9){
        nib_c = (INT8C)(nib + 0x37U);
    }else{
        nib_c = (INT8C)(nib + 0x30U);
    }
    LcdDispChar(nib_c);
}   

/*****************************************************************************************
* LcdDispStrg()
*  PARAMETERS: *strg - pointer to the NULL terminated string to be displayed.
*  DESCRIPTION: Displays the string pointed to by strg.
*****************************************************************************************/
void LcdDispStrg(const INT8C *const strg) {

    INT8C *sptr = (INT8C *)strg;

    while(*sptr != '\0') {
        LcdDispChar(*sptr);
        sptr++;
    }
}

/*****************************************************************************************
** LcdMoveCursor()
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*  DESCRIPTION: Moves the cursor to [row,col].
*****************************************************************************************/
void LcdMoveCursor(const INT8U row, const INT8U col) {

    if(row == 1) {
        lcdWrCmd((INT8U)(LCD_LINE1_ADDR + col - 1));
    }else{
        lcdWrCmd((INT8U)(LCD_LINE2_ADDR + col - 1));
    }
}

/*****************************************************************************************
** LcdDispDecByte()
*  PARAMETERS: b - the byte to be displayed.
*              lz - (Binary)Display leading zeros if TRUE.
*                   Delete leading zeros if FALSE.
*  DESCRIPTION: Displays the byte, b, in decimal.
*               Deletes leading zeros if lz is zero. Digits are right justified if leading
*               zeros are deleted.
*  RETURNS: None
*  Note: Not recommended for new designs. Use LcdDispDecWord().
*****************************************************************************************/
void LcdDispDecByte(const INT8U b, const INT8U lz) {

    INT8U bin = (INT8U)b;
    INT8U lzt = lz;
    INT8C huns, tens, ones;
   
    ones = (INT8C)((bin % 10) + '0');   /* Convert to decimal digits        */
    bin    = bin / 10;
    tens = (INT8C)((bin % 10) + '0');
    huns = (INT8C)((bin / 10) + '0');

    if((huns == '0') && (lzt == 0)){
        LcdDispChar(' ');
    }else{
        lzt = TRUE;
        LcdDispChar(huns);
    }
    if((tens == '0') && (lzt == 0)){
        LcdDispChar(' ');
    }else{
        LcdDispChar(tens);
    }
    LcdDispChar(ones);
}

/*****************************************************************************************
* LcdDispDecWord() - Outputs a decimal value of a 32-bit word.
*    Parameters: bin is the word to be sent,
*                maxlz is the maximum number of leading zeros to be shown. Range 1-10
*****************************************************************************************/
void LcdDispDecWord(INT32U bin, INT8U maxlz){
    INT8C digits[10];
    INT32U lbin = bin;
    INT8U num_zeros = maxlz;
    INT8U dig_num;

    //Clamp leading zeros to acceptable values
    if(num_zeros > 10){
        num_zeros = 10;
    }else if(num_zeros < 1){
        num_zeros = 1;
    }else{
    }
    //Convert to ascii
    dig_num = 0;
    while(dig_num < 10){
        digits[dig_num] = (INT8C)((lbin % 10) +'0');
        lbin = lbin/10;
        dig_num++;
    }
    //Display ascii digits
    dig_num = 9;
    while(dig_num > 0){
        if((digits[dig_num] != '0') || (dig_num < num_zeros)){
            LcdDispChar(digits[dig_num]);
        }else{
        }
        dig_num--;
    }
    LcdDispChar(digits[0]);
}

/*****************************************************************************************
** LcdCursor(INT8U on, INT8U blink)
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*  Changes LCD cursor state - four possible combinations.
*  RETURNS: None
*****************************************************************************************/
void LcdCursor(const INT8U on, const INT8U blink) {

    INT8U curcmd;
    
    if(on == 0){
        curcmd = 0x0CU;     //Cursor off
    }else{
        curcmd = 0x0EU;     //Cursor on
    }
    if(blink != 0) {
        curcmd |= 0x01;     //Cursor blink
    }else{
                            //Cursor no blink
    }
    lcdWrCmd(curcmd);
}

/*****************************************************************************************
** lcdDly500ns(void)
*  	Delays, at least, 500ns
*   Designed for 180MHz max clock.
 * 	Tdly >= (45ns)i (at 180MHz)
 * Currently set to ~540ns with i=12.
 * TDM 01/20/2013
 * TDM 11/03/2015
*****************************************************************************************/
static void lcdDly500ns(void){
	INT32U i;
	for(i=0;i<12;i++){
	}
}

/*****************************************************************************************
** lcdDly40us(void)
* 	Clock frequency independent because it uses lcdDly500ns.
*****************************************************************************************/
static void lcdDly40us(void){
    INT8U cnt;
    for(cnt=80;cnt > 0;cnt--){
        lcdDly500ns();
    }
}

/*****************************************************************************************
** lcdDlyms(INT8U ms)
*   Delays, at least, ms milliseconds. Maximum ~255ms.
*   Note, based on lcdDly500ns() so not very accurate but always greater than ms
*   milliseconds
*****************************************************************************************/
static void lcdDlyms(const INT8U ms){
    INT32U cnt;
    for(cnt = (INT32U)ms*2000;cnt > 0;cnt--){
        lcdDly500ns();
    }
}

/*****************************************************************************************
* LcdBSpace()
*   Moves cursor back one space.
*****************************************************************************************/
void LcdBSpace(void) {
    lcdWrCmd(LCD_BS_CMD);
}

/*****************************************************************************************
* LcdFSpace()
*   Moves cursor right one space.
*****************************************************************************************/
void LcdFSpace(void) {
    lcdWrCmd(LCD_FS_CMD);
}
/****************************************************************************************/
