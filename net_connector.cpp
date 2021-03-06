﻿/*
 * net_connector.cpp
 *
 *  Created on: 2013年12月16日
 *      Author: jimm
 */

#include "../common/common_api.h"
#include "net_connector.h"
#include "net_typedef.h"
#include "net_timer.h"
#include "net_handler.h"
#include "net_connection.h"
#include "net_errordef.h"
#include "net_connmgt.h"
#include "net_reactor.h"
#include "net_api.h"
#ifndef WIN32
#include <netdb.h>
#endif

NETEVENT_NAMESPACE_BEGIN

CConnector::CConnector(CNetHandler *pNetHandler, IPacketParserFactory *pFactory, IIOHandler *pIOHandler)
{
	m_pNetHandler = pNetHandler;
	m_pPacketParserFactory = pFactory;
	m_pIOHandler = pIOHandler;
}

int32_t CConnector::Connect(const char *szRemoteAddr, uint16_t nPort, uint32_t nTimeout/* = 3000*/)
{
	if (NULL == szRemoteAddr)
	{
		return E_REMOTEIP;
	}

	CConnection *pConn = m_pNetHandler->GetConnMgt().CreateConnection(m_pNetHandler, m_pPacketParserFactory, m_pIOHandler);
	if(pConn == NULL)
	{
		return E_NULLPOINTER;
	}

	struct hostent *pHostEnt = gethostbyname(szRemoteAddr);
	if(pHostEnt == NULL)
	{
		return E_NULLPOINTER;
	}

	if(pHostEnt->h_addr_list[0] == NULL)
	{
		return E_NULLPOINTER;
	}

	struct in_addr nIPAddr = *(struct in_addr *)pHostEnt->h_addr_list[0];
	char *szRemoteIP = inet_ntoa(nIPAddr);

	pConn->Open();

	//更新套接字类型和状态
	pConn->SetSessionType(enmSessionType_Communicate);

	uint32_t nRet = S_OK;
	do
	{
		//判断套接字类型
		if (enmSessionType_Communicate != pConn->GetSessionType())
		{
			nRet = E_SOCKETTYPE;
			break;
		}

		//套接字是否打开
		if ((enmInvalidSocketFD == pConn->GetSocketFD()) || (enmSessionStatus_Opened != pConn->GetSessionStatus()))
		{
			nRet = E_SOCKETNOTCREATED;
			break;
		}

		pConn->SetPeerAddress(szRemoteIP);
		pConn->SetPeerPort(nPort);
		pConn->SetPeerAddress((uint32_t)inet_addr(szRemoteIP));

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(szRemoteIP);
		addr.sin_port = htons(nPort);

		CConnectTimer *pConnTimer = m_pNetHandler->GetConnectTimerMgt().CreateConnectTimer(pConn,
				static_cast<ConnectTimerProc>(&CConnection::OnTimerEvent), nTimeout);
		if(pConnTimer == NULL)
		{
			nRet = E_UNKNOWN;
			break;
		}

		pConn->SetConnectTimer(pConnTimer);

		//与服务器端建立连接
		int32_t ret = connect(pConn->GetSocketFD(), (const struct sockaddr*)&addr, sizeof(addr));
		if (0 != ret)
		{
#ifdef WIN32
				if((ErrorNo() != WSAEINPROGRESS) && (ErrorNo() != WSAEWOULDBLOCK))
#else
				if(ErrorNo() != EINPROGRESS)
#endif
			{
				nRet = E_SOCKETCONNECT;
				break;
			}
			else
			{
				m_pNetHandler->RegistEvent(pConn, mask_read | mask_write);
				pConn->SetSessionStatus(enmSessionStatus_Connecting);
				nRet = E_SOCKET_CONNECTING;
				break;
			}
		}
		else
		{
			m_pNetHandler->RegistEvent(pConn, mask_read | mask_write);
			//更新套接字状态
			pConn->SetSessionStatus(enmSessionStatus_Connected);

			pConn->Connected();
		}
	}while(false);

	if((nRet != S_OK) && (nRet != E_SOCKET_CONNECTING))
	{
		pConn->Close();
	}

	return S_OK;
}

IPacketParserFactory *CConnector::GetPacketParserFactory()
{
	return m_pPacketParserFactory;
}

IIOHandler *CConnector::GetIOHandler()
{
	return m_pIOHandler;
}

CNetHandler *CConnector::GetNetHandler()
{
	return m_pNetHandler;
}

//读事件回调
int32_t CConnector::OnRead(int32_t nErrorCode)
{
	return S_OK;
}

//写事件回调
int32_t CConnector::OnWrite(int32_t nErrorCode)
{
	return S_OK;
}

//异常事件回调
int32_t CConnector::OnError(int32_t nErrorCode)
{
	return S_OK;
}


NETEVENT_NAMESPACE_END
