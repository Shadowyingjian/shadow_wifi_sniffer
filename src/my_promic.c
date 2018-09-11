#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "tcpip.h"
#include "wifi/wifi_conf.h"

#ifndef CONFIG_WLAN
#define CONFIG_WLAN 1
#endif

#if CONFIG_WLAN
#include <platform/platform_stdlib.h>

// Add extra interfaces to make release sdk able to determine promisc API linking

void my_promisc_deinit(void *padapter)
{
#ifdef CONFIG_PROMISC
	_my_promisc_deinit(padapter);
#endif
}

int my_promisc_recv_func(void *padapter, void *rframe)
{
  //Never reach here if not define CONFIG_PROMISC
#ifdef CONFIG_PROMISC
	return _my_promisc_recv_func(padapter, rframe);
#else
	return 0;
#endif  
 
}

int my_promisc_set(rtw_rcr_level_t enabled, void (*callback)(unsigned char*, unsigned int, void*), unsigned char len_used)
{
#ifdef CONFIG_PROMISC
	return _my_promisc_set(enabled, callback, len_used);
#else
	return -1;
#endif
}


unsigned char my_is_promisc_enabled(void)
{
#ifdef CONFIG_PROMISC
	return _my_is_promisc_enabled();
#else
	return 0;
#endif

}

int my_promisc_get_fixed_channel(void *fixed_bssid, u8 *ssid, int *ssid_length)
{
 #ifdef CONFIG_PROMISC
	return _my_promisc_get_fixed_channel(fixed_bssid, ssid, ssid_length);
#else
	return 0;
#endif
}

// End of Add extra interfaces

/*
company_printf(const char* fmt,...);
自定义printf函数
example: 
company_printf("hello-world!");
output:         Inthings:       hello-world!
*
*
*/

int company_printf(const char* fmt, ...)
{
 char printf_buf[1024];
 char * company_logo_buf2 = "Inthings:";
 printf("\t %s \t",company_logo_buf2);
 //printf("%s %s ",__DATE__,__TIME__);
 va_list args;                                        /* args为定义的一个指向可变参数的变量，va_list以及下边要用到的va_start,va_end都是是在定义
                                                                  可变参数函数中必须要用到宏， 在stdarg.h头文件中定义 */
 int printed;
 va_start(args, fmt);                               //初始化args的函数，使其指向可变参数的第一个参数，fmt是可变参数的前一个参数

 printed = vsprintf(printf_buf, fmt, args);
 va_end(args);                                     //结束可变参数的获取
 puts(printf_buf);
  return printed;
}
struct my_eth_frame{
      struct my_eth_frame *prev;
      struct my_eth_frame *next;
      unsigned char da[6];
      unsigned char sa[6];
      unsigned int len;
      unsigned char type;
      signed char rssi;
      unsigned char ssid[32];
      unsigned int ssid_len;
      unsigned char data[500];
      unsigned char f_type;
};

#if CONFIG_INIC_CMD_RSP
#if defined(__IAR_SYSTEMS_ICC__)||defined (__GNUC__)
#pragma pack(1)
#endif

struct my_inic_eth_frame{
        unsigned char da[6];
        unsigned char sa[6];
        unsigned int len;
        unsigned char type;
        unsigned char ssid[32];
        unsigned int ssid_len;
};

#if defined(__IAR_SYSTEMS_ICC__)||defined (__GNUC__)
#pragma pack()
#endif

static struct my_inic_eth_frame *my_inic_frame,*my_inic_frame_tail = NULL;

static int my_inic_frame_cnt = 0;
#define MY_MAX_INIC_FRAME_NUM 50  //maximum packets for each channel

extern void inic_c2h_msg(const char *atcmd, char status, char *msg, u16 msg_len);
#endif

struct my_eth_buffer{
      struct my_eth_frame *head;
      struct my_eth_frame *tail;
};


static struct my_eth_buffer my_eth_buffer;

#ifdef CONFIG_PROMISC
#define MY_MAX_PACKET_FILTER_INFO 5
#define MY_FILTER_ID_INIT_VALUE 10
rtw_packet_filter_info_t my_paff_array[MY_MAX_PACKET_FILTER_INFO]={0, 0, 0, 0, 0};
static u8 my_packet_filter_enable_num = 0;


void my_promisc_init_packet_filter()
{
	int i = 0;
	for(i=0; i<MY_MAX_PACKET_FILTER_INFO; i++){
		my_paff_array[i].filter_id = MY_FILTER_ID_INIT_VALUE;
		my_paff_array[i].enable = 0;
		my_paff_array[i].patt.mask_size = 0;
		my_paff_array[i].rule = RTW_POSITIVE_MATCHING;
		my_paff_array[i].patt.mask = NULL;
		my_paff_array[i].patt.pattern = NULL;
	}
	my_packet_filter_enable_num = 0;
}

