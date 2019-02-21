/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  /* USER CODE END Header */

  /* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcdcmd.h"
#include "GUI.h"
#include "mainMenu.h"
#include "guivars.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
Variables variables;
uint8_t cnt;
RTC_TimeTypeDef sTime;
RTC_TimeTypeDef sTimePrev;
RTC_DateTypeDef sDate;
RTC_DateTypeDef sDatePrev;
extern Lines line[4];
extern Handles handles;
WM_MESSAGE msg;
WM_MESSAGE msgButton;
extern GUI_Vars gui_Vars;
extern TimeCalibration timeCalibr;
uint8_t tickSecond = 0;
extern uint16_t backgroundBuffer[18];
extern LongPressCNT longPressCNT;
extern DaylightSaving daylightSaving;
GUI_ALLOC_INFO pInfo;
uint8_t doAfterStart = 0;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

CRC_HandleTypeDef hcrc;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

SRAM_HandleTypeDef hsram1;

osThreadId defaultTaskHandle;
osThreadId GUIHandle;
osThreadId TaskLine0Handle;
osThreadId TaskLine1Handle;
osThreadId TaskLine2Handle;
osThreadId TaskLine3Handle;
osThreadId LCD_LinesTimeHandle;
osMessageQId queueLCDLinesTimeHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM7_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
void StartDefaultTask(void const * argument);
void vTaskGUI(void const * argument);
void vTaskLine0(void const * argument);
void vTaskLine1(void const * argument);
void vTaskLine2(void const * argument);
void vTaskLine3(void const * argument);


/* USER CODE BEGIN PFP */
void delay(uint32_t delayTime);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void delay(uint32_t delayTime)
{
	uint32_t i;
	for (i = 0; i < delayTime; i++);
}
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	if (RTC_IT_SEC)
	{

		tickSecond = 1;

	}
}
void longPressControl(void)
{
	char text[9];
	if (longPressCNT.direction == 0)
	{
		return;
	}
	if (longPressCNT.direction < 0)
	{
		if (longPressCNT.value <= longPressCNT.lowerLimit)
		{
			return;
		}

	}
	if (longPressCNT.direction > 0)
	{
		if (longPressCNT.value >= longPressCNT.upperLimit)
		{
			return;
		}
	}
	if (longPressCNT.it && BUTTON_IsPressed(longPressCNT.button))
	{
		longPressCNT.itCNT++;
		if (longPressCNT.itCNT > 3)
		{
			longPressCNT.value += longPressCNT.direction;
			if (gui_Vars.menuState >= MENU_STATE_LINE1SETUP_PULSE && gui_Vars.menuState <= MENU_STATE_LINE4SETUP_PULSE)
			{
				sprintf(text, "%4d", longPressCNT.value * LINE_WIDTH_MULT);
			}
			else if ((gui_Vars.menuState == MENU_STATE_TIME_SUMWIN || gui_Vars.menuState == MENU_STATE_TIMECALIBRATION || (gui_Vars.menuState >= MENU_STATE_LINE1SETUP && gui_Vars.menuState <= MENU_STATE_LINE4SETUP)) && longPressCNT.lowerLimit < 0)
			{
				if (longPressCNT.value > 0)
				{
					sprintf(text, "+%d", longPressCNT.value);
				}
				else if (longPressCNT.value < 0)
				{
					sprintf(text, "%d", longPressCNT.value);
				}
				else if (longPressCNT.value == 0)
				{
					sprintf(text, "%d", 0);
				}

			}

			else
			{
				sprintf(text, "%02d", longPressCNT.value);
			}
			HEADER_SetItemText(longPressCNT.header, longPressCNT.headerItem, text);
			HEADER_SetTextColor(longPressCNT.header, GUI_WHITE);
			longPressCNT.it = 0;
			longPressCNT.itCNT = 4;
		}
	}
	else
	{
		longPressCNT.itCNT = 0;
	}




}

