/***************************************************************************//**
 * @file  mesh_proxy.h
 * @brief Mesh proxy header file
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

#ifndef MESH_PROXY_H
#define MESH_PROXY_H

#include "native_gecko.h"

/***************************************************************************//**
 * \defgroup MeshProxy
 * \brief Mesh proxy events handling and proxy connection management.
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup MeshProxy
 * @{
 ******************************************************************************/

/// number of active mesh proxy connections
extern uint8_t num_mesh_proxy_conn;

/***************************************************************************//**
 *  Handling of mesh proxy events.
 *
 *  @param[in] pEvt  Pointer to incoming event.
 ******************************************************************************/
void handle_mesh_proxy_events(struct gecko_cmd_packet *pEvt);

/** @} (end addtogroup MeshProxy) */

#endif /* MESH_PROXY_H */
