/*
 * zsummerX License
 * -----------
 * 
 * zsummerX is licensed under the terms of the MIT license reproduced below.
 * This means that zsummerX is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2014 YaweiZhang <yawei_zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */


//! frameģ��ѹ������
//! frame��װ�����粿��ʹ�õ���ģʽ, �����Ҫʹ�ö��߳� ��Ҫ��ҵ������û������̳߳ش���.
//! �����Ҫ��zsummerX�����粿��ʹ�ö��߳� ��ο�tcpTestʵ������zsummerX��ԭʼ�ӿ�ʵ��.


#include <zsummerX/FrameHeader.h>
#include <zsummerX/FrameTcpSessionManager.h>
#include <zsummerX/FrameMessageDispatch.h>
#include <unordered_map>
using namespace zsummer::log4z;

//! Ĭ����������
std::string g_remoteIP = "0.0.0.0"; //�����Ϊ�������ʹ�� �����Ǽ�����ַ
unsigned short g_remotePort = 81; //ͬ��
unsigned short g_startType = 0;  //0 ��Ϊ����������, 1 ��Ϊ�ͻ�������
unsigned short g_maxClient = 1; //����Ƿ���� ���������ƿͻ��˵ĸ��� �����Ļᱻ�ߵ�, ����ǿͻ��� �����������Ŀͻ�������.
unsigned short g_sendType = 0; //0 ping-pong test, 1 flood test
unsigned int   g_intervalMs = 0; // �����flood test, ����ļ��Ӧ�ô���0, ��λ�Ǻ���.



//! g_testStr������ѹ�������з���ͳһ��С����Ϣ.
std::string g_testStr;

//!�շ�������ͳ������
unsigned long long g_totalEchoCount = 0;
unsigned long long g_lastEchoCount = 0;
unsigned long long g_totalSendCount = 0;
unsigned long long g_totalRecvCount = 0;
void MonitorFunc()
{
	LOGI("per seconds Echos Count=" << (g_totalEchoCount - g_lastEchoCount) / 5
		<< ", g_totalSendCount[" << g_totalSendCount << "] g_totalRecvCount[" << g_totalRecvCount << "]");
	g_lastEchoCount = g_totalEchoCount;
	CTcpSessionManager::getRef().CreateTimer(5000, MonitorFunc);
};

/*
* ���Դ����ж�����4��Э�� ����ʵ���������ƺ�echo����.
* ECHO��������Э��� һ�������ͻ��˷��� һ����������˻ظ�.
*/
#define C2S_HEARTBEAT ProtocolID(10000)
#define S2C_HEARTBEAT ProtocolID(10000)

#define C2S_ECHO_REQ ProtocolID(10002)
#define S2C_ECHO_ACK ProtocolID(10003)

