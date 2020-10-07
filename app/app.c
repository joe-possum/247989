/***************************************************************************//**
 * @file  app.c
 * @brief Application code
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

/* Bluetooth stack headers */
#include "native_gecko.h"
#include "gatt_db.h"

/* Timer definitions */
#include "app_timer.h"

/* Switch app headers */
#include "mesh_proxy.h"

/* Coex header */
#include "coexistence-ble.h"

#include <stdio.h>
#include <darwin_log.h>

/***************************************************************************//**
 * @addtogroup Application
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup app
 * @{
 ******************************************************************************/

/*******************************************************************************
 * Provisioning bearers defines.
 ******************************************************************************/
#define PB_ADV   0x1 ///< Advertising Provisioning Bearer
#define PB_GATT  0x2 ///< GATT Provisioning Bearer

/// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/// Address of the Primary Element of the Node
static uint16_t _my_address = 0;
/// number of active Bluetooth connections
static uint8_t num_connections = 0;
/// handle of the last opened LE connection
static uint8_t conn_handle = 0xFF;
/// Flag for indicating that provisioning procedure is finished
static uint8_t provisioning_finished = 0;

/*******************************************************************************
 * Function prototypes.
 ******************************************************************************/
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);
static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *pEvt);

/***************************************************************************//**
 * Initialise used bgapi classes.
 ******************************************************************************/
static void gecko_bgapi_classes_init(void)
{
   gecko_bgapi_class_dfu_init();
   gecko_bgapi_class_system_init();
   gecko_bgapi_class_le_gap_init();
   gecko_bgapi_class_le_connection_init();
   gecko_bgapi_class_gatt_server_init();
   gecko_bgapi_class_hardware_init();
   gecko_bgapi_class_flash_init();
   gecko_bgapi_class_test_init();
   gecko_bgapi_class_mesh_node_init();
   gecko_bgapi_class_mesh_proxy_init();
   gecko_bgapi_class_mesh_proxy_server_init();
   gecko_bgapi_class_mesh_lpn_init();
   gecko_bgapi_class_mesh_generic_client_init();
   gecko_bgapi_class_mesh_scene_client_init();
}

/*******************************************************************************
 * Main application code.
 * @param[in] pConfig  Pointer to stack configuration.
 ******************************************************************************/
void appMain(const gecko_configuration_t *pConfig)
{
   // Initialize stack
   gecko_stack_init(pConfig);
   gecko_bgapi_classes_init();

   // Initialize coexistence interface. Parameters are taken from HAL config.
   gecko_initCoexHAL();

   while(1) {
      // Event pointer for handling events
      struct gecko_cmd_packet* evt;

      // Check for stack event
      evt = gecko_wait_event();

      bool pass = mesh_bgapi_listener(evt);
      if(pass) {
         handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
      }
      else {
         LOG_GECKO_EVENT(evt);
      }
   }
}

/***************************************************************************//**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 *
 * @param[in] pAddr  Pointer to Bluetooth address.
 ******************************************************************************/
static void set_device_name(bd_addr *pAddr)
{
   char name[20];
   uint16_t res;

   // create unique device name using the last two bytes of the Bluetooth address
   sprintf(name,"Gateway %02x:%02x", pAddr->addr[1],pAddr->addr[0]);

   LOG("Device name: '%s'\n",name);

   // write device name to the GATT database
   res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8_t *)name)->result;
   if(res) {
      ELOG("gecko_cmd_gatt_server_write_attribute_value() failed: 0x%x\n",res);
   }
}

/***************************************************************************//**
 * This function is called to initiate factory reset. Factory reset may be
 * initiated by keeping one of the WSTK pushbuttons pressed during reboot.
 * Factory reset is also performed if it is requested by the provisioner
 * (event gecko_evt_mesh_node_reset_id).
 ******************************************************************************/
static void initiate_factory_reset(void)
{
   LOG("factory reset\n");

   /* if connection is open then close it before rebooting */
   if(conn_handle != 0xFF) {
      gecko_cmd_le_connection_close(conn_handle);
   }

   /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
      that have been configured for this node */
   gecko_cmd_flash_ps_erase_all();
   // reboot after a small delay
   gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000),
                                     FACTORY_RESET_TIMER,
                                     SINGLE_SHOT);
}

/***************************************************************************//**
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] pEvt    Pointer to incoming event.
 ******************************************************************************/
