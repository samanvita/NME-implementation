/*
* Copyright (c) 2015 NXP B.V.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of NXP B.V. nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* This file is part of the Contiki operating system.
*
* Author: Theo van Daele <theo.van.daele@nxp.com>
*
*/
#include "contiki.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "rich.h"
#include "rest-engine.h"
#include <stdio.h> 
#include "sys/clock.h"
#include "sys/rtimer.h"

//Rpl related 
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl-private.h"


/* #include <AppHardwareApi.h> */
static void set_tx_power_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_tx_power_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

//Lavan
#include "net/ip/uip-debug.h"


static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;

static struct etimer et;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

extern resource_t															 
res_get_pktsent,
res_set_parent,													      
res_get_parent,														   
res_b1_sep_b2,													        
res_l2framessent,
res_all_frames_sent,
res_all_frames_dropped,
res_l2framesdropped,
res_battery,
res_myparent,
res_lcwp_etx,
res_chunks ;


/*---------------------------------------------------------------------------*/
PROCESS(start_app, "START_APP");
AUTOSTART_PROCESSES(&start_app);
/*---------------------------------------------------------------------------*/

/*********** RICH sensor/ resource ************************************************/
RESOURCE(resource_set_tx_power, 
         "title=\"Set TX Power\"",
         NULL,
         set_tx_power_handler,
         set_tx_power_handler,
         NULL);
static void
set_tx_power_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  const uint8_t *request_content = NULL;
  int request_content_len;
  int tx_level;

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    request_content_len = REST.get_request_payload(request, &request_content);
    tx_level = atoi((const char *)request_content);
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tx_level);
  }

  
}

RESOURCE(resource_get_tx_power, 
         "title=\"Get TX Power\"",
         get_tx_power_handler,
         NULL,
         NULL,
         NULL);
static void
get_tx_power_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  printf("COAP:: get_tx_power_handler\n ");
  int tx_level;
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len = 0;
    
    NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tx_level);
    printf("CoAP tx level : %d", tx_level);
    CONTENT_PRINTF("%d", tx_level);
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  }

   clock_delay(1000); 
  // Added for testing purpose //13Jan2016
  printf ("sixtop : sixtop_addCells called\n");
  sixtop_addCells(0xffff, 1);// sixtop_addCells(linkaddr_t* neighbor, uint16_t numCells);

}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*---------------------------------------------------------------------------*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_app, ev, data)
{
  PROCESS_BEGIN();
  static int is_coordinator = 0;
 
  /* Start RICH stack */
  if(is_coordinator) {
    uip_ipaddr_t prefix;
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rich_init(&prefix);
  } else {
    rich_init(NULL);
  }
  printf("Starting RPL node\n");

  
  rest_init_engine(); 

  rest_activate_resource(&resource_set_tx_power, "Set-TX-Power");
  rest_activate_resource(&resource_get_tx_power, "Get-TX-Power");
  rest_activate_resource(&res_get_pktsent, "Get-Packet-Rate");
  rest_activate_resource(&res_get_parent, "Get-Parent");
  rest_activate_resource(&res_set_parent, "Set-Parent");
  rest_activate_resource(&res_b1_sep_b2, "test/b1sepb2");
  rest_activate_resource(&res_battery,"node/battery");
  rest_activate_resource(&res_l2framessent,"node/frames_sent_per_parent");
  rest_activate_resource(&res_l2framesdropped,"node/frames_dropped_per_parent");
  rest_activate_resource(&res_all_frames_sent,"node/all_frames_sent");
  rest_activate_resource(&res_all_frames_dropped,"node/all_frames_dropped");
  rest_activate_resource(&res_lcwp_etx,"node/link_cost_with_parent");
  rest_activate_resource(&res_myparent,"node/my_parent");
  /*+++++++++++++++++++++++++*/
  PROCESS_END();
}