int my_promisc_add_packet_filter(u8 filter_id, rtw_packet_filter_pattern_t *patt, rtw_packet_filter_rule_t rule)
{
	int i = 0;
	while(i < MY_MAX_PACKET_FILTER_INFO){
		if(my_paff_array[i].filter_id == MY_FILTER_ID_INIT_VALUE){
			break;
		}
		i++;	
	}

	if(i == MY_MAX_PACKET_FILTER_INFO)
		return -1;

	my_paff_array[i].filter_id = filter_id;

	my_paff_array[i].patt.offset= patt->offset;
	my_paff_array[i].patt.mask_size = patt->mask_size;
	my_paff_array[i].patt.mask = pvPortMalloc(patt->mask_size);
	memcpy(my_paff_array[i].patt.mask, patt->mask, patt->mask_size);
	my_paff_array[i].patt.pattern= pvPortMalloc(patt->mask_size);
	memcpy(my_paff_array[i].patt.pattern, patt->pattern, patt->mask_size);

	my_paff_array[i].rule = rule;
	my_paff_array[i].enable = 0;

	return 0;
}


int my_promisc_enable_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MY_MAX_PACKET_FILTER_INFO){
		if(my_paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MY_MAX_PACKET_FILTER_INFO)
		return -1;

	my_paff_array[i].enable = 1;
	my_packet_filter_enable_num++;
	return 0;
}

int my_promisc_disable_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MY_MAX_PACKET_FILTER_INFO){
		if(my_paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MY_MAX_PACKET_FILTER_INFO)
		return -1;

	my_paff_array[i].enable = 0;
	my_packet_filter_enable_num--;
	return 0;
}


int my_promisc_remove_packet_filter(u8 filter_id)
{
	int i = 0;
	while(i < MY_MAX_PACKET_FILTER_INFO){
		if(my_paff_array[i].filter_id == filter_id)
			break;
		i++;
	}

	if(i == MY_MAX_PACKET_FILTER_INFO)
		return -1;

	my_paff_array[i].filter_id = MY_FILTER_ID_INIT_VALUE;
	my_paff_array[i].enable = 0;
	my_paff_array[i].patt.mask_size = 0;
	my_paff_array[i].rule = 0;
	if(my_paff_array[i].patt.mask){
		vPortFree(my_paff_array[i].patt.mask);
		my_paff_array[i].patt.mask = NULL;
	}
	
	if(my_paff_array[i].patt.pattern){
		vPortFree(my_paff_array[i].patt.pattern);
		my_paff_array[i].patt.pattern = NULL;
	}
	return 0;
}
#endif

/*	Make callback simple to prevent latency to wlan rx when promiscuous mode */
static void my_promisc_callback(unsigned char *buf, unsigned int len, void* userdata)
{
	struct my_eth_frame *frame = (struct my_eth_frame *) pvPortMalloc(sizeof(struct my_eth_frame));
	
	if(frame) {
		frame->prev = NULL;
		frame->next = NULL;
		memcpy(frame->da, buf, 6);
		memcpy(frame->sa, buf+6, 6);
		frame->len = len;
		frame->rssi = ((ieee80211_frame_info_t *)userdata)->rssi;
		taskENTER_CRITICAL();

		if(my_eth_buffer.tail) {
			my_eth_buffer.tail->next = frame;
			frame->prev = my_eth_buffer.tail;
			my_eth_buffer.tail = frame;
		}
		else {
			my_eth_buffer.head = frame;
			my_eth_buffer.tail = frame;
		}

		taskEXIT_CRITICAL();
	}
}


struct my_eth_frame* my_retrieve_frame(void)
{
	struct my_eth_frame *frame = NULL;

	taskENTER_CRITICAL();

	if(my_eth_buffer.head) {
		frame = my_eth_buffer.head;

		if(my_eth_buffer.head->next) {
			my_eth_buffer.head = my_eth_buffer.head->next;
			my_eth_buffer.head->prev = NULL;
		}
		else {
			my_eth_buffer.head = NULL;
			my_eth_buffer.tail = NULL;
		}
	}

	taskEXIT_CRITICAL();

	return frame;
}