/*��������״̬
*��������Դ����� �ѷ���˺Ϳͻ��˵������������������һ������.
* ����������ʵ�ֲ�����:
*  �����ӽ����¼�����ʱ ��ʼ������ʱ���
*  ���յ��Է�������Ϣ�����ʱ���
*  �ڱ���������ʱ������ʱ���ʱ��(ͬʱ�����������Է�)
*  �����ӶϿ��¼����������ʱ���
*
* �����������ڱ����Դ������ǵ���� ������涨���Э�����ͬ �������Ӱ��.
* �����Ҫ��������Я�����������, ������ʱ���ӳ�, ����԰���������ΪREQ-ACK��ʽ(���һ��ACK����). ��REQ�з��ͱ���ʱ�� ��ACKʱ����ʱ���.
*/
class CStressHeartBeatManager
{
public:
	CStressHeartBeatManager()
	{
		//! ע���¼�����Ϣ
		CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CStressHeartBeatManager::OnConnecotrConnected, this,
			std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterOnMyConnectorHeartbeatTimer(std::bind(&CStressHeartBeatManager::OnConnecotrHeartbeatTimer, this,
			std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CStressHeartBeatManager::OnConnecotrDisconnect, this,
			std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterConnectorMessage(S2C_HEARTBEAT, std::bind(&CStressHeartBeatManager::OnMsgServerHeartbeat, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


		CMessageDispatcher::getRef().RegisterOnSessionEstablished(std::bind(&CStressHeartBeatManager::OnSessionEstablished, this,
			std::placeholders::_1, std::placeholders::_2));
		CMessageDispatcher::getRef().RegisterOnMySessionHeartbeatTimer(std::bind(&CStressHeartBeatManager::OnSessionHeartbeatTimer, this,
			std::placeholders::_1, std::placeholders::_2));
		CMessageDispatcher::getRef().RegisterOnSessionDisconnect(std::bind(&CStressHeartBeatManager::OnSessionDisconnect, this,
			std::placeholders::_1, std::placeholders::_2));
		CMessageDispatcher::getRef().RegisterSessionMessage(C2S_HEARTBEAT, std::bind(&CStressHeartBeatManager::OnMsgConnectorHeartbeat, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	}
	
	void OnConnecotrConnected(ConnectorID cID)
	{
		m_connectorHB[cID] = time(NULL);
		LOGI("connect sucess. cID=" << cID);
	}
	void OnConnecotrHeartbeatTimer(ConnectorID cID)
	{
		auto iter = m_connectorHB.find(cID);
		if (iter == m_connectorHB.end() || time(NULL) - iter->second > HEARTBEART_INTERVAL/1000*2)
		{
			LOGI("server  lost. cID=" << cID << ", timeout=" << time(NULL) - iter->second);
			CTcpSessionManager::getRef().BreakConnector(cID);
		}
		WriteStreamPack pack;
		pack << C2S_HEARTBEAT;
		CTcpSessionManager::getRef().SendOrgConnectorData(cID, pack.GetStream(), pack.GetStreamLen());
	}
	void OnConnecotrDisconnect(ConnectorID cID)
	{
		m_connectorHB.erase(cID);
		LOGI("Disconnect. cID=" << cID);
	}

	void OnMsgServerHeartbeat(ConnectorID cID, ProtocolID pID, ReadStreamPack & pack)
	{
		auto iter = m_connectorHB.find(cID);
		if (iter != m_connectorHB.end())
		{
			iter->second = time(NULL);
		}
	}

	void OnSessionEstablished(AccepterID aID, SessionID sID)
	{
		m_sessionHB[sID] = time(NULL);
		LOGI("remote session connected. sID=" << sID);
	}
	void OnSessionHeartbeatTimer(AccepterID aID, SessionID sID)
	{
		auto iter = m_sessionHB.find(sID);
		if (iter == m_sessionHB.end() || time(NULL) - iter->second > HEARTBEART_INTERVAL/1000 * 2)
		{
			LOGI("remote session lost. sID=" << sID << ", timeout=" << time(NULL) - iter->second);
			CTcpSessionManager::getRef().KickSession(aID, sID);
		}
		WriteStreamPack pack;
		pack << S2C_HEARTBEAT;
		CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, pack.GetStream(), pack.GetStreamLen());
	}
	void OnSessionDisconnect(AccepterID aID, SessionID sID)
	{
		LOGI("remote session Disconnect. sID=" << sID );
		m_sessionHB.erase(sID);
	}
	void OnMsgConnectorHeartbeat(AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & pack)
	{
		auto iter = m_sessionHB.find(sID);
		if (iter != m_sessionHB.end())
		{
			iter->second = time(NULL);
		}
	}

protected:

private:
	//! ע����¼����������session or connector�� ������¼�����ʱ�� Ҫȥ���Ҷ�Ӧ��������Ϣ
	std::unordered_map<SessionID, time_t> m_sessionHB;
	std::unordered_map<ConnectorID, time_t> m_connectorHB;
};


/* ���Կͻ���handler��
* ��������ͻ���������ping-pongѹ������, �����̲����ڶ�ʱ�� �������� 
*  ��connector�ɹ����͵�һ��send��(ping), Ȼ����ÿ��recv��ʱ��(pong)�������send��(pong).
* ��������ͻ���������floodѹ������, ��ÿ�����Ӷ���Ҫ����һ����ʱ�� ��������
*  ��connector�ɹ��� send��ʱ������, ����ָ����ʱ�����ʱ�����send.
*/
class CStressClientHandler
{
public:
	CStressClientHandler()
	{
		CMessageDispatcher::getRef().RegisterOnConnectorEstablished(std::bind(&CStressClientHandler::OnConnected, this, std::placeholders::_1));
		CMessageDispatcher::getRef().RegisterConnectorMessage(S2C_ECHO_ACK,
			std::bind(&CStressClientHandler::msg_ResultSequence_fun, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		CMessageDispatcher::getRef().RegisterOnConnectorDisconnect(std::bind(&CStressClientHandler::OnConnectDisconnect, this, std::placeholders::_1));
	}

	void OnConnected (ConnectorID cID)
	{
		LOGI("OnConnected. ConnectorID=" << cID );
		WriteStreamPack ws;
		ws << C2S_ECHO_REQ << "client request one REQ.";
		CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());
		g_totalSendCount++;
		if (g_sendType != 0 && g_intervalMs > 0)
		{
			CTcpSessionManager::getRef().CreateTimer(g_intervalMs, std::bind(&CStressClientHandler::SendFunc, this, cID));
			m_sessionStatus[cID] = true;
		}
	};
	void OnConnectDisconnect(ConnectorID cID)
	{
		m_sessionStatus[cID] = false;
	}

	void msg_ResultSequence_fun(ConnectorID cID, ProtocolID pID, ReadStreamPack & rs)
	{
		std::string msg;
		rs >> msg;
		g_totalRecvCount++;
		g_totalEchoCount++;

		if (g_sendType == 0 || g_intervalMs == 0) //echo send
		{
			WriteStreamPack ws;
			ws << C2S_ECHO_REQ << g_testStr;
			CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());
			g_totalSendCount++;
		}
	};
	void SendFunc(ConnectorID cID)
	{
		if (g_totalSendCount - g_totalRecvCount < 10000)
		{
			WriteStreamPack ws;
			ws << C2S_ECHO_REQ << g_testStr;
			CTcpSessionManager::getRef().SendOrgConnectorData(cID, ws.GetStream(), ws.GetStreamLen());
			g_totalSendCount++;
		}
		if (m_sessionStatus[cID])
		{
			CTcpSessionManager::getRef().CreateTimer(g_intervalMs, std::bind(&CStressClientHandler::SendFunc, this, cID));
		}
	};
private:
	std::unordered_map<ConnectorID, bool> m_sessionStatus;
};


/*
* �����handler��
* ������߼���Ϊ�� ֻҪ�յ���Ϣ�� ����echo��ȥ�Ϳ���.
*/
class CStressServerHandler
{
public:
	CStressServerHandler()
	{
		CMessageDispatcher::getRef().RegisterSessionMessage(C2S_ECHO_REQ,
			std::bind(&CStressServerHandler::msg_RequestSequence_fun, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	}

	void msg_RequestSequence_fun (AccepterID aID, SessionID sID, ProtocolID pID, ReadStreamPack & rs)
	{
		std::string msg;
		rs >> msg;
		msg += " echo";
		WriteStreamPack ws;
		ws << S2C_ECHO_ACK << msg;
		CTcpSessionManager::getRef().SendOrgSessionData(aID, sID, ws.GetStream(), ws.GetStreamLen());
		g_totalEchoCount++;
		g_totalSendCount++;
		g_totalRecvCount++;
	};
};

int main(int argc, char* argv[])
{

#ifndef _WIN32
	//! linux����Ҫ���ε�һЩ�ź�
	signal( SIGHUP, SIG_IGN );
	signal( SIGALRM, SIG_IGN ); 
	signal( SIGPIPE, SIG_IGN );
	signal( SIGXCPU, SIG_IGN );
	signal( SIGXFSZ, SIG_IGN );
	signal( SIGPROF, SIG_IGN ); 
	signal( SIGVTALRM, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGCHLD, SIG_IGN);
#endif
	if (argc == 2 && 
		(strcmp(argv[1], "--help") == 0 
		|| strcmp(argv[1], "/?") == 0))
	{
		cout << "please input like example:" << endl;
		cout << "tcpTest remoteIP remotePort startType maxClient sendType interval" << endl;
		cout << "./tcpTest 0.0.0.0 81 0" << endl;
		cout << "startType: 0 server, 1 client" << endl;
		cout << "maxClient: limit max" << endl;
		cout << "sendType: 0 echo send, 1 direct send" << endl;
		cout << "interval: send once interval" << endl;
		return 0;
	}
	if (argc > 1)
		g_remoteIP = argv[1];
	if (argc > 2)
		g_remotePort = atoi(argv[2]);
	if (argc > 3)
		g_startType = atoi(argv[3]);
	if (argc > 4)
		g_maxClient = atoi(argv[4]);
	if (argc > 5)
		g_sendType = atoi(argv[5]);
	if (argc > 6)
		g_intervalMs = atoi(argv[6]);

	if (g_startType == 0)
		ILog4zManager::GetInstance()->Config("server.cfg");
	else
		ILog4zManager::GetInstance()->Config("client.cfg");
	ILog4zManager::GetInstance()->Start();

	LOGI("g_remoteIP=" << g_remoteIP << ", g_remotePort=" << g_remotePort << ", g_startType=" << g_startType
		<< ", g_maxClient=" << g_maxClient << ", g_sendType=" << g_sendType << ", g_intervalMs=" << g_intervalMs);


	ILog4zManager::GetInstance()->SetLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_INFO);



	CTcpSessionManager::getRef().Start();

	g_testStr.resize(1024, 's');

	CTcpSessionManager::getRef().CreateTimer(5000, MonitorFunc);

	//������������handler��ʵ�� ֻҪ��������, ���캯���л�ע���Ӧ�¼�
	CStressHeartBeatManager statusManager;

	//���ﴴ������handler�Ϳͻ���handler ��������������ͬ��Ӳ�ͬ��ɫ.
	CStressClientHandler client;
	CStressServerHandler server;
	if (g_startType) //client
	{
		//��Ӷ��connector.
		for (int i = 0; i < g_maxClient; ++i)
		{
			tagConnctorConfigTraits traits;
			traits.cID = InvalidConnectorID+1+i;
			traits.remoteIP = g_remoteIP;
			traits.remotePort = g_remotePort;
			traits.reconnectInterval = 5000;
			traits.reconnectMaxCount = 50;
			CTcpSessionManager::getRef().AddConnector(traits);
		}
	}
	else
	{
		//���acceptor
		tagAcceptorConfigTraits traits;
		traits.aID = 1;
		traits.listenPort = g_remotePort;
		traits.maxSessions = g_maxClient;
		//traits.whitelistIP.push_back("127.0.");
		CTcpSessionManager::getRef().AddAcceptor(traits);
	}

	//������ѭ��.
	CTcpSessionManager::getRef().Run();

	return 0;
}

