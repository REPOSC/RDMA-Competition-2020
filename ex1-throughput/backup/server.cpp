#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct SocketCreationError{};
struct SocketBindError{};
struct SocketListenError{};

class Server {
    public:
        Server(int port){
            _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_sock == -1){
                perror("create");
                throw SocketCreationError();
            }
            memset(&_serv_addr, 0, sizeof(_serv_addr));
            _serv_addr.sin_family = AF_INET;
            _serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            _serv_addr.sin_port = htons(port);
            if (bind(_sock, (struct sockaddr *)(&_serv_addr), sizeof(_serv_addr)) == -1){
                perror("bind");
                throw SocketBindError();
            }
        }
        void init_bytes(int all_bytes){
            if (listen(_sock, LISTEN_NUM)  == -1){
                perror("listen");
                throw SocketListenError();
            }
            _all_bytes = all_bytes;
            buffer = new char[_all_bytes + 1];
        }
        void run(int times){
            _connfd = accept(_sock, NULL, NULL);
            for (int i = 0; i < times; ++i){
                int received_bytes = 0;
                while (received_bytes < _all_bytes){
                    int this_bytes = read(_connfd, buffer, _all_bytes);
                    received_bytes += this_bytes;
                }
                buffer[0] = 1; buffer[2] = 0;
                write(_connfd, buffer, 1);    
            }                    
        }
        ~Server(){
            delete []buffer;
            close(_connfd);
            close(_sock);
        }
    private:
        const int LISTEN_NUM = 5;        
        int _sock;
        struct sockaddr_in _serv_addr;
        char * buffer;
        int _all_bytes;
        int _connfd;
};

int main(){
    int ALL_COUNT = 11;
    int EVERY_TIMES_COUNT = 10;
    Server server(6000);
    server.init_bytes(10240);
    server.run(ALL_COUNT * EVERY_TIMES_COUNT);
    return 0;
}