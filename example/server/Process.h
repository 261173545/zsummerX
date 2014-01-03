/*
 * ZSUMMER License
 * -----------
 * 
 * ZSUMMER is licensed under the terms of the MIT license reproduced below.
 * This means that ZSUMMER is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2010-2013 YaweiZhang <yawei_zhang@foxmail.com>.
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


//! zsummer�Ĳ��Է���ģ��(��Ӧzsummer�ײ������װ���ϲ���Ʋ��Է���) ����Ϊ����˼ܹ��е� gateway����/agent����/ǰ�˷���, �ص��Ǹ߲�����������
//! IOServerͷ�ļ� ���ฺ����Schedule�׹�����client socket. ÿ��IOServer����һ���߳�.

#ifndef ZSUMMER_PROCESS_H_
#define ZSUMMER_PROCESS_H_

#include "header.h"

class CProcess
{
public:
	CProcess(bool bInitiative, unsigned int interval, std::string ip, unsigned short port);
	//! ������ֹͣ
	bool Start();
	void Stop();
	//! �߳�
	void Run();
	//!
	inline zsummer::network::CZSummer & GetZSummer(){return m_summer;}
private:
	zsummer::network::CZSummer m_summer;
	std::thread	 m_thread;
	bool  m_bRunning;
	bool  m_bInitiative; //! �Ƿ���������echo��
	unsigned int  m_nInterval; //! ���ͼ��
	std::string m_ip;
	unsigned short m_port;

public:
	template<class H>
	void Post(const H &h)
	{
		m_summer.Post(h);
	}
	//����һ��socket
	void RecvSocketPtr(std::shared_ptr<zsummer::network::CTcpSocket> sockptr);

	//all use
public:
	inline unsigned long long GetTotalRecvLen() {return m_nTotalRecvLen;}
	inline void AddTotalRecvLen(unsigned long long len) { m_nTotalRecvLen += len;}

	inline unsigned long long GetTotalSendLen() {return m_nTotalSendLen;}
	inline void AddTotalSendLen(unsigned long long len) { m_nTotalSendLen += len;}

	inline unsigned long long GetTotalRecvCount() {return m_nTotalRecvCount;}
	inline void AddTotalRecvCount(unsigned long long len) { m_nTotalRecvCount += len;}

	inline unsigned long long GetTotalSendCount() {return m_nTotalSendCount;}
	inline void AddTotalSendCount(unsigned long long len) { m_nTotalSendCount += len;}
		
	inline unsigned long long GetTotalOpen() {return m_nTotalOpen;}
	inline void AddTotalOpen(unsigned long long len) { m_nTotalOpen += len;}
	inline unsigned long long GetTotalClosed() {return m_nTotalClosed;}
	inline void AddTotalClosed(unsigned long long len) { m_nTotalClosed += len;}
private:
	unsigned long long  m_nTotalRecvLen;
	unsigned long long  m_nTotalSendLen;
	unsigned long long  m_nTotalRecvCount;
	unsigned long long  m_nTotalSendCount;
	unsigned long long  m_nTotalOpen;
	unsigned long long  m_nTotalClosed;
	


	//only connects use
public:
	inline unsigned long long GetTotalEcho() {return m_nTotalEcho;}
	inline void AddTotalEcho(unsigned long long len) { m_nTotalEcho += len;}

	inline unsigned long long GetTotalEchoTime() {return m_nTotalEchoTime;}
	inline void AddTotalEchoTime(unsigned long long len) { m_nTotalEchoTime += len;}
private:
	unsigned long long m_nTotalEcho;
	unsigned long long m_nTotalEchoTime;
	




};

#endif