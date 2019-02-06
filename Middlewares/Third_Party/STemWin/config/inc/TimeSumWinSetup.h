#ifndef TIMESUMWINSETUP_H
#define TIMESUMWINSETUP_H
#define ID_WINDOW_SUMWINSETUP (GUI_ID_USER + 0x70)
#define ID_BUTTON_SUMWINSETUP_Zplus (GUI_ID_USER + 0x71)
#define ID_HEADER_SUMWINSETUP_VALSNAMES (GUI_ID_USER + 0x72)
#define	ID_HEADER_SUMWINSETUP_VALS (GUI_ID_USER + 0x73)
#define	ID_BUTTON_SUMWINSETUP_SUMWIN_ON (GUI_ID_USER + 0x74)
#define	ID_BUTTON_SUMWINSETUP_ENTER (GUI_ID_USER + 0x75)
#define	ID_BUTTON_SUMWINSETUP_Zminus (GUI_ID_USER + 0x76)	
#define	ID_BUTTON_SUMWINSETUP_SUMWIN_OFF (GUI_ID_USER + 0x77)
#define	ID_BUTTON_SUMWINSETUP_BACK (GUI_ID_USER + 0x78)
#define	ID_HEADER_SUMWINSETUP_STATVALS (GUI_ID_USER + 0x79)
#define	ID_HEADER_SUMWINSETUP_TOP (GUI_ID_USER + 0x7A)
#include "DIALOG.h"
#include "guivars.h"
#include "string.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "TimeCalibrate.h"
#include "callbacks.h"
#include "TimeDateSetup.h"
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern DaylightSaving daylightSaving;
WM_HWIN CreateTimeSumWinSetupWindow(void);

#endif