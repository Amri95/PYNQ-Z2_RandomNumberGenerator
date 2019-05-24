/******************************************************************************
 *  Copyright (c) 2016, Xilinx, Inc.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1.  Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *
 *  2.  Redistributions in binary form must reproduce the above copyright 
 *      notice, this list of conditions and the following disclaimer in the 
 *      documentation and/or other materials provided with the distribution.
 *
 *  3.  Neither the name of the copyright holder nor the names of its 
 *      contributors may be used to endorse or promote products derived from 
 *      this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION). HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/******************************************************************************
 *
 *
 * @file arduino_grove_oled.c
 *
 * IOP code (MicroBlaze) for grove OLE35046P.
 * The grove OLED has to be connected to an arduino interface 
 * via a shield socket.
 * Grove OLED is write only, and has IIC interface.
 * Hardware version 1.1.
 * http://www.seeedstudio.com/wiki/Grove_-_OLED_Display_0.96%22
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- --- ------- -----------------------------------------------
 * 1.00a yrq 04/13/16 release
 * 1.00d yrq 07/26/16 separate pmod and arduino
 *
 * </pre>
 *
 *****************************************************************************/

#include "circular_buffer.h"
#include "timer.h"
#include "i2c.h"

#define V_REF 3.30

// Mailbox commands
#define CONFIG_IOP_SWITCH       0x1
#define SET_NORMAL_DISPLAY      0x3 
#define SET_INVERSE_DISPLAY     0x5 
#define SET_GRAY_LEVEL          0x7 
#define SET_PAGE_MODE           0x9 
#define SET_HORIZONTAL_MODE     0xB 
#define SET_TEXT_XY             0xD
#define CLEAR_DISPLAY           0xF 
#define SET_CONTRAST_LEVEL      0x11
#define PUT_STRING              0x13
#define SET_HORIZONTAL_SCROLL   0x15
#define ENABLE_SCROLL           0x17
#define DISABLE_SCROLL          0x19

#define PAGE_MODE               01
#define HORIZONTAL_MODE         02

#define OLED_Address            0x3c
#define OLED_Command_Mode       0x80
#define OLED_Data_Mode          0x40

#define OLED_Display_Off_Cmd    0xAE
#define OLED_Display_On_Cmd     0xAF

#define OLED_Normal_Display_Cmd     0xA6
#define OLED_Inverse_Display_Cmd    0xA7
#define OLED_Activate_Scroll_Cmd    0x2F
#define OLED_Dectivate_Scroll_Cmd   0x2E
#define OLED_Set_ContrastLevel_Cmd  0x81

#define Scroll_Left             0x00
#define Scroll_Right            0x01

#define Scroll_2Frames          0x7
#define Scroll_3Frames          0x4
#define Scroll_4Frames          0x5
#define Scroll_5Frames          0x0
#define Scroll_25Frames         0x6
#define Scroll_64Frames         0x1
#define Scroll_128Frames        0x2
#define Scroll_256Frames        0x3

void oled_init(void);

void sendCommand(unsigned char command);
void sendData(unsigned char Data);

void setNormalDisplay();
void setInverseDisplay();
void setGrayLevel();
void setPageMode();
void setHorizontalMode();
void setTextXY();
void clearDisplay();
void setContrastLevel();
void putChar(unsigned char c);
void putString();
void setHorizontalScrollProperties();
void activateScroll();
void deactivateScroll();

/*
 * 8x8 Font ASCII 32 - 127 Implemented
 * Users can modify this to support more characters(glyphs)
 * BasicFont is placed in code memory.
 * This font can be freely used without any restriction.
 */
