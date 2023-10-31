#pragma once
#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <omp.h>

#define SERVER_PORT_NUMBER_SUB 27001
#define MAX_MESSAGE_LEN 512

#define SERVER_IP_ADDERESS "127.0.0.1"
#define SUBSCRIBER_IP_ADDERESS "127.0.0.1"
#define IP_ADDRESS_LEN 16
#define SUBSCRIBER_PORT 25000
#define ACCESS_BUFFER_SIZE 1024
#define SERVER_PORT_NUMBER_PUB 27000

#define NUMBER_OF_PUBLISHERS 5
#define NUMBER_OF_SUBSCRIBERS 5
#define MAX_THREADS 10
#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);} 

typedef struct Poruka_st
{
    int topicId;
    char message[MAX_MESSAGE_LEN];
}Poruka;

typedef struct PublisherThreadParam_st {
    int topicId;
    bool shutdownEngine = false;
}PublisherThreadParam;

typedef struct SubscriberThreadParam_st {
    char topicIds[7];
    bool shutdownEngine = false;
}SubscriberThreadParam;

CRITICAL_SECTION cs;
DWORD WINAPI CreatePublisher(LPVOID lpvThreadParam);
DWORD WINAPI CreateSubscriber(LPVOID lpvThreadParam);

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

int main() {

    SubscriberThreadParam subscriberThreadParam[5];
    PublisherThreadParam publisherThreadParam[5];

    if (InitializeWindowsSockets() == false) {
        return 1;
    }
    InitializeCriticalSection(&cs);
    
    printf("\nPress any key to start stress test..\n");
    getch();

    srand(time(NULL));
    
    for (int i = 0;i < 5;i++) {
        publisherThreadParam[i].topicId = ((rand() % 5) + 1);
    }

    int n = 3;
    char jedanniz[7], tempstring[3];
    memset(jedanniz, 0, sizeof(jedanniz));

    for (int i = 0;i < 5;i++) {
        while (n > 0) {           
            sprintf(tempstring, "%d,", ((rand() % 5) + 1));
            strcat(jedanniz, tempstring);
            n--;
        }
        n = 3;
        jedanniz[strlen(jedanniz) - 1] = '\0';     
        strcpy(subscriberThreadParam[i].topicIds, jedanniz);
        memset(jedanniz, 0, sizeof(jedanniz));
    }

    HANDLE allThreads[MAX_THREADS];
    DWORD ThreadIds[MAX_THREADS], retVal;
    int threadCounter = 0;

    for (int i = 0; i < NUMBER_OF_SUBSCRIBERS; i++)
    {
        allThreads[threadCounter] = CreateThread(NULL, 0, &CreateSubscriber, &subscriberThreadParam[i], 0, &ThreadIds[threadCounter]);
        threadCounter++;          
    }


    for (int i = 0; i < NUMBER_OF_PUBLISHERS; i++)
    {
        allThreads[threadCounter] = CreateThread(NULL, 0, &CreatePublisher, &publisherThreadParam[i], 0, &ThreadIds[threadCounter]);
        threadCounter++;       
    }

    printf("\nPress any key to close stress test..\n");
    getch();

    for (int i = 0;i < 5;i++)
        publisherThreadParam[i].shutdownEngine = true;
    for (int i = 0;i < 5;i++)
        subscriberThreadParam[i].shutdownEngine = true;

    retVal = WaitForMultipleObjects(10, allThreads, TRUE, INFINITE);
    if ((retVal >= WAIT_OBJECT_0) && (retVal <= (WAIT_OBJECT_0 + 10 - 1)))
        printf("Uspesno izvrsena funkcija");

    for (int i = 0; i < NUMBER_OF_SUBSCRIBERS + NUMBER_OF_PUBLISHERS; i++)
        SAFE_DELETE_HANDLE(allThreads[i]);
    
    DeleteCriticalSection(&cs);
    
	return 0;
}

DWORD WINAPI CreatePublisher(LPVOID lpvThreadParam) {

    PublisherThreadParam* publisherThreadParam = (PublisherThreadParam*)lpvThreadParam;
    sockaddr_in serverAddress;
    int sockAddrLen = sizeof(struct sockaddr);
    int serverPort = SERVER_PORT_NUMBER_PUB;
    int iResult;

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
    serverAddress.sin_port = htons((u_short)serverPort);

    SOCKET publisherSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (publisherSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    Poruka poruka; 
    poruka.topicId = publisherThreadParam->topicId;

    int broj_poruka = 10000000;
    while (publisherThreadParam->shutdownEngine != true) {
        while (true)
        {
            if (publisherThreadParam->shutdownEngine == true)
                break;
            char porukica[MAX_MESSAGE_LEN];
            memset(porukica, 0, sizeof(porukica));
            sprintf(porukica, "Ovo je poruka %d", broj_poruka);           
            strcpy_s(poruka.message, MAX_MESSAGE_LEN, porukica);
            iResult = sendto(publisherSocket, (char*)&poruka, sizeof(Poruka), 0, (LPSOCKADDR)&serverAddress, sockAddrLen);
            if (iResult == SOCKET_ERROR)
            {
                fprintf(stderr, "send failed with error: %ld\n", WSAGetLastError());
                break;
            }
            //Sleep(100);
            //broj_poruka--;
        }
    }
    closesocket(publisherSocket);
    
    return 0;
}

DWORD WINAPI CreateSubscriber(LPVOID lpvThreadParam) {

    SubscriberThreadParam* subscriberThreadParam = (SubscriberThreadParam*)lpvThreadParam;
    EnterCriticalSection(&cs);
    FILE* file = fopen("port.txt", "r+");
    int portNumber;
    fscanf(file, "%d", &portNumber);
    fclose(file);
    remove("port.txt");
    file = fopen("port.txt", "w+");
    int t = portNumber + 1;
    fprintf(file, "%d", t);
    fclose(file);
    LeaveCriticalSection(&cs);

    char accessBuffer[ACCESS_BUFFER_SIZE];

    sockaddr_in subscriberAddress;
    int sockAddrLen = sizeof(struct sockaddr);
    int subscriberPort = portNumber;
    int iResult;

    memset((char*)&subscriberAddress, 0, sizeof(subscriberAddress));
    subscriberAddress.sin_family = AF_INET;
    subscriberAddress.sin_addr.s_addr = inet_addr(SUBSCRIBER_IP_ADDERESS);
    subscriberAddress.sin_port = htons((u_short)subscriberPort);

    sockaddr_in serverAddress;
    int serverPort = SERVER_PORT_NUMBER_SUB;
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
  
    char buffer[MAX_MESSAGE_LEN];
    memset(buffer, 0, sizeof(buffer));

    char* teme = (char *)lpvThreadParam;
    strcpy(buffer, teme);

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
        if (subscriberThreadParam->shutdownEngine == true)
            break;
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
        printf("Client %d -> %s : %s\n", subscriberPort, tema, p->message);                 
    }
    closesocket(subscriberSocket); 
    return 0;
}