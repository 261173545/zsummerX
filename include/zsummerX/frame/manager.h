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
 * Copyright (C) 2010-2015 YaweiZhang <yawei.zhang@foxmail.com>.
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


#ifndef ZSUMMER_TCPSESSION_MANAGER_H_
#define ZSUMMER_TCPSESSION_MANAGER_H_

#include "config.h"
namespace zsummer
{
	namespace network
	{

		class SessionManager
		{
		private:
			SessionManager();

		public://!get the single and global object pointer   
			static SessionManager & getRef();
			inline static SessionManager * getPtr(){ return &getRef(); };
		public:
			//Ҫʹ��SessionManager�����ȵ���start����������.
			bool start();

			//����ӿ�˵��:
			// stopXXXXϵ�нӿڿ������źŴ������е���, Ҳֻ�и�ϵ�к����������źŴ�������ʹ��.
			// һЩstopXXX�ӿ��ṩ���֪ͨ, ����Ҫ����setStopXXXXȥע��ص�����.
			void stopAccept();
			void stopClients();
			void setStopClientsHandler(std::function<void()> handler);
			void stopServers();
			void setStopServersHandler(std::function<void()> handler);

			//�˳���Ϣѭ��.
			void stop();

			//������ǰ�̲߳���ʼ��Ϣѭ��. Ĭ��ѡ������ȽϺ�. ��ϣ���и�ϸ���ȵĿ���run��ʱ���Ƽ�ʹ��runOnce
			void run();

			//ִ��һ����Ϣ����, ���isImmediatelyΪtrue, �����۵�ǰ������������ ����Ҫ��������, ����Ƕ�뵽����һ���߳������ʹ��
			//Ĭ��Ϊfalse,  ���û��������Ϣ���¼���Ϣ �������һС��ʱ��, ����Ϣ֪ͨ�����̱�����.
			void runOnce(bool isImmediately = false);

			//handle: std::function<void()>
			//switch initiative, in the multi-thread it's switch call thread simultaneously.
			//Ͷ��һ��handler��SessionManager���߳���ȥ����, �̰߳�ȫ.
			template<class H>
			void post(H &&h){ _summer->post(std::move(h)); }

			//������ʱ�� ��λ�Ǻ��� ���̰߳�ȫ, ���ж��߳��µ����������POST��ʵ��.
			template <class H>
			zsummer::network::TimerID createTimer(unsigned int delayms, H &&h){ return _summer->createTimer(delayms, std::move(h)); }
			//ȡ����ʱ��.  ע��, ����ڶ�ʱ���Ļص�handler��ȡ����ǰ��ʱ�� ��ʧ�ܵ�.
			bool cancelTimer(unsigned long long timerID){ return _summer->cancelTimer(timerID); }


			//! add acceptor under the configure.
			AccepterID addAcceptor(const ListenConfig &traits);
			bool getAcceptorConfig(AccepterID aID, std::pair<ListenConfig, ListenInfo> & config);
			AccepterID getAccepterID(SessionID sID);

			//! add connector under the configure.
			SessionID addConnector(const ConnectConfig & traits);
			bool getConnectorConfig(SessionID sID, std::pair<ConnectConfig, ConnectInfo> & config);
			TcpSessionPtr getTcpSession(SessionID sID);

			//send data.
			void sendSessionData(SessionID sID, const char * orgData, unsigned int orgDataLen);
			//send data.
			void sendSessionData(SessionID sID, ProtoID pID, const char * userData, unsigned int userDataLen);

			//close session socket.
			void kickSession(SessionID sID);

		public:
			//statistical information
			//ͳ����Ϣ.
			std::string getRemoteIP(SessionID sID);
			unsigned short getRemotePort(SessionID sID);
			unsigned long long _totalConnectCount = 0;
			unsigned long long _totalAcceptCount = 0;
			unsigned long long _totalConnectClosedCount = 0;
			unsigned long long _totalAcceptClosedCount = 0;
            
			unsigned long long _totalSendCount = 0;
			unsigned long long _totalSendBytes = 0;
            unsigned long long _totalSendMessages = 0;
			unsigned long long _totalRecvCount = 0;
			unsigned long long _totalRecvBytes = 0;
			unsigned long long _totalRecvMessages = 0;
			unsigned long long _totalRecvHTTPCount = 0;
			time_t _openTime = 0;

		private:
			friend class TcpSession;
			// һ��established״̬��session�Ѿ��ر�. ��session�������.
			void onSessionClose(AccepterID aID, SessionID sID, const TcpSessionPtr &session);

			// һ��established״̬��session�Ѿ��رջ�������ʧ��, ��Ϊ��connect ��Ҫ�ж��Ƿ���Ҫ����.
			void onConnect(SessionID cID, bool bConnected, const TcpSessionPtr &session);

			//accept��������.
			void onAcceptNewClient(zsummer::network::NetErrorCode ec, const TcpSocketPtr & s, const TcpAcceptPtr & accepter, AccepterID aID);
		private:

			//��Ϣѭ��
			EventLoopPtr _summer;

			//! ����һ�������Ϊ������Ϣѭ���Ŀ����͹ر��õ�
			bool  _running = true;  //Ĭ���ǿ���, ������ں��ʵ�ʱ���˳���Ϣѭ��.
			bool _stopAccept = false; //ֹͣaccept�µ�����.
			char _stopClients = 0; //�ص����пͻ�������.
			std::function<void()> _funClientsStop; // ���пͻ��˶����رպ���ִ������ص�.
			char _stopServers = 0; //�ص���������������.
			std::function<void()> _funServerStop; // �����������ӱ��رպ�ִ�еĻص�.


			//!����һ��ID�������ɶ�Ӧ��unique ID. 
			SessionID _lastAcceptID = 0; //accept ID sequence. range  [0 - (unsigned int)-1)
			SessionID _lastSessionID = 0;//session ID sequence. range  [0 - __MIDDLE_SEGMENT_VALUE)
			SessionID _lastConnectID = 0;//connect ID sequence. range  [__MIDDLE_SEGMENT_VALUE - -1)

			//!�洢��ǰ������������session��Ϣ��accept��������Ϣ.
			std::unordered_map<AccepterID, TcpAcceptPtr> _mapAccepterPtr;
			std::unordered_map<SessionID, TcpSessionPtr> _mapTcpSessionPtr;

			//!�洢��Ӧ��������Ϣ.
			std::unordered_map<SessionID, std::pair<ConnectConfig, ConnectInfo> > _mapConnectorConfig;
			std::unordered_map<AccepterID, std::pair<ListenConfig, ListenInfo> > _mapAccepterConfig;
		public:
		};



	}
}


#endif
