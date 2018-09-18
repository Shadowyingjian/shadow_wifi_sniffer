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
  u8 Protocol:2;  //�汾��һ��Ϊ00
  u8 Type:2;      //00 ����֡ 01 ����֡ 
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
  u8 ToDS:1;  //��DS
  u8 FromDS:1;  //����DS
  u8 MoreFlag:1; //����ֶ�
  u8 Retry:1;  //�ش�
  u8 PwrMgmt:1; //��Դ����
  u8 MoreData:1;  //��������
  u8 Protectedframe:1; //�ܱ���֡
  u8 Order:1;  //˳��
} framectrl_80211,*lpframectrl_80211;
//To DS		From DS		Address1	Address2	Address3	Address4
//	0			0			DA			SA		 BSSID		  N/A
//	0			1			DA		 BSSID		   SA		  N/A
//	1			0		 BSSID			SA		   DA		  N/A
//	1			1			RA			TA		   DA		  SA
//DS�Ƿֲ�ʽϵͳ�ļ�ƣ��������������ֲ�ͬ����֡�й��ڵ�ַ�Ľ�����ʽ��
//To DS=0,From DS=0:��ʾStation֮���AD Hoc���Ƶ�ͨ�ţ����߿����졢�����졣
//To DS=0,From DS=1:Station���յ��졣
//To DS=1,From DS = 0:Station���͵��졣
//To DS=1,From DS = 1:�����Ž����ϵ������졣
//SRC��Դ��ַ��SA��������̫���е�һ�������Ƿ�֡�������ַ������̫����WIFI��֡��ʽת����ʱ�򣬻������ֱ�Ӹ��ơ�
//DST��Ŀ�ĵ�ַ��DA��������̫���е�һ�����������ս�������֡�ĵ�ַ������̫����WIFI��֡��ʽת����ʱ�򣬻������ֱ�Ӹ��ơ�
//TX��Ҳ����Transmiter��TA������ʾ����������Ŀǰʵ�ʷ���֡�ߵĵ�ַ�������������֡���ˣ�Ҳ������ת��ʱ���·�ɣ���
//RX��Ҳ����Receiver��RA������ʾ���������У�Ŀǰʵ�ʽ���֡�ߵĵ�ַ�����������յĽ����ߣ�Ҳ�����ǽ���֡�Ա�ת���������ߵ�AP����
//ע�����������е�Station��Ҫͨ�ţ����뾭��AP�����С�ת������
//��ʵ��Tx��Rx�����������еķ����գ�Ҳ����Radio����Src��Dst�������ķ���Դ�ͽ�����

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
  signed rssi:8; //��ʾ�ð����ź�ǿ��
  unsigned rate:4; 
  unsigned is_group:1;
  unsigned:1;
  unsigned sig_mode:2;  //��ʾ�ð��Ƿ���11n�İ�,��0��ʾ��11n����0��ʾ11n
  unsigned legacy_length:12;//�������11n�İ�������ʾ���ĳ���
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;//�����11n�İ�������ʾ���ĵ��Ʊ������У���Чֵ��0-76
    unsigned CWB:1;//�����11n�İ�������ʾ�Ƿ�ΪHT40�İ�
    unsigned HT_length:16;//�����11n�İ�������ʾ���ĳ���
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;//�����11n�İ�������ʾ�Ƿ�ΪLDPC�İ�
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;//��ʾ�ð����ڵ��ŵ�
    unsigned:12;
};  //12Bytes

struct sniffer_buf{
	struct RxControl rx_ctrl;
	struct framectrl_80211 fr_ctrl;
	u8 buf[46];//��ieee80211��ͷ
	u16 cnt;//���ĸ���
	u16 len[1];//���ĳ���
};



























#endif //_WIFI802EE_H