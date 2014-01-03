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
//! main�ļ�


#include "header.h"
#include "Schedule.h"
#include "Process.h"
using namespace zsummer::log4z;
int g_nTotalLinked = 0;


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
	if (argc < 6 )
	{
		cout <<"please input like example:" << endl;
		cout << "./test ip port type maxClient interval" << endl;
		cout <<"type:1 server" << endl;
		cout <<"type:2 client" << endl;
		cout <<"type:3 client ping-pong" << endl;
		return 0;
	}
	std::string ip = argv[1];
	unsigned short port = atoi(argv[2]);
	unsigned short type = atoi(argv[3]);
	unsigned int maxClient = atoi(argv[4]);
	unsigned int interval = atoi(argv[5]);
	if (type == 1)
	{
		//! ������־����
		ILog4zManager::GetInstance()->Config("server.cfg");
		ILog4zManager::GetInstance()->Start();
	}
	else
	{
				//! ������־����
		ILog4zManager::GetInstance()->Config("client.cfg");
		ILog4zManager::GetInstance()->Start();
	}
	LOGI("ip=" << ip << ", port=" << port << ", type=" << type << ", clients=" << maxClient << ", interval=" << interval);



	//! ����������
	CSchedule schedule;
	schedule.Start(ip, port, type, maxClient, interval);


	//main�߳����ڷ���״̬ͳ�������

	unsigned long long nLast[10] = {0};
	unsigned long long temp[10] = {0};
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		memset(&temp, 0, sizeof(temp));

		for (std::vector<CProcess *>::const_iterator iter = schedule.m_process.begin(); iter != schedule.m_process.end(); ++iter)
		{
			temp[0] += (*iter)->GetTotalRecvLen();
			temp[1] += (*iter)->GetTotalSendLen();
			temp[2] += (*iter)->GetTotalRecvCount();
			temp[3] += (*iter)->GetTotalSendCount();
			temp[4] += (*iter)->GetTotalOpen();
			temp[5] += (*iter)->GetTotalClosed();
			temp[6] += (*iter)->GetTotalRecvLen();
			temp[7] += (*iter)->GetTotalEcho();
			temp[8] += (*iter)->GetTotalEchoTime();
		}
		LOGD("Linked[" << temp[4] 
			<<"]  Closed[" << temp[5]
			<<"]  RecvSpeed[" << (temp[0] - nLast[0])/1024.0/1024.0/5.0
			<<"]  SendSpeed[" << (temp[1] - nLast[1])/1024.0/1024.0/5.0
			<<"]  RecvSpeed[" << (temp[2] - nLast[2])/5.0
			<<"]  SendSpeed[" << (temp[3] - nLast[3])/5.0
			<<"]  EchoSpeed[" << (temp[7]- nLast[7])/5.0
			<<"]  DelayTime[" << (temp[8]-nLast[8])/1.0/(temp[7]-nLast[7] == 0 ? 1 :temp[7]-nLast[7])
			<<"].");


		memcpy(&nLast, &temp, sizeof(temp));
	}

	schedule.Stop();

	//��־������Ҫ��ʽ����ֹͣ�ӿ�
	//InterfaceLogger::GetInstance()->Stop();
	return 0;
}

