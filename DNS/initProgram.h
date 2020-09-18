#pragma once
#include "head.h"

void listTitle() {
	printf("Class:2018211319\n");//小组信息
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
void initTransferTable() {
	int i = 0;
	for (; i < MAX_ID_TARNSFER_TABLE_LENGTH; i++)
		isDone[i] = true;
}