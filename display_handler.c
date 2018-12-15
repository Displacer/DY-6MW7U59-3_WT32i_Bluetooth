/*
 * idx  16        17      18       19        20          21
 * 01 - SAT       FM      NAME     RPT       DISC IN     C
 * 02 - XM        1 (FM)  TRACK    D. (RPT)  1           D
 * 04 - MD        2       ST       RDM       2           C
 * 08 - AUTO.P    3       RDS      D. (RDM)  3           I
 * 10 - CD        4       PTY      REG       4           R
 * 20 - MP3       LP      DISC     TAG       5           C
 * 40 - HD RADIO  2 (LP)  FOLDER   TP        6           L
 * 80 - CT        4       WMA      NAME      LOAD (LED)  E
 */

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include <string.h>
#include "display_handler.h"
#include "command_handler.h"
#include "usart_opts.h"
#include "config.h"
#include "iwrap.h"

extern enum Mode mode;
extern enum EMainFSM main_fsm;
extern uint8_t Mode_itterupt;

uint8_t defaultScreen[DISPLAY_BUFFER_SIZE] =
{ 0x21, 0xFE, '-', 'M', 'I', 'T', 'S', 'U', 'B', 'I', 'S', 'H', 'I', '-', 0xFF,
      0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D };
uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayRxBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayStringBuffer[DISPLAY_STRING_SIZE];
uint8_t displayDataBuffer[DISPLAY_DATA_SIZE];

int greets_counter = 0;
uint8_t isAux;
uint8_t btLastState = 0;

enum displayState
{
   begin, scroll, end, reverse_scroll
} display_state;

void GetMode()
{
   if (GPIO_ReadOutputDataBit(GPIOA, BT_STBY_PIN)) // if bt module activated
   {
      if (!(isAux || main_fsm == BT_ACTIVE))
         Bluetooth_off();
      return;
   }
   if (displayRxBuffer[16] & 0x10)
   { //All magic constants - for details see table above
      mode = CD;
      return;
   }
   if (displayRxBuffer[17] & 0x01)
   {
      mode = FM;
      return;
   }
   if (displayRxBuffer[3] == 'W')
   {
      if (displayRxBuffer[2] == 'M')
      {
         mode = MW;
         return;
      }
      if (displayRxBuffer[2] == 'L')
      {
         mode = LW;
         return;
      }
   }
   if (memcmp(&displayRxBuffer[6], (uint8_t*) "AUX", 3) == 0)
   {
      mode = AUX;
      return;
   }
   //mode = Normal;
}

int offset; // in future will be moved to constants
uint8_t offset_step;
uint8_t delay;
uint8_t DISPLAY_STRING_DELAY = 10;
uint8_t OFFSET_STEP_DELAY = 1;

void resetDisplayState()
{
   display_state = begin;
   offset = 0;
   delay = 0;
}
void ClearDisplayString()
{
   displayDataBuffer[0] = 0x00;
   displayStringBuffer[0] = 0x00;
}
void HandleDisplayData()
{
   if (CheckChksum(displayRxBuffer, DISPLAY_BUFFER_SIZE) == ERROR)
      return;

   memcpy(displayBuffer, displayRxBuffer, DISPLAY_BUFFER_SIZE);

   //CD mode change ' on : (for details see table)
   if (mode == CD && displayBuffer[11] == '\'')
      displayBuffer[11] = ':';

   isAux = memcmp(&displayRxBuffer[6], (uint8_t*) "AUX", 3) == 0;

   if (mode == Bluetooth)
   {

      if (*displayDataBuffer != 0)
      {

         switch (display_state)
         {

         case begin:
            if (delay < DISPLAY_STRING_DELAY)
               delay++;
            else
               display_state = scroll;
            break;
         case scroll:
            if (displayDataBuffer[offset + DISPLAY_STRING_SIZE] != 0)
            {
               if (offset_step++ == OFFSET_STEP_DELAY)
               {
                  offset++;
                  offset_step = 0;
               }
            }
            else
            {
               display_state = end;
            }
            break;
         case end:
            if (delay > 0)
               delay--;
            else
            {
               display_state = reverse_scroll;
            }
            break;
         case reverse_scroll:
            if (offset != 0)
               offset--;
            else
               display_state = begin;
            break;
         }

         memcpy(displayStringBuffer, &displayDataBuffer[offset],
               DISPLAY_STRING_SIZE);
      }

      if (isAux)
      {

         if (*displayStringBuffer == 0 || playbackState == stop)
         {
            displayBuffer[2] = BLUETOOTH_CHAR;
            memcpy(&displayBuffer[3], (uint8_t*) " Bluetooth ", 11);
         }
         else
         {

            for (uint8_t i = 0; i < DISPLAY_STRING_SIZE; i++)
            {
               if (displayStringBuffer[i] == 0)
                  displayBuffer[i + 2] = 0x2E;
               else
                  displayBuffer[i + 2] = displayStringBuffer[i];
               //SdisplayBuffer[18] |= 0x03; //track name sign
            }
         }

         //if (playbackState == pause)
         //{
         //   displayBuffer[2] = BLUETOOTH_CHAR;
         //   memcpy(&displayBuffer[3], (uint8_t*) " Paused... ", 11);
         //   resetDisplayState();
         //}
      }
      else
         resetDisplayState();
   }

   if (main_fsm == BT_PREPARE && isAux)
   {
      memcpy(&displayBuffer[3], (uint8_t*) "           ", 11);
   }

   if (greets_counter < 30)
   {
      memcpy(displayBuffer, defaultScreen, DISPLAY_BUFFER_SIZE);
   }

   if (displayBuffer[0] == ACC_OFF)
   {

      Mode_itterupt = MODE_ITTERUPT_CYCLES * 2;
      greets_counter = 0; // ACC OFF byte for dimm display
      if (mode == Bluetooth)
      {
         btLastState = 1;
         Bluetooth_off();
      }
      main_fsm = NORMAL_STATE;
      mode = PowerOff;
   }
   else
      greets_counter++;

// cnt++;
// cnt2 = cnt / 5;
// displayBuffer[2] = mode + 48;
   //displayBuffer[3] = btLastState + 48;
// displayBuffer[4] = (ADC_GetConversionValue(ADC1) / 1000) % 10 + 48;
// displayBuffer[5] = (ADC_GetConversionValue(ADC1) / 100) % 10 + 48;
// displayBuffer[6] = (ADC_GetConversionValue(ADC1) / 10) % 10 + 48;
// displayBuffer[7] = ADC_GetConversionValue(ADC1) % 10 + 48;
   //displayBuffer[14] = 0x00;
   SendDisplayData();
}
void SendDisplayData()
{
   uint8_t chksum = 0;

   for (int i = 0; i < DISPLAY_BUFFER_SIZE - 1; i++)
   {
      chksum += displayBuffer[i];
   }
   displayBuffer[DISPLAY_BUFFER_SIZE - 1] = chksum;
   USART2SendDMA();
}
