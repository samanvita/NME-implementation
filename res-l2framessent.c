/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
#include "csma.h"


static char content[REST_MAX_CHUNK_SIZE];
static int content_len = 0;
#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }
#define PRINT6ADDR(addr) printf(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])

//int frames[10]={250,270,310,275,450,175,200,390,560,225};

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_l2framessent,
                  "title=\"Periodic demo\";obs",
                  NULL,
                  res_get_handler,
                  NULL,
                  NULL,
                  5 * CLOCK_SECOND,
                  res_periodic_handler);

/*
 * Use local resource state that is accessed by res_get_handler() and altered by res_periodic_handler() or PUT or POST.
 */
static int32_t event_counter = 0;

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */

   const uint8_t  *payload = NULL;
          char parent[100] ;
          int pay_len;
  int i=0;
  unsigned int accept = -1;
  uip_ipaddr_t ip,*stat_ip;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    content_len=0;
    pay_len = REST.get_request_payload(request, &payload);
    strcpy( parent,payload); 
    uiplib_ip6addrconv((const char*)parent , &ip);    
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    //snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "FRAMES SENT %lu", framessent());
   for(i=0;i<parent_count();i++)
   {
    stat_ip=(StatIp(i));
    if((((uint8_t *)stat_ip)[0]==((uint8_t *)&ip)[0])  &&
         (((uint8_t *)stat_ip)[1]==((uint8_t *)&ip)[1])  &&
         (((uint8_t *)stat_ip)[2]==((uint8_t *)&ip)[2])  &&
         (((uint8_t *)stat_ip)[3]==((uint8_t *)&ip)[3])  &&
         (((uint8_t *)stat_ip)[4]==((uint8_t *)&ip)[4])  &&
         (((uint8_t *)stat_ip)[5]==((uint8_t *)&ip)[5])  &&
         (((uint8_t *)stat_ip)[6]==((uint8_t *)&ip)[6])  &&
         (((uint8_t *)stat_ip)[7]==((uint8_t *)&ip)[7])  &&
         (((uint8_t *)stat_ip)[8]==((uint8_t *)&ip)[8])  &&
         (((uint8_t *)stat_ip)[9]==((uint8_t *)&ip)[9])  &&
         (((uint8_t *)stat_ip)[10]==((uint8_t *)&ip)[10])  &&
         (((uint8_t *)stat_ip)[11]==((uint8_t *)&ip)[11])  &&
         (((uint8_t *)stat_ip)[12]==((uint8_t *)&ip)[12])  &&
         (((uint8_t *)stat_ip)[13]==((uint8_t *)&ip)[13])  &&
         (((uint8_t *)stat_ip)[14]==((uint8_t *)&ip)[14])  &&
         (((uint8_t *)stat_ip)[15]==((uint8_t *)&ip)[15]) )
    {
      CONTENT_PRINTF( "%d",StatFrameSent(i));
      break;
    }
   }
   if(i==parent_count())
   {
    content_len=1;
    content[0]='0';
    content[1]='\0';
   }
    REST.set_response_payload(response, (uint8_t *)content, content_len);
  
  }

//  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
// REST.set_header_max_age(response, res_l2framessent.periodic->period / CLOCK_SECOND);
// REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "FRAMES SENT %lu", framessent()));

  /* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}
/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler()
{
  /* Do a periodic task here, e.g., sampling a sensor. */
  ++event_counter;

  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
  if(1) {
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_l2framessent);
  }
}


