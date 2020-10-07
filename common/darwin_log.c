/******************************************************************************
* (C) Copyright 2019 Darwin Tech, LLC, http://www.darwintechnologiesllc.com
*******************************************************************************
* This file is licensed under the Darwin Tech Embedded Software License Agreement.
* See the file "Darwin Tech - Embedded Software License Agreement.pdf" for 
* details. Read the terms of that agreement carefully.
*
* Using or distributing any product utilizing this software for any purpose
* constitutes acceptance of the terms of that agreement.
******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "native_gecko.h"
#include "darwin_log.h"

#ifdef DARWIN_DEBUG
void DumpHex(void *AdrIn,int Len)
{
   unsigned char *Adr = (unsigned char *) AdrIn;
   int i = 0;
   int j;

   while(i < Len) {
      for(j = 0; j < 16; j++) {
         if((i + j) == Len) {
            break;
         }
         LOG_RAW("%02x ",Adr[i+j]);
      }

      LOG_RAW(" ");
      for(j = 0; j < 16; j++) {
         if((i + j) == Len) {
            break;
         }
         if(isprint(Adr[i+j])) {
            LOG_RAW("%c",Adr[i+j]);
         }
         else {
            LOG_RAW(".");
         }
      }
      i += 16;
      LOG_RAW("\n");
   }
}


const struct {
   uint32 ID;
   const char *Desc;
} EventLookup[] = {
   {gecko_evt_system_boot_id,"system_boot"},
   {gecko_evt_system_external_signal_id,"system_external_signal"},
   {gecko_evt_system_awake_id,"system_awake"},
   {gecko_evt_system_hardware_error_id,"system_hardware_error"},
   {gecko_evt_le_gap_scan_response_id,"le_gap_scan_response"},
   {gecko_evt_le_gap_adv_timeout_id,"le_gap_adv_timeout"},
   {gecko_evt_le_gap_scan_request_id,"le_gap_scan_request"},
   {gecko_evt_le_connection_opened_id,"le_connection_opened"},
   {gecko_evt_le_connection_closed_id,"le_connection_closed"},
   {gecko_evt_le_connection_parameters_id,"le_connection_parameters"},
   {gecko_evt_le_connection_rssi_id,"le_connection_rssi"},
   {gecko_evt_le_connection_phy_status_id,"le_connection_phy_status"},
   {gecko_evt_gatt_mtu_exchanged_id,"gatt_mtu_exchanged"},
   {gecko_evt_gatt_service_id,"gatt_service"},
   {gecko_evt_gatt_characteristic_id,"gatt_characteristic"},
   {gecko_evt_gatt_descriptor_id,"gatt_descriptor"},
   {gecko_evt_gatt_characteristic_value_id,"gatt_characteristic_value"},
   {gecko_evt_gatt_descriptor_value_id,"gatt_descriptor_value"},
   {gecko_evt_gatt_procedure_completed_id,"gatt_procedure_completed"},
   {gecko_evt_gatt_server_attribute_value_id,"gatt_server_attribute_value"},
   {gecko_evt_gatt_server_user_read_request_id,"gatt_server_user_read_request"},
   {gecko_evt_gatt_server_user_write_request_id,"gatt_server_user_write_request"},
   {gecko_evt_gatt_server_characteristic_status_id,"gatt_server_characteristic_status"},
   {gecko_evt_gatt_server_execute_write_completed_id,"gatt_server_execute_write_completed"},
   {gecko_evt_hardware_soft_timer_id,"hardware_soft_timer"},
   {gecko_evt_test_dtm_completed_id,"test_dtm_completed"},
   {gecko_evt_sm_passkey_display_id,"sm_passkey_display"},
   {gecko_evt_sm_passkey_request_id,"sm_passkey_request"},
   {gecko_evt_sm_confirm_passkey_id,"sm_confirm_passkey"},
   {gecko_evt_sm_bonded_id,"sm_bonded"},
   {gecko_evt_sm_bonding_failed_id,"sm_bonding_failed"},
   {gecko_evt_sm_list_bonding_entry_id,"sm_list_bonding_entry"},
   {gecko_evt_sm_list_all_bondings_complete_id,"sm_list_all_bondings_complete"},
   {gecko_evt_sm_confirm_bonding_id,"sm_confirm_bonding"},
   {gecko_evt_user_message_to_host_id,"user_message_to_host"},
   {0}
};


void LogGeckoEvent(void *Pkt,const char *Function)
{
   int i;
   struct gecko_cmd_packet *p = (struct gecko_cmd_packet *) Pkt;

   uint32 ID = BGLIB_MSG_ID(p->header);

   for(i = 0; EventLookup[i].ID != 0; i++) {
      if(ID == EventLookup[i].ID) {
         LOG_RAW("%s: %s\n",Function,EventLookup[i].Desc);
         break;
      }
   }

   if(EventLookup[i].ID == 0) {
      LOG_RAW("%s: unknown event ID 0x%x\n",Function,ID);
   }
}

#ifdef sli_bt_cmd_handler_delegate
#undef sli_bt_cmd_handler_delegate
void sli_bt_cmd_handler_delegate(uint32_t header, gecko_cmd_handler, const void*);

void SLI_BT_CMD_HANDLER_DELEGATE(uint32_t ID, gecko_cmd_handler handler, const void* arg,const char *Function)
{
   int Len = strlen(Function);
   if(Len > 10) {
      LOG_RAW("cmd: %s\n",&Function[10]);
   }
   else {
      LOG_RAW("cmd: %s\n",Function);
   }
   sli_bt_cmd_handler_delegate(ID,handler,arg);
}
#endif
#endif

void ErrorBreakPoint(const char *Funct,int Line)
{
   if(Funct != NULL) {
      LOG_RAW("%s: Error on line %d\n",Funct,Line);
   }
}



