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
       
        //!register message 
        typedef std::function < void(TcpSessionPtr&, zsummer::proto4z::ReadStream &) > OnMessageFunction;


        using ProtoID = zsummer::proto4z::ProtoInteger;
        const ProtoID InvalidProtoID = -1;

        class MessageDispatcher
        {
        private:
            MessageDispatcher(){}
            typedef std::unordered_map<unsigned short, std::vector<OnMessageFunction> > MapProtoDispatch;
        public:
            static MessageDispatcher & getRef();
            inline static MessageDispatcher * getPtr(){ return &getRef(); }
            ~MessageDispatcher(){};
            //ע����Ϣ����handler, ����������ϢID����ע��, �ص������ǰ����Ѿ�����õ���Э��ID�Լ�attach�õ�ReadStream. ����ֱ��ʹ��ReadStream������Ӧ��Э������.
            inline void registerSessionMessage(ProtoID protoID, const OnMessageFunction & msgfun){ _mapSessionDispatch[protoID].push_back(msgfun); }
            void dispatchSessionMessage(TcpSessionPtr  &session, ProtoID pID, zsummer::proto4z::ReadStream & msg);
        private:
            //!message
            MapProtoDispatch _mapSessionDispatch;
        };
    }
}

#endif


