#pragma once
#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define SERVER_PORT_NUMBER 27000
#define MAX_MESSAGE_LEN 512
#define SERVER_IP_ADDERESS "127.0.0.1"

typedef struct Poruka_st
{
    int topicId;
    char message[MAX_MESSAGE_LEN];
}Poruka;
bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return false;
    }
    return true;
}

int main()
{
    sockaddr_in serverAddress;
    int sockAddrLen = sizeof(struct sockaddr);
    int serverPort = SERVER_PORT_NUMBER;
    int iResult;
    if (InitializeWindowsSockets() == false) {
        return 1;
    }

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
    serverAddress.sin_port = htons((u_short)serverPort);

    SOCKET publisherSocket = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);

    if (publisherSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Odaberite temu:\n");
    printf("1.muzika\n");
    printf("2.sport\n");
    printf("3.politika\n");
    printf("4.film\n");
    printf("5.istorija\n");
    int tema;
    scanf_s("%d", &tema);
    Poruka poruka;
    poruka.topicId = tema;
    getchar();
    while (true)
    {
        printf("Poruka?");
        gets_s(poruka.message, MAX_MESSAGE_LEN);
        printf("\n");
        if (strlen(poruka.message) == 0)
            continue;
        if (strcmp(poruka.message,"q")==0)
            break;
        if (strlen(poruka.message) == 0)
            continue;
        iResult = sendto(publisherSocket, (char*)&poruka, sizeof(Poruka), 0, (LPSOCKADDR)&serverAddress, sockAddrLen);
        if (iResult == SOCKET_ERROR)
        {
            fprintf(stderr, "send failed with error: %ld\n", WSAGetLastError());
            break;
        }
    }
    closesocket(publisherSocket);
    WSACleanup();
    getchar();
}

