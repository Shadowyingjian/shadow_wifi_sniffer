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
#include "serial_api.h" //uart 
#include "serial_ex_api.h"  //uart
   
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
/*UART pin location:     
   UART0: 
   PA_23  (TX)
   PA_18  (RX)
   */
#define UART_TX PA_23
#define UART_RX PA_18

  
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
    if( g_times % 10  ){
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


//测试串口输出
volatile char rc = 0;
char data[100] = {0};
char *rxdata = NULL;
uint8_t rxdata2[1024] = {0};
uint8_t rxdata3[1024] = {0};


int rxdatacount = 0;
int rx_flag = -1;
int rx_count = -1;
serial_t sobj;
void uart_send_string2(serial_t *sobj,char *pstr)
{  
    unsigned int i = 0;
    while( *(pstr + i )!=0){
            serial_putc(sobj,*(pstr + i));
            i++;
    }
 
}

void uart_irq( uint32_t id, SerialIrq event)
{
      serial_t *sobj = (void *)id;
      if( event == RxIrq)
      {
           rc = serial_getc(sobj);
           serial_putc(sobj,rc);
           rxdatacount++;
          data[rxdatacount] = rc;
          if( rc == '\n')
          {
              memcpy( rxdata,data,rxdatacount);
              memcpy ( rxdata2,data,rxdatacount);
               printf("rxdata = %s \r\n", rxdata);
            printf("rxdata2 = \r\n");
            for (int i = 1; i < rxdatacount; i++)
            {
                printf("%c", rxdata2[i]);
                rxdata3[i - 1] = rxdata2[i];
            }
            memcpy(rxdata, rxdata3, sizeof(rxdata3));
            printf("\r\n");
            printf("rxdata3 = %s \r\n", rxdata3);
            rxdatacount = 0;
            uart_send_string2(sobj, rxdata);
            printf("\r\n receive newline \r\n");
            memset(data, 0, sizeof(data));
            memset(rxdata2, 0, sizeof(rxdata2));
            memset(rxdata3, 0, sizeof(rxdata3));
              
          }
           
          
          
      }
       if (event == TxIrq && rc != 0)
    {
        uart_send_string2(sobj, "\r\n8195a$");
        rc = 0;
    }


}

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
        
        
       
        //uart test
         serial_init(&sobj, UART_TX, UART_RX);
    serial_baud(&sobj, 115200);
    serial_format(&sobj, 8, ParityNone, 1);

    uart_send_string2(&sobj, "UART IRQ API Demo...\r\n");
    uart_send_string2(&sobj, "Hello World!!\n");
    uart_send_string2(&sobj, "\r\n8195a$");
    serial_irq_handler(&sobj, uart_irq, (uint32_t)&sobj);
    serial_irq_set(&sobj, RxIrq, 1);
    serial_irq_set(&sobj, TxIrq, 1);
        while(1){
        // MY_ATWM(param);
          my_promsic_demo(3,0);
        }
      
          
       
         
       
	/* Kill init thread after all init tasks done */
	vTaskDelete(NULL);
}

void wifi_sniffer()
{
	if(xTaskCreate(wifi_sniffer_init_thread, ((const char*)"wifi_sniffer_init"), STACKSIZE, NULL, tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
}
