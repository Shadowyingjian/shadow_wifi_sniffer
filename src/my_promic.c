
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "tcpip.h"
#include "wifi/wifi_conf.h"
#include "my_promic.h"
#ifndef CONFIG_WLAN
#define CONFIG_WLAN 1
#endif

/*
//添加这部分是为了实现去重输出
uint64_t timestamp = 121;
uint64_t lasttime = 0;
uint32_t g_second = 0;

uint64_t g_times = 0;
uint8_t mac_bssid_old[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t mac_mobile_old[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t g_3macs[18] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t last_ssid[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint32_t last_time = 0;

uint8_t mac1[6];
uint8_t mac2[6];
uint8_t mac3[6];

uint16_t seq_channel = 1;
#define VDATA_MAX_MAC_NUM 128
#define VDATA_MAX_MAC_ROUTER 128
#define VDATA_MAX_MAC_SSID 128


vdata_device_t vdata_device[VDATA_MAX_MAC_NUM];
vdata_router_t vdata_router[VDATA_MAX_MAC_ROUTER];
vdata_ssid_t vdata_ssid[VDATA_MAX_MAC_SSID];

void vdata_init_device(void);
void vdata_init_router(void);
void vdata_init_ssid(void);

int64_t vdata_insert_device(uint8_t *mac, uint64_t time);
int64_t vdata_insert_router(uint8_t *mac, uint64_t time);
int8_t vdata_insert_ssid(uint8_t *new_bssid, uint8_t bbssidlen, uint64_t time);

void trans_int_to_char(uint8_t *s, uint8_t *array, uint16_t len);
void trans_int_to_string(uint8_t *s, uint8_t data);
void str_encrypt(uint8_t *s, uint8_t *array, uint16_t len);
//volatile int *rand;
uint8_t G_KEY;
*/


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
//添加去重函数
void vdata_init_device(void)
{
    uint16_t i = 0;
    for(i = 0; i < VDATA_MAX_MAC_NUM; i++){
        vdata_device[i].mac = -1;
        vdata_device[i].rssi = 0;
        vdata_device[i].time = 0;
    }
}

void vdata_init_router(void)
{
    uint16_t i = 0;
    for(i = 0; i < VDATA_MAX_MAC_ROUTER; i++){
        vdata_router[i].mac = -1;
        vdata_router[i].rssi = 0;
        vdata_router[i].time = 0;
    }
}

void vdata_init_ssid(void)
{
    uint16_t i;
    for(i = 0; i < VDATA_MAX_MAC_SSID; i++){
        bzero(vdata_ssid[i].ssid,sizeof(vdata_ssid[i].ssid));
        vdata_ssid[i].len=0;
    }
}

int64_t mac_2_int64(uint8_t *str)
{
	int64_t res = -1;
	res = ((int64_t)str[0] << 40) | ((int64_t)str[1] << 32) | ((int64_t)str[2] << 24) | ((int64_t)str[3] << 16) | ((int64_t)str[4] << 8) | str[5];
	return res;
}

int64_t vdata_insert_device(uint8_t *mac, uint64_t time)
{
	int64_t  new_mac = -1;
    uint16_t index = 0 , i = 0;
    uint64_t cmp_time = 0xFFFFFFFF;
    //new_mac = mac_2_int64(mac) + ((int64_t)type << 48);
    new_mac = mac_2_int64(mac);
    for(i = 0; i < VDATA_MAX_MAC_NUM; i++){
        if(cmp_time > vdata_device[i].time){
            cmp_time = vdata_device[i].time;
            index = i;
        }
        if(vdata_device[i].mac == -1){
            vdata_device[i].mac = new_mac;
            vdata_device[i].time= time;
            return new_mac;
        }else if(vdata_device[i].mac == new_mac){
            if(time - vdata_device[i].time > 6){
                vdata_device[i].time = time;
                return new_mac;
            }else{
                return -1;
            }
        }
    }
    if(i >= VDATA_MAX_MAC_NUM){
        vdata_device[index].mac = new_mac;
        vdata_device[index].time = time;
        return new_mac;
    }
    return -1;
}

