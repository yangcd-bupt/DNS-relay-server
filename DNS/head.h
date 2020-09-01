#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include<cstdio>
#include<cstring>
#include <time.h>
#include<iostream>
#include <WinSock2.h>

#define MAX_IP_LENGTH 16
#define MAX_URL_LENGTH 65
#define MAX_FILE_NAME_LENGTH 100
#define MAX_IP_URL_REFLECT_TABLE_LENGTH 1000
#define MAX_ID_TARNSFER_TABLE_LENGTH 100
#define MAX_CACHE_LENGTH 10
#define MAX_BUF_SIZE 513
using namespace std;

struct sockaddr_in localName, externName;//AF_INET地址
struct sockaddr_in client, out;

//序号数
int Number=0;
bool IPV4 = true;

int lengthClient = sizeof(client);
SOCKET localSocket, outSocket; //本地套接字和外部套接字
WSADATA WsaData;  //存储Windows套接字初始化信息

//debug等级
int debugLevel = 0;

//dnsrelay文件路径
char filePath[MAX_FILE_NAME_LENGTH] = "C:\\Users\\starry_sky\\Desktop\\计算机网络课程设计\\DNS\\Release\\dnsrelay.txt";

//外部DNS服务器IP地址
char outDNSServerIP[MAX_IP_LENGTH] = "114.114.114.114";

//本地IP、URL对照表
int TotalTableNumber = 0;
int CurrentTableNumber = 0;
char IPTable[MAX_IP_URL_REFLECT_TABLE_LENGTH][MAX_IP_LENGTH];
char URLTable[MAX_IP_URL_REFLECT_TABLE_LENGTH][MAX_URL_LENGTH];

//ID转换表
int CurrentIDNumber = 0;
unsigned short oldIDtable[MAX_ID_TARNSFER_TABLE_LENGTH];
bool isDone[MAX_ID_TARNSFER_TABLE_LENGTH];
struct sockaddr_in IDClient[MAX_ID_TARNSFER_TABLE_LENGTH];

//cache用来存储外部查询的IP、URL
int CacheCount = 0;
int TotalCacheNumber = 0;
int CurrentCacheNumber = 0;
char URLCache[MAX_CACHE_LENGTH][MAX_URL_LENGTH];
char IPCache[MAX_CACHE_LENGTH][MAX_IP_LENGTH];

void convertToURL(char* url, char* requestURL);
void outPutCurrentTime();
