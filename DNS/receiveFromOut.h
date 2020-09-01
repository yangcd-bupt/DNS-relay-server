#pragma once
#include "head.h"

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
					if (debugLevel >= 2) {
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