static void my_promisc_test(int duration, unsigned char len_used)
{
	int ch;
	unsigned int start_time;
	struct my_eth_frame *frame;
	my_eth_buffer.head = NULL;
	my_eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE, my_promisc_callback, len_used);

	for(ch = 1; ch <= 13; ch ++) {
		if(wifi_set_channel(ch) == 0)
			printf("\n\n\rSwitch to channel(%d)", ch);

		start_time = xTaskGetTickCount();

		while(1) {
			unsigned int current_time = xTaskGetTickCount();

			if((current_time - start_time) < (duration * configTICK_RATE_HZ)) {
				frame = my_retrieve_frame();

				if(frame) {
					int i;
					printf("\n\rDA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->da[i]);
					printf(", SA:");
					for(i = 0; i < 6; i ++)
						printf(" %02x", frame->sa[i]);
					printf(", len=%d", frame->len);
					printf(", RSSI=%d", frame->rssi);
#if CONFIG_INIC_CMD_RSP
					if(my_inic_frame_tail){
						if(my_inic_frame_cnt < MY_MAX_INIC_FRAME_NUM){
							memcpy(my_inic_frame_tail->da, frame->da, 6);
							memcpy(my_inic_frame_tail->sa, frame->sa, 6);
							my_inic_frame_tail->len = frame->len;
							my_inic_frame_tail++;
							my_inic_frame_cnt++;
						}
					}
#endif	
					vPortFree((void *) frame);
				}
				else
					vTaskDelay(1);	//delay 1 tick
			}
			else
				break;	
		}
#if CONFIG_INIC_CMD_RSP
		if(inic_frame){
			inic_c2h_msg("ATWM", RTW_SUCCESS, (char *)my_inic_frame, sizeof(struct my_inic_eth_frame)*my_inic_frame_cnt);
			memset(my_inic_frame, '\0', sizeof(struct my_inic_eth_frame)*MY_MAX_INIC_FRAME_NUM);
				my_inic_frame_tail = my_inic_frame;
				my_inic_frame_cnt = 0;
			rtw_msleep_os(10);
		}
#endif
	}

	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);

	while((frame = my_retrieve_frame()) != NULL)
		vPortFree((void *) frame);
}

static void my_promisc_callback_all(unsigned char *buf, unsigned int len, void* userdata)
{
	struct my_eth_frame *frame = (struct my_eth_frame *) pvPortMalloc(sizeof(struct my_eth_frame));
	
	if(frame) {
		frame->prev = NULL;
		frame->next = NULL;
       
		memcpy(frame->da, buf+4, 6);
		memcpy(frame->sa, buf+10, 6);
                //add by shadow
                memcpy(frame->data,buf,sizeof(frame->data));
                
                
                
                
               // frame->ssid_len = buf[49];
               // memcpy(frame->ssid,buf+50,buf[49]);
		frame->len = len;
		/*  
		* type is the first byte of Frame Control Field of 802.11 frame
		* If the from/to ds information is needed, type could be reused as follows:
		* frame->type = ((((ieee80211_frame_info_t *)userdata)->i_fc & 0x0100) == 0x0100) ? 2 : 1;
		* 1: from ds; 2: to ds
		*/		
		frame->type = *buf;
		frame->rssi = ((ieee80211_frame_info_t *)userdata)->rssi;
                
                /*
                add by shadow for know wifi ssid
                */
               
                if(frame->type == 0x40)
                {
                  frame->ssid_len =frame->data[25];
                  memcpy(frame->ssid,buf+26,frame->ssid_len);
                //  printf("\r\n ssid_len = %d ,ssid = %s \r\n",frame->ssid_len,frame->ssid);
                }
                else{                
                   frame->ssid_len = frame->data[37];
                   memcpy(frame->ssid,buf+38,frame->ssid_len);
                 //  printf("\r\n ssid_len = %d ,ssid = %s \r\n",frame->ssid_len,frame->ssid);
                }
               

		taskENTER_CRITICAL();

		if(my_eth_buffer.tail) {
			my_eth_buffer.tail->next = frame;
			frame->prev = my_eth_buffer.tail;
			my_eth_buffer.tail = frame;
		}
		else {
			my_eth_buffer.head = frame;
			my_eth_buffer.tail = frame;
		}

		taskEXIT_CRITICAL();
	}
}

