#include "can.h"
#include "string.h"


void CanRxHandler(CanRxMsg* RxMessage)
{
	if (RxMessage->IDE == CAN_Id_Standard)
	{
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
	TxMessage.StdId = 0x327;  	
	TxMessage.ExtId = 0x00;  
	TxMessage.IDE = CAN_Id_Standard;  				
	TxMessage.RTR = CAN_RTR_DATA;  					
	TxMessage.DLC = 5;
	memset(TxMessage.Data, 0x00, 8);
	CanSendMessage(&TxMessage);
	TxMessage.Data[0] = beep_type;
	CanSendMessage(&TxMessage);
	TxMessage.Data[0] = NO_BEEP;
	CanSendMessage(&TxMessage);
}