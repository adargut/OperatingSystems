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
#include <stdbool.h>
#include <sys/signal.h>

static bool busy = false;
static unsigned int total_PCC[95] = { 0 };

void handler(int sig)
{
	while(1){
		if(!busy){ /*if server is not busy print the counters and exit with 0*/
			for (int i = 0; i < 95; i++){
				char c = i + 32;
				printf("char '%c' : %u times\n",c,total_PCC[i]);
			}
			exit(0);
		}
	}
}
int main(int argc, char *argv[])
{
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
    uint32_t N_buff[1];
    int serv_resp[100];
    // Define signal
    signal(SIGINT, handler);
    // Socket
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(strerror(errno));
        exit(-1);
    }
    memset(&serv_addr, 0, addrsize);
    int yes = 1;
    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    serv_addr.sin_family = AF_INET;
    // INADDR_ANY = any local machine address
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    // Bind socket
    if (0 != bind(listenfd,
                  (struct sockaddr *) &serv_addr,
                  addrsize))
    {
        printf("\n Error : Bind Failed. %s \n", strerror(errno));
        return 1;
    }
    // Listen on port
    if (0 != listen(listenfd, 10))
    {
        printf("\n Error : Listen Failed. %s \n", strerror(errno));
        return 1;
    }

    while (true)
    {
        // Accept a connection.
        // Can use NULL in 2nd and 3rd arguments
        // but we want to print the client socket details
        connfd = accept(listenfd,
                        (struct sockaddr *) &peer_addr, &addrsize);
        busy = true;
        if (connfd < 0)
        {
            printf("\n Error : Accept Failed. %s \n", strerror(errno));
            return 1;
        }
        getsockname(connfd, (struct sockaddr *) &my_addr, &addrsize);
        getpeername(connfd, (struct sockaddr *) &peer_addr, &addrsize);
        printf("Server: Client connected.\n"
               "\t\tClient IP: %s Client Port: %d\n"
               "\t\tServer IP: %s Server Port: %d\n",
               inet_ntoa(peer_addr.sin_addr),
               ntohs(peer_addr.sin_port),
               inet_ntoa(my_addr.sin_addr),
               ntohl(my_addr.sin_port));

        nread = read(connfd,
                     N_buff,
                     4);
        printf("server got N %d\n", N_buff[0]);
        printf("what is nread? %d\n", nread);

        uint32_t printable = 0;

        while (true)
        {
            nread = read(connfd,
                         data_buff,
                         N_buff[0]);
            if (nread <= 0)
                break;
            data_buff[nread] = '\0';
            printf("server got msg: %s\n", data_buff);
            printf("server read %d\n", nread);
            for (int i = 0; i < nread; i++)
            {
                if (data_buff[i] <= 126 && data_buff[i] >= 32)
                {
                    total_PCC[data_buff[i] - 32]++;
                    printable++;
                }
            }

            char *the_msg = malloc(300);
            strcat(the_msg, "# of printable characters: ");
            char str[300];
            sprintf(str, "%d", printable);
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
        // close connection
        busy = false;
        close(connfd);
    }
    exit(0);
}