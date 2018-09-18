#ifndef _WIFI802EE_H_
#define _WIFI802EE_H_

 
#define FRAME_TYPE_MANAGEEMENT 0
#define FRAME_TYPE_CONTROL 1
#define FRAME_TYPE_DATA 2
#define FRAME_SUBTYPE_PROBE_REQUEST 0x04
#define FRAME_SUBTYPE_PROBE_RESPONSE 0x05
#define FRAME_SUBTYPE_BEACON 0x08
#define FRAME_SUBTYPE_AUTH 0x0b
#define FRAME_SUBTYPE_DEAUTH 0x0c
#define FRAME_SUBTYPE_DATA 0x14
typedef unsigned char       u8;
typedef __UINT16_T_TYPE__ uint16;
 typedef __UINT8_T_TYPE__ uint8;
typedef struct framectrl_80211
{
     //buf[0];
  u8 Protocol:2;  //版本，一般为00
  u8 Type:2;      //00 管理帧 01 控制帧 
  u8 Subtype:4;  //0000 0001 request /response
                 //0010 0011 reassociation request/response
                 //0100 0101 probe request/response
                 //0110 0111 reserved
                 //1000 Beacon
                 //1001 Announcement traffic indication message (ATM)
                 //1010 Disassociation
                 //1011 Authentication
                 //1100 Deauthentication
                 //1101 1111 -Reserved
   
  //buf[1]
  u8 ToDS:1;  //到DS
  u8 FromDS:1;  //来自DS
  u8 MoreFlag:1; //更多分段
  u8 Retry:1;  //重传
  u8 PwrMgmt:1; //电源管理
  u8 MoreData:1;  //更多数据
  u8 Protectedframe:1; //受保护帧
  u8 Order:1;  //顺序
} framectrl_80211,*lpframectrl_80211;
//To DS		From DS		Address1	Address2	Address3	Address4
//	0			0			DA			SA		 BSSID		  N/A
//	0			1			DA		 BSSID		   SA		  N/A
//	1			0		 BSSID			SA		   DA		  N/A
//	1			1			RA			TA		   DA		  SA
//DS是分布式系统的简称，我们用它以区分不同类型帧中关于地址的解析方式。
//To DS=0,From DS=0:表示Station之间的AD Hoc类似的通信，或者控制侦、管理侦。
//To DS=0,From DS=1:Station接收的侦。
//To DS=1,From DS = 0:Station发送的侦。
//To DS=1,From DS = 1:无线桥接器上的数据侦。
//SRC：源地址（SA），和以太网中的一样，就是发帧的最初地址，在以太网和WIFI中帧格式转换的时候，互相可以直接复制。
//DST：目的地址（DA），和以太网中的一样，就是最终接受数据帧的地址，在以太网和WIFI中帧格式转换的时候，互相可以直接复制。
//TX：也就是Transmiter（TA），表示无线网络中目前实际发送帧者的地址（可能是最初发帧的人，也可能是转发时候的路由）。
//RX：也就是Receiver（RA），表示无线网络中，目前实际接收帧者的地址（可能是最终的接收者，也可能是接收帧以便转发给接收者的AP）。
//注：无线网络中的Station想要通信，必须经过AP来进行“转发”。
//其实，Tx和Rx是无线网络中的发和收，也就是Radio；而Src和Dst是真正的发送源和接收者

typedef struct probe_request_80211
{
   struct framectrl_80211 framectrl;
   uint16 duration;
   uint8 rdaddr[6];
   uint8 tsaddr[6];
   uint8 bssid[6];
   uint16 number;
} probe_request,*pprobe_request;

typedef struct tagged_parameter
{
   /*  SSID parameter */
  uint8 tag_number;
  uint8 tag_length;
} tagged_parameter,*ptagged_paramter;

struct RxControl {
  signed rssi:8; //表示该包的信号强度
  unsigned rate:4; 
  unsigned is_group:1;
  unsigned:1;
  unsigned sig_mode:2;  //表示该包是否是11n的包,，0表示非11n，非0表示11n
  unsigned legacy_length:12;//如果不是11n的包，它表示包的长度
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;//如果是11n的包，它表示包的调制编码序列，有效值：0-76
    unsigned CWB:1;//如果是11n的包，它表示是否为HT40的包
    unsigned HT_length:16;//如果是11n的包，它表示包的长度
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;//如果是11n的包，它表示是否为LDPC的包
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;//表示该包所在的信道
    unsigned:12;
};  //12Bytes

struct sniffer_buf{
	struct RxControl rx_ctrl;
	struct framectrl_80211 fr_ctrl;
	u8 buf[46];//含ieee80211包头
	u16 cnt;//包的个数
	u16 len[1];//包的长度
};



























#endif //_WIFI802EE_H