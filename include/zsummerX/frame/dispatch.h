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




#ifndef ZSUMMER_MESSAGE_DISPATCHER_H_
#define ZSUMMER_MESSAGE_DISPATCHER_H_
#include "session.h"
namespace zsummer
{
    namespace network
    {
        /*
            MessageDispatcher ��һ��Э����Ϣ�ɷ���, ��һ������singleton.
            ��Ϣ�ɷ�����������, ����Ҫʹ�����ȵ��ö�Ӧ��registerXXXϵ�нӿڽ���ע��.
            ��һ�����¼����͵�֪ͨ, �����ӽ���, ���ӶϿ�, ���ӵĶ�ʱ���ص�.
            �ڶ���Ϊ��Ϣ���͵Ĵ���֪ͨ,  ��Ϣ����֪ͨ��TCP��HTTP,  TCP������, �ֱ���Ԥ����֪ͨ,����Э�鴦���Ĭ��Э�鴦��. HTTPֻ��һ����Ϣ����ص�. 
             ����ʹ�òμ�����,ע�ͺ�ʾ��.
        */
        using ProtoID = unsigned short;
        //!register message with original net pack, if return false other register will not receive this message.
        class TcpSession;
        typedef  std::shared_ptr<TcpSession> TcpSessionPtr;
        typedef std::function < bool(TcpSessionPtr&, const char * /*blockBegin*/, typename zsummer::proto4z::Integer /*blockSize*/) > OnPreMessageFunction;

        //!register message 
        typedef std::function < void(TcpSessionPtr&, zsummer::proto4z::ReadStream &) > OnMessageFunction;
        //!register message 
        typedef std::function < void(TcpSessionPtr&, ProtoID, zsummer::proto4z::ReadStream &) > OnDefaultMessageFunction;
        //!register event 
        typedef std::function < void(TcpSessionPtr&) > OnSessionEstablished;
        typedef std::function < void(TcpSessionPtr&) > OnSessionDisconnect;

        //register http proto message
        typedef std::function < void(TcpSessionPtr&, const zsummer::proto4z::PairString &, const zsummer::proto4z::HTTPHeadMap& /*head*/, const std::string & /*body*/) > OnHTTPMessageFunction;

        //register pulse timer .  you can register this to implement heartbeat . 
        typedef std::function < void(TcpSessionPtr&, unsigned int/*pulse interval*/) > OnSessionPulseTimer;



        class MessageDispatcher
        {
        private:
            MessageDispatcher(){}
            typedef std::unordered_map<ProtoID, std::vector<OnMessageFunction> > MapProtoDispatch;
        public:
            static MessageDispatcher & getRef();
            inline static MessageDispatcher * getPtr(){ return &getRef(); }
            ~MessageDispatcher(){};

            //ע����ϢԤ����handler, �յ�������Ϣ����ԭʼ�����ݰ��ɷ������handler,  ���handler�ķ���ֵΪfalse, �����������Ե�, ���ᱻ������Ϣ����handler����, ����ͬ��������PreMessage��handler.
            inline void registerSessionPreMessage(const OnPreMessageFunction & msgfun){ _vctPreSessionDispatch.push_back(msgfun); }
            //ע����Ϣ����handler, ����������ϢID����ע��, �ص������ǰ����Ѿ�����õ���Э��ID�Լ�attach�õ�ReadStream. ����ֱ��ʹ��ReadStream������Ӧ��Э������.
            inline void registerSessionMessage(ProtoID protoID, const OnMessageFunction & msgfun){ _mapSessionDispatch[protoID].push_back(msgfun); }
            //ע��Ĭ�ϵ���Ϣ����handler, ���һ��Э���Ҳ�����Ӧ��Э�鴦��handler, ��ύ��Ĭ�ϵĴ���handlerȥ����.
            inline void registerSessionDefaultMessage(const OnDefaultMessageFunction & msgfun){ _vctDefaultSessionDispatch.push_back(msgfun); }

