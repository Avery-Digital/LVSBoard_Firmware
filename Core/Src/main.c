/*******************************************************************************
 * @file    Src/main.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Main Entry point
 *******************************************************************************
 * Copyright (c) 2025 Avery Digital Data
 * All rights reserved.
 ******************************************************************************/

/* Note (Omid):
 * STM32H735IG Clocking Configuration gives you a headache if you don't use
 * STM32CubeMX and accordingly these configuration here are extracted utilizing
 * this tool for configuring the PLLs and HW-SW filters of the configuration file.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private function prototypes -----------------------------------------------*/
static void MPUConfig( void);
void MCUInit( void);
void MCUClockConfig( void);
void PeripheralInit( void);
void PeripheralClockConfig( void);
ErrorStatus BoardInit( void);
ErrorStatus BoardReset( void);
ErrorStatus BoardCheck( void);

/* The application entry point */
int main(void)
{
	ErrorStatus Status = SUCCESS;

	/* Configure MCU */
	MCUInit();
	MCUClockConfig();

	/* Disable Systick */

	/* Configure peripherals */
	PeripheralInit();
	PeripheralClockConfig();

	/* Configure the Board */
	Status += BoardInit();
	while (Status != SUCCESS);

	/* BIST Check */
	Status += BoardCheck();

	/* Configure the Board */
	if (Status == SUCCESS)
	{
		Status += BoardReset();
		while (Status != SUCCESS);
	}
	else
	{
		Status += BoardReset();
	}

	while (1)
	{
		/* Check for pending TX response from command handler */
		if (cmd_tx_request.pending)
		{
			if (CommDriver_TxReady())
			{
				CommDriver_SendPacket(
					cmd_tx_request.msg1,
					cmd_tx_request.msg2,
					cmd_tx_request.cmd1,
					cmd_tx_request.cmd2,
					cmd_tx_request.payload,
					cmd_tx_request.length
				);
				cmd_tx_request.pending = false;
			}
		}
	}
}

/* =========================  System Configuration  ========================= */
/* The system is configured as follow :
 *			VDD(V)                         = 3.3
 *			Supply configuration           = SMPS + LDO
 *			SMPS Output voltage            = 2.5V
 *			LDO Mode                       = Scale0 mode
 *			Flash Latency(WS)              = 3
 * ========================================================================== */
void MCUInit( void)
{
	/* MPU Configuration */
	MPUConfig();

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);

	/* System interrupt init */
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* SysTick_IRQn interrupt configuration */
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

	/* Flash latency config */
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
	while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3);

	/* Configure SMPS and LDO for maximum performance */
	LL_PWR_ConfigSupply(LL_PWR_SMPS_2V5_SUPPLIES_LDO);
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
	while (!LL_PWR_IsActiveFlag_VOS());
}

/* ======================== MCU Clock Configuration ========================= */
/* The MCU Clock is configured as follow :
 *			Clock_Source                   = HSE
 *			PLL_DIVM1                      = 8
 *			PLL_DIVN1                      = 256
 *			PLL_DIVP1                      = 1					// 512MHz
 *			SYSCLK(Hz)                     = 512000000
 *			D1CPRE Prescaler               = 1					// 512MHz
 *			D1CPRE Clock(Hz)               = 512000000
 *			HPRE Prescaler                 = 2					// 256MHz
 *			HCLK Clock (Hz)                = 256000000
 *			D1PPRE Prescaler               = 2					// 128MHz
 *			D2PPRE1 Prescaler              = 2					// 128MHz
 *			D2PPRE2 Prescaler              = 2					// 128MHz
 *			D3PPRE Prescaler               = 2					// 128MHz
 * ========================================================================== */
