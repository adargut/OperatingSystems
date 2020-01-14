#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <errno.h>

#include <string.h>

#include <sys/types.h>

#include <time.h>

#include <assert.h>

#include <ctype.h>

// MINIMAL ERROR HANDLING FOR EASE OF READING

static int total_PCC[94] = { 0 };

int main(int argc, char * argv[]) {
    int totalsent = -1;
    int nread = -1;
    int len = -1;
    int n = 0;
    int listenfd = -1;
    int connfd = -1;

    struct sockaddr_in serv_addr;
    struct sockaddr_in my_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in);

    char data_buff[1024];
    int N_buff[2];
    int serv_resp[100];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    memset( & serv_addr, 0, addrsize);

    serv_addr.sin_family = AF_INET;
    // INADDR_ANY = any local machine address
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(10000);

    if (0 != bind(listenfd,
                  (struct sockaddr * ) & serv_addr,
                  addrsize)) {
        printf("\n Error : Bind Failed. %s \n", strerror(errno));
        return 1;
    }

    if (0 != listen(listenfd, 10)) {
        printf("\n Error : Listen Failed. %s \n", strerror(errno));
        return 1;
    }

    while (1) {
        // Accept a connection.
        // Can use NULL in 2nd and 3rd arguments
        // but we want to print the client socket details
        connfd = accept(listenfd,
                        (struct sockaddr * ) & peer_addr, &
                                addrsize);

        if (connfd < 0) {
            printf("\n Error : Accept Failed. %s \n", strerror(errno));
            return 1;
        }

        getsockname(connfd, (struct sockaddr * ) & my_addr, & addrsize);
        getpeername(connfd, (struct sockaddr * ) & peer_addr, & addrsize);
        printf("Server: Client connected.\n"
               "\t\tClient IP: %s Client Port: %d\n"
               "\t\tServer IP: %s Server Port: %d\n",
               inet_ntoa(peer_addr.sin_addr),
               ntohs(peer_addr.sin_port),
               inet_ntoa(my_addr.sin_addr),
               ntohl(my_addr.sin_port));



        while (1) {
            nread = read(connfd,
                         N_buff,
                         4);
            printf("server got N %d\n", N_buff[0]);
            printf("what is nread? %d\n", nread);
            if (nread > 0) break;
        }

        uint32_t printable = 0;

        while (1) {
            printf("hiya\n");
            nread = read(connfd,
                         data_buff,
                         100);
            if (nread <= 0)
                break;
            data_buff[nread + 1] = '\0';
            printf("what did server read? %s\n", data_buff);
            printf("server read %d\n", nread);
            for (int i = 0; i < nread; i++) {
                if (isprint(data_buff[i])) {
                    total_PCC[data_buff[i]]++;
                    printable++;
                    printf("what is it? %d\n", data_buff[i]);
                }
            }
            char *the_msg = malloc(300);
            strcat(the_msg, "# of printable characters: ");
            char str[300];
            sprintf(str, "%d", printable);
            printf("what is str? %s\n", str);
            strcat(the_msg, str);
            printf("server sent response: %s\n", the_msg);
            printf("the total #of chars: %d\n", strlen(data_buff));
            break;
        }

        serv_resp[0] = printable;

        nread = write(connfd,
                           serv_resp,
                      sizeof(serv_resp) - 1);
        // check if error occured (client closed connection?)
        assert(nread >= 0);
        printf("server sent: %d\n", serv_resp[0]);
        // close socket
        close(connfd);
    }
}