static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *pEvt)
{
   uint16_t result;
   volatile bool FactoryReset = false;

   if(NULL == pEvt) {
      return;
   }

   LOG_GECKO_EVENT(pEvt);
   switch(evt_id) {
      case gecko_evt_system_boot_id:
         if(FactoryReset) {
            initiate_factory_reset();
         }
         else {
            struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();

            set_device_name(&pAddr->address);

            // Initialize Mesh stack in Node operation mode, it will generate initialized event
            result = gecko_cmd_mesh_node_init()->result;
            LOG("gecko_cmd_mesh_node_init returned: 0x%x\n",result);
            if(result) {
               ELOG("gecko_cmd_mesh_node_init failed: 0x%x\n",result);
            }
         }
         break;

      case gecko_evt_hardware_soft_timer_id:
         switch(pEvt->data.evt_hardware_soft_timer.handle) {
            case FACTORY_RESET_TIMER:
               // reset the device to finish factory reset
               gecko_cmd_system_reset(0);
               break;

            case RESTART_TIMER:
               // restart timer expires, reset the device
               gecko_cmd_system_reset(0);
               break;

            case PROVISIONING_TIMER:
               // toggle LED to indicate the provisioning state
               if(!provisioning_finished) {
               }
               break;

            default:
               break;
         }

         break;

      case gecko_evt_mesh_node_initialized_id:
         LOG("node initialized\n");

         // Initialize generic client models
         result = gecko_cmd_mesh_generic_client_init_on_off()->result;
         if(result) {
            LOG("mesh_generic_client_init_on_off failed, code 0x%x\n", result);
         }
         result = gecko_cmd_mesh_generic_client_init_lightness()->result;
         if(result) {
            LOG("mesh_generic_client_init_lightness failed, code 0x%x\n", result);
         }
         result = gecko_cmd_mesh_generic_client_init_ctl()->result;
         if(result) {
            LOG("mesh_generic_client_init_ctl failed, code 0x%x\n", result);
         }
         result = gecko_cmd_mesh_generic_client_init_common()->result;
         if(result) {
            LOG("mesh_generic_client_init_common failed, code 0x%x\n", result);
         }

         // Initialize scene client model
         result = gecko_cmd_mesh_scene_client_init(0)->result;
         if(result) {
            LOG("mesh_scene_client_init failed, code 0x%x\n", result);
         }

         struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(pEvt->data);

         if(pData->provisioned) {
            LOG("node is provisioned. address:%x, ivi:%ld\n", pData->address, pData->ivi);

            _my_address = pData->address;

         }
         else {
            LOG("node is unprovisioned\n");

            LOG("starting unprovisioned beaconing...\n");
            // Enable ADV and GATT provisioning bearer
            gecko_cmd_mesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);
         }
         break;

      case gecko_evt_system_external_signal_id:
         break;

      case gecko_evt_mesh_node_provisioning_started_id:
         LOG("Started provisioning\n");
         // start timer for blinking LEDs to indicate which node is being provisioned
         gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(250),
                                           PROVISIONING_TIMER,
                                           REPEATING);
         break;

      case gecko_evt_mesh_node_provisioned_id:
         provisioning_finished = 1;
         LOG("node provisioned, got address=%x\n", pEvt->data.evt_mesh_node_provisioned.address);
         // stop LED blinking when provisioning complete
         gecko_cmd_hardware_set_soft_timer(TIMER_STOP,
                                           PROVISIONING_TIMER,
                                           REPEATING);
         break;

      case gecko_evt_mesh_node_provisioning_failed_id:
         LOG("provisioning failed, code 0x%x\n", pEvt->data.evt_mesh_node_provisioning_failed.result);
         /* start a one-shot timer that will trigger soft reset after small delay */
         gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000),
                                           RESTART_TIMER,
                                           SINGLE_SHOT);
         break;

      case gecko_evt_mesh_node_key_added_id:
         LOG("got new %s key with index 0x%x\n",
             pEvt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
             pEvt->data.evt_mesh_node_key_added.index);
         break;

      case gecko_evt_mesh_node_model_config_changed_id:
         LOG("model config changed\n");
         break;

      case gecko_evt_mesh_node_config_set_id:
         LOG("model config set\n");
         break;

      case gecko_evt_le_connection_opened_id:
         LOG("evt:gecko_evt_le_connection_opened_id\n");
         num_connections++;
         conn_handle = pEvt->data.evt_le_connection_opened.connection;
         break;

      case gecko_evt_le_connection_closed_id:
         /* Check if need to boot to dfu mode */
         if(boot_to_dfu) {
            /* Enter to DFU OTA mode */
            gecko_cmd_system_reset(2);
         }

         LOG("evt:conn closed, reason 0x%x\n", pEvt->data.evt_le_connection_closed.reason);
         conn_handle = 0xFF;
         if(num_connections > 0) {
            if(--num_connections == 0) {
            }
         }
         break;

      case gecko_evt_mesh_node_reset_id:
         LOG("evt gecko_evt_mesh_node_reset_id\n");
         initiate_factory_reset();
         break;

      case gecko_evt_le_connection_parameters_id:
         LOG("connection params: interval %d, timeout %d\n",
             pEvt->data.evt_le_connection_parameters.interval,
             pEvt->data.evt_le_connection_parameters.timeout);
         break;

      case gecko_evt_le_gap_adv_timeout_id:
         // these events silently discarded
         break;

      case gecko_evt_gatt_server_user_write_request_id:
         if(pEvt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
            /* Set flag to enter to OTA mode */
            boot_to_dfu = 1;
            /* Send response to Write Request */
            gecko_cmd_gatt_server_send_user_write_response(
                                                          pEvt->data.evt_gatt_server_user_write_request.connection,
                                                          gattdb_ota_control,
                                                          bg_err_success);

            /* Close connection to enter to DFU OTA mode */
            gecko_cmd_le_connection_close(pEvt->data.evt_gatt_server_user_write_request.connection);
         }
         break;

      case gecko_evt_mesh_proxy_connected_id:
      case gecko_evt_mesh_proxy_disconnected_id:
         handle_mesh_proxy_events(pEvt);
         break;

      default:
         //LOG("unhandled evt: %8.8x class %2.2x method %2.2x\n", evt_id, (evt_id >> 16) & 0xFF, (evt_id >> 24) & 0xFF);
         break;
   }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
