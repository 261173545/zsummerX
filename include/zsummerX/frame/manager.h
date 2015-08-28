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
#include "dispatch.h"
namespace zsummer
{
    namespace network
    {


        inline SessionBlock * DefaultCreateBlock();
        inline void DefaultFreeBlock(SessionBlock *sb);

        inline OnBlockCheckResult DefaulBlockCheck(const char * begin, unsigned int len, unsigned int bound, unsigned int blockLimit)
        {
            auto ret = zsummer::proto4z::checkBuffIntegrity(begin, len, bound, blockLimit);
            return std::make_pair((BLOCK_CHECK_TYPE)ret.first, (unsigned int)ret.second);
        }
        inline void DefaultBlockDispatch(TcpSessionPtr session, const char * begin, int len)
        {
            zsummer::proto4z::ReadStream rs(begin, len);
            MessageDispatcher::getRef().dispatchSessionMessage(session, rs.getProtoID(), rs);
        }

        inline OnBlockCheckResult DefaultHTTPBlockCheck(const char * begin, unsigned int len, unsigned int bound,
            bool hadHeader, bool & isChunked, PairString& commonLine, MapString & head, std::string & body)
        {
            unsigned int used = 0;
            auto ret = zsummer::proto4z::checkHTTPBuffIntegrity(begin, len, bound, hadHeader, isChunked, commonLine, head, body, used);
            return std::make_pair((BLOCK_CHECK_TYPE)ret, used);
        }

        inline void DefaultHTTPBlockDispatch(TcpSessionPtr session, const PairString & commonLine, const MapString &head, const std::string & body)
        {

        }


        /*
            SessionManager��һ������singleton, ��һ����zsummerX�ײ�ӿڵĸ߼���װ, �����Ҫ�Լ���װ ����Բο�frame����������example�е����ӽ��з�װ��ʹ��.
            ��������ṩ����������ĸ߼��Ŀɲ����ӿ�, ������������ģ�鵥��, ��������ѭ��, ���ιرղ������繦�� ����˳�����ѭ��,  ��Ӷ�������ӿ�, ��Ӷ������, ��������,
                ���̵߳�֪ͨ����post, ����ȡ����ʱ��, ��ȡ������Ϣ, ���ö�ʱ���, ���ö������������ͼ��, �����Ƿ�֧��flash Policy, �������������������, ����Э���Ƕ����Ƶ�TCPЭ��(proto4zЭ����), 
                ����Э����HTTP(����WEB�������Ϳͻ���ʹ��, �ܷ������һЩSDK��֤��ƽ̨����),  �ɻ�ȡ����ʱ������״̬����.
            ��Ϣ�����handlerע�� �ο��ļ�dispatch.h
        */
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

            //�˳���Ϣѭ��.
            void stop();

            //������ǰ�̲߳���ʼ��Ϣѭ��. Ĭ��ѡ������ȽϺ�. ��ϣ���и�ϸ���ȵĿ���run��ʱ���Ƽ�ʹ��runOnce
            bool run();

            //ִ��һ����Ϣ����, ���isImmediatelyΪtrue, �����۵�ǰ������������ ����Ҫ��������, ����Ƕ�뵽����һ���߳������ʹ��
            //Ĭ��Ϊfalse,  ���û��������Ϣ���¼���Ϣ �������һС��ʱ��, ����Ϣ֪ͨ�����̱�����.
            bool runOnce(bool isImmediately = false);

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



            AccepterID addAccepter(const std::string& listenIP, unsigned short listenPort);
            AccepterOptions & getAccepterOptions(AccepterID aID);
            bool openAccepter(AccepterID aID);
            AccepterID getAccepterID(SessionID sID);


            SessionID addConnecter(const std::string& remoteIP, unsigned short remotePort);
            SessionOptions & getConnecterOptions(SessionID cID);
            bool openConnecter(SessionID cID);
            TcpSessionPtr getTcpSession(SessionID sID);

            std::string getRemoteIP(SessionID sID);
            unsigned short getRemotePort(SessionID sID);

            //send data.
            void sendSessionData(SessionID sID, const char * orgData, unsigned int orgDataLen);


            //close session socket.
            void kickSession(SessionID sID);

        public:
            //statistical information
            inline unsigned long long getStatInfo(int stat){ return _statInfo[stat]; }
            unsigned long long _statInfo[STAT_SIZE];
        public:
            //����ӿ�˵��:
            // stopXXXXϵ�нӿڿ������źŴ������е���, Ҳֻ�и�ϵ�к����������źŴ�������ʹ��.
            // һЩstopXXX�ӿ��ṩ���֪ͨ, ����Ҫ����setStopXXXXȥע��ص�����.
            void stopAccept();
            void stopClients();
            void setStopClientsHandler(std::function<void()> handler);
            void stopServers();
            void setStopServersHandler(std::function<void()> handler);
        public:
            SessionBlock * CreateBlock();
            void FreeBlock(SessionBlock * sb);
        private:
            std::queue<SessionBlock*> _freeBlock;
        private:
            friend class TcpSession;
            // һ��established״̬��session�Ѿ��ر�. 
            void onSessionClose(TcpSessionPtr session, bool clean);

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
            std::unordered_map<SessionID, TcpSessionPtr> _mapTcpSessionPtr;
            std::unordered_map<AccepterID, AccepterOptions > _mapAccepterOptions;
        };




        inline SessionBlock * DefaultCreateBlock()
        {
            return SessionManager::getRef().CreateBlock();
        }

        inline void DefaultFreeBlock(SessionBlock *sb)
        {
            SessionManager::getRef().FreeBlock(sb);
        }


    }
}


#endif
