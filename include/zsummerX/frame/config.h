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


//! common header 

#ifndef ZSUMMER_CONFIG_H_
#define ZSUMMER_CONFIG_H_

#include <iostream>
#include <queue>
#include <iomanip>
#include <string.h>
#include <signal.h>
#include <unordered_map>
#include <log4z/log4z.h>
#include <proto4z/proto4z.h>

#ifdef WIN32
#include "../iocp/iocp_impl.h"
#include "../iocp/tcpsocket_impl.h"
#include "../iocp/udpsocket_impl.h"
#include "../iocp/tcpaccept_impl.h"
#elif defined(__APPLE__) || defined(__SELECT__)
#include "../select/select_impl.h"
#include "../select/udpsocket_impl.h"
#include "../select/tcpsocket_impl.h"
#include "../select/tcpaccept_impl.h"
#else
#include "../epoll/epoll_impl.h"
#include "../epoll/tcpsocket_impl.h"
#include "../epoll/udpsocket_impl.h"
#include "../epoll/tcpaccept_impl.h"
#endif
#include "dispatch.h"
namespace zsummer
{
    namespace network
    {
        //! define network type
        typedef unsigned int SessionID;
        const SessionID InvalidSeesionID = -1;
        typedef unsigned int AccepterID;
        const AccepterID InvalidAccepterID = -1;
        typedef unsigned short ProtoID;
        const ProtoID InvalidProtoID = -1;


        //! define session id range.
        const unsigned int __MIDDLE_SEGMENT_VALUE = 300 * 1000 * 1000;
        inline bool isSessionID(unsigned int unknowID){ return unknowID < __MIDDLE_SEGMENT_VALUE ? true : false; }
        inline bool isConnectID(unsigned int unknowID){ return unknowID != InvalidSeesionID && !isSessionID(unknowID); }
        inline unsigned int nextSessionID(unsigned int curSessionID){ return (curSessionID + 1) % __MIDDLE_SEGMENT_VALUE; }
        inline unsigned int nextConnectID(unsigned int curSessionID){ return (curSessionID + 1 < __MIDDLE_SEGMENT_VALUE || curSessionID + 1 == InvalidSeesionID) ? __MIDDLE_SEGMENT_VALUE : curSessionID + 1; }

        //----------------------------------------

        //receive buffer length  and send buffer length 
        const unsigned int MAX_BUFF_SIZE = 100 * 1024;
        const unsigned int MAX_SEND_PACK_SIZE = 20 * 1024;

        enum ProtoType
        {
            PT_TCP,
            PT_HTTP,
        };

        enum INTEGRITY_RET_TYPE
        {
            IRT_SUCCESS = 0, //�ɹ�, ���ص�ǰblock��С
            IRT_SHORTAGE = 1, //����, ���ص�ǰblock����Ҫ�Ĵ�С
            IRT_CORRUPTION = 2, //����, ��Ҫ�ر�socket
        };

        //!TCP������Э��Ľ������ �����Զ���, Ĭ��ʹ��proto4z��
        //!len ��ǰ���յ���block��С
        //!boundLen ��ǰ��������С, block��С���ܳ�����������С
        using CheckTcpIntegrity = std::function<std::pair<INTEGRITY_RET_TYPE, unsigned int>
                            (const char * /*begin*/, unsigned int /*len*/, unsigned int /*boundLen*/)>;

        //!dispatch����, Ĭ��ʹ��zsummerX.frame.MessageDispatcher
        using DispatchBlock = std::function<bool(TcpSessionPtr &  /*session*/, const char * /*blockBegin*/, int /*blockSize*/)>;

