#ifndef LINESETUPPULSE_H
	#define LINESETUPPULSE_H	

	#define ID_WINDOW_LINESETUP_PULSE   (GUI_ID_USER + 0x60)
	#define ID_BUTTON_LINESETUP_PULSE_MSECplus   (GUI_ID_USER + 0x61)
	#define ID_HEADER_LINESETUP_PULSE_VALSNAMES   (GUI_ID_USER + 0x62)
	#define ID_HEADER_LINESETUP_PULSE_VALS   (GUI_ID_USER + 0x63)
	#define ID_BUTTON_LINESETUP_PULSE_POLplus   (GUI_ID_USER + 0x64)
	//#define ID_BUTTON_2   (GUI_ID_USER + 0x65)
	#define ID_BUTTON_LINESETUP_PULSE_ENTER   (GUI_ID_USER + 0x66)
	#define ID_BUTTON_LINESETUP_PULSE_ONOFF   (GUI_ID_USER + 0x67)
	#define ID_BUTTON_LINESETUP_PULSE_MSECminus   (GUI_ID_USER + 0x68)
	#define ID_BUTTON_LINESETUP_PULSE_POLminus   (GUI_ID_USER + 0x69)
	//#define ID_BUTTON_7   (GUI_ID_USER + 0x6A)
	#define ID_BUTTON_LINESETUP_PULSE_BACK   (GUI_ID_USER + 0x6B)
	#define ID_HEADER_LINESETUP_PULSE_STATUS   (GUI_ID_USER + 0x6C)
	#define ID_HEADER_LINESETUP_PULSE   (GUI_ID_USER + 0x6D)

	#include "DIALOG.h"
	#include "guivars.h"
	#include "callbacks.h"
	#include "string.h"
	#include "stdio.h"
	#include "stm32f1xx_hal.h"
	#include "cmsis_os.h"
	#include "sram.h"
	#include "LineSetup.h"

	extern GUI_Vars gui_Vars;
	extern Lines line[4];
	extern RTC_TimeTypeDef sTime;
	extern LongPressCNT longPressCNT;
	WM_HWIN CreateLineSetupPulseWindow(void);

#endif
