/**************************************************************************//**
 * @main_series1_GG11.c
 * @brief Demonstrates USART1 as SPI slave.
 * @version 0.0.2
 ******************************************************************************
 * @section License
 * <b>Copyright 2018 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#define RX_BUFFER_SIZE   10
#define TX_BUFFER_SIZE   RX_BUFFER_SIZE

uint8_t RxBuffer[RX_BUFFER_SIZE];
uint8_t RxBufferIndex = 0;

uint8_t TxBuffer[TX_BUFFER_SIZE] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
uint8_t TxBufferIndex = 0;



/**************************************************************************//**
 * @brief USART0 TX IRQ Handler
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
  // Send and receive incoming data
  USART0->TXDATA = (uint32_t)TxBuffer[TxBufferIndex];
  TxBufferIndex++;

  // Stop sending once we've gone through the whole TxBuffer
  if (TxBufferIndex == TX_BUFFER_SIZE)
  {
    TxBufferIndex = 0;
  }
}

/**************************************************************************//**
 * @brief USART0 RX IRQ Handler
 *****************************************************************************/
void USART0_RX_IRQHandler(void)
{
  if (USART0->STATUS & USART_STATUS_RXDATAV)
  {

    // Read data
    RxBuffer[RxBufferIndex] = USART_RxDataGet(USART0);
    RxBufferIndex++;

    if (RxBufferIndex == RX_BUFFER_SIZE)
    {
      // Putting a break point here to view the full RxBuffer,
      // The RxBuffer should be: 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
      RxBufferIndex = 0;
    }

  }

}

/**************************************************************************//**
 * @brief Initialize USART0
 *****************************************************************************/
void initUSART0 (void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART0, true);

  // Configure GPIO mode
  GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 1);    // US0_CLK is input
  GPIO_PinModeSet(gpioPortE, 13, gpioModeInput, 1);    // US0_CS is input
  GPIO_PinModeSet(gpioPortE, 10, gpioModeInput, 1);    // US0_TX (MOSI) is input
  GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 1); // US0_RX (MISO) is push pull

  // Start with default config, then modify as necessary
  USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;
  config.master    = false;
  config.clockMode = usartClockMode0; // clock idle low, sample on rising/first edge
  config.msbf      = true;            // send MSB first
  config.enable    = usartDisable;    // making sure to keep USART disabled until we've set everything up
  USART_InitSync(USART0, &config);
  USART0->CTRL |= USART_CTRL_SSSEARLY;

  // Set USART pin locations
  USART0->ROUTELOC0 = (USART_ROUTELOC0_CLKLOC_LOC0) | // US0_CLK       on location 0 = PE12 per datasheet section 6.4 = EXP Header pin 8
                      (USART_ROUTELOC0_CSLOC_LOC0)  | // US0_CS        on location 0 = PE13 per datasheet section 6.4 = EXP Header pin 10
                      (USART_ROUTELOC0_TXLOC_LOC0)  | // US0_TX (MOSI) on location 0 = PE10 per datasheet section 6.4 = EXP Header pin 4
                      (USART_ROUTELOC0_RXLOC_LOC0);   // US0_RX (MISO) on location 0 = PE11 per datasheet section 6.4 = EXP Header pin 6

  // Enable USART pins
  USART0->ROUTEPEN = USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_CSPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

  // Enabling TX interrupts to transfer whenever
  // there is room in the transmit buffer
  // This should immediately trigger to load the first byte of our TX buffer
  USART_IntClear(USART0, USART_IF_TXBL);
  USART_IntEnable(USART0, USART_IEN_TXBL);
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);

  // Enable USART0 RX interrupts
  USART_IntClear(USART0, USART_IF_RXDATAV);
  USART_IntEnable(USART0, USART_IEN_RXDATAV);
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  NVIC_EnableIRQ(USART0_RX_IRQn);

  // Enable USART0
  USART_Enable(USART0, usartEnable);
}

/**************************************************************************//**
 * @brief Main function
 *****************************************************************************/
int main(void)
{
  // Initialize chip
  CHIP_Init();

  // Initialize USART0 as SPI slave
  initUSART0();

  //packets will begin coming in as soon as the master code starts
  while(1);
}

