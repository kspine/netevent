/*
 * net_typedef.h
 *
 *  Created on: 2013��11��4��
 *      Author: jimm
 */

#ifndef NET_TYPEDEF_H_
#define NET_TYPEDEF_H_

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif


#ifdef WIN32
typedef SOCKET		SocketFD;
enum
{
	enmInvalidSocketFD		= INVALID_SOCKET,		//��Ч�׽���
};
#else
typedef int32_t			SocketFD;
enum
{
	enmInvalidSocketFD		= -1,					//��Ч�׽���
};
#endif

//�׽�������
typedef enum enmSocketType
{
	enmSocketType_Listen				= 0,		//�����׽���
	enmSocketType_Communicate			= 1,		//ͨ���׽���
} SocketType;

//�׽�������
typedef enum enmSocketAttr
{
	enmSocketAttr_Unknown				= 0,		//δ֪�׽���
	enmSocketAttr_Outside				= 1,		//�ⲿ�׽���
	enmSocketAttr_Inside				= 2,		//�ڲ��׽���
} SocketAttr;

//�׽�������״̬
typedef enum enmSocketStatus
{
	enmSocketStatus_Closed				= 0,		//
	enmSocketStatus_Opened				= 1,
	enmSocketStatus_Connecting			= 2,
	enmSocketStatus_Connected			= 3,
	enmSocketStatus_Error				= 4,
	enmSocketStatus_Available			= 5,
} SocketStatus;

enum
{
	enmRecvBufferSize				= 0x00040000,		//���ջ�������С
	enmSendBufferSize				= 0x00040000,		//���ͻ�������С
};


enum
{
	enmMaxConnectCount					= 500,			//����������
	enmDefaultConnectionTimeout			= 1000,			//Ĭ�����ӳ�ʱʱ��
};


enum
{
	enmMaxMessageSize					= 0x40000,	//�����Ϣ����, 1024*256, 256k
	enmMaxIPAddressLength				= 20,	//IP��ַ��󳤶�
};

/*********************************************************************
* ʱ��������س�������
*********************************************************************/
#define US_PER_MS				((int64_t)1000)
#define US_PER_SECOND			((int64_t)(1000 * US_PER_MS))
#define US_PER_MINUTE			((int64_t)(60 * US_PER_SECOND))
#define US_PER_HOUR				((int64_t)(60 * US_PER_MINUTE))
#define US_PER_DAY				((int64_t)(24 * US_PER_HOUR))

#define MS_PER_SECOND			1000
#define SECOND_PER_MINUTE		60
#define MINUTE_PER_HOUR			60
#define HOUR_PER_DAY			24
#define DAY_PER_YEAR			365

#define SECOND_PER_HOUR			(MINUTE_PER_HOUR * SECOND_PER_MINUTE)
#define SECOND_PER_DAY			(HOUR_PER_DAY * SECOND_PER_HOUR)
#define SECOND_PER_WEEK			(SECOND_PER_DAY * 7)
#define SECOND_PER_YEAR			(DAY_PER_YEAR * SECOND_PER_DAY)



#endif /* NET_TYPEDEF_H_ */