const unsigned char BasicFont[][8]=
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x07,0x00,0x07,0x00,0x00,0x00},
  {0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00},
  {0x00,0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00},
  {0x00,0x23,0x13,0x08,0x64,0x62,0x00,0x00},
  {0x00,0x36,0x49,0x55,0x22,0x50,0x00,0x00},
  {0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x22,0x1C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x2A,0x1C,0x2A,0x08,0x00,0x00},
  {0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00},
  {0x00,0xA0,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
  {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
  {0x00,0x3E,0x51,0x49,0x45,0x3E,0x00,0x00},
  {0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00},
  {0x00,0x62,0x51,0x49,0x49,0x46,0x00,0x00},
  {0x00,0x22,0x41,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x00},
  {0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00},
  {0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x00},
  {0x00,0x01,0x71,0x09,0x05,0x03,0x00,0x00},
  {0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00},
  {0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00},
  {0x00,0x00,0xAC,0x6C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00},
  {0x00,0x14,0x14,0x14,0x14,0x14,0x00,0x00},
  {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x51,0x09,0x06,0x00,0x00},
  {0x00,0x32,0x49,0x79,0x41,0x3E,0x00,0x00},
  {0x00,0x7E,0x09,0x09,0x09,0x7E,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x22,0x1C,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x51,0x72,0x00,0x00},
  {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00},
  {0x00,0x41,0x7F,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x40,0x41,0x3F,0x01,0x00,0x00},
  {0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00},
  {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00},
  {0x00,0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00},
  {0x00,0x7F,0x04,0x08,0x10,0x7F,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00},
  {0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x00},
  {0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00},
  {0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x00},
  {0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00},
  {0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00},
  {0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x00},
  {0x00,0x3F,0x40,0x38,0x40,0x3F,0x00,0x00},
  {0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00},
  {0x00,0x03,0x04,0x78,0x04,0x03,0x00,0x00},
  {0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00},
  {0x00,0x41,0x41,0x7F,0x00,0x00,0x00,0x00},
  {0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x00},
  {0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00},
  {0x00,0x01,0x02,0x04,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00},
  {0x00,0x7F,0x48,0x44,0x44,0x38,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x28,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x48,0x7F,0x00,0x00},
  {0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00},
  {0x00,0x08,0x7E,0x09,0x02,0x00,0x00,0x00},
  {0x00,0x18,0xA4,0xA4,0xA4,0x7C,0x00,0x00},
  {0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00},
  {0x00,0x00,0x7D,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x80,0x84,0x7D,0x00,0x00,0x00,0x00},
  {0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x00},
  {0x00,0x41,0x7F,0x40,0x00,0x00,0x00,0x00},
  {0x00,0x7C,0x04,0x18,0x04,0x78,0x00,0x00},
  {0x00,0x7C,0x08,0x04,0x7C,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x38,0x00,0x00,0x00},
  {0x00,0xFC,0x24,0x24,0x18,0x00,0x00,0x00},
  {0x00,0x18,0x24,0x24,0xFC,0x00,0x00,0x00},
  {0x00,0x00,0x7C,0x08,0x04,0x00,0x00,0x00},
  {0x00,0x48,0x54,0x54,0x24,0x00,0x00,0x00},
  {0x00,0x04,0x7F,0x44,0x00,0x00,0x00,0x00},
  {0x00,0x3C,0x40,0x40,0x7C,0x00,0x00,0x00},
  {0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x00},
  {0x00,0x3C,0x40,0x30,0x40,0x3C,0x00,0x00},
  {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00},
  {0x00,0x1C,0xA0,0xA0,0x7C,0x00,0x00,0x00},
  {0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00},
  {0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x36,0x08,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x01,0x02,0x01,0x00,0x00},
  {0x00,0x02,0x05,0x05,0x02,0x00,0x00,0x00} 
};

unsigned char grayH;
unsigned char grayL;
char addressingMode;
static i2c device;

void sendCommand(u8 command){
   u8 buffer[2];
   buffer[0] = OLED_Command_Mode;
   buffer[1] = command;
   i2c_write(device, OLED_Address, buffer, 2);
}

void sendData(u8 data){
   u8 buffer[2];
   buffer[0] = OLED_Data_Mode;
   buffer[1] = data;
   i2c_write(device, OLED_Address, buffer, 2);
}

void oled_init(){
    // Unlock OLED driver IC MCU interface
    sendCommand(0xFD);
    sendCommand(0x12);
    //Set display off
    sendCommand(0xAE);
    // Switch on display
    sendCommand(0xAF);
    // Set normal display
    sendCommand(0xA4);
    // Init gray level for text. Default:Brightest White
    grayH= 0xF0;
    grayL= 0x0F;
}

void setContrastLevel(){
    unsigned char ContrastLevel;
    ContrastLevel = (u8) MAILBOX_DATA(0);
    sendCommand(OLED_Set_ContrastLevel_Cmd);
    sendCommand(ContrastLevel);
}

void setHorizontalMode(){
    addressingMode = HORIZONTAL_MODE;
    sendCommand(0x20);             //set addressing mode
    sendCommand(0x00);             //set horizontal addressing mode
}

void setPageMode(){
    addressingMode = PAGE_MODE;
    sendCommand(0x20);             //set addressing mode
    sendCommand(0x02);             //set page addressing mode
}

void setTextXY(int Row, int Column){
    sendCommand(0xB0 + Row);                   //set page address
    sendCommand(0x00 + (8*Column & 0x0F));     //set column lower address
    sendCommand(0x10 + ((8*Column>>4)&0x0F));  //set column higher address
}

void clearDisplay(){
    int i,j;
    sendCommand(OLED_Display_Off_Cmd);     //display off
    for(j=0;j<8;j++)
    {
        setTextXY(j,0);
        {
            for(i=0;i<16;i++)             //clear all columns
            {
                putChar(' ');
            }
        }
    }
    sendCommand(OLED_Display_On_Cmd);     //display on
    setTextXY(0,0);
}

void setGrayLevel(){
    unsigned char grayLevel;
    grayLevel = (u8) MAILBOX_DATA(0);
    grayH = (grayLevel << 4) & 0xF0;
    grayL =  grayLevel & 0x0F;
}

void putChar(unsigned char C){
    if(C < 32 || C > 127) //Ignore non-printable ASCII characters.
    {
        C=' '; //Space
    }
    
    int i;
    for(i=0;i<8;i++){
        sendData(BasicFont[C-32][i]);
    }
}

void putString(){
    int length;
    char ch;
    int i;

    length=MAILBOX_DATA(0);
    for(i=0; i<length; i++){
        ch = MAILBOX_DATA(1+i);
        putChar(ch);
    }

}

void setHorizontalScrollProperties(){
    unsigned char direction;
    unsigned char startPage;
    unsigned char endPage;
    unsigned char scrollSpeed;
    
    direction = (u8) MAILBOX_DATA(0);
    startPage = (u8) MAILBOX_DATA(1);
    endPage = (u8) MAILBOX_DATA(2);
    scrollSpeed = (u8) MAILBOX_DATA(3);
    
    if(Scroll_Right == direction)
    {
        sendCommand(0x26);//Scroll Right
    }
    else
    {
        sendCommand(0x27);//Scroll Left
    }
    sendCommand(0x00);
    sendCommand(startPage);
    sendCommand(scrollSpeed);
    sendCommand(endPage);
    sendCommand(0x00);
    sendCommand(0xFF);
}

void activateScroll(){
    sendCommand(OLED_Activate_Scroll_Cmd);
}

void deactivateScroll(){
    sendCommand(OLED_Dectivate_Scroll_Cmd);
}

void setNormalDisplay(){
    sendCommand(OLED_Normal_Display_Cmd);
}

void setInverseDisplay(){
    sendCommand(OLED_Inverse_Display_Cmd);
}

int main(void)
{
   int cmd;
   int Row, Column;

   device = i2c_open_device(0);
   oled_init();

   while(1){

     // wait and store valid command
      while((MAILBOX_CMD_ADDR & 0x01)==0); 
      cmd = MAILBOX_CMD_ADDR; 

      switch(cmd){
         case CONFIG_IOP_SWITCH:
            // use dedicated I2C - no operation needed
            oled_init();
            clearDisplay();
            MAILBOX_CMD_ADDR = 0x0;
            break;
            
         case SET_NORMAL_DISPLAY:
            setNormalDisplay();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_INVERSE_DISPLAY:
            setInverseDisplay();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_GRAY_LEVEL:
            setGrayLevel(0xFF);
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_PAGE_MODE:   
            setPageMode();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_HORIZONTAL_MODE:
            setHorizontalMode();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_TEXT_XY:
            Row = (u32) MAILBOX_DATA(0);
            Column = (u32) MAILBOX_DATA(1);
            setTextXY(Row, Column);
            MAILBOX_CMD_ADDR = 0x0;
            break;
            
         case CLEAR_DISPLAY:
            clearDisplay();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_CONTRAST_LEVEL:
            setContrastLevel();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
         
         case PUT_STRING:
            putString();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case SET_HORIZONTAL_SCROLL:
            setHorizontalScrollProperties();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case ENABLE_SCROLL:
            activateScroll();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         case DISABLE_SCROLL:
            deactivateScroll();
            MAILBOX_CMD_ADDR = 0x0; 
            break;
            
         default:
            MAILBOX_CMD_ADDR = 0x0; // reset command
            break;
      }
   }
   return 0;
}