void LinesInit(void)
{
	uint8_t i = 0;
	uint16_t dataInBKP;
	uint16_t buff;
	//2 ����� * 4 = 8 ����
	// 15    14    13    12   11   10    9    8    7    6    5    4    3    2    1    0
	// \status/		\---width--/	\--------------------hours*minutes-----------------/	
	//    |				  |										|
	//	  0 = STOP		  |										|
	//	  1 = RUN		  |										|
	//				 (0..7)*375	= 0,375...3000 sec. 			|
	//					0 sec = line status OFF					|
	//													1440 minutes (1 day)	
	line[0].xSemaphore = xSemaphoreCreateCounting(720, 0);
	line[1].xSemaphore = xSemaphoreCreateCounting(720, 0);
	line[2].xSemaphore = xSemaphoreCreateCounting(720, 0);
	line[3].xSemaphore = xSemaphoreCreateCounting(720, 0);

	line[0].LineGPIOpos = LINE0_POS_OUTPUT_GPIO_Port;
	line[0].LineGPIOneg = LINE0_NEG_OUTPUT_GPIO_Port;
	line[0].LinePinPos = LINE0_POS_OUTPUT_Pin;
	line[0].LinePinNeg = LINE0_NEG_OUTPUT_Pin;

	line[1].LineGPIOpos = LINE1_POS_OUTPUT_GPIO_Port;
	line[1].LineGPIOneg = LINE1_NEG_OUTPUT_GPIO_Port;
	line[1].LinePinPos = LINE1_POS_OUTPUT_Pin;
	line[1].LinePinNeg = LINE1_NEG_OUTPUT_Pin;

	line[2].LineGPIOpos = LINE2_POS_OUTPUT_GPIO_Port;
	line[2].LineGPIOneg = LINE2_NEG_OUTPUT_GPIO_Port;
	line[2].LinePinPos = LINE2_POS_OUTPUT_Pin;
	line[2].LinePinNeg = LINE2_NEG_OUTPUT_Pin;

	line[3].LineGPIOpos = LINE3_POS_OUTPUT_GPIO_Port;
	line[3].LineGPIOneg = LINE3_NEG_OUTPUT_GPIO_Port;
	line[3].LinePinPos = LINE3_POS_OUTPUT_Pin;
	line[3].LinePinNeg = LINE3_NEG_OUTPUT_Pin;

	for (i = 0; i < LINES_AMOUNT; ++i)
	{
		dataInBKP = rtc_read_backup_reg(i + BKP_LINE1_OFFSET);
		line[i].Hours = (dataInBKP & 0b11111111111) / 60;
		line[i].Minutes = (dataInBKP & 0b11111111111) % 60;
		line[i].Width = (dataInBKP >> 11) & 0b111;
		line[i].Status = (dataInBKP >> 14) & 0b11;
		//�������� �� ������, � ���� ���, �� ��� �� �����, � ���� �����. 
		if ((line[i].Hours > 23) || (line[i].Minutes > 59) || (line[i].Width > 15) || (line[i].Status > 2))
		{
			line[i].Minutes = 0;
			line[i].Hours = 0;
			line[i].Width = 0;
			line[i].Status = LINE_STATUS_OFF;
		}

	}

	dataInBKP = rtc_read_backup_reg(BKP_LINES_TIMEZONE_OFFSET);
	for (i = 1; i < LINES_AMOUNT; ++i)
	{
		buff = (dataInBKP >> ((i - 1) * 5));
		if (buff & 0b10000)
		{
			line[i].TimeZone = (char)(~(buff & 0b1111));
		}
		else
		{
			line[i].TimeZone = (char)(buff & 0b1111);
		}
	}

	readDaylightSavingFromBKP();


	if (sTime.Hours == 1 && sTime.Minutes == 2 && sTime.Seconds == 30 && isDaylightSavingTimeEU(sDate.Date, sDate.Month, sDate.WeekDay))
	{
		doAfterStart = true;       //���� ����� 01:02:00 � ������� ���� - ���� �������� �� ������/������ �����
	}
	else
	{
		doAfterStart = false;
	}
	//�������� ���������� ������� �� �����1 �� ��������� � ���������� �������, ������� �� BKP


}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_FSMC_Init();
	MX_CRC_Init();
	MX_SPI1_Init();
	MX_RTC_Init();
	MX_TIM7_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_ADC3_Init();
	/* USER CODE BEGIN 2 */
	//GUI_Init();
	HAL_RTCEx_SetSecond_IT(&hrtc);
	//Init_SSD1289();

	variables.calibrated = 1;
	LinesInit();
	readLinesPolarityFromBKP();

	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
				/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
				/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
				/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* definition and creation of GUI */
	osThreadDef(GUI, vTaskGUI, osPriorityNormal, 0, 600);
	GUIHandle = osThreadCreate(osThread(GUI), NULL);

	/* definition and creation of TaskLine0 */
	osThreadDef(TaskLine0, vTaskLine0, osPriorityNormal, 0, 128);
	TaskLine0Handle = osThreadCreate(osThread(TaskLine0), NULL);

	/* definition and creation of TaskLine1 */
	osThreadDef(TaskLine1, vTaskLine1, osPriorityNormal, 0, 128);
	TaskLine1Handle = osThreadCreate(osThread(TaskLine1), NULL);

	/* definition and creation of TaskLine2 */
	osThreadDef(TaskLine2, vTaskLine2, osPriorityNormal, 0, 128);
	TaskLine2Handle = osThreadCreate(osThread(TaskLine2), NULL);

	/* definition and creation of TaskLine3 */
	osThreadDef(TaskLine3, vTaskLine3, osPriorityNormal, 0, 128);
	TaskLine3Handle = osThreadCreate(osThread(TaskLine3), NULL);

	/* definition and creation of LCD_LinesTime */
	//osThreadDef(LCD_LinesTime, vTaskLCDLinesTime, osPriorityNormal, 0, 128);
	//LCD_LinesTimeHandle = osThreadCreate(osThread(LCD_LinesTime), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
				/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Create the queue(s) */
	/* definition and creation of queueLCDLinesTime */
	//osMessageQDef(queueLCDLinesTime, 4, int16_t);
	//queueLCDLinesTimeHandle = osMessageCreate(osMessageQ(queueLCDLinesTime), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
				/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */


	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0)
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	}
	else
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	}
	//RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	//RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
	if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0)
	{
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
		PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	}
	else
	{
		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	}
	//PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
	//PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */
	/**Common config
	*/
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}
	/**Configure Regular Channel
	*/
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */
	/**Common config
	*/
	hadc2.Instance = ADC2;
	hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc2) != HAL_OK)
	{
		Error_Handler();
	}
	/**Configure Regular Channel
	*/
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

	/* USER CODE BEGIN ADC3_Init 0 */

	/* USER CODE END ADC3_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC3_Init 1 */

	/* USER CODE END ADC3_Init 1 */
	/**Common config
	*/
	hadc3.Instance = ADC3;
	hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc3) != HAL_OK)
	{
		Error_Handler();
	}
	/**Configure Regular Channel
	*/
	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC3_Init 2 */

	/* USER CODE END ADC3_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	if (HAL_CRC_Init(&hcrc) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

	/* USER CODE BEGIN RTC_Init 0 */
	uint8_t i = 0;
	uint32_t counter = 0, hours = 0;
	uint16_t read = 0, high1, high2, low;
	/* USER CODE END RTC_Init 0 */

	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef DateToUpdate = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */
	/**Initialize RTC Only
	*/
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;

	if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0)
	{
		if (HAL_RTC_Init(&hrtc) != HAL_OK)
		{
			Error_Handler();
		}
	}
	else
	{
		HAL_PWR_EnableBkUpAccess();
		__HAL_RCC_BKP_CLK_ENABLE();
		/* Peripheral clock enable */
		__HAL_RCC_RTC_ENABLE();
		/* RTC interrupt Init */
		HAL_NVIC_SetPriority(RTC_IRQn, 15, 0);
		HAL_NVIC_EnableIRQ(RTC_IRQn);
	}
	/* USER CODE BEGIN Check_RTC_BKUP */
	read = rtc_read_backup_reg(BKP_DATE_OFFSET);
	if (!isCRC_OK_BKP())
	{
		DateToUpdate.Date = 12;
	}
	if (((read & 0b1111111000000000) >> 9) < 100 && ((read & 0b111100000) >> 5) < 13 && (read & 0b11111) < 32)
	{
		DateToUpdate.Date = read & 0b11111;
		DateToUpdate.Month = (read & 0b111100000) >> 5;
		DateToUpdate.Year = (read & 0b1111111000000000) >> 9;

		DateToUpdate.WeekDay = (DateToUpdate.Date += DateToUpdate.Month < 3 ? DateToUpdate.Year-- : DateToUpdate.Year - 2, 23 * DateToUpdate.Month / 9 + DateToUpdate.Date + 4 + DateToUpdate.Year / 4 - DateToUpdate.Year / 100 + DateToUpdate.Year / 400) % 7;
		DateToUpdate.Date = read & 0b11111;
		DateToUpdate.Month = (read & 0b111100000) >> 5;
		DateToUpdate.Year = (read & 0b1111111000000000) >> 9;
		high1 = RTC->CNTH;
		low = RTC->CNTL;
		high2 = RTC->CNTH;
		if (high1 != high2)
		{
			counter = RTC->CNTL;
			counter |= (high2 << 16);
		}
		else
		{
			counter = low;
			counter |= (high1 << 16);
		}

		hours = counter / 3600U;
		if (hours >= 24U)
		{

			//increaseDay(&DateToUpdate);

		}
	}
	else
	{
		DateToUpdate.Date = 21;
		DateToUpdate.Month = 1;
		DateToUpdate.Year = 19;
		DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
	}



	/* USER CODE END Check_RTC_BKUP */

	/**Initialize RTC and set the Time and Date
	*/
	//sTime.Hours = 0x23;
	//sTime.Minutes = 0x51;
	//sTime.Seconds = 0x0;

	//if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	//{
	//	Error_Handler();
	//}
	//DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
	//DateToUpdate.Month = RTC_MONTH_DECEMBER;
	//DateToUpdate.Date = 0x17;
	//DateToUpdate.Year = 0x18;

	if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

	/* USER CODE BEGIN TIM7_Init 0 */

	/* USER CODE END TIM7_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM7_Init 1 */

	/* USER CODE END TIM7_Init 1 */
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 50;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 62999;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM7_Init 2 */

	/* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
		LINE0_NEG_OUTPUT_Pin | LINE1_NEG_OUTPUT_Pin | LINE2_NEG_OUTPUT_Pin | LINE3_NEG_OUTPUT_Pin
		| LCD_RESET_Pin,
		GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC,
		OUTPUT_LINE1_Pin | OUTPUT_LINE2_Pin | OUTPUT_LINE3_Pin | OUTPUT_LINE4_Pin
		| LINE0_POS_OUTPUT_Pin | LINE1_POS_OUTPUT_Pin,
		GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SRAM_CS_GPIO_Port, SRAM_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, LINE3_POS_OUTPUT_Pin | LINE2_POS_OUTPUT_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : LINE0_NEG_OUTPUT_Pin LINE1_NEG_OUTPUT_Pin LINE2_NEG_OUTPUT_Pin LINE3_NEG_OUTPUT_Pin
							 LCD_RESET_Pin */
	GPIO_InitStruct.Pin = LINE0_NEG_OUTPUT_Pin | LINE1_NEG_OUTPUT_Pin | LINE2_NEG_OUTPUT_Pin | LINE3_NEG_OUTPUT_Pin
		| LCD_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : OUTPUT_LINE1_Pin OUTPUT_LINE2_Pin OUTPUT_LINE3_Pin OUTPUT_LINE4_Pin
							 LINE0_POS_OUTPUT_Pin LINE1_POS_OUTPUT_Pin */
	GPIO_InitStruct.Pin = OUTPUT_LINE1_Pin | OUTPUT_LINE2_Pin | OUTPUT_LINE3_Pin | OUTPUT_LINE4_Pin
		| LINE0_POS_OUTPUT_Pin | LINE1_POS_OUTPUT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : SRAM_CS_Pin */
	GPIO_InitStruct.Pin = SRAM_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SRAM_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LINE3_POS_OUTPUT_Pin LINE2_POS_OUTPUT_Pin */
	GPIO_InitStruct.Pin = LINE3_POS_OUTPUT_Pin | LINE2_POS_OUTPUT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pin : NOT_PEN_Pin */
	GPIO_InitStruct.Pin = NOT_PEN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(NOT_PEN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : TOUCH_CS_Pin */
	GPIO_InitStruct.Pin = TOUCH_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{
	FSMC_NORSRAM_TimingTypeDef Timing;


	hsram1.Instance = FSMC_NORSRAM_DEVICE;
	hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

	hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
	hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_NOR;
	hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
	hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
	hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
	hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

	Timing.AddressSetupTime = 2;
	Timing.AddressHoldTime = 0;
	Timing.DataSetupTime = 5;
	Timing.BusTurnAroundDuration = 0;
	Timing.CLKDivision = 0;
	Timing.DataLatency = 0;
	Timing.AccessMode = FSMC_ACCESS_MODE_A;


	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
	{
		Error_Handler();
	}




	__HAL_AFIO_FSMCNADV_DISCONNECTED();

}

/* USER CODE BEGIN 4 */



/* FSMC initialization function */
/*
static void MX_FSMC_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing;


  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_NOR;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

  Timing.AddressSetupTime = 2;
  Timing.AddressHoldTime = 0;
  Timing.DataSetupTime = 5;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 0;
  Timing.DataLatency = 0;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;


  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
	Error_Handler( );
  }




  __HAL_AFIO_FSMCNADV_DISCONNECTED();

} */
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
  /* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

	/* USER CODE BEGIN 5 */
	CalibrDataRead(&variables);




	/* Infinite loop */
	for (;;)
	{


		osDelay(50);

		touch_control(&variables);
		if (longPressCNT.it)
		{
			longPressControl();
		}



	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_vTaskGUI */
/**
* @brief Function implementing the GUI thread.
* @param argument: Not used
* @retval None
*/
void sendMsgToMainMenu(uint16_t message)
{
	if (handles.hMainMenu != 0)
	{
		msg.MsgId = message;
		msg.Data.v = 0xFF;
		WM_SendMessage(handles.hMainMenu, &msg);
	}
}


/* USER CODE END Header_vTaskGUI */
void vTaskGUI(void const * argument)
{
	/* USER CODE BEGIN vTaskGUI */
	//int xPos, yPos;

	uint16_t i = 0;

	GUI_Init();
	//Calibrate(&variables);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	CreateMainMenu();
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	line[0].TimeZone = daylightSaving.timeZone;
	pollLinesOutput(10);


	/* Infinite loop */
	for (; ;)
	{

		GUI_Delay(250);
		GUI_ALLOC_GetMemInfo(&pInfo);

		if (tickSecond) //��. callback  HAL_RTCEx_RTCEventCallback
		{

			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			//����� ���� ����������� ������ ������ � BKP �������



			//����������
			if (sTime.Hours == 0 && sTime.Minutes == 0 && sTime.Seconds == 0)
			{
				timeCalibr.isCalibrated = false;
				daylightSaving.needToShift = true;
			}
			//��������� ���������� � 01:02:00
			if (doAfterStart || (sTime.Hours == 1 && sTime.Minutes == 2 && sTime.Seconds == 30))
			{

				if (timeCalibr.seconds != 0 && timeCalibr.days != 0 && timeCalibr.isCalibrated == false) //���� ���������� ��������
				{
					timeCalibr.daysPassed++;
					if (timeCalibr.daysPassed == timeCalibr.days) //���� ������ ���� ����������
					{
						if (timeCalibr.seconds > 0) //���� �������� �������
						{
							sTime.Seconds += timeCalibr.seconds;                  //��������� �������
							if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
							{
								Error_Handler();
							}
						}
						if (timeCalibr.seconds < 0) //���� ������� �������
						{
							sTime.Seconds += timeCalibr.seconds;                   //��������� �������
							if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
							{
								Error_Handler();
							}
						}
						timeCalibr.daysPassed = 0;                  // 1 => 0
						timeCalibr.isCalibrated = true;

					}

				}

				if (doAfterStart || (daylightSaving.needToShift&& daylightSaving.enableDLS&&isDaylightSavingTimeEU(sDate.Date, sDate.Month, sDate.WeekDay)))
				{
					sTime.Hours += daylightSaving.timeShift;
					pollLinesOutput(10);
					daylightSaving.needToShift = false;
					if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
					{
						Error_Handler();
					}
				}
				doAfterStart = false;

			}
			if (sTimePrev.Seconds == 59) //������ ������
			{
				for (i = 0; i < LINES_AMOUNT; ++i)
				{
					if (line[i].Pulses >= 0)
					{
						xSemaphoreGive(line[i].xSemaphore);
					}
					else
					{
						line[i].Pulses++;
					}
				}

				//for (i = 0; i < sizeof(line) / 3; i++)
				//{
				//	if (line[i].Status == LINE_STATUS_RUN)	//���� ����� ��������, �� ������ ����������� ���������� � ����������
				//	{

				//		line[i].Minutes++;
				//		//����� ����� ����������� ����� �������� �� GPIO
				//		if (line[i].Minutes == 60)
				//		{
				//			line[i].Minutes = 0;
				//			line[i].Hours++;
				//			if (line[i].Hours == 24)
				//			{
				//				line[i].Hours = 0;

				//			}
				//		}
				//		if (gui_Vars.menuState == MENU_STATE_MAIN) TFT_MainMenu_ShowLineTime();
				//		saveLineToBKP(i);
				//	}
				//}
				/****************����� �������� �������� Lines***************************************/
				//if(gui_Vars.menuState == MENU_STATE_MAIN) TFT_MainMenu_ShowLineTime();
			}
			if (sTimePrev.Hours == 23 && sTime.Hours == 0) //�������� ����
			{

				//DateToUpdate.Date = read & 0b11111;
				//DateToUpdate.Month = (read & 0b111100000) >> 5;
				//DateToUpdate.Year = (read & 0b1111111000000000) >> 9;

				saveDateToBKP();
				switch (gui_Vars.menuState) {
				case MENU_STATE_MAIN:
					sendMsg(handles.hMainMenu, WM_DATE_UPDATE);
					break;
				case MENU_STATE_TIMEDATESETUP:
					sendMsg(handles.hTimeDateSetupMenu, WM_DATE_UPDATE);
					break;
				}
			}
			switch (gui_Vars.menuState) {
			case MENU_STATE_MAIN:
				sendMsg(handles.hMainMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_TIMESETUP:
				sendMsg(handles.hTimeSetupMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE1SETUP:
				sendMsg(handles.hLineSetupMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE2SETUP:
				sendMsg(handles.hLineSetupMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE3SETUP:
				sendMsg(handles.hLineSetupMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE4SETUP:
				sendMsg(handles.hLineSetupMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE1SETUP_PULSE:
				sendMsg(handles.hLineSetupPulseMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE2SETUP_PULSE:
				sendMsg(handles.hLineSetupPulseMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE3SETUP_PULSE:
				sendMsg(handles.hLineSetupPulseMenu, WM_SEC_UPDATE);
				break;
			case MENU_STATE_LINE4SETUP_PULSE:
				sendMsg(handles.hLineSetupPulseMenu, WM_SEC_UPDATE);
				break;

			}
			tickSecond = 0;
			sTimePrev = sTime;
		}

	}
	/* USER CODE END vTaskGUI */
}

/* USER CODE BEGIN Header_vTaskLine0 */
/**
* @brief Function implementing the TaskLine0 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLine0 */
void vTaskLine0(void const * argument)
{
	/* USER CODE BEGIN vTaskLine0 */
		/* Infinite loop */
	for (;;)
	{
		xSemaphoreTake(line[0].xSemaphore, portMAX_DELAY);
		lineSendSignal(0);
	}
	/* USER CODE END vTaskLine0 */
}

/* USER CODE BEGIN Header_vTaskLine1 */
/**
* @brief Function implementing the TaskLine1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLine1 */
void vTaskLine1(void const * argument)
{
	/* USER CODE BEGIN vTaskLine1 */
		/* Infinite loop */
	for (;;)
	{
		xSemaphoreTake(line[1].xSemaphore, portMAX_DELAY);
		lineSendSignal(1);
	}
	/* USER CODE END vTaskLine1 */
}

/* USER CODE BEGIN Header_vTaskLine2 */
/**
* @brief Function implementing the TaskLine2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLine2 */
void vTaskLine2(void const * argument)
{
	/* USER CODE BEGIN vTaskLine2 */
		/* Infinite loop */
	for (;;)
	{
		xSemaphoreTake(line[2].xSemaphore, portMAX_DELAY);
		lineSendSignal(2);
	}
	/* USER CODE END vTaskLine2 */
}

/* USER CODE BEGIN Header_vTaskLine3 */
/**
* @brief Function implementing the TaskLine3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLine3 */
void vTaskLine3(void const * argument)
{
	/* USER CODE BEGIN vTaskLine3 */
		/* Infinite loop */
	for (;;)
	{
		xSemaphoreTake(line[3].xSemaphore, portMAX_DELAY);
		lineSendSignal(3);
	}
	/* USER CODE END vTaskLine3 */
}

/* USER CODE BEGIN Header_vTaskLCDLinesTime */
/**
* @brief Function implementing the LCD_LinesTime thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLCDLinesTime */


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
				/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
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
	/* USER CODE BEGIN 6 */
				/* User can add his own implementation to report the file name and line number,
				   tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
				   /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
