#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

int calculation(int opndCnt, int data[], char op);
void ErrorHandling(char *message);
#define MAX_PACKET_SIZE  120

int main(void)
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSize;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	hServSock = socket(PF_INET, SOCK_STREAM, 0);  

	if(hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(9000);

	if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	// 클라이언트에게 서비스할 준비가 완료...
	listen(hServSock, 3);
	int flag;
	while (1) {
		printf("Server> 클라이언트 연결 요청 대기중.\n");

		clntAdrSize = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		if (hClntSock == -1) {
			printf("<ERROR> accept 실행 오류.\n");
		}
		else {
			printf("Server> client(IP:%s, Port:%d) 연결됨.\n",
				inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
		}

		flag = 1;
		int rcvSum, rcvTotal, ret, result;
		char opndCnt, msg[MAX_PACKET_SIZE];
		while (flag) {
			printf("Server> 계산 요청 대기 중.\n");
			// >. recv/send를 통해 계산 처리...
			// 1. client가 보낸 전체 메세지를 수신
			//    - recv( 1 바이트 ) opndCnt
			//    - recv(opndCnt*sizeof(int)+1) 반복 나머지 모든 msg 수신까지
			//    - ret = recv(); ret <= 0 종료 flag = 0
			recv(hClntSock, &opndCnt, sizeof(opndCnt), 0);
			printf("Server> 피연산자 수 = %d.\n", opndCnt);

			rcvSum = 0; // 수신 누적치.
			rcvTotal = opndCnt * sizeof(int) + 1; // 수신 목표치.
			while (rcvSum < rcvTotal) {
				ret = recv(hClntSock, &msg[rcvSum], rcvTotal-rcvSum, 0);
				if (ret <= 0) {
					printf("<ERROR> recv() 오류.\n");
					flag = 0;
					break;
				}
				else {
					rcvSum = rcvSum + ret;
					printf("Server> recv %d bytes. sum=%d, total=%d\n", ret, rcvSum, rcvTotal);
				}				
			}
			if (flag == 1) {
				// 2. 계산 수행
				//    - result = calculation((int)opndCnt, (int*)msg, char op)
				result = calculation((int)opndCnt, (int*)msg, msg[rcvTotal-1]);
				printf("Server> 계산 결과 = %d.\n", result);

				// 3. send(result) to client 전달
				send(hClntSock, (char*)&result, sizeof(result), 0);
				printf("Server> 계산 결과 client로 전달.\n");
			}
		}




		printf("Server> close socket with client.\n");
		closesocket(hClntSock);
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