static void my_promisc_test_all(int duration, unsigned char len_used)
{
	int ch;
	unsigned int start_time;
	struct my_eth_frame *frame;
	my_eth_buffer.head = NULL;
	my_eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE_2, my_promisc_callback_all, len_used);

	for(ch = 1; ch <= 13; ch ++) {
		if(wifi_set_channel(ch) == 0)
			printf("\n\n\rSwitch to channel(%d)", ch);

		start_time = xTaskGetTickCount();

		while(1) {
			unsigned int current_time = xTaskGetTickCount();

			if((current_time - start_time) < (duration * configTICK_RATE_HZ)) {
				frame = my_retrieve_frame();
                                if(frame){
                                    int i = 0;
                                    printf("\n\r |%02X|DA:", frame->type); 
                                    for(i=0;i<6;i++)
                                      printf("%02X",frame->da[i]);
                                    printf("|SA:");
                                    for(i=0;i<6;i++)
                                      printf("%02X",frame->sa[i]);
                                    printf("|%d|%d|%s\r\n",frame->rssi,ch,frame->ssid);
                                    memset(frame->ssid,0,sizeof(frame->ssid));
                                
                               
/*
				if(frame) {
					int i;
					printf("\n\rTYPE: 0x%x, ", frame->type);
					printf("DA:");
					for(i = 0; i < 6; i ++)
						printf(" %02X", frame->da[i]);
					printf(", SA:");
					for(i = 0; i < 6; i ++)
						printf(" %02X", frame->sa[i]);
					printf(", len=%d", frame->len);
					printf(", RSSI=%d", frame->rssi); 
                                       printf(", SSID_LEN=%d",frame->ssid_len);
                                       printf(" ,SSID=%s",frame->ssid);
                                       memset(frame->ssid,0,sizeof(frame->ssid));
                                       printf("\r\n");
*/                                  
#if CONFIG_INIC_CMD_RSP
					if(my_inic_frame_tail){
						if(my_inic_fr*ame_cnt < MY_MAX_INIC_FRAME_NUM){
							memcpy(my_inic_frame_tail->da, frame->da, 6);
							memcpy(my_inic_frame_tail->sa, frame->sa, 6);
							my_inic_frame_tail->len = frame->len;
							my_inic_frame_tail->type = frame->type;
							my_inic_frame_tail++;
							my_inic_frame_cnt++;
						}
					}
#endif	
					vPortFree((void *) frame);
				}
				else
					vTaskDelay(1);	//delay 1 tick
			}
			else
				break;	
		}
#if CONFIG_INIC_CMD_RSP
		if(my_inic_frame){
			inic_c2h_msg("ATWM", RTW_SUCCESS, (char *)my_inic_frame, sizeof(struct my_inic_eth_frame)*my_inic_frame_cnt);
			memset(my_inic_frame, '\0', sizeof(struct my_inic_eth_frame)*MY_MAX_INIC_FRAME_NUM);
				my_inic_frame_tail = my_inic_frame;
				my_inic_frame_cnt = 0;
			rtw_msleep_os(10);
		}
#endif
	}

	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);

	while((frame = my_retrieve_frame()) != NULL)
		vPortFree((void *) frame);
}

void my_promsic_demo(int duration, unsigned char len_used)
{
  #if CONFIG_INIC_CMD_RSP
	my_inic_frame_tail = my_inic_frame = pvPortMalloc(sizeof(struct my_inic_eth_frame)*MY_MAX_INIC_FRAME_NUM);
	if(my_inic_frame == NULL){
		inic_c2h_msg("ATWM", RTW_BUFFER_UNAVAILABLE_TEMPORARY, NULL, 0);
		return;
	}
#endif
        #ifdef CONFIG_PROMISC
	wifi_init_packet_filter();
	#endif
        
        my_promisc_test_all(duration, 0);
        
}
void my_cmd_promisc(int argc, char **argv)
{
	int duration;
#if CONFIG_INIC_CMD_RSP
	my_inic_frame_tail = my_inic_frame = pvPortMalloc(sizeof(struct my_inic_eth_frame)*MY_MAX_INIC_FRAME_NUM);
	if(my_inic_frame == NULL){
		inic_c2h_msg("ATWM", RTW_BUFFER_UNAVAILABLE_TEMPORARY, NULL, 0);
		return;
	}
#endif
	#ifdef CONFIG_PROMISC
	wifi_init_packet_filter();
	#endif
	if((argc == 2) && ((duration = atoi(argv[1])) > 0))
		//promisc_test(duration, 0);
		my_promisc_test_all(duration, 0);
	else if((argc == 3) && ((duration = atoi(argv[1])) > 0) && (strcmp(argv[2], "with_len") == 0))
		my_promisc_test(duration, 1);
	else
		printf("\n\rUsage: %s DURATION_SECONDS [with_len]", argv[0]);
#if CONFIG_INIC_CMD_RSP
	if(my_inic_frame)
		vPortFree(inic_frame);
	my_inic_frame_tail = NULL;
	my_inic_frame_cnt = 0;
#endif
}
#endif	//#if CONFIG_WLAN
