#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>

struct SocketCreationError{};
struct SocketConnectError{};
struct SocketCheckError{};

#define MB 1024*1024

class Client{
    public:
        Client(const char * ip, int port){
            srand(time(NULL));
            _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_sock == -1){
                perror("create");
                throw SocketCreationError();
            }
            memset(&_serv_addr, 0, sizeof(_serv_addr));
            _serv_addr.sin_family = AF_INET;
            _serv_addr.sin_addr.s_addr = inet_addr(ip);
            _serv_addr.sin_port = htons(port);
        }
        void init_bytes(int all_bytes){
            _all_bytes = all_bytes;
            if (connect(_sock, (struct sockaddr *)(&_serv_addr), sizeof(sockaddr)) < 0){
                perror("connect");
                throw SocketConnectError();
            }            
            buffer = new char[all_bytes];
            for (int i = 0; i < all_bytes; ++i){
                buffer[i] = (char)(rand() % 255);
            }
        }
        int run_and_gettime(int every_bytes){
            int times = _all_bytes / every_bytes;
            struct timeval t1, t2;
            gettimeofday(&t1, NULL);
            for (int i = 0; i < times; ++i){
                write(_sock, buffer, every_bytes);                
            }            
            read(_sock, buffer, 1);
            if (buffer[0] != 1){
                perror("check");
                throw SocketCheckError();
            }
            gettimeofday(&t2, NULL);
            int deltaT = (t2.tv_sec-t1.tv_sec) * 1000000 + t2.tv_usec-t1.tv_usec;
            return deltaT;            
        }
        ~Client(){
            delete []buffer;
            close(_sock);
        }
    private:
        int _sock;
        char * buffer;
        int _all_bytes;
        struct sockaddr_in _serv_addr;
};

int main(int argc, char ** argv){
    if (argc != 2){
        fprintf(stderr, "Usage: client `ipaddr`\n");
        exit(-1);
    }
    int ALL_COUNT = 11;
    int EVERY_TIME_COUNT = 10;
    printf("IP: %s\n", argv[1]);
    Client client(argv[1], 10586);
    client.init_bytes(MB);
    int * every_bytes = new int[ALL_COUNT];
    for (int i = 0; i < ALL_COUNT; ++i){
        every_bytes[i] = 1 << i;
        printf("%d: ", every_bytes[i]);
        for (int j = 0; j < EVERY_TIME_COUNT; ++j){
            double time_cost = client.run_and_gettime(every_bytes[i]) / 1000. / 1000.;
            double bw = 1. / 1000 / time_cost;
            printf("%lfGBPS ", bw);
        }
        usleep(100000);
        printf("\n");   
    }
    return 0;
}