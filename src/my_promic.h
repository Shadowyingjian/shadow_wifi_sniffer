#ifndef _MY_PROMIC_H_
#define _MY_PROMIC_H_

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
      unsigned char *ptr_data;
      
      
};

struct my_inic_eth_frame{
        unsigned char da[6];
        unsigned char sa[6];
        unsigned int len;
        unsigned char type;
        unsigned char ssid[32];
        unsigned int ssid_len;
};
struct my_eth_buffer{
      struct my_eth_frame *head;
      struct my_eth_frame *tail;
};

typedef struct{
    int64_t mac;
    uint8_t rssi;
    uint64_t time;
}vdata_device_t;

typedef struct{
    int64_t mac;
    uint8_t rssi;
    uint8_t ssid_len;
    uint8_t ssid[32];
    uint64_t time;
}vdata_router_t;

typedef struct{
    uint8_t ssid[32];
    uint8_t len;
    uint8_t mac[6];
    uint64_t time;
}vdata_ssid_t;

static struct my_eth_buffer my_eth_buffer;


#endif //_MY_PROMIC_H_