int64_t vdata_insert_router(uint8_t *mac, uint64_t time)
{
	int64_t  new_mac = -1;
    uint16_t index = 0 , i = 0;
    uint64_t cmp_time = 0xFFFFFFFFFFFFFFFF;
    //new_mac = mac_2_int64(mac) + ((int64_t)type << 48);
    new_mac = mac_2_int64(mac);
    for(i = 0; i < VDATA_MAX_MAC_NUM; i++){
        if(cmp_time > vdata_router[i].time){
            cmp_time = vdata_router[i].time;
            index = i;
        }
        if(vdata_router[i].mac == -1){
            vdata_router[i].mac = new_mac;
            vdata_router[i].time= time;
            return new_mac;
        }else if(vdata_router[i].mac == new_mac){
            if(time - vdata_router[i].time > 60){
                vdata_router[i].time = time;
                return new_mac;
            }else{
                return -1;
            }
        }
    }

    if(i >= VDATA_MAX_MAC_NUM){
        vdata_router[index].mac = new_mac;
        vdata_router[index].time = time;
        return new_mac;
    }
    return -1;
}

int8_t vdata_insert_ssid(uint8_t *new_bssid, uint8_t bssidlen, uint64_t time)
{
	if (bssidlen > 32){
		bssidlen = 32;
	}
	int64_t  new_mac = -1;
    uint16_t index = 0 , i = 0;
    uint64_t cmp_time = 0xFFFFFFFFFFFFFFFF;
    int j;
    for(i = 0; i < VDATA_MAX_MAC_NUM; i++){
        if(cmp_time > vdata_ssid[i].time){
            cmp_time = vdata_ssid[i].time;
            index = i;
        }
        if(vdata_ssid[i].len==0){
            memcpy(vdata_ssid[i].ssid,new_bssid,bssidlen);
            vdata_ssid[i].len = bssidlen;
            for(j= 0; j < 6; j++){
            	//vdata_ssid[i].mac[j] = *rand % 0xFF;
              vdata_ssid[i].mac[j] =   0xFF;
            }
            vdata_ssid[i].time = time;
            return 0;
        }else if(memcmp(vdata_ssid[i].ssid,new_bssid,bssidlen) == 0 && vdata_ssid[i].len == bssidlen){
            if(time - vdata_ssid[i].time > 0){
                vdata_ssid[i].time = time;
                return 1;
            }else{
                return -2;
            }
        }
    }
    if(i >= VDATA_MAX_MAC_NUM){
    	bzero(vdata_ssid[index].ssid,sizeof(vdata_ssid[index].ssid));
        memcpy(vdata_ssid[index].ssid,new_bssid,bssidlen);
        vdata_ssid[index].len = bssidlen;
        vdata_ssid[index].time = time;
        return 0;
    }
    return -1;
}

void trans_int_to_char(uint8_t *s, uint8_t *array, uint16_t len)
{
	int i;
	int temp1;
	int temp2;
	int count = 0;
	memset( s, 0, len*2+1 );
	if( array == NULL ){
		return;
	}
	for( i = 0; i < len; i++ )
	{
		temp1 = 0;
		temp2 = 0;
		temp1 = array[i] / 16; // 比如fe,那么表示取得f
		temp2 = array[i] % 16; // 比如fe,那么表示取得e
		if( temp1 > 9 ) // 转换成16进制的字符
			s[count] = 'A'+temp1 -10;
		else
			s[count] = '0' + temp1;
		count++;
		if( temp2 > 9 ) // 转换成16进制的字符
			s[count] = 'A' + temp2 - 10;
		else
			s[count] = '0' + temp2;
		count++;
	}
	s[count]='\0';
	return;
}


void trans_int_to_string(uint8_t *s, uint8_t data)
{
	uint8_t i = 0 , j = 0;
	for(i = 0;i < 2;i++){
		j = ((data & 0xF0) >> 4);
		if((j >= 0)&&(j <= 9)){
			*s++ = j + '0';
			data <<= 4;
			continue;
		}
		if((j >= 0x0A)&&(j <= 0x0F)){
			*s++ = j + 'A' - 0x0A;
			data <<= 4;
			continue;
		}
		data <<= 4;
	}
	*s = '\0';
	return;
}

void str_encrypt(uint8_t *s, uint8_t *array, uint16_t len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		s[i] = array[i] ^ G_KEY;
	}
	s[len] = '\n';
	s[len+1] = '\0';
}

