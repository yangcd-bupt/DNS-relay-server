#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "head.h"

#pragma comment(lib, "Ws2_32.lib")

void listTitle() {
	printf("Class:2018211319\n");
	printf("TeamMember:Qiao Sen,Wang Mingzi,Yang Chengdong\n");
	printf("\n");

	printf("DNSRELAY, Version 1.00\n");
	printf("Usage: dnsrelay [-d | -dd] [<dns-server>] [<db-file>]\n");
	printf("\n");
}

void readParameters(int argc, char* argv[]) {
	bool setDNSFlag = false;
	bool setFilePath = false;
	if (argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == 'd')
			debugLevel = 1;
		if (argv[1][2] == 'd')
			debugLevel = 2;
	}

	if (argc > 2) {
		setDNSFlag = true;
		strcpy(outDNSServerIP, argv[2]);
	}

	if (argc > 3) {
		setFilePath = true;
		strcpy(filePath, argv[3]);
	}

	if (setDNSFlag)
		printf("Set Out DNS Server IP:%s\n", outDNSServerIP);
	else
		printf("Use Default Out DNS Server IP:%s\n", outDNSServerIP); 

	if (setFilePath)
		printf("Set File Path:%s\n", filePath);
	else
		printf("Use Default File Path:%s\n", filePath);

	printf("Debug level : %d\n", debugLevel);
}
void readIPURLReflectTable() {
	FILE* fp = NULL;
	if ((fp = fopen(filePath, "r+")) == NULL) {
		perror("fail to read!!!");
		exit(1);
	}
	while (!feof(fp))
	{
		fscanf(fp, "%s %s", IPTable[CurrentTableNumber], URLTable[CurrentTableNumber]);
		if (debugLevel >= 2)
			printf("IP:%s->URL:%s\n", IPTable[CurrentTableNumber], URLTable[CurrentTableNumber]);
		CurrentTableNumber++;
		TotalTableNumber++;
	}
	if (debugLevel >= 0)
		printf("读取成功\n");
		if (debugLevel >= 1)
		printf("读取的行数为：%d\n", TotalTableNumber);

	fclose(fp);
}
void outPutCurrentTime()
{
	time_t t;
	struct tm* lt;   
	time(&t);//获取时间戳。    
	lt = localtime(&t);//转为时间结构。    
	printf("%d/%d/%d %d:%d:%d", lt->tm_year + 1900, lt->tm_mon+1, lt->tm_mday,lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果
}
void initTransferTable() {
	int i = 0;
	for (; i < MAX_ID_TARNSFER_TABLE_LENGTH; i++)
		isDone[i] = true;
}
//将形为6github3com0的url转为github.com形式的requestIP
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
void receiveFromOut()
{
	char buffer[MAX_BUF_SIZE], requestURL[MAX_URL_LENGTH];
	//将BUF清零
	for (int i = 0; i < MAX_BUF_SIZE; i++)
		buffer[i] = 0;
	int bufferLength = -1;
	bufferLength = recvfrom(outSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) & out, &lengthClient); //从外部接受DNS数据包
	if (bufferLength > -1)//如果存在DNS数据包
	{
		unsigned short* newID = (unsigned short*)malloc(sizeof(unsigned short));
		memcpy(newID, buffer, sizeof(unsigned short));//获取数据包头部的ID，即转换后的ID
		int index = (*newID) - 1;//将转换后的ID转换为ID转换表的索引，即行数
		free(newID);//释放空间
		memcpy(buffer, &oldIDtable[index], sizeof(unsigned short));//使用旧ID替换转换后的ID
		isDone[index] = TRUE;//完成请求后，将isDone表中对应的项设置为true
		client = IDClient[index];//获取客户端发送者
		bufferLength = sendto(localSocket, buffer, bufferLength, 0, (SOCKADDR*)& client, sizeof(client));
		int QDCount = ntohs(*((unsigned short*)(buffer + 4))), ANCount = ntohs(*((unsigned short*)(buffer + 6)));
		char* bufferLocation = buffer + 12;//跳过DNS包头的指针
		char ip[16];
		int ipPart1, ipPart2, ipPart3, ipPart4;
		for (int i = 0; i < QDCount; i++)
		{
			convertToURL(bufferLocation, requestURL);
			while (*bufferLocation > 0)//读取标识符前的计数跳过这个url
				bufferLocation += (*bufferLocation) + 1;
			bufferLocation += 5; //跳过url后的信息，指向下一个报文
			for (int i = 0; i < ANCount; ++i)
			{
				if ((unsigned char)* bufferLocation == 0xc0) //检测是否为指针
					bufferLocation += 2;
				else 
				{
					while (*bufferLocation > 0)
						bufferLocation += (*bufferLocation) + 1;
					++bufferLocation;//指向url后面的内容
				}
				unsigned short responseType = ntohs(*(unsigned short*)bufferLocation);  //回复类型
				bufferLocation += 2;
				unsigned short responseClass = ntohs(*(unsigned short*)bufferLocation); //回复类
				bufferLocation += 2;
				unsigned short responseHighTTL = ntohs(*(unsigned short*)bufferLocation);//生存时间高位
				bufferLocation += 2;
				unsigned short responseLowTTL = ntohs(*(unsigned short*)bufferLocation); //生存时间低位
				bufferLocation += 2;
				int TTL = (((int)responseHighTTL) << 16) | responseLowTTL;    //组合成生存时间
				int dataLength = ntohs(*(unsigned short*)bufferLocation);   //数据长度
				bufferLocation += 2;
			
				if (responseType == 1) //如果是A类型
				{
					ipPart1 = (unsigned char)* bufferLocation++;
					ipPart2 = (unsigned char)* bufferLocation++;
					ipPart3 = (unsigned char)* bufferLocation++;
					ipPart4 = (unsigned char)* bufferLocation++;

					sprintf(ip, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);//将 ipPart1, ipPart2, ipPart3, ipPart4拼接为IP地址
				}

				//将URL与IP存放到Cache中
				strcpy(URLCache[CacheCount], requestURL);
				strcpy(IPCache[CacheCount], ip);
				CacheCount++;
				//如果cache表已存满，则从cache表的首位开始替换
				if (CacheCount == MAX_CACHE_LENGTH)
					CacheCount = 0;
				TotalCacheNumber++;
				if (TotalCacheNumber > MAX_CACHE_LENGTH)
					TotalCacheNumber = MAX_CACHE_LENGTH;
				if (debugLevel >= 1)
				{
					
						cout << Number << ":  ";
						outPutCurrentTime();
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << "1.0.0.127.in-addr.arpa, TYPE 12 ,CLASS 1" << endl;
						Number++;
						cout << Number << ":  ";
						outPutCurrentTime();
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << endl;
						IPV4 = false;
						Number++;
						char outBuffer[2];
						if (debugLevel >= 2 ) {
							cout << "----------------------------------------" << endl;
							cout << "Type :  " << responseType << "  Class :  " << responseClass << "  TTL :  " << TTL << "  dataLength :  " << dataLength << endl;
							cout << "buffer :  ";
							for (int i = 0; i < bufferLength; i++) {
								_itoa((unsigned short)buffer[i], outBuffer, 16);
								cout << outBuffer[0] << outBuffer[1] << " ";
							}
							cout << endl;
							cout << "----------------------------------------" << endl;
						}
					}
				
				
			}
		}
	}

}
void receiveFromLocal()
{
	char buffer[MAX_BUF_SIZE], url[MAX_URL_LENGTH];
	//将BUF清零
	for (int i = 0; i < MAX_BUF_SIZE; i++)
		buffer[i] = 0;
	int bufferLength = -1;
	char currentIP[MAX_IP_LENGTH];
	bufferLength = recvfrom(localSocket, buffer, sizeof buffer, 0, (struct sockaddr*) & client, &lengthClient);
	//被查询的域名
	char requestURL[MAX_URL_LENGTH];
	
	if (bufferLength > 0) {
		//将buffer中的域名部分存入url中
		memcpy(url, &(buffer[12]), bufferLength);
		convertToURL(url, requestURL);

		//在Cache中查询该域名
		for (CurrentCacheNumber = 0; CurrentCacheNumber < TotalCacheNumber; CurrentCacheNumber++)
		{
			if (strcmp(requestURL, URLCache[CurrentCacheNumber]) == 0)
				break;
		}
		//如果未在Cache中找到该域名
		if (CurrentCacheNumber == TotalCacheNumber)
		{
			//在IP-URL对照表中查找该域名
			for (CurrentTableNumber = 0; CurrentTableNumber < TotalTableNumber; CurrentTableNumber++) {
				if (strcmp(requestURL, URLTable[CurrentTableNumber]) == 0)
					break;
			}
			//未能在IP-URL对照表中查到
			if (CurrentTableNumber == TotalTableNumber)
			{
				//向外部DNS服务器请求查询该域名
				for (CurrentIDNumber = 0; CurrentIDNumber < MAX_ID_TARNSFER_TABLE_LENGTH; CurrentIDNumber++)
					if (isDone[CurrentIDNumber] == true)
						break;
				if (CurrentIDNumber == MAX_ID_TARNSFER_TABLE_LENGTH)
				{
					;//表满
				}
				else {
					unsigned short* oldID = (unsigned short*)malloc(sizeof(unsigned short));
					memcpy(oldID, buffer, sizeof(unsigned short)); 
					//将oldID存入ID转换表
					oldIDtable[CurrentIDNumber] = *oldID;
					//将isDone设为false
					isDone[CurrentIDNumber] = false;
					IDClient[CurrentIDNumber] = client;
					CurrentIDNumber += 1;
					memcpy(buffer, &CurrentIDNumber, sizeof(unsigned short));
					bufferLength = sendto(outSocket, buffer, bufferLength, 0, (struct sockaddr*) & externName, sizeof(externName));//向外部服务器发送查询请求
				}
			}
			else {

				strcpy(currentIP, IPTable[CurrentTableNumber]);
				//查询该域名是否在黑名单中
				char sendbuf[MAX_BUF_SIZE];
				int currenLength = 0;
				if ((strcmp("0.0.0.0", currentIP)) == 0) {
					//如果在黑名单中则返回未能查询到该域名IP地址
					memcpy(sendbuf, buffer, bufferLength); 
					unsigned short flag = htons(0x8180);
					memcpy(&sendbuf[2], &flag, sizeof(unsigned short)); //设置首部标志域
					flag = htons(0x0000);	//将回答个数设置为1
					memcpy(&sendbuf[6], &flag, sizeof(unsigned short));
				}
				else {
					memcpy(sendbuf, buffer, bufferLength); 
					unsigned short flag = htons(0x8180);
					memcpy(&sendbuf[2], &flag, sizeof(unsigned short)); //设置首部标志域
					flag = htons(0x0001);	//将回答个数设置为1
					memcpy(&sendbuf[6], &flag, sizeof(unsigned short));

				}
				char answer[16];
				unsigned short Name = htons(0xc00c);  //设置域名指针
				memcpy(answer, &Name, sizeof(unsigned short));
				currenLength += sizeof(unsigned short);

				unsigned short Type = htons(0x0001);  //类型
				memcpy(answer + currenLength, &Type, sizeof(unsigned short));
				currenLength += sizeof(unsigned short);

				unsigned short Class = htons(0x0001);  //类
				memcpy(answer + currenLength, &Class, sizeof(unsigned short));
				currenLength += sizeof(unsigned short);

				unsigned long TTL = htonl(0x7b); //生存时间
				memcpy(answer + currenLength, &TTL, sizeof(unsigned long));
				currenLength += sizeof(unsigned long);

				unsigned short IPLength = htons(0x0004);  //IP长度
				memcpy(answer + currenLength, &IPLength, sizeof(unsigned short));
				currenLength += sizeof(unsigned short);

				unsigned long IP = (unsigned long)inet_addr(currentIP); //将字符形式IP转换为16进制形式的IP
				memcpy(answer + currenLength, &IP, sizeof(unsigned long));
				currenLength += sizeof(unsigned long);
				currenLength += bufferLength;
				memcpy(sendbuf + bufferLength, answer, sizeof(answer));

				bufferLength = sendto(localSocket, sendbuf, currenLength, 0, (SOCKADDR*)& client, sizeof(client)); //将报文发送回客户端
				if (debugLevel >= 1)
				{
					if (IPV4) {
						cout << Number << ":  ";
						outPutCurrentTime();
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << "1.0.0.127.in-addr.arpa, TYPE 12 ,CLASS 1" << endl;
						Number++;
						cout << Number << ":* ";
						outPutCurrentTime();
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << endl;
						IPV4 = false;
						Number++;
					}
					else {
						cout << Number << ":  ";
						outPutCurrentTime();
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << ",  " << "TYPE  28" << ",  " <<  "CLASS  1" << endl;
						Number++;
			
						
						IPV4 = true;
					}
					
					}
				char outBuffer[2];
				if (debugLevel >= 2 ) {
					cout << "----------------------------------------" << endl;
					cout << "buffer :  ";
					for (int i = 0; i < bufferLength; i++) {
						_itoa((unsigned short)buffer[i], outBuffer, 16);
						cout << outBuffer[0] << outBuffer[1] << " ";
					}
					cout << endl;
					cout << "----------------------------------------" << endl;
				}
				
			}

		}
		else {
			strcpy(currentIP, IPCache[CurrentCacheNumber]);
			char sendbuf[MAX_BUF_SIZE];
			memcpy(sendbuf, buffer, bufferLength); 
			unsigned short flag = htons(0x8180);
			memcpy(&sendbuf[2], &flag, sizeof(unsigned short)); //设置首部标志位
			flag = htons(0x0001);	//设置回答个数
			memcpy(&sendbuf[6], &flag, sizeof(unsigned short));

			int currentLength = 0;
			char answer[16];
			unsigned short Name = htons(0xc00c); //设置首部标志位
			memcpy(answer, &Name, sizeof(unsigned short));
			currentLength += sizeof(unsigned short);

			unsigned short Type = htons(0x0001);  //类型
			memcpy(answer + currentLength, &Type, sizeof(unsigned short));
			currentLength += sizeof(unsigned short);

			unsigned short Class = htons(0x0001);  //类
			memcpy(answer + currentLength, &Class, sizeof(unsigned short));
			currentLength += sizeof(unsigned short);

			unsigned long TTL = htonl(0x7b); //生存时间
			memcpy(answer + currentLength, &TTL, sizeof(unsigned long));
			currentLength += sizeof(unsigned long);

			unsigned short IPLength = htons(0x0004);  //IP长度
			memcpy(answer + currentLength, &IPLength, sizeof(unsigned short));
			currentLength += sizeof(unsigned short);

			unsigned long IP = (unsigned long)inet_addr(currentIP); //将IP从字符类型转换为16进制
			memcpy(answer + currentLength, &IP, sizeof(unsigned long));
			currentLength += sizeof(unsigned long);
			currentLength += bufferLength;
			memcpy(sendbuf + bufferLength, answer, sizeof(answer));

			bufferLength = sendto(localSocket, sendbuf, currentLength, 0, (SOCKADDR*)& client, sizeof(client)); //将DNS报文发送带客户端
			if (debugLevel >= 1)
			{
				if (IPV4) {
					cout << Number << ":  ";
					outPutCurrentTime();
					cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << "1.0.0.127.in-addr.arpa, TYPE 12 ,CLASS 1" << endl;
					Number++;
					cout << Number << ":* ";
					outPutCurrentTime();
					cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << endl;
					IPV4 = false;
					Number++;
				}
				else {
					cout << Number << ":  ";
					outPutCurrentTime();
					cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << ",  " << "TYPE  28"<< ",  " <<  "CLASS  1" << endl;
					Number++;
					IPV4 = true;
				}
			
				}
			
			char outBuffer[2];
			if (debugLevel >= 2 ) {
				cout << "----------------------------------------" << endl;
				cout << "buffer :  ";
				for (int i = 0; i < bufferLength; i++) {
					_itoa((unsigned short)buffer[i], outBuffer, 16);
					cout << outBuffer[0] << outBuffer[1] << " ";
				}
				cout << endl;
				cout << "----------------------------------------" << endl;
			}
			
		}
	}
	
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