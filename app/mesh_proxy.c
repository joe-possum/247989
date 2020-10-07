/***************************************************************************//**
 * @file  mesh_proxy.c
 * @brief Mesh proxy module implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include "mesh_proxy.h"
#include "darwin_log.h"

/***************************************************************************//**
 * @addtogroup MeshProxy
 * @{
 ******************************************************************************/

/// number of active mesh proxy connections
uint8_t num_mesh_proxy_conn = 0;

/***************************************************************************//**
 *  Handling of mesh proxy events.
 *
 *  @param[in] pEvt  Pointer to incoming event.
 ******************************************************************************/
void handle_mesh_proxy_events(struct gecko_cmd_packet *pEvt)
{
  switch (BGLIB_MSG_ID(pEvt->header)) {
    case gecko_evt_mesh_proxy_connected_id:
      LOG("evt:gecko_evt_mesh_proxy_connected_id\n");
      num_mesh_proxy_conn++;
      break;

    case gecko_evt_mesh_proxy_disconnected_id:
      LOG("evt:gecko_evt_mesh_proxy_disconnected_id\n");
      if (num_mesh_proxy_conn > 0) {
         num_mesh_proxy_conn--;
      }
      break;
    default:
      break;
  }
}

/** @} (end addtogroup MeshProxy) */
