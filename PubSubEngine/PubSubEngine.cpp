#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)
#include "hashTable.h"
#include "queue.h"

#define SERVER_PORT 27001
#define SERVER_PORT2 27000
#define SERVER_PORT3 27002
#define SUBSCRIBER_IP_ADDERESS "127.0.0.1"

#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16
#define SERVER_SLEEP_TIME 50
#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);} 


HashTable *subscribers;
Queue *poruke;
typedef struct Poruka_st
{
    int topicId;
    char message[MAX_MESSAGE_LEN];
}Poruka;

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
DWORD WINAPI receiveMessagesFromSubscribers(LPVOID lpParam)
{  
    bool *shutdownEngine = (bool*)lpParam;
    sockaddr_in serverAddress;
    int serverPort = SERVER_PORT;
    int sockAddrLen = sizeof(struct sockaddr);
    char accessBuffer[ACCESS_BUFFER_SIZE];
    int iResult;

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);
    SOCKET serverSocket = socket(AF_INET,      
        SOCK_DGRAM,   
        IPPROTO_UDP); 
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    iResult = bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    while (1)
    {
        if (*shutdownEngine)
        {
            break;
        }
        sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(sockaddr_in));
        memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
        FD_SET set;
        timeval timeVal;
        FD_ZERO(&set);
        FD_SET(serverSocket, &set);
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
            Sleep(SERVER_SLEEP_TIME);
            continue;
        }

        iResult = recvfrom(serverSocket,
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
        char* token = strtok(accessBuffer, ",");
        while (token != NULL)
        {
            if (!tableHasKey(subscribers, atoi(token)))
                addListToTable(subscribers, atoi(token));           
            addDataToTable(subscribers, atoi(token), clientPort);
            token = strtok(NULL, ",");
        }      
    }
    if (closesocket(serverSocket) != 0) {
        printf("Greska: %d", WSAGetLastError());
    }
    
}
DWORD WINAPI ReceiveMessagesFromPublishers(LPVOID lpParam)
{
    bool *shutdownEngine = (bool*)lpParam;
    sockaddr_in serverAddress;
    int serverPort = SERVER_PORT2;
    int sockAddrLen = sizeof(struct sockaddr);
    char accessBuffer[ACCESS_BUFFER_SIZE];
    int iResult;

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);
    SOCKET serverSocket = socket(AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    iResult = bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress));

    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);

    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    while (1)
    {
        if (*shutdownEngine)
        {
            break;
        }
        sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(sockaddr_in));
        memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
        FD_SET set;
        timeval timeVal;
        FD_ZERO(&set);
        FD_SET(serverSocket, &set);
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
            Sleep(SERVER_SLEEP_TIME);
            continue;
        }

        iResult = recvfrom(serverSocket,
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
        
        if (!tableHasKey(subscribers, p->topicId))
            addListToTable(subscribers, p->topicId);
        
        Enqueue(&poruke, p->topicId, p->message);
    }
    if (closesocket(serverSocket) != 0) {
        printf("Greska: %d", WSAGetLastError());
    }   
}

DWORD WINAPI SendMessagesToSubscribers(LPVOID lpParam)
{
    bool *shutdownEngine = (bool*)lpParam;
    sockaddr_in serverAddress;
    int sockAddrLen = sizeof(struct sockaddr);
    int serverPort = SERVER_PORT3;  
    char accessBuffer[ACCESS_BUFFER_SIZE];
    int iResult;

    sockaddr_in clientAddress;
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(SUBSCRIBER_IP_ADDERESS);

    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);
    SOCKET serverSocket = socket(AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP);
    while (true)
    {
        if (*shutdownEngine)
        {
            break;
        }
        if (IsEmptyQueue(poruke))
        {
            Sleep(500);
            continue;
        }
        char message[512];
        int topic;      
        memset(message, '0', 512);    
        Dequeue(&poruke, message, &topic);
        List* lista = getListFromTable(subscribers, topic);      
        ListItem* current = lista->head;
        while (current != NULL)
        {
            clientAddress.sin_port =htons(current->data);
            Poruka p;
            strcpy_s(p.message, message);       
            p.topicId = topic;
            sendto(serverSocket, (char*)&p, sizeof(Poruka), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);
            current = current->next;
        }
    }
    if (closesocket(serverSocket) != 0) {
        printf("Greska: %d", WSAGetLastError());
    }
    return NULL;
}

int main()
{
    if (InitializeWindowsSockets() == false) {
        return 1;
    }
    subscribers = table_init();
    InitializeQueue(&poruke);
    bool shutdownEngine = false;
    DWORD receiveMessagesFromSubscribersThreadId, receiveMessagesFromPublishersThreadId,sendMessagesThreadId, retVal;
    HANDLE threadHandles[3];
    threadHandles[0] = CreateThread(NULL, 0, &receiveMessagesFromSubscribers, &shutdownEngine, 0, &receiveMessagesFromSubscribersThreadId);
    threadHandles[1] = CreateThread(NULL, 0, &ReceiveMessagesFromPublishers, &shutdownEngine, 0, &receiveMessagesFromPublishersThreadId);
    threadHandles[2] = CreateThread(NULL, 0, &SendMessagesToSubscribers, &shutdownEngine, 0, &sendMessagesThreadId);
    printf("Pub-sub engine started.Press <Enter> to shutdown.");
    getchar();
    shutdownEngine = true;
    retVal = WaitForMultipleObjects(3, threadHandles, TRUE, INFINITE);
    if ((retVal >= WAIT_OBJECT_0) && (retVal <= (WAIT_OBJECT_0 + 3 - 1)))
        printf("Uspesno zavrsene niti.");
    for (int i = 0;i < 3;i++)
        SAFE_DELETE_HANDLE(threadHandles[i]);
    FreeQueue(&poruke);
    deleteTable(subscribers);
    WSACleanup();
    return 0;
}
