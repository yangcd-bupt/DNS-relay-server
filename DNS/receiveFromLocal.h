#pragma once
#include "head.h"

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
					unsigned short flag = htons(0x8183);
					memcpy(&sendbuf[2], &flag, sizeof(unsigned short)); //设置首部标志域
					flag = htons(0x0001);	//将回答个数设置为1
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
						cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << ",  " << "TYPE  A" << ",  " << "CLASS  1" << endl;
						Number++;


						IPV4 = true;
					}

				}
				char outBuffer[2];
				if (debugLevel >= 2) {
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
					cout << "  " << "Client" << "  " << "127.0.0.1" << "     " << requestURL << ",  " << "TYPE  A" << ",  " << "CLASS  1" << endl;
					Number++;
					IPV4 = true;
				}

			}

			char outBuffer[2];
			if (debugLevel >= 2) {
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