#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char *message);
int calculation(int opndCnt, int data[], char op);
#define MAX_PACKET_SIZE  120

int main(void)
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSize;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	hServSock = socket(PF_INET, SOCK_DGRAM, 0); // UDP ��� ����...

	if(hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(9000);

	if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	// Ŭ���̾�Ʈ���� ������ �غ� �Ϸ�...

	int flag;
	
	clntAdrSize = sizeof(clntAdr);

	flag = 1;
	int rcvSum, rcvTotal, ret, result;
	char opndCnt, msg[MAX_PACKET_SIZE];

	while(flag){
		printf("Server> ��� ��û ��� ��.\n");
		// >. recv/send�� ���� ��� ó��...
		// 1. client������ ��ü �޽����� ����
		//    - recv( 1 ����Ʈ ) opndCnt
		//    - recv(opndCnt*sizeof(int)+1) �ݺ� ������ ��� msg ���ű���
		//    - ret = recv();  ret <= 0 ���� flag = 0

		ret = recvfrom(hServSock, msg, MAX_PACKET_SIZE, 0,
			(SOCKADDR*)&clntAdr, &clntAdrSize);
		printf("Server> Client(IP:%s, Port:%d) ��� ��û msg ����\n",
			inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port) );

		
		// 2. ��� ����
		//    - result = calculation((int)opndCnt, (int*)msg, char op)
		opndCnt = msg[0];
		result = calculation(   (int)opndCnt, (int*)&msg[1],
								msg[opndCnt*sizeof(int)+1]);
		printf("Server> ��� ��� = %d.\n", result);

		// 3. send(result) to client ����
		sendto( hServSock, (char*)&result, sizeof(result), 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr) );
		printf("Server> ��� ��� client�� ����.\n");
	}	

	closesocket(hServSock);
	WSACleanup();
	return 0;
}

int calculation(int opndCnt, int data[], char op)
{
	int result, i;
	result = data[0];
	switch (op) {
	case '+':
		for (i = 1; i < opndCnt; i++) {
			result = result + data[i];
		}
		break;
	case '-':
		for (i = 1; i < opndCnt; i++) {
			result = result - data[i];
		}
		break;
	case '*':
		for (i = 1; i < opndCnt; i++) {
			result = result * data[i];
		}
		break;
	}
	return result;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
