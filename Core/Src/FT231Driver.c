/*******************************************************************************
 * @file    Src/FT231Driver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   FT231 Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "FT231Driver.h"

/* Private Variables ---------------------------------------------------------*/
static volatile uint32_t CharactersReceived;
static volatile uint8_t	CRPendingLF 	= 0;
static char		RXString[FT231_Buffer]	= {0};
static char		TXString[FT231_Buffer]	= {0};

/* Public Variables ---------------------------------------------------------*/
char				Message[FT231_Buffer]		= {0};
volatile uint32_t	MessageSize					= 0;
volatile uint8_t	UserMessage					= 0;
volatile uint8_t	ModeBusMessage				= 0;
volatile uint8_t 	ColonDetected				= 0;

/* Private functions ----------------------------------------------------------*/
ErrorStatus FT231MessageWrapUp( void)
{
	ErrorStatus Status = SUCCESS;

	/* Finish the received string */
	RXString[CharactersReceived] = 0x0;

	if (CharactersReceived>0)
	{
		if ((UserMessage + ModeBusMessage) == 0)
		{
			/* Copy RXString to Command */
			strcpy(Message, RXString);
			MessageSize = CharactersReceived;

			/* Set the flags */
			if (Message[0] == ':')
				ModeBusMessage = 1;
			else
				UserMessage = 1;
		}
	}

	if (ColonDetected)
	{
		/* Echo MODBUS packet here */
		/* Wait for other answer to go through*/
		while (!UARTDMATxAvail(USB_FT231.UART_DMATx));

	    // append CRLF
		RXString[CharactersReceived++] = '\r';
		RXString[CharactersReceived++] = '\n';

	    /* Send the data to DMA */
	    Status += UARTDMATxPush(USB_FT231.UART_DMATx, (uint8_t*)RXString, CharactersReceived);
	}
	else
	{
		/* Feed a new line */
		UARTTransmitString ( USB_FT231.UART_Interface, "");
	}

	/* Ready for next string */
	CharactersReceived = 0;
	ColonDetected = 0;
	UARTTransmitByte ( USB_FT231.UART_Interface, '>');

	return Status;
}

ErrorStatus FT231Reception( uint8_t ByteReceived)
{
	/* Search for a enter key */
	switch (ByteReceived)
	{
	case ':':
		/* Enable next package reception */
		if (CRPendingLF) CRPendingLF = 0;

		/* Stop Echoing */
		ColonDetected = 1;

		/* Receive the byte and store it */
		RXString[CharactersReceived] = ByteReceived;
		CharactersReceived++;
		break;
	/* Characters */
    case 'a' ... 'z':
		ByteReceived = ByteReceived & 0xDF;
	case '?':
	case '0' ... '9':
	case 'A' ... 'Z':
		/* Enable next package reception */
		if (CRPendingLF) CRPendingLF = 0;

		if (CharactersReceived < FT231_Buffer-1)
		{
			/* Receive the byte and store it */
			RXString[CharactersReceived] = ByteReceived;
			CharactersReceived++;
			if (!ColonDetected)
				UARTTransmitByte ( USB_FT231.UART_Interface, ByteReceived);
		}
		break;
	/* Back space */
    case '\b':
		/* Enable next package reception */
		if (CRPendingLF) CRPendingLF = 0;

		/* Delete the last character */
		if (CharactersReceived > 0)
        {
            CharactersReceived--;
            RXString[CharactersReceived] = '\0';
			if (!ColonDetected)
			{
				UARTTransmitByte( USB_FT231.UART_Interface, '\b');
				UARTTransmitByte( USB_FT231.UART_Interface, ' ');
				UARTTransmitByte( USB_FT231.UART_Interface, '\b');
			}
        }
        break;
    /*
     * Windows sends CR+LF while Linux only sends LF
     * CR -> OD
     * LF -> 0A
     */
    case '\n':
		if (CRPendingLF)
			CRPendingLF = 0;
		else
			FT231MessageWrapUp ();
		break;
    case '\r':
    	if (!CRPendingLF)
    		CRPendingLF = 1;
    	FT231MessageWrapUp ();
		break;
	default:
		break;
	}
	return SUCCESS;
}

/* Public functions -----------------------------------------------------------*/
ErrorStatus FT231Init( void)
{
	ErrorStatus Status = SUCCESS;

	/* USB initialization */
	Status += UARTInit( USB_FT231.UART_Interface);
	Status += GPIOInit( USB_FT231.nRST_Pin);

	/* USB Reset */
	Status += GPIOReset( USB_FT231.nRST_Pin);
	Delay( FT231_RSTDelay);
	Status += GPIOSet( USB_FT231.nRST_Pin);
	Delay( FT231_PONDelay);

	/* Ready for next string */
	CharactersReceived = 0;

	/* USB DMA initialization */
	Status += UARTDMARxInit( USB_FT231.UART_DMARx);
	Status += UARTDMATxInit( USB_FT231.UART_DMATx);

	return Status;
}

ErrorStatus FT231Reset( void)
{
	ErrorStatus Status = SUCCESS;

	/* Ready for next string */
	CharactersReceived = 0;
	UARTTransmitString( USB_FT231.UART_Interface, "");
	UARTTransmitByte ( USB_FT231.UART_Interface, '>');

	return Status;
}

ErrorStatus FT231Answer( char *Message_String)
{
	ErrorStatus Status = SUCCESS;
	uint32_t length;

	/* Wait for other answer to go through*/
	while (!UARTDMATxAvail(USB_FT231.UART_DMATx));

	/* Copy Message */
    strcpy(TXString, Message_String);
    length = strlen(TXString);

    // append CRLF and prompt '>'
    TXString[length++] = '\r';
    TXString[length++] = '\n';
    TXString[length++] = '>';

    /* Send the data to DMA */
    Status += UARTDMATxPush(USB_FT231.UART_DMATx, (uint8_t*)TXString, length);

	return Status;
}

ErrorStatus FT231AnswerWOS( char *Message_String)
{
	ErrorStatus Status = SUCCESS;
	uint32_t length;

	/* Wait for other answer to go through*/
	while (!UARTDMATxAvail(USB_FT231.UART_DMATx));

	/* Copy Message */
    strcpy(TXString, Message_String);
    length = strlen(TXString);

    // append CRLF
    TXString[length++] = '\r';
    TXString[length++] = '\n';

    /* Send the data to DMA */
    Status += UARTDMATxPush(USB_FT231.UART_DMATx, (uint8_t*)TXString, length);

	return Status;
}

ErrorStatus FT231ReadDMABuffer( void)
{
	ErrorStatus Status = SUCCESS;
    uint8_t Byte;

    if (UserMessage || ModeBusMessage)
    	Status += ERROR;
    else
    {
    	while (UARTDMARxPOP( USB_FT231.UART_DMARx, &Byte) == SUCCESS)
       	{
    		FT231Reception( Byte);
    		if (UserMessage || ModeBusMessage)
    			break;
       	}
    }

    return Status;
}

