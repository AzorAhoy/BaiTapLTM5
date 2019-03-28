// ChatServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "winsock2.h"


int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR*)&addr, sizeof(addr));
	listen(listener, 5);

	SOCKET sockets[64];
	WSAEVENT events[64];
	int numEvents = 0;

	//SOCKET clients[64];
	//int numClients = 0;

	SOCKET registeredClients[64];
	int registered = 0;
	char * ids[64];

	//int ret;

	char buf[256];

	char id[64];
	char cmd[64];
	char tmp[64];

	char errorMsg[] = "Syntax Error. Please try again.\n";

	char sendBuf[256];
	char targetID[64];

	WSAEVENT newEvent = WSACreateEvent();
	WSAEventSelect(listener, newEvent, FD_ACCEPT);

	sockets[numEvents] = listener;
	events[numEvents] = newEvent;
	numEvents++;

	int ret;
	//char buf[256];

	while (true)
	{
		ret = WSAWaitForMultipleEvents(numEvents, events, FALSE, 5000, FALSE);
		if (ret == WSA_WAIT_FAILED)
			break;
		if (ret == WSA_WAIT_TIMEOUT)
		{
			printf("Time out..\n");
			continue;
		}

		int index = ret - WSA_WAIT_EVENT_0;
		for (int i = index; i < numEvents; i++)
		{
			ret = WSAWaitForMultipleEvents(1, &events[i], TRUE, 0, FALSE);
			if (ret == WSA_WAIT_FAILED)
				continue;
			if (ret == WSA_WAIT_TIMEOUT)
				continue;

			WSANETWORKEVENTS nwEvents;
			WSAEnumNetworkEvents(sockets[i], events[i], &nwEvents);
			WSAResetEvent(events[i]);

			if (nwEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (nwEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				{
					printf("FD_ACCEPT failed\n");
					continue;
				}

				if (numEvents == WSA_MAXIMUM_WAIT_EVENTS)
				{
					printf("Too many connection\n");
					continue;
				}

				SOCKET client = accept(sockets[i], NULL, NULL);

				newEvent = WSACreateEvent();
				WSAEventSelect(client, newEvent, FD_READ | FD_CLOSE);

				events[numEvents] = newEvent;
				sockets[numEvents] = client;
				numEvents++;

				printf("New client connected %d\n", client);
			}

			if (nwEvents.lNetworkEvents & FD_READ)
			{
				if (nwEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					printf("FD_READ failed\n");
					continue;
				}

				ret = recv(sockets[i], buf, sizeof(buf), 0);
				if (ret <= 0)
				{
					printf("FD_READ failed\n");
					continue;
				}

				buf[ret] = 0;
				printf("Received from %d: %s\n", sockets[i], buf);

				// Kiem tra trang thai cua client
				// Va xu ly du lieu theo trang thai tuong ung

				int j = 0;
				for (; j < registered; j++) {
					if (sockets[i] == registeredClients[j])
						break;
				}
				if (j == registered)
				{
					// Trang thai chua dang nhap
					// Kiem tra cu phap client_id: [id]
					ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
					if (ret == 2)
					{
						if (strcmp(cmd, "client_id:") == 0)
						{
							char okMsg[] = "Dung cu phap. Hay nhap thong diep muon gui.\n";
							send(sockets[i], okMsg, strlen(okMsg), 0);

							// Luu client dang nhap thanh cong vao mang
							registeredClients[registered] = sockets[i];
							ids[registered] = (char *)malloc(strlen(id) + 1);
							memcpy(ids[registered], id, strlen(id) + 1);
							++registered;
						}
						else
							send(sockets[i], errorMsg, strlen(errorMsg), 0);
					}
					else
						send(sockets[i], errorMsg, strlen(errorMsg), 0);
				}
				else
				{
					// Trang thai da dang nhap
					ret = sscanf(buf, "%s", targetID);
					if (ret == 1)
					{
						if (strcmp(targetID, "all") == 0)
						{
							sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetID) + 1);

							for (int j = 0; j < registered; j++)
								if (registeredClients[j] != sockets[i])
									send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
						}
						else
						{
							sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetID) + 1);

							for (int j = 0; j < registered; j++)
								if (strcmp(ids[j], targetID) == 0)
									send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
						}
					}
				}
				buf[ret] = 0;
				//printf("Received: %s\n", buf);
			}

			if (nwEvents.lNetworkEvents & FD_CLOSE)
			{
				if (nwEvents.iErrorCode[FD_CLOSE_BIT] != 0)
				{
					printf("FD_CLOSE failed\n");
					continue;
				}

				closesocket(sockets[i]);
				//Xoa socket va event khoi mang
				printf("Client disconnected");
			}
		}
	}
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