uint32_t vdata_time_last_device = 0;
uint8_t vdata_macprint[32];
void print_mac_device(uint8_t *mac,uint8_t rssi,uint8_t *mac2, uint8_t ssid_len, uint8_t *ptr_ssid)
{
  if( g_second - vdata_time_last_device > 6){
       mac_mobile_old[0] = 0xff;
       mac_mobile_old[1] = 0xff;
       mac_mobile_old[2] = 0xff;
       mac_mobile_old[3] = 0xff;
       mac_mobile_old[4] = 0xff;
       mac_mobile_old[5] = 0xff;
       vdata_time_last_device = g_second;
  }
  if( memcmp( mac_mobile_old, mac,6 )!= 0 )
  {
    if( vdata_insert_device(mac, g_second) > 0 ){
      if( mac2 == NULL ){
          printf("\r\n 01|%02X%02X%02X%02X%02X%02X|%02X\n\0",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],rssi);
          
      }else{
          printf("\r\n 02|%02X%02X%02X%02X%02X%02X|%02X|%02X%02X%02X%02X%02X%02X|%02d|\0",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],rssi,mac2[0],mac2[1],mac2[2],mac2[3],mac2[4],mac2[5],ssid_len);
          
          if(ssid_len < 33){
             printf(" %s\r\n",ptr_ssid);
          }
          else{
             printf("\r\n");
          }
          
      }
      memcpy(mac_mobile_old ,mac, 6);
      vdata_time_last_device = g_second;
      
    }
  
  }
  
}


uint32_t vdata_time_last_router = 0;
uint8_t vdata_routerprint[32];
void print_mac_router(uint8_t *mac, uint8_t rssi, uint8_t ssid_len, uint8_t *ptr_ssid )
{
  if( g_second - vdata_time_last_router > 6){
      mac_bssid_old[0] = 0xff;
      mac_bssid_old[1] = 0xff;
      mac_bssid_old[2] = 0xff;
      mac_bssid_old[3] = 0xff;
      mac_bssid_old[4] = 0xff;
      mac_bssid_old[5] = 0xff;
      vdata_time_last_router = g_second;
  }
  
  if( memcmp(mac_bssid_old,mac,6) != 0 ){
    if( vdata_insert_router( mac,g_second) > 0 ){
        printf("\r\n 00|%02X%02X%02X%02X%02X%02X|%02X|%02d|",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],rssi,ssid_len);
        if(ssid_len < 33){
             printf(" %s\r\n",ptr_ssid);
          }
          else{
             printf("\r\n");
          }
       
        memcpy( mac_bssid_old,mac,6);
        vdata_time_last_router = g_second;
    } 
  }
  

}
*/
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


#if CONFIG_INIC_CMD_RSP
#if defined(__IAR_SYSTEMS_ICC__)||defined (__GNUC__)
#pragma pack(1)
#endif



#if defined(__IAR_SYSTEMS_ICC__)||defined (__GNUC__)
#pragma pack()
#endif

static struct my_inic_eth_frame *my_inic_frame,*my_inic_frame_tail = NULL;

static int my_inic_frame_cnt = 0;
#define MY_MAX_INIC_FRAME_NUM 50  //maximum packets for each channel

extern void inic_c2h_msg(const char *atcmd, char status, char *msg, u16 msg_len);
#endif





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

