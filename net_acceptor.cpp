﻿/*
 * net_acceptor.cpp
 *
 *  Created on: 2013年12月16日
 *      Author: jimm
 */

#include "net_acceptor.h"
#include "net_handler.h"
#include "net_connection.h"
#include "net_connmgt.h"
#include "net_reactor.h"
#include <time.h>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

NETEVENT_NAMESPACE_BEGIN

CAcceptor::CAcceptor(CNetHandler *pNetHandler, IPacketParserFactory *pFactory, IIOHandler *pIOHandler)
{
	m_pNetHandler = pNetHandler;
	m_pPacketParserFactory = pFactory;
	m_pIOHandler = pIOHandler;
}

int32_t CAcceptor::Bind(const char *szLocalIP, uint16_t nPort)
{
	Open();

	if((m_nSessionType == enmSessionType_Listen) || (nPort > 0))
	{
		//设置套接字属性
		if(set_reuse_addr(m_nSocketFD) != 0)
		{
			Close();
			return E_SOCKETOPTION;
		}
		//填充服务器地址&端口信息
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		if((NULL == szLocalIP) || (strlen(szLocalIP) == 0))
		{
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		else
		{
			addr.sin_addr.s_addr = inet_addr(szLocalIP);
		}
		addr.sin_port = htons(nPort);

        int32_t ret = ::bind(m_nSocketFD, (struct sockaddr *)&addr, sizeof(addr));
		if (0 != ret)
		{
			Close();
			return E_SOCKETBIND;
		}

		//开始监听
		ret = listen(m_nSocketFD, SOMAXCONN);
		if (0 != ret)
		{
			Close();
			return E_SOCKETLISTEN;
		}

		m_nLocalAddress = addr.sin_addr.s_addr;
		m_nLocalPort = nPort;

		m_pNetHandler->RegistEvent(this, mask_read);
	}

	//设置为非阻塞
	set_non_block(m_nSocketFD);

	//更新套接字类型和状态
	m_nSessionType = enmSessionType_Listen;
	m_nSessionStatus = enmSessionStatus_Opened;

	return S_OK;
}

int32_t CAcceptor::Bind(uint16_t nPort)
{
	return Bind(NULL, nPort);
}

IPacketParserFactory *CAcceptor::GetPacketParserFactory()
{
	return m_pPacketParserFactory;
}

IIOHandler *CAcceptor::GetIOHandler()
{
	return m_pIOHandler;
}

//读事件回调
int32_t CAcceptor::OnRead(int32_t nErrorCode)
{
	return S_OK;
}

//写事件回调
int32_t CAcceptor::OnWrite(int32_t nErrorCode)
{
	return S_OK;
}

//异常事件回调
int32_t CAcceptor::OnError(int32_t nErrorCode)
{
	return S_OK;
}

//接收连接回调(重载此函数，可以在这里做些连接信息初始化的工作)
int32_t CAcceptor::OnAccept(SocketFD nAcceptFD, struct sockaddr_in stPeerAddress,
		socklen_t nPeerAddressLen)
{
	CConnMgt &stConnMgt = m_pNetHandler->GetConnMgt();
	CConnection *pConnection = stConnMgt.CreateConnection(m_pNetHandler, m_pPacketParserFactory, m_pIOHandler);
	pConnection->SetSocketFD(nAcceptFD);
	pConnection->SetSessionStatus(enmSessionStatus_Connected);
	pConnection->SetPeerAddress((uint32_t)stPeerAddress.sin_addr.s_addr);
	pConnection->SetPeerPort(stPeerAddress.sin_port);
	pConnection->SetCreateTime(time(NULL));
	pConnection->SetSourceRole(false);

	set_non_block(nAcceptFD);

	stConnMgt.RegistConnection(pConnection);

	m_pNetHandler->RegistEvent(pConnection, mask_read | mask_write);
	m_pIOHandler->OnOpened(pConnection);

	return S_OK;
}


NETEVENT_NAMESPACE_END
