#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>


int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Error : Incorrect args count \n");
        return 1;
    }

    int   sockfd     = -1;
    int   bytes_read =  0;
    int   sent       =  0;
    int   ip         = inet_aton(argv[1]);
    int   port       = argv[2];
    char* path       = argv[3];

    FILE* file;
    int   N;
    char  send_buff[1024];


    file = fopen(path, "r");
    if (file == NULL) {
        printf("Error : Opening file failed \n");
        return 1;
    }

    for (char c = getc(file); c != EOF; c = getc(file)) {
        // Increment count of chars
        N++;
    }

    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr;   // where we actually connected through
    struct sockaddr_in peer_addr; // where we actually connected to
    socklen_t addrsize = sizeof(struct sockaddr_in );

    memset(send_buff, 0,sizeof(send_buff));
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    // print socket details
    getsockname(sockfd,
                (struct sockaddr*) &my_addr,
                &addrsize);
    printf("Client: socket created %s:%d\n",
           inet_ntoa((my_addr.sin_addr)),
           ntohs(my_addr.sin_port));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Note: htons for endiannes
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    printf("Client: connecting...\n");
    // Note: what about the client port number?
    // connect socket to the target address
    if( connect(sockfd,
                (struct sockaddr*) &serv_addr,
                sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed. %s \n", strerror(errno));
        return 1;
    }

    // print socket details again
    getsockname(sockfd, (struct sockaddr*) &my_addr,   &addrsize);
    getpeername(sockfd, (struct sockaddr*) &peer_addr, &addrsize);
    printf("Client: Connected. \n"
           "\t\tSource IP: %s Source Port: %d\n"
           "\t\tTarget IP: %s Target Port: %d\n",
           inet_ntoa((my_addr.sin_addr)),    ntohs(my_addr.sin_port),
           inet_ntoa((peer_addr.sin_addr)),  ntohs(peer_addr.sin_port));

    // read data from server into recv_buff
    // block until there's something to read
    // print data to screen every time

    // Send N: the size, to the client
    sent = write(sockfd,
                 N,
                 sizeof(N)
                 );
    assert (sent >= 0); // Check if we didn't write anything


//    while( 1 )
//    {
//        bytes_read = read(sockfd,
//                          recv_buff,
//                          sizeof(recv_buff) - 1);
//        if( bytes_read <= 0 )
//            break;
//        recv_buff[bytes_read] = '\0';
//        puts( recv_buff );
//    }

    close(sockfd); // is socket really done here?
    close(file);
    //printf("Write after close returns %d\n", write(sockfd, recv_buff, 1));
    return 0;
}
