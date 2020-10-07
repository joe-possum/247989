/***************************************************************************//**
 * @file
 * @brief Application timer header file
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef APP_TIMER_H
#define APP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/***************************************************************************************************
   Public Macros and Definitions
***************************************************************************************************/

/// Timer Frequency used
#define TIMER_CLK_FREQ ((uint32_t)32768)
/// Convert milliseconds to timer ticks. Maximum valid parameter is 65535999 ms
#define TIMER_MS_2_TIMERTICK(ms) ((uint32_t)((TIMER_CLK_FREQ * (uint64_t)ms) / 1000))
/// Stop timer
#define TIMER_STOP   0
/// Repeating timer mode
#define REPEATING    0
/// Single shot timer mode
#define SINGLE_SHOT  1

/** Application timer enumeration. */
typedef enum {
  /** Restart timer.
   *  This is a single-shot timer used for software reset after provisioning
   *  failed. */
  RESTART_TIMER = 0,
  /** Factory reset timer.
   *  This is a single-shot timer used for software reset after deleting
   *  persistent storage. */
  FACTORY_RESET_TIMER,
  /** Provisioning timer.
   *  This is an auto-reload timer used for LED blinking during provisioning. */
  PROVISIONING_TIMER
} appTimer_t;

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */

#endif /* APP_TIMER_H */
