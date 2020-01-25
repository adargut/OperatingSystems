#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

uint32_t char_statistics[127];
int canceled = 0;

uint32_t get_N(int fd, uint32_t* result)
{
    uint32_t N;
    char *bytes = (char*)&N;
    int left = sizeof(uint32_t);
    int bytes_read;
    while (left > 0)
    {
        bytes_read = read(fd, bytes, left);
        if (bytes_read < 0)
        {
            if (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)
            {
                perror("Error: ");
                return 1;
            }
            else
            {
                perror("Error: ");
                exit(1);
            }
        }
        else
        {
            bytes += bytes_read;
            left -= bytes_read;
        }
    }
    bytes-=sizeof(uint32_t);
    *result = ntohl(*((uint32_t*)(bytes)));
    return 0;
}

int get_message(int fd, char* message, uint32_t N)
{
    uint32_t bytes_read;
    uint32_t left = N;
    //read file contents to <message>
    while(left > 0)
    {
        bytes_read = read(fd,
                          message,
                          left);
        if( bytes_read < 0 )
        {
            if (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)
            {
                perror("Error: ");
                return 1;
            }
            else
            {
                perror("Error: ");
                exit(1);
            }
        }
        else
        {
            message += bytes_read;
            left -= bytes_read;
        }
    }
    return 0;
}

uint32_t count_printable(char* message, uint32_t N)
{
    uint32_t printable = 0;
    for (uint32_t i = 0; i < N; i++)
    {
        if(message[i] >= 32 && message[i] <= 126)
        {
            printable++;
        }
    }
    return printable;
}

int send_printable(int fd, uint32_t printable)
{
    uint32_t temp = htonl(printable);
    char *bytes = (char*)&temp;
    int left = sizeof(uint32_t);
    int bytes_written;
    while (left > 0)
    {
        bytes_written = write(fd, bytes, left);
        if (bytes_written < 0)
        {
            if (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)
            {
                perror("Error: ");
                return 1;
            }
            else
            {
                perror("Error: ");
                exit(1);
            }
        }
        else
        {
            bytes += bytes_written;
            left -= bytes_written;
        }
    }
    return 0;
}

void update_statistics(char* message, uint32_t N)
{
    for (uint32_t i = 0; i < N; i++)
    {
        if(message[i] >= 32 && message[i] <= 126)
        {
            char_statistics[(int)(message[i])]++;
        }
    }
}

int handle_client(int fd)
{
    //printf("began handling client\n");
    uint32_t N = 0;
    if(get_N(fd, &N) != 0) return 0;
    uint32_t printable = 0;
    char* message = malloc(N);
    if(get_message(fd, message, N) != 0)
    {
        free(message);
        return 0;
    }
    printable = count_printable(message, N);
    if(send_printable(fd, printable) != 0)
    {
        free(message);
        return 0;
    }
    update_statistics(message, N);
    free(message);
    return 0;
}

void print_statistics()
{
    for (char i = 32; i < 127; i++)
    {
        printf("char ’%c’ : %u times\n", i, char_statistics[(uint32_t)i]);
    }
}

void handle_sigint(int signum, siginfo_t* info, void* ptr)
{
    canceled = 1;
}

int main(int argc, char** argv)
{
    //setup...
    //register signal handler for SIG_INT
    struct sigaction sigint_action;
    memset(&sigint_action, 0, sizeof(sigint_action));
    sigint_action.sa_sigaction = handle_sigint;
    sigint_action.sa_flags = SA_SIGINFO;
    if(0 != sigaction(SIGINT, &sigint_action, NULL))
    {
        fprintf(stderr, "SIGINT handler registration failed: %s\n", strerror(errno));
        return -1;
    }

    int listenfd  = -1;
    int connfd    = -1;
    if (argc != 2)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return -1;
    }
    unsigned short port = (unsigned short)(atoi(argv[1]));

    socklen_t addrsize = sizeof(struct sockaddr_in );
    struct sockaddr_in serv_addr;
    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed: ");
    memset( &serv_addr, 0, addrsize );

    serv_addr.sin_family = AF_INET;
    // INADDR_ANY = any local machine address
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if( 0 != bind( listenfd,
                   (struct sockaddr*) &serv_addr,
                   addrsize ) )
    {
        fprintf(stderr, "Error : Bind Failed. %s \n", strerror(errno));
        return 1;
    }

    if( 0 != listen( listenfd, 10 ) )
    {
        fprintf(stderr, "Error : Listen Failed. %s \n", strerror(errno));
        return 1;
    }

    while( canceled == 0 )
    {
        // Accept a connection.
        connfd = accept( listenfd,
                         NULL,
                         NULL);

        //printf("accepted connection\n");
        if( connfd < 0 )
        {
            if(errno != EINTR)
                printf("\n Error : Accept Failed. %s \n", strerror(errno));
        }
        else
        {
            handle_client(connfd);
            close(connfd);
        }
    }
    print_statistics();
    close(listenfd);
    return 0;
}