        //!HTTPЭ��������, Ĭ��ʹ��proto4z��
        //!hadHeader �����һ���հ�����chunked�ĺ���С��. ֧��chunked
        //!isChunked Ӧ���������HTTP�����ǲ���chunked
        //!commonLine ָ����GET, POST . �����GET ���� GET->PARAM����
        //!head HTTPͷ����ֵ��
        //!body ����
        //!usedLen ʹ���˶����ֽ�
        using PairString = zsummer::proto4z::PairString; 
        using HTTPHeadMap = zsummer::proto4z::HTTPHeadMap;
        using CheckHTTPIntegrity = std::function<INTEGRITY_RET_TYPE(const char * /*begin*/, unsigned int /*len*/, unsigned int /*boundLen*/,
            bool /*hadHeader*/, bool & /*isChunked*/, PairString& /*commonLine*/,
            HTTPHeadMap & /*head*/, std::string & /*body*/, unsigned int &/*usedCount*/)>;
        using DispatchHTTPMessage = std::function<bool
            (TcpSessionPtr /*session*/, const zsummer::proto4z::PairString & /*commonLine*/, const zsummer::proto4z::HTTPHeadMap &/*head*/, const std::string & /*body*/)>;
        
        
        struct SessionTraits 
        {
#pragma region CUSTOM
            ProtoType       _protoType = PT_TCP;
            std::string     _rc4TcpEncryption = ""; //empty is not encryption
            bool            _openFlashPolicy = false;
            bool            _setNoDelay = true;
            unsigned int    _pulseInterval = 5000;

            CheckTcpIntegrity _checkTcpIntegrity = zsummer::proto4z::checkBuffIntegrity;
            DispatchBlock _dispatchBlock = dispatchSessionMessage;
            CheckHTTPIntegrity _checkHTTPIntegrity = zsummer::proto4z::checkHTTPBuffIntegrity;
            DispatchHTTPMessage _dispatchHTTP = dispatchHTTPMessage;
#pragma endregion CUSTOM


#pragma region CONNECT
            //! valid traits when session is connect
            unsigned int _reconnectMaxCount = 0; // try reconnect max count
            unsigned int _reconnectInterval = 5000; //million seconds;
            bool         _reconnectCleanAllData = true;//clean all data when reconnect;
#pragma endregion CONNECT

        };

        struct AccepterExtend
        {
            AccepterID _aID = InvalidAccepterID;
            TcpAcceptPtr _accepter;
            std::string        _listenIP;
            unsigned short    _listenPort = 0;
            unsigned int    _maxSessions = 5000;
            std::vector<std::string> _whitelistIP;
            unsigned long long _totalAcceptCount = 0;
            unsigned long long _currentLinked = 0;
            SessionTraits _sessionTraits;
        };


        class Any
        {
        public:
            Any(unsigned long long n)
            {
                _integer = n;
            }
            Any(const std::string & str)
            {
                _isInteger = false;
                _string = str;
            }
            bool _isInteger = true;
            unsigned long long _integer;
            std::string _string;
        };
        













        inline zsummer::log4z::Log4zStream & operator << (zsummer::log4z::Log4zStream &os, const SessionTraits & traits)
        {
            os << "{ " << "_protoType=" << traits._protoType
                << ", _rc4TcpEncryption=" << traits._rc4TcpEncryption
                << ", _openFlashPolicy=" << traits._openFlashPolicy
                << ", _setNoDelay=" << traits._setNoDelay
                << ", _pulseInterval=" << traits._pulseInterval
                << ", _reconnectMaxCount=" << traits._reconnectMaxCount
                << ", _reconnectInterval=" << traits._reconnectInterval
                << ", _reconnectCleanAllData=" << traits._reconnectCleanAllData
                << ", _totalConnectCount=" << traits._totalConnectCount
                << ", _curReconnectCount=" << traits._curReconnectCount
                << "}";
            return os;
        }


        inline zsummer::log4z::Log4zStream & operator << (zsummer::log4z::Log4zStream &os, const AccepterExtend & extend)
        {
            os << "{"
                << "_aID=" << extend._aID
                << ", _listenIP=" << extend._listenIP
                << ", _listenPort=" << extend._listenPort
                << ", _maxSessions=" << extend._maxSessions
                << ",_totalAcceptCount=" << extend._totalAcceptCount
                << ", _currentLinked=" << extend._currentLinked
                << ",_whitelistIP={";

            for (auto str : extend._whitelistIP)
            {
                os << str << ",";
            }
            os << "}";
            os << ", SessionTraits=" << extend._sessionTraits;
            os << "}";
            return os;
        }








    }
}





#endif

