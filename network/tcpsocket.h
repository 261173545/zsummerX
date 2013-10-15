/*
 * ZSUMMER_11X License
 * -----------
 * 
 * ZSUMMER_11X is licensed under the terms of the MIT license reproduced below.
 * This means that ZSUMMER_11X is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2013 YaweiZhang <yawei_zhang@foxmail.com>.
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





#ifndef _ZSUMMER_11X_TCPSOCKET_H_
#define _ZSUMMER_11X_TCPSOCKET_H_
#ifdef WIN32
#include "iocp/tcpsocket_impl.h"
#else
#include "epoll/tcpsocket_impl.h"
#endif
namespace zsummer
{
	namespace network
	{
		class CTcpSocket
		{
		public:
#ifdef WIN32
			CTcpSocket(std::string remoteIP, unsigned short remotePort):m_impl(INVALID_SOCKET, remoteIP, remotePort){}
			CTcpSocket(SOCKET s, std::string remoteIP, unsigned short remotePort):m_impl(s, remoteIP, remotePort){}
#else
			CTcpSocket(std::string remoteIP, unsigned short remotePort):m_impl(-1, remoteIP, remotePort){}
			CTcpSocket(int fd, std::string remoteIP, unsigned short remotePort):m_impl(fd, remoteIP, remotePort){}
#endif // WIN32

			
			~CTcpSocket(){}
			inline bool Initialize(CZSummer & summer)
			{
				return m_impl.Initialize(summer);
			}
			template<typename H>
			inline bool DoConnect(const H &h)
			{
				return m_impl.DoConnect(h);
			}
			template<typename H>
			inline bool DoSend(char * buf, unsigned int len, const H &h)
			{
				return m_impl.DoSend(buf, len, h);
			}
			template<typename H>
			inline bool DoRecv(char * buf, unsigned int len, const H &h)
			{
				return m_impl.DoRecv(buf, len, h);
			}
			inline bool GetPeerInfo(std::string& remoteIP, unsigned short &remotePort)
			{
				return m_impl.GetPeerInfo(remoteIP, remotePort);
			}
			inline bool DoClose()
			{
				return m_impl.DoClose();
			}
			friend class CTcpAcceptImpl;
		private:
			CTcpSocketImpl m_impl;
		};
	};
	
};























#endif











