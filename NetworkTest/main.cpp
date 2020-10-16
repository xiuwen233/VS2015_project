#include<winsock2.h>
#include<iostream>
#include<string>
#include <windows.h>  


#pragma comment(lib,"ws2_32.lib")


using  namespace std;

char _t_HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}


int Str2ToHex(string str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.length();
	//data.SetSize(len/2);
	for (int i = 0; i<len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str[i];
		t = _t_HexChar(h);
		t1 = _t_HexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		if (t == 0)
		{
			printf("c = %c  int = %d \n", t, t);
		}
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;
}

string Hex_2_str( char *p, int len) {
	char *buf = (char *)malloc(2 * len);
	int i, j = 0;
	for (i = 0; i < len; i++) {
		sprintf(buf + 2 * i, "%02x", p[i]);
	}
	string str = buf;
	free(buf);
	return str;
}



DWORD WINAPI cpu1(LPVOID  lppargram) {
	SOCKET socket = (SOCKET)lppargram;
	cout << "u1 socket2 is ready to start" << endl;
	string data = "04";
	
	char data1[512] = { 0 };
	memset(data1,  0, sizeof(data1) );
	//const char * sendData;
	//sendData = data.c_str();
	int lengh = Str2ToHex(data, data1);

	int ret = send(socket, data1, strlen(data1), 0);
	if (ret == SOCKET_ERROR) {
		cout << "u1 send message error" << endl;
		return 0;
	}

	char msg[100];
	int num = 0;
	while (true)
	{
		cout << "u1 ready to receive datat" << endl;
		num = recv(socket, msg, 100, 0);
		if (num > 0)
		{
			
			cout << "u1 functions  " << Hex_2_str(msg, num) << endl;
			

		}
		else if (num == 0)
		{
			cout << "u1 ready to out" << endl;
		}
		else
		{
			cout << "u1 it is error" << endl;
		}
	}
	
}


DWORD WINAPI cpu2(LPVOID  lppargram) {
	SOCKET socket = (SOCKET)lppargram;
	cout << "u2 socket2 is ready to start" << endl;
	string data = "0203030303";

	char data1[512] = { 0 };
	memset(data1, 0, sizeof(data1));
	//const char * sendData;
	//sendData = data.c_str();
	int lengh = Str2ToHex(data, data1);

	int ret = send(socket, data1, strlen(data1), 0);
	if (ret == SOCKET_ERROR) {
		cout << "u2 send message error" << endl;
		return 0;
	}

	char msg[100];
	int num = 0;
	while (true)
	{
		cout << "u2 ready to receive datat" << endl;
		num = recv(socket, msg, 100, 0);
		if (num > 0)
		{

			cout << "u2 functions  " << Hex_2_str(msg, num) << endl;


		}
		else if (num == 0)
		{
			cout << "u2 ready to out" << endl;
		}
		else
		{
			cout << "u2 it is error" << endl;
		}
	}

}


DWORD WINAPI cpu3(LPVOID  lppargram) {
	SOCKET socket = (SOCKET)lppargram;
	cout << "u3 socket2 is ready to start" << endl;
	string data = "0203030303";

	char data1[512] = { 0 };
	memset(data1, 0, sizeof(data1));
	//const char * sendData;
	//sendData = data.c_str();
	int lengh = Str2ToHex(data, data1);

	int ret = send(socket, data1, strlen(data1), 0);
	if (ret == SOCKET_ERROR) {
		cout << "u3 send message error" << endl;
		return 0;
	}

	char msg[100];
	int num = 0;
	while (true)
	{
		cout << "u3 ready to receive datat" << endl;
		num = recv(socket, msg, 100, 0);
		if (num > 0)
		{

			cout << "u3 functions  " << Hex_2_str(msg, num) << endl;


		}
		else if (num == 0)
		{
			cout << "u3 ready to out" << endl;
		}
		else
		{
			cout << "u3 it is error" << endl;
		}
	}

}


int manger_socket()
{

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsdata;

	if (WSAStartup(sockVersion, &wsdata) != 0)
	{
		cout << "init failed" << endl;
		return -1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "Socket error" << endl;
		return 1;
	}


	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(666);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;   //inet_addr("127.0.0.1")  INADDR_ANY
	

	if (bind(serverSocket, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
		cout << "Bind error" << endl;
		return 1;
	}

	//开始监听
	if (listen(serverSocket, 10) == SOCKET_ERROR) {
		cout << "Listen error" << endl;
		return 1;
	}


	SOCKET clientSocket;
	sockaddr_in client_sin;
	char msg[100];//存储传送的消息
	int flag = 0;//是否已经连接上

	char sendBuf[20] = { '\0' };
	int len = sizeof(client_sin);
	int num = 0;
	while (true) {
		
		cout << "等待连接..." << endl;
		clientSocket = accept(serverSocket, (sockaddr*)&client_sin, &len);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Accept error" << endl;
			return 1;
		}
	   
		num++;
		//inet_ntop(AF_INET, (void*)&client_sin.sin_addr, sendBuf, 16);
		cout << "接收到一个链接：" << inet_ntoa(client_sin.sin_addr) << endl;  //inet_ntoa  net_ntoa(client_sin.sin_addr)
		
		
		if (strcmp(inet_ntoa(client_sin.sin_addr), "192.168.201.1") == 0)
		{
			CreateThread(NULL, 0, cpu1, (LPVOID)clientSocket, 0, NULL);
		}
		else if (strcmp(inet_ntoa(client_sin.sin_addr), "192.168.201.2") == 0)
		{
			CreateThread(NULL, 0, cpu2, (LPVOID)clientSocket, 0, NULL);
		}
		else if(strcmp(inet_ntoa(client_sin.sin_addr), "192.168.201.3") == 0)
		{
			CreateThread(NULL, 0, cpu3, (LPVOID)clientSocket, 0, NULL);
		}
		cout << "下一个数据" << endl;
		/*int num = recv(clientSocket, msg, 100, 0);
		if (num > 0)
		{
			msg[num] = '\0';
			cout << "Client say: " << msg << endl;

		}
		else if(num == 0)
		{
			cout << "no data" << endl;
		}
		else
		{
			cout << "it is error" << endl;
		}*/

	/*	string data;
		getline(cin, data);
		const char * sendData;
		sendData = data.c_str();
		send(clientSocket, sendData, strlen(sendData), 0);*/
		//closesocket(clientSocket);

		if (num > 3)
		{
			break;
		}
	}

	closesocket(serverSocket);
	//WSACleanup();
	


	return 0;

}


int main()
{
	//manger_socket();
	string data1 = "0200100010";
	string data2 = "0201100110";
	char msg[512] = { 0 };
	memset(msg,  0 , sizeof(msg));
	int ret = Str2ToHex(data1, msg);
	printf(" length is  %d  \n", ret);
	for (int i = 0; i < ret; i++)
	{
		printf("%02x %d \t", msg[i], msg[i]);
	}
	printf("\n");


	memset(msg, 0, sizeof(msg));
	ret = Str2ToHex(data2, msg);
	printf(" length is  %d  \n", ret);
	for (int i = 0; i < ret; i++)
	{
		printf("%02x %d \t", msg[i], msg[i]);
	}
	printf("\n");

	int num9 = 0;
	char ch = num9;
	char chch = (char)num9;
	printf("num9 char = %d  (char)= %d \n", ch, chch);


	num9 = 20;
    ch = num9;
    chch = (char)num9;
	printf("num9 char = %d  (char)= %d", ch, chch);

	getchar();
	return 0;
}
