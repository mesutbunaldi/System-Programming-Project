
/*
 *  MESUT BUNALDI
 *  111044077
 *  SYSTEM PROGRAMMING
 *  FINAL PROJECT
 *
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 5555




typedef struct Client{
    char name[32];
    char priority[1]; //Q: Quality C:Cost  T:Time
    int denoting_degree;//hesaplama yapılmasını istediğimiz değer
    char server_address[16];
    char port_address[4];

}Client;
typedef struct Provider{
    int  thread_id;
    char name[32];
    int performance;
    int price;
    int time;
    double cosVal;
    double calcTime;
    int servedClient;
}Provider;

Provider provider;
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    Client Client1;
    strcpy(Client1.name, argv[1]);//name
    strcpy(Client1.priority, argv[2]);//öncelik türü Q C T
    Client1.denoting_degree = atoi(argv[3]);//value
    //strcpy(Client1.port_address,itoa(PORT));
    char buffer[1024] = {0};


    sock = socket(AF_INET, SOCK_STREAM, 0);


    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));


    send(sock, &Client1, sizeof(Client), 0);
    printf("Client %s is requesting %s %d from server 127.0.0.1:%d\n", Client1.name, Client1.priority,
           Client1.denoting_degree, PORT);

    valread = read(sock, &provider, sizeof(Provider));
    printf("Provider %s completed task number 1: cos(%d)=%lf in %3.2f seconds.\n", provider.name,
           Client1.denoting_degree, provider.cosVal,
           provider.calcTime);//buraya işlem sonucu ve ne kadar zamanda tamamlanoığı yazılacak

    return 0;
}

