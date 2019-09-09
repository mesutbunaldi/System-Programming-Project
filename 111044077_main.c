/*
 *  MESUT BUNALDI
 *  111044077
 *  SYSTEM PROGRAMMING
 *  FINAL PROJECT
 *
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>



#define ERROR 				-1


#define PORT 5555

#define QUEUE_SIZE	2 // provider için sırada bekleyebilecek client isteği sayısı
//eger en iyi result u verecek provider müsait değilse bir sonraki en iyi hesaplamayı yapabilecek
//provider a client in isteğini yönlendirmeliyimmm

typedef struct Server{
    char name[32];
    int  socket[10];
}Server;


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

typedef struct Client{
    char name[32];
    char priority[1]; //Q: Quality C:Cost  T:Time
    int denoting_degree;//hesaplama yapılmasını istediğimiz değer
    char server_address[16];
    char port_address[4];
}Client;

int server_fd, new_socket, valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char buffer[1024] = {0};
void sighandler(int signum);
pthread_cond_t cond_provider;
pthread_mutex_t lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int countOfProvider=0;
pthread_t providerThreads[100];
int checkThread=0;
int bestPerformanceIndex=0;
//dosyadan provierları okuyan fonksiyonumuz
void getProviders(char* pFileName,Provider providers[]);
Provider providers[100];
Client tmpClient;
//providers içinde en iyi olanların indexini döndüren fonksiyon
int getBestPerformance(Provider providers[],char type[]);


//////////////////////////////////////////////////////////////////

//function for proviers threads
void* providerFunction();
//////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    getProviders(argv[2], providers);
    printf("Logs kept at %s \n", argv[3]);

    printf("%d provider threads created\n", countOfProvider);
    FILE *fdLog;
    fdLog = fopen(argv[3], "w+");

    signal(SIGINT, sighandler);
    //en yüksek kalitede üretim yapan provider ın indexi


    //providers içinde en yüksek Q T C değerlerine sahip providerları bul ve id lerini sakla

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond_provider, NULL);

               //provider threads
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("Failed to create socket");
        return ERROR;
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if ( bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == ERROR){
        perror("Failed to bind the socket to port");
        return ERROR;
    }


    printf("Name\tPerformance\tPrice\tDuration\n");
    fprintf(fdLog,"Name\tPerformance\tPrice\tDuration\n");
    for (int k = 0; k < countOfProvider; ++k) {
        providers[k].servedClient=0;//init servedClent
        printf("%s\t%d\t\t%d\t%d\n", providers[k].name, providers[k].performance, providers[k].price,
               providers[k].time);
        fprintf(fdLog,"%s\t%d\t\t%d\t%d\n", providers[k].name, providers[k].performance, providers[k].price,
               providers[k].time);
    }

    for (int i = 0; i < countOfProvider; i++) {
        checkThread=1;
        pthread_create(&providerThreads[i], NULL, providerFunction, (void *) &providers[i]);

    }

    //do stuff


    while (1) {

        Client Client1;


        if (listen(server_fd, 0)== -1){
            perror("Failed to listen socket");
            return ERROR;
        }
       // printf("Server is waiting for client connections at port 5555\n");
        new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
        valread = read(new_socket, &Client1, sizeof(Client));
        strcpy(tmpClient.name,Client1.name);
        tmpClient.denoting_degree=Client1.denoting_degree;
            checkThread=0;
            int providerIndex= getBestPerformance(providers,Client1.priority);
            pthread_create(&providerThreads[providerIndex], NULL, providerFunction, (void *) &providers[providerIndex]);
            pthread_cond_signal(&cond_provider);



        printf("Client %s (%s %d) connected, forwarded to provider %s\n", Client1.name,Client1.priority,Client1.denoting_degree,providers[providerIndex].name);
        fprintf(fdLog,"Client %s (%s %d) connected, forwarded to provider %s\n", Client1.name,Client1.priority,Client1.denoting_degree,providers[providerIndex].name);



        //talep değişkenlerini alıp create thread ile yeni bir thread oluşturuyoruz ve
        //uygun providera isteği iletiyoruz
        fflush(stdout);


    }

    for (int j = 0; j < countOfProvider; ++j) {
        pthread_join(providerThreads[j], NULL);
    }
    pthread_cond_destroy(&cond_provider);
    pthread_mutex_destroy(&lock);
    fclose(fdLog);

    return 0;
}



void* providerFunction(void * providers) {

    if (checkThread == 1)
        printf("Provider %s waiting for tasks\n", ((Provider *) providers)->name);
        //fprintf(fdLog,"Provider %s waiting for tasks\n", ((Provider *) providers)->name);

    while (1) {
        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond_provider, &lock);

        //burada cos hesapla ve random time kadar sleep yap
        srand(time(NULL));
        int result = (rand() % (15 - 5)) + 5;

        ((Provider *) providers)->calcTime = result;
        printf("Provider %s is processing task : %d\n", tmpClient.name, tmpClient.denoting_degree);
        //fprintf(fdLog,"Provider %s is processing task : %d\n", tmpClient.name, tmpClient.denoting_degree);

        ((Provider *) providers)->cosVal = cos(tmpClient.denoting_degree);
        ((Provider *) providers)->servedClient += 1;
        sleep(result);
        printf("Provider %s completed task number 1: cos(%d)=%lf in %3.2f seconds.\n", ((Provider *) providers)->name,
               tmpClient.denoting_degree, ((Provider *) providers)->cosVal, ((Provider *) providers)->calcTime);
        //fprintf(fdLog,"Provider %s completed task number 1: cos(%d)=%lf in %3.2f seconds.\n", ((Provider *) providers)->name,
              // tmpClient.denoting_degree, ((Provider *) providers)->cosVal, ((Provider *) providers)->calcTime);

        send(new_socket, providers, sizeof(Provider), 0);

        pthread_mutex_unlock(&lock);
        checkThread = 0;

        pthread_exit(NULL);

    }
}
//zaman , maliyet ya da kalite değerlerinden en iyisini sağlayan sağlayısı indexini döndüren fonk
int getBestPerformance(Provider providers[],char type[]){

   if(strcmp(type,"Q")==0) {
       int maximum = providers[0].performance;
       int location = 0;
       for (int c = 1; c < countOfProvider; c++) {
           if (providers[c].performance > maximum) {
               maximum = providers[c].performance;
               location = c ;
           }
       }
       return location;
   }else if(strcmp(type,"T")==0 ){//en az zaman veya en düşük maliyet
       int minTime = providers[0].time;
       int location = 0;
       for (int c = 1; c < countOfProvider; c++) {
           if (providers[c].time < minTime) {
               minTime = providers[c].time;
               location = c + 1;
           }
       }
       return location;
   }
   else if(strcmp(type,"C")==0){//en az zaman veya en düşük maliyet
       int minCost = providers[0].price;
       int location = 0;
       for (int c = 1; c < countOfProvider; c++) {
           if (providers[c].price < minCost) {
               minCost = providers[c].price;
               location = c;
           }
       }
       return location;
   }
}


//provider bilglerini dosyadan aldığımız fonksiyon
void getProviders(char* pFileName,Provider providers[]) {
    FILE *fdProvider;
    Provider *nP;
    fdProvider = fopen(pFileName, "r");
    int counter = 0, temp = -1;
    size_t buffer_size = 1024;
    char *line = (char *) malloc(buffer_size * sizeof(char));
    char *token;
    while (-1 != getline(&line, &buffer_size, fdProvider)) {
        if (counter != 0) {
            token = strtok(line, " ");
            int chck = 0;
            while (token != NULL) {
                if (chck == 0) {
                    strcpy(providers[counter - 1].name, token);
                } else {
                    if (chck == 1)
                        providers[counter - 1].performance = atoi(token);
                    else if (chck == 2)
                        providers[counter - 1].price = atoi(token);
                    else if (chck == 3)
                        providers[counter - 1].time = atoi(token);
                }
                token = strtok(NULL, " ");
                chck++;
            }
        }
        providers[counter].thread_id = counter;
        counter++;
    }
    countOfProvider = counter - 1;
    fclose(fdProvider);
}


void sighandler(int signum) {



    printf("Termination signal received\n"
           "Terminating all clients\n"
           "Terminating all providers\nStatistics\nName\t"
           "Number of clients served\n");
    //fprintf(fdLog,"Termination signal received\n"
               //   "Terminating all clients\n"
             //     "Terminating all providers\nStatistics\nName\t"
                  //"Number of clients served\n");

    for (int j = 0; j < countOfProvider; ++j) {
        printf("%s\t%d\n", providers[j].name, providers[j].servedClient);
       // fprintf(fdLog,"%s\t%d\n", providers[j].name, providers[j].servedClient);
    }
    for (int i = 0; i < countOfProvider; ++i) {
        pthread_cancel(providerThreads[i]);
    }

    exit(1);

}