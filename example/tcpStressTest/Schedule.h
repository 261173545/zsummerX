﻿/*
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

//! zsummer的测试服务模块(对应zsummer底层网络封装的上层设计测试服务) 可视为服务端架构中的 gateway服务/agent服务/前端服务, 特点是高并发高吞吐量
//! Schedule头文件 该类负责监听端口, accept client, 并把accept到的socket分发给IOServer池去处理.


#ifndef ZSUMMER_SCHEDULE_H_
#define ZSUMMER_SCHEDULE_H_
#include "header.h"
class CProcess;
class CSchedule
{
public:
	CSchedule();
	//! 启动与停止
	void Start();
	void Stop();
	//! 线程
	void Run();

	void OnAccept(zsummer::network::ErrorCode ec, CTcpSocketPtr sockptr);

	void doConnect(unsigned int maxClient);

	//! IOServer池
	std::vector<CProcess *> m_process;
	int						m_iCurProcess = 0;


	zsummer::network::CZSummerPtr m_summer;
	std::thread	 m_thread;
	zsummer::network::CTcpAcceptPtr m_accept;
	bool	m_bRunning = false;

};


#endif