            //event. can use method isSessionID or isConnectID to resolution who is the sessionID
            //ע�����ӽ����ͶϿ���֪ͨ, ��������������ӶϿ��������������ӶϿ� ��ʹ��isSessionID()��isConnectID()���ж�.
            inline void registerOnSessionEstablished(const OnSessionEstablished & fun){ _vctOnSessionEstablished.push_back(fun); }
            inline void registerOnSessionDisconnect(const OnSessionDisconnect & fun){ _vctOnSessionDisconnect.push_back(fun); }

            //heartbeat
            //���ݼ����������������������е�_pulseInterval������ʱ�ص�,  ���ӽ�����ĵ�һ�ε����� ����ʱ��+_pulseInterval ��ʱ��.
            inline void registerOnSessionPulse(const OnSessionPulseTimer & fun) { _vctOnSessionPulse.push_back(fun); }
            //http

            //ע��HTTPЭ��.
            //������. PairString��commondLine��Ϣ.  ����GET���� http://127.0.0.1:8081/index.php?id=1,  commondLine.first="GET", second="/index.php?id=1"
            //commondLine�ֱ���GET,POST,  ��������Իظ���Ϣ ��Ϊfirst����ΪHTTP/1.1, second�Ǵ�����. 200�ǳɹ�.
            //body���ݴ洢����ԭʼ��HTTP��body����, ֧��chunked, ����Ҫ�Լ��������ѡ���Ƿ����urlencode�������б����.
            inline void registerOnSessionHTTPMessage(const OnHTTPMessageFunction & fun) { _vctSessionHTTPMessage.push_back(fun); }

        public:
            bool dispatchPreSessionMessage(TcpSessionPtr  session, const char * blockBegin, zsummer::proto4z::Integer blockSize);
            void dispatchSessionMessage(TcpSessionPtr  session, ProtoID pID, zsummer::proto4z::ReadStream & msg);
            void dispatchOnSessionEstablished(TcpSessionPtr  session);
            void dispatchOnSessionDisconnect(TcpSessionPtr  session);
            void dispatchOnSessionPulse(TcpSessionPtr  session, unsigned int pulseInterval);
            bool dispatchSessionHTTPMessage(TcpSessionPtr session, const zsummer::proto4z::PairString & commonLine, const zsummer::proto4z::HTTPHeadMap &head, const std::string & body);
        private:
            //!message
            MapProtoDispatch _mapSessionDispatch;
            std::vector<OnPreMessageFunction> _vctPreSessionDispatch;
            std::vector<OnDefaultMessageFunction> _vctDefaultSessionDispatch;

            //http
            std::vector<OnHTTPMessageFunction> _vctSessionHTTPMessage;

            //event
            std::vector<OnSessionEstablished> _vctOnSessionEstablished;
            std::vector<OnSessionDisconnect> _vctOnSessionDisconnect;

            std::vector<OnSessionPulseTimer> _vctOnSessionPulse;
        };


        inline void dispatchSessionMessage(TcpSessionPtr  & session, const char * blockBegin, int blockSize)
        {
            bool preCheck = MessageDispatcher::getRef().dispatchPreSessionMessage(session, blockBegin, blockSize);
            if (!preCheck)
            {
                LCW("Dispatch Message failed. ");
            }
            else
            {
                ReadStream rs(blockBegin, blockSize);
                ProtoID protoID = rs.getProtoID();
                MessageDispatcher::getRef().dispatchSessionMessage(session, protoID, rs);
            }
        }

        inline bool  dispatchHTTPMessage(TcpSessionPtr session, const zsummer::proto4z::PairString & commonLine, const zsummer::proto4z::HTTPHeadMap &head, const std::string & body)
        {
            MessageDispatcher::getRef().dispatchSessionHTTPMessage(session, commonLine, head, body);
        }




    }
}

#endif


