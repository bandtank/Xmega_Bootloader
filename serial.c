#include "serial.h"

/*! \brief Initializing UART communcation.
 *
 *  This function initializes the UART communication with generic parameters as mentioned below.
 *  Both Enabling both TRASMISSION and RECEPTION
 *  BAUD RATE configured to BRREG_VALUE
 *  As this is important function of initializing the UART, it has to be called prior to start the communication.
 *
 */
void initbootuart(void)
{
	Port(MY_UART).DIRSET = TX_Pin(MY_UART);
	Uart(MY_UART).BAUDCTRLA = BRREG_VALUE;
	Uart(MY_UART).BAUDCTRLB = (SCALE_VALUE << USART_BSCALE_gp) & USART_BSCALE_gm;
	Uart(MY_UART).CTRLB = USART_RXEN_bm | USART_TXEN_bm; // enable receive and transmit
}

/*! \brief Transmitting a character UART communcation.
 *
 *  This function takes the unsigned char input given to the function and transmits out in the UART communication.
 *
 *  This function is called whenever a single character has to be transmitted in the UART communication.
 *  \param  c     Character value to be transmitted.
 *
 */
void sendchar(unsigned char c)
{
	Uart(MY_UART).DATA = c; // prepare transmission
	while (!(Uart(MY_UART).STATUS & (1 << USART_DREIF_bp)));
}

/*! \brief Receiving a character in UART communcation.
 *
 *  This function confirms the reception of data in UART, receives that character and returns the received character to the called function.
 *
 *  This function is called whenever a single charater has to be received from the UART communication.
 *
 *  \return  Character value received from UART communication.
 */

unsigned char recchar(void)
{
	while(!(Uart(MY_UART).STATUS & USART_RXCIF_bm));  // wait for data
	return Uart(MY_UART).DATA;
}

