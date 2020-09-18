#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "head.h"
#include "initProgram.h"
#include "receiveFromLocal.h"
#include "receiveFromOut.h"

#pragma comment(lib, "Ws2_32.lib")


void outPutCurrentTime()
{
	time_t t;
	struct tm* lt;   
	time(&t);//获取时间戳。    
	lt = localtime(&t);//转为时间结构。    
	printf("%d/%d/%d %d:%d:%d", lt->tm_year + 1900, lt->tm_mon+1, lt->tm_mday,lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果
}

//将形为6github3com0的url转为github.com形式的requestURL
void convertToURL(char* url, char* requestURL)
{
	int i = 0, j = 0, k = 0, len = strlen(url);
	while (i < len)
	{
		if (url[i] > 0 && url[i] <= 63) //如果url长度在(0,64)之间
		{
			for (j = url[i], i++; j > 0; j--, i++, k++) //复制该url
				requestURL[k] = url[i];
		}
		if (url[i] != 0) //检测是否结束，即结尾是否为0
		{
			requestURL[k] = '.';
			k++;
		}
	}
	requestURL[k] = '\0'; 
}


int main(int argc, char* argv[]) {
	listTitle();
	//读取命令行参数
	readParameters(argc, argv);
	//读取IP-域名对照表
	readIPURLReflectTable();

	//初始化Win Socket服务
	WSAStartup(MAKEWORD(2, 2), &WsaData);
	
	//创建本地和外部的Socket
	localSocket = socket(AF_INET, SOCK_DGRAM, 0);
	outSocket = socket(AF_INET, SOCK_DGRAM, 0);

	
	//将Socket接口改为无阻塞模式
	int nonBlock = 1;
	ioctlsocket(outSocket, FIONBIO, (u_long FAR*) & nonBlock);
	ioctlsocket(localSocket, FIONBIO, (u_long FAR*) & nonBlock);
	
	localName.sin_family = AF_INET;//Address family AF_INET代表TCP / IP协议族
	localName.sin_addr.s_addr = INADDR_ANY;    //设置本地地址为任意IP地址
	localName.sin_port = htons(53); //设置DNS接口为53

	externName.sin_family = AF_INET; //Address family AF_INET代表TCP / IP协议族
	externName.sin_addr.s_addr = inet_addr(outDNSServerIP);   //设置外部DNS服务器IP地址
	externName.sin_port = htons(53);  //设置DNS接口为53

	//设置套接字的选项,避免出现本地端口被占用情况
	int reuse = 1;
	setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)& reuse, sizeof(reuse));

	//绑定该套接字到53端口
	if (bind(localSocket, (struct sockaddr*) & localName, sizeof(localName)) < 0)
	{
		if (debugLevel >= 1)
			printf("Bind socket port failed.\n");
		exit(1);
	}
	//初始化ID对照表
	initTransferTable();

	for (;;)
	{
		receiveFromLocal();
		receiveFromOut();
	}
}