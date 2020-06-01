/**************************************************************************//**
 * @main_gg11_tg11.c
 * @brief TThis project demonstrates controlling servo motors using pulse width modulation generated by the TIMER
 * module. The GPIO pin specified in the readme.txt is configured for output and
 * outputs a 1kHz, 30% duty cycle signal. The duty cycle can be adjusted by
 * writing to the CCVB or changing the global dutyCyclePercent variable.
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
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "bsp.h"

// Note: change this to set the desired output frequency in Hz
#define PWM_FREQ 1000

// Note: change this to set the desired duty cycle (used to update CCVB value)
static volatile int dutyCyclePercent = 30;

// stores 1 msTicks from the SysTick timer
volatile uint32_t msTicks;


/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER1 that changes the duty cycle
 *
 * @note
 *    This handler doesn't actually dynamically change the duty cycle. Instead,
 *    it acts as a template for doing so. Simply change the dutyCyclePercent
 *    global variable here to dynamically change the duty cycle.
 *****************************************************************************/
void TIMER1_IRQHandler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER1);
  TIMER_IntClear(TIMER1, flags);

  // Update CCVB to alter duty cycle starting next period
  TIMER_CompareBufSet(TIMER1, 0, (TIMER_TopGet(TIMER1) * dutyCyclePercent) / 100);
}

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGpio(void)
{
  // Enable GPIO and clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure PC13 as output for our PWM signal
  GPIO_PinModeSet(gpioPortC, 13, gpioModePushPull, 0);
}

/**************************************************************************//**
 * @brief
 *    TIMER initialization
 *****************************************************************************/
void initTimer(void)
{
  // Enable clock for TIMER1 module
  CMU_ClockEnable(cmuClock_TIMER1, true);

  // Configure TIMER1 Compare/Capture for output compare
  // Use PWM mode, which sets output on overflow and clears on compare events
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  TIMER_InitCC(TIMER1, 0, &timerCCInit);

  // Route TIMER1 CC0 to location 0 and enable CC0 route pin
  // TIM1_CC0 #0 is GPIO Pin PC13
  TIMER1->ROUTELOC0 |=  TIMER_ROUTELOC0_CC0LOC_LOC0;
  TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

  // Set top value to overflow at the desired PWM_FREQ frequency
  TIMER_TopSet(TIMER1, CMU_ClockFreqGet(cmuClock_TIMER1) / PWM_FREQ);

  // Set compare value for initial duty cycle
  TIMER_CompareSet(TIMER1, 0, (TIMER_TopGet(TIMER1) * dutyCyclePercent) / 100);

  // Initialize the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(TIMER1, &timerInit);

  // Enable TIMER1 compare event interrupts to update the duty cycle
  TIMER_IntEnable(TIMER1, TIMER_IEN_CC0);
  NVIC_EnableIRQ(TIMER1_IRQn);
}

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       // increment counter necessary in Delay()
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

/**************************************************************************//**
 * @brief
 *    Main function
 *****************************************************************************/
int main(void)
{
  // Chip errata
  CHIP_Init();

  // Init DCDC regulator with kit specific parameters
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  EMU_DCDCInit(&dcdcInit);

  // Initializations
  initGpio();
  initTimer();

  // Setup SysTick Timer for 1 msec interrupts
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  while (1)
  {
    for(int i = 0; i < 20; i++)
    {
    	dutyCyclePercent = i;
    	Delay(60);
    }

    for(int i = 20; i >= 0; i--)
    {
    	dutyCyclePercent = i;
    	Delay(60);
    }
  }
}

