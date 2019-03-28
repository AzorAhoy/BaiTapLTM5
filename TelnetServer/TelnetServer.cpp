// TelnetServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
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

	WSAEVENT newEvent = WSACreateEvent();
	WSAEventSelect(listener, newEvent, FD_ACCEPT);

	sockets[numEvents] = listener;
	events[numEvents] = newEvent;
	numEvents++;

	int ret;
	char buf[256];
	char id[64];
	char cmd[64];
	char tmp[64];
	//char buf[256];
	char fileBuf[256];
	char cmdBuf[256];
	char targetID[64];

	SOCKET registeredClients[64];
	int registered = 0;
	char * ids[64];

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
				buf[ret] = 0;
				if (ret <= 0)
				{
					printf("FD_READ failed\n");
					continue;
				}

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
						int found = 0;
						FILE *f = fopen("users.txt", "r");
						while (fgets(fileBuf, sizeof(fileBuf), f))
						{
							printf("%s\n", fileBuf);
							if (strcmp(buf, fileBuf) == 0)
							{
								found = 1;
								break;
							}
						}
						fclose(f);

						if (found == 1)
						{
							char msg[] = "Dang nhap thanh cong. Hay nhap lenh.\n";
							send(sockets[i], msg, strlen(msg), 0);
							registeredClients[registered] = sockets[i];
							ids[registered] = (char *)malloc(strlen(id) + 1);
							memcpy(ids[registered], id, strlen(id) + 1);
							++registered;
							break;
						}
						else
						{
							char msg[] = "Dang nhap that bai. Hay thu lai.\n";
							send(sockets[i], msg, strlen(msg), 0);
						}
						sockets[numEvents] = sockets[i];
						numEvents++;
					}
					else
					{
						char msg[] = "Dang nhap that bai. Hay thu lai.\n";
						send(sockets[i], msg, strlen(msg), 0);
					}

				}
				else
				{
					// Trang thai da dang nhap
					
					if (buf[ret - 1] == '\n')
					{
						buf[ret - 1] = 0;
					}
					//printf("Received from %d: %s\n", clients[i], buf);

					sprintf(cmdBuf, "%s > out1.txt", buf);
					system(cmdBuf);

					FILE *f = fopen("out1.txt", "r");
					while (fgets(fileBuf, sizeof(fileBuf), f))
					{
						send(sockets[i], fileBuf, strlen(fileBuf), 0);
					}
					fclose(f);
				}

				
				printf("Received: %s\n", buf);
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
