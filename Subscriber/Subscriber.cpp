#pragma once
#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define SERVER_PORT_NUMBER 27001
#define MAX_MESSAGE_LEN 512

#define SERVER_IP_ADDERESS "127.0.0.1"
#define SUBSCRIBER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16
#define SUBSCRIBER_PORT 25000
#define ACCESS_BUFFER_SIZE 1024
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
    FILE* file = fopen("port.txt", "r+");
    int portNumber;
    fscanf(file, "%d", &portNumber);
    fclose(file);
    remove("port.txt");
    file = fopen("port.txt", "w+");
    int t = portNumber + 1;
    fprintf(file, "%d", t);
    fclose(file);

    char accessBuffer[ACCESS_BUFFER_SIZE];

    sockaddr_in subscriberAddress;
    int sockAddrLen = sizeof(struct sockaddr);
    int subscriberPort = portNumber;
    int iResult;
    if (InitializeWindowsSockets() == false) {
        return 1;
    }

    memset((char*)&subscriberAddress, 0, sizeof(subscriberAddress));
    subscriberAddress.sin_family = AF_INET;
    subscriberAddress.sin_addr.s_addr = inet_addr(SUBSCRIBER_IP_ADDERESS);
    subscriberAddress.sin_port = htons((u_short)subscriberPort);
    
    sockaddr_in serverAddress;
    int serverPort = SERVER_PORT_NUMBER;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
    serverAddress.sin_port = htons((u_short)serverPort);

    SOCKET subscriberSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    iResult = bind(subscriberSocket, (LPSOCKADDR)&subscriberAddress, sizeof(subscriberAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %ld\n", WSAGetLastError());
        return 1;
    }
    printf("Teme na koje se pretplacujete?(Brojeve odvojiti zapetama)\n");
    printf("1.muzika\n");
    printf("2.sport\n");
    printf("3.politika\n");
    printf("4.film\n");
    printf("5.istorija\n");
    char buffer[MAX_MESSAGE_LEN];
    gets_s(buffer, MAX_MESSAGE_LEN);
    iResult = sendto(subscriberSocket, buffer, MAX_MESSAGE_LEN, 0, (LPSOCKADDR)&serverAddress, sockAddrLen);
    if (iResult == SOCKET_ERROR)
    {
        printf("sendto failed with error: %d\n", WSAGetLastError());
        closesocket(subscriberSocket);
        WSACleanup();
        return 1;
    }

    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(subscriberSocket, FIONBIO, &nonBlockingMode);

    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    while (1)
    {
        sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(sockaddr_in));
        memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
        FD_SET set;
        timeval timeVal;
        FD_ZERO(&set);
        FD_SET(subscriberSocket, &set);
        timeVal.tv_sec = 0;
        timeVal.tv_usec = 0;

        iResult = select(0, &set, NULL, NULL, &timeVal);

        if (iResult == SOCKET_ERROR)
        {
            fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
            continue;
        }

        if (iResult == 0)
        {
            Sleep(50);
            continue;
        }

        iResult = recvfrom(subscriberSocket,
            accessBuffer,
            ACCESS_BUFFER_SIZE,
            0,
            (LPSOCKADDR)&clientAddress,
            &sockAddrLen);

        if (iResult == SOCKET_ERROR)
        {
            printf("recvfrom failed with error: %d\n", WSAGetLastError());
            continue;
        }

        char ipAddress[IP_ADDRESS_LEN];
        strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(clientAddress.sin_addr));
        int clientPort = ntohs((u_short)clientAddress.sin_port);
        Poruka* p = (Poruka*)accessBuffer;
        char tema[40];
        switch (p->topicId)
        {
            case 1:
                strcpy(tema, "Muzika");
                break;
            case 2:
                strcpy(tema, "Sport");
                break;
            case 3:
                strcpy(tema, "Politika");
                break;
            case 4:
                strcpy(tema, "Film");
                break;
            case 5:
                strcpy(tema, "Istorija");
                break;

        }
        printf("%s : \n", tema);
        printf("%s\n", p->message);
    }
    closesocket(subscriberSocket);
    WSACleanup();
    return 0; 
}

