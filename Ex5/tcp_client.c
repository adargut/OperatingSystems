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

#include <stdbool.h>

off_t get_file_length(FILE * file) {
    fpos_t position; // fpos_t may be a struct and store multibyte info
    off_t length; // off_t is integral type, perhaps long long

    fgetpos(file, & position); // save previous position in file

    fseeko(file, 0, SEEK_END); // seek to end
    length = ftello(file); // determine offset of end

    fsetpos(file, & position); // restore position

    return length;
}

// MINIMAL ERROR HANDLING FOR EASE OF READING

int main(int argc, char * argv[]) {
    int totalsent = 0;
    int sockfd = -1;
    int nread = -1;
    int bytes_sent = 0;
    int N = 0;
    int port = atoi(argv[2]);
    char * ip = argv[1];
    char * path = argv[3];
    char send_buff[2500];
    char N_buff[10];
    char serv_resp[1024] = { 0 };
    FILE * file;

    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr; // where we actually connected through
    struct sockaddr_in peer_addr; // where we actually connected to
    socklen_t addrsize = sizeof(struct sockaddr_in);

    if (argc != 4) {
        printf("Error : Incorrect args count \n");
        return 1;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        printf("Error : Opening file failed \n");
        return 1;
    }

    memset(send_buff, 0, sizeof(N_buff));
    N = get_file_length(file);
    N_buff[0] = ntohl(N);
    N_buff[1] = "\0";
    printf("client got N: %d\n", N);

    memset(send_buff, 0, sizeof(send_buff));


    while (fgets(send_buff, 1024, file) != NULL)
        printf(" %s ", send_buff);


      printf("client got msg:%s\n",send_buff);


      if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    printf("Client sending: %s\n", send_buff);

    // print socket details
    getsockname(sockfd,
                (struct sockaddr * ) & my_addr, &
                        addrsize);
    printf("Client: socket created %s:%d\n",
           inet_ntoa((my_addr.sin_addr)),
           ntohs(my_addr.sin_port));

    memset( & serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Note: htons for endiannes
    serv_addr.sin_addr.s_addr = inet_addr(ip); // hardcoded...

    printf("Client: connecting...\n");
    // Note: what about the client port number?
    // connect socket to the target address
    if (connect(sockfd,
                (struct sockaddr * ) & serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed. %s \n", strerror(errno));
        return 1;
    }

    // print socket details again
    getsockname(sockfd, (struct sockaddr * ) & my_addr, & addrsize);
    getpeername(sockfd, (struct sockaddr * ) & peer_addr, & addrsize);
    printf("Client: Connected. \n"
           "\t\tSource IP: %s Source Port: %d\n"
           "\t\tTarget IP: %s Target Port: %d\n",
           inet_ntoa((my_addr.sin_addr)), ntohs(my_addr.sin_port),
           inet_ntoa((peer_addr.sin_addr)), ntohs(peer_addr.sin_port));


    int notwritten = strlen(send_buff);

    // Write N from client to server
    bytes_sent = write(sockfd,
                        N_buff,
                        4);

    // Write message from client to server
    while (notwritten > 0) {
        printf("client writing message...\n");
        // notwritten = how much we have left to write
        // totalsent  = how much we've written so far
        // nsent = how much we've written in last write() call */
        bytes_sent = write(sockfd,
                           send_buff + totalsent,
                           notwritten);
        // check if error occured (client closed connection?)
        assert(bytes_sent >= 0);
        printf("Client: wrote %d bytes\n", bytes_sent);

        totalsent += bytes_sent;
        notwritten -= bytes_sent;
    }

    // Read response from the server
    while (1) {
        printf("hi\n");
        nread = read(sockfd,
                     serv_resp,
                     sizeof(serv_resp) - 1);
        printf("client got response from server: %d\n", serv_resp[0]);
        if (nread > 0) break;
    }

    close(sockfd); // is socket really done here?
    close(file);
    //printf("Write after close returns %d\n", write(sockfd, recv_buff, 1));
    return 0;
}