void my_promisc_callback_all(unsigned char *buf, unsigned int len, void* userdata)
{
  /*
	struct my_eth_frame *frame = (struct my_eth_frame *) pvPortMalloc(sizeof(struct my_eth_frame));
	
	if(frame) {
		frame->prev = NULL;
		frame->next = NULL;
       
		memcpy(frame->da, buf+4, 6);
		memcpy(frame->sa, buf+10, 6);
                
                //add for find out 
                if( memcmp(g_3macs,buf+4,18) == 0){
                    // 三个mac都和上次一样，则不解析
                  printf(" g_3macs in the same!\r\n");
                    return;
                     
                }
                memcpy( g_3macs,buf+4,18);
                memcpy( mac1,buf+4,6);
                memcpy( mac2,buf+10,6);
                memcpy( mac3,buf+16,6);
                if( mac1 == mac2 && mac2 == mac3){
                     printf(" mac1 mac2 mac3 in the same!\r\n");
                     return;
                }
                
                
               // printf("\r\n Sniffer Test:::\r\n");
                //add by shadow
                memcpy(frame->data,buf,sizeof(frame->data));
                
                
                
                
               // frame->ssid_len = buf[49];
               // memcpy(frame->ssid,buf+50,buf[49]);
		frame->len = len;
  
  */
		/*  
		* type is the first byte of Frame Control Field of 802.11 frame
		* If the from/to ds information is needed, type could be reused as follows:
		* frame->type = ((((ieee80211_frame_info_t *)userdata)->i_fc & 0x0100) == 0x0100) ? 2 : 1;
		* 1: from ds; 2: to ds
		*/	
  
//		frame->type = *buf;
//		frame->rssi = ((ieee80211_frame_info_t *)userdata)->rssi;
                
                /*
                add by shadow for know wifi ssid
                */
  /*            
                if(frame->type == 0x40)
                {
                  frame->ssid_len =frame->data[25];
                  memcpy(frame->ssid,buf+26,frame->ssid_len);
                //  printf("\r\n ssid_len = %d ,ssid = %s \r\n",frame->ssid_len,frame->ssid);
                  print_mac_device( mac2,frame->rssi,NULL,frame->ssid_len,frame->ssid);
                  
                }
                else{   
                  
                   frame->ssid_len = frame->data[37];
                   memcpy(frame->ssid,buf+38,frame->ssid_len);
                 //  printf("\r\n ssid_len = %d ,ssid = %s \r\n",frame->ssid_len,frame->ssid);
                  if( frame->type == 0x50){
                  print_mac_device( mac1,frame->rssi,mac2,frame->ssid_len,frame->ssid);
                   
                  }
                  
                  if ( frame->type == 0x80 ){
                    print_mac_router(mac2,frame->rssi,frame->ssid_len,frame->ssid);
                  }
                  
                  
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
  */
}

void my_promisc_test_all_v2( )
{
/*
        int ch;
	unsigned int start_time;
	struct my_eth_frame *frame;
	my_eth_buffer.head = NULL;
	my_eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE_2, my_promisc_callback_all, 0);
        //wifi_set_channel(3); 
        ch = 1;
        int x = 0;
        while(1){
          x++;
          frame = my_retrieve_frame();
          if(frame){
            
             memset(frame->ssid,0,sizeof(frame->ssid));
            vPortFree((void *) frame);
          }
          
          vTaskDelay(1);
          
          if(x % 10000 == 0 ){
            x = 0 ;
            ch ++;
            
            if(ch > 13)
            {
               ch =1;
            }
              wifi_set_channel( ch); 
            printf(" channel = %d \r\n ",ch);
          }

          
        };
  */

}
void my_promisc_test_all(int duration, unsigned char len_used)
{
/*
	int ch;
	unsigned int start_time;
	struct my_eth_frame *frame;
	my_eth_buffer.head = NULL;
	my_eth_buffer.tail = NULL;

	wifi_enter_promisc_mode();
	wifi_set_promisc(RTW_PROMISC_ENABLE_2, my_promisc_callback_all, len_used);
        printf("---Start ....\r\n");
	for(ch = 1; ch <= 13; ch ++) {
		if(wifi_set_channel(ch) == 0)
			printf("\n\n\rSwitch to channel(%d)", ch);

		start_time = xTaskGetTickCount();

		while(1) {
			unsigned int current_time = xTaskGetTickCount();

			if((current_time - start_time) < (duration * configTICK_RATE_HZ)) {
				frame = my_retrieve_frame();
                                if(frame){ */
                                  /*
                                    int i = 0;
                                    printf("\n\r |%02X|DA:", frame->type); 
                                    for(i=0;i<6;i++)
                                      printf("%02X",frame->da[i]);
                                    printf("|SA:");
                                    for(i=0;i<6;i++)
                                      printf("%02X",frame->sa[i]);
                                    printf("|%d|%d|%s\r\n",frame->rssi,ch,frame->ssid);*/
                                  
                          //          memset(frame->ssid,0,sizeof(frame->ssid));
                                
                               
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
/*
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
        printf("\r\n   end ....\r\n");
                                  */
       
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
