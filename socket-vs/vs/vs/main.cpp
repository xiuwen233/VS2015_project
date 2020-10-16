#include<stdio.h>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")


/*WORD sockVersion = MAKEWORD(2, 2);
WSADATA data;
int ret = 0;
if (WSAStartup(sockVersion, &data) != 0)
{
	return;
}
sockaddr_in serAddr;
serAddr.sin_family = AF_INET;
serAddr.sin_port = htons(atoi(socketinfo.host_port));
inet_pton(AF_INET, socketinfo.host_ip, &serAddr.sin_addr);

SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (sclient == INVALID_SOCKET)
{

	return;
}
if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
{  //连接失败 
	closesocket(sclient);
	return;
}
host_ip.socket_cpu = sclient;

char senddata[256] = { 0 };
memset(senddata, 0, sizeof(senddata));

ret = send(sclient, senddata, lengh, 0);
if (ret == SOCKET_ERROR)
{

	return;
}

char recData[255];

while (true)
{
	ret = recv(sclient, recData, 255, 0);
	....................
}*/



int main()
{
	WSACleanup();
	
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsdata;
	SOCKET serverSocket;
	
	if (WSAStartup(sockVersion, &wsdata) != 0)
	{
		//待处理
		printf("WSAStartup failed \n");
		return  -1;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		//待处理
		printf("serverSocket new failed \n");
		return -1;
	}
	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(666);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;   //inet_addr("127.0.0.1")  INADDR_ANY  inet_addr("192.168.1.0");

	if (::bind(serverSocket, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
	{
		//待处理
		printf("bind error \n");
		return -1;
	}

	//开始监听
	if (listen(serverSocket, 10) == SOCKET_ERROR)
	{
		//待处理
		printf("listen error ");
		return -1;
	}

	SOCKET clientSocket;
	sockaddr_in client_sin;
	int len = sizeof(client_sin);
	int socketnum = 0;
	int num_acceprt = 0;

	//printf("ready to accept\n");
	
	clientSocket = accept(serverSocket, (sockaddr*)&client_sin, &len);
	printf("connect ed\n");

	int recv_len = 0;
	int send_len = 0;
	char recv_buf[512];
	char send_buf[512];
	while (1) {
		memset(recv_buf,0,sizeof(recv_buf));
		recv_len = recv(clientSocket, recv_buf, 512, 0);
		if (recv_len < 0)
		{
			printf( "接受失败！\n" );
			break;
		}
		else
		{
			printf("客户端信息: %s \n", recv_buf) ;
		}
		printf("请输入回复信息: \n");
		memset(send_buf, 0, sizeof(send_buf));
		scanf("%s",send_buf);
		send_len = send(clientSocket, send_buf, 100, 0);
		if (send_len < 0)
		{
			printf("发送失败！\n" );
			break;
		}
	}

	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();
	return 0;

}