void MCUClockConfig( void)
{
	/* Configure the HSE port */
	LL_AHB4_GRP1_EnableClock( LL_AHB4_GRP1_PERIPH_GPIOH);

	/* Enable HSE */
	LL_RCC_HSE_Enable();
	while(LL_RCC_HSE_IsReady() != 1);

	/* Enable channels
	 * Channels can be enabled only when the PLL is disabled.
	 */
	LL_RCC_PLL1_Disable();
	while(LL_RCC_PLL1_IsReady() != 0);

	LL_RCC_PLL1P_Enable();

	/* Configure PLL1 */
	LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);
	LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
	LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
	LL_RCC_PLL1_SetM(8);
	LL_RCC_PLL1_SetN(256);
	LL_RCC_PLL1_SetP(1);
	LL_RCC_PLL1_SetQ(2);
	LL_RCC_PLL1_SetR(2);

	/* Turn PLL1 On */
	LL_RCC_PLL1_Enable();
	while(LL_RCC_PLL1_IsReady() != 1);

	/* Check channels */
	while(LL_RCC_PLL1P_IsEnabled() != 1);

	/* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
	LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);

	/* Set SYSCLK source to PLL1P */
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1);

	/* Set prescalers */
	LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
	LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
	LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);

	LL_Init1msTick(SYSCLK);
	LL_SetSystemCoreClock(SYSCLK);
}

/* MPU Configuration */
void MPUConfig( void)
{
  /* Disables the MPU */
  LL_MPU_Disable();

  /* Initializes and configures the Region and the memory to be protected */
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0, 0x87, 0x0, LL_MPU_REGION_SIZE_4GB|LL_MPU_TEX_LEVEL0|LL_MPU_REGION_NO_ACCESS|LL_MPU_INSTRUCTION_ACCESS_DISABLE|LL_MPU_ACCESS_SHAREABLE|LL_MPU_ACCESS_NOT_CACHEABLE|LL_MPU_ACCESS_NOT_BUFFERABLE);

  /* Enables the MPU */
  LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);

}

void PeripheralInit( void)
{
	/* For future use */
}

/* ===================== Peripheral Clock Configuration ===================== */
/* The Peripheral Clock is configured as follow :
 *			Clock_Source                   = HSE
 *			PLL_DIVM3                      = 8
 *			PLL_DIVN3                      = 128
 *			PLL_DIVP3                      = 2					// 128MHz
 *			PLL_DIVQ3                      = 2					// 128MHz
 *			PLL_DIVR3                      = 2					// 128MHz
 * ========================================================================== */
void PeripheralClockConfig( void)
{
	/* PLL3
	 * SPI1 and SPI3 use PLL3P
	 * USART3 uses PLL3Q
	 * I2C1 and I2C4 use PLL3R
	 */

	/* Enable channels
	 * Channels can be enabled only when the PLL is disabled.
	 */
	LL_RCC_PLL3_Disable();
	while(LL_RCC_PLL3_IsReady() != 0);

	LL_RCC_PLL3P_Enable();
	LL_RCC_PLL3Q_Enable();
	LL_RCC_PLL3R_Enable();

	/* Configure PLL3 */
	LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_2_4);
	LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
	LL_RCC_PLL3_SetM(8);
	LL_RCC_PLL3_SetN(128);
	LL_RCC_PLL3_SetP(2);
	LL_RCC_PLL3_SetQ(2);
	LL_RCC_PLL3_SetR(2);

	/* Turn PLL3 On */
	LL_RCC_PLL3_Enable();
	while(LL_RCC_PLL3_IsReady() != 1);

	/* Check channels */
	while (LL_RCC_PLL3P_IsEnabled() != 1);
	while (LL_RCC_PLL3Q_IsEnabled() != 1);
	while (LL_RCC_PLL3R_IsEnabled() != 1);

}

ErrorStatus BoardInit( void)
{
	ErrorStatus Status = SUCCESS;

	/* Visual & Terminal communication initialization */
	Status += StatusManager_Init();

	/* Communication driver initialization (USART3 + DMA + FT231 reset) */
	CommDriver_Init();
	CommandInterface_Init();
	CommDriver_StartRx();

	/* Low Voltage Switcher initialization */
	Status += EVSInit( &VS_ADG714);

 	return Status;
}

ErrorStatus BoardReset( void)
{
	ErrorStatus Status = SUCCESS;

	/* Reset Status Manager */
	Status += StatusManager_Reset();

	/* Reset Voltage Switcher */
	Status += EVSReset( &VS_ADG714);

 	return Status;
}

ErrorStatus BoardCheck( void)
{
	ErrorStatus Status = SUCCESS;

	/* Check Voltage Switcher */
	Status += EVSCheck( &VS_ADG714);

	/* Set BIST */
	if(Status == SUCCESS)
		BISTPass = 1;
	else
		BISTPass = 0;

 	return Status;
}

/* In case of any error */
void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
		/* Loop here */
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */
