#include "can.h"
#include "string.h"
#include "display_handler.h"

uint16_t fuel_consumption;
uint32_t fuel_cons_cnt;
uint8_t can_cnt = 0;

uint16_t GetLitersPerHour()
{
	uint16_t res = fuel_consumption * 8.0;
	res /= 100;
	return res;
}
extern uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
void CanRxHandler(CanRxMsg* RxMessage)
{
	if (RxMessage->IDE == CAN_Id_Standard)
	{
		if (RxMessage->StdId == CAN_ID_FUEL_CONSUMPTION)		
		{			
			if (RxMessage->DLC == 6)
			{
				can_cnt++;
				uint16_t tmp =  RxMessage->Data[0];
				tmp <<= 8;
				tmp |= RxMessage->Data[1];
				if (tmp == 0xFFFF)
				{
					tmp = 0;
				}				
				fuel_cons_cnt += tmp;
				if (can_cnt == 10)
				{
					can_cnt = 0;
					fuel_consumption = fuel_cons_cnt / 10;					
					fuel_cons_cnt = 0;					
				}							
			}
		}
		if (RxMessage->StdId == 0x000)		
		{
			RxMessage->DLC = 	0x00;
			RxMessage->ExtId = 	0x00;
			RxMessage->FMI = 	0x00;
			RxMessage->IDE = 	0x00;
			RxMessage->RTR = 	0x00;
			RxMessage->StdId = 	0x00;
			RxMessage->Data[0] = 0x00;
			RxMessage->Data[1] = 0x00;
			RxMessage->Data[2] = 0x00;
			RxMessage->Data[3] = 0x00;
			RxMessage->Data[4] = 0x00;
			RxMessage->Data[5] = 0x00;
			RxMessage->Data[6] = 0x00;
			RxMessage->Data[7] = 0x00;
		}
	}
}
void CanSendMessage(CanTxMsg* TxMessage)
{
	CAN_Transmit(CAN1, TxMessage);	
}

void CanBeep(uint8_t beep_type)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = CAN_ID_BEEP;  	
	TxMessage.ExtId = 0x00;  
	TxMessage.IDE = CAN_Id_Standard;  				
	TxMessage.RTR = CAN_RTR_DATA;  					
	TxMessage.DLC = 5;
	memset(TxMessage.Data, 0x00, 8);
	
	TxMessage.Data[0] = beep_type;
	CanSendMessage(&TxMessage);
}
