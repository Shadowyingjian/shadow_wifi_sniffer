/*
 *  Hello World
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" 

#include "main.h"
#include "main_test.h"
   
#include "atcmd_wifi.h"
#include <lwip_netconf.h>
#include "tcpip.h"
#include <dhcp/dhcps.h>
#include "my_promic.h"   
   
#if CONFIG_WLAN
#include "wifi_conf.h"
#include "wlan_intf.h"
#include "wifi_constants.h"
#include <wlan/wlan_test_inc.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>   
#endif
#include "lwip_netconf.h"
#include <platform/platform_stdlib.h>

#ifndef CONFIG_INIT_NET
#define CONFIG_INIT_NET             1
#endif
#ifndef CONFIG_INTERACTIVE_MODE
#define CONFIG_INTERACTIVE_MODE     1
#endif

  

extern void promisc_test_all(int duration, unsigned char len_used);
extern void vdata_init_device(void);
extern void vdata_init_router(void);
extern void my_promisc_callback_all(unsigned char *buf, unsigned int len, void* userdata);
extern void my_promisc_test_all_v2();


#define STACKSIZE                   (512 + 768)

xSemaphoreHandle uart_rx_interrupt_sema_wifisniffer = NULL;

//添加这部分是测试定时器
#include "timer_api.h"
gtimer_t my_timer1;
extern uint64_t g_times;
extern uint64_t timestamp;
extern uint32_t g_second;
extern uint16_t seq_channel;
int set_channel_flag = -1;
void timer1_timeout_handler( uint32_t id)
{
    g_times++;
    timestamp++;
    if( g_times % 10 == 0 ){
         g_second++;
      
    }
    
    
    
    if( g_times % 20 ==0){
        seq_channel += 3;
        if( seq_channel >13 ){
             seq_channel = 1;
        }
    }
    
    if(g_second %10 == 0)
    {
     wifi_init_packet_filter();
         vdata_init_device();
         vdata_init_router();
    }
    
   // set_channel_flag = wifi_set_channel(seq_channel);
  //if( set_channel_flag != 0 ){
   //  printf("\r\n set channel faild \r\n");
  // }
    
    

}

#ifndef MAX_ARGC
#define MAX_ARGC 12
#endif
#ifdef CONFIG_PROMISC
void MY_ATWM(void *arg){ 
	
        my_cmd_pr();
}
#endif


void wifi_sniffer_init_thread(void *param)
{

#if CONFIG_INIT_NET
#if CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	LwIP_Init();
#endif
#endif
#if CONFIG_WIFI_IND_USE_THREAD
	wifi_manager_init();
#endif
#if CONFIG_WLAN
	wifi_on(RTW_MODE_STA);
#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_set_autoreconnect(1);
#endif
	printf("\n\r%s(%d), Available heap 0x%x", __FUNCTION__, __LINE__, xPortGetFreeHeapSize());	
#endif

#if CONFIG_INTERACTIVE_MODE
 	/* Initial uart rx swmaphore*/
	vSemaphoreCreateBinary(uart_rx_interrupt_sema_wifisniffer);
	xSemaphoreTake(uart_rx_interrupt_sema_wifisniffer, 1/portTICK_RATE_MS);
	start_interactive_mode();
#endif	
        wifi_init_packet_filter();
        
         vdata_init_device();
         vdata_init_router();
        //Initial a periodical timer
        gtimer_init(&my_timer1,TIMER2);
        
        gtimer_start_periodical( &my_timer1, 100000, (void*)timer1_timeout_handler,NULL);
        
        while(1){
         MY_ATWM(param);
        }
      
          
      
         // my_promisc_test_all_v2();
       
         
       
	/* Kill init thread after all init tasks done */
	vTaskDelete(NULL);
}

void wifi_sniffer()
{
	if(xTaskCreate(wifi_sniffer_init_thread, ((const char*)"wifi_sniffer_init"), STACKSIZE, NULL, tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
}
