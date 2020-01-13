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
    int totalsent = -1;
    int sockfd = -1;
    int bytes_sent = 0;
    int N = 0;
    int port = atoi(argv[2]);
    char * ip = argv[1];
    char * path = argv[3];
    char send_buff[1024];
    FILE * file;

    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr; // where we actually connected through
    struct sockaddr_in peer_addr; // where we actually connected to
    socklen_t addrsize = sizeof(struct sockaddr_in);

    if (argc != 4) {
        printf("Error : Incorrect args count \n");
        return 1;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        printf("Error : Opening file failed \n");
        return 1;
    }

//      for (char c = getc(file); c != EOF; ) {
          // Increment count of chars
//          N++;
//          if (N > 100) {
//              break;
//          }
//      }

//      N = get_file_length(file);

    printf("Client got N: %d\n", N);

    memset(send_buff, 0, sizeof(send_buff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

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

    // keep looping until nothing left to read
    while (notwritten > 0) {
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
    // send data to server
    //  while( 1 )
    //  {
    //    bytes_sent = write(sockfd,
    //                       send_buff,
    //                       sizeof(send_buff) - 1);
    //    if( bytes_sent <= 0 )
    //      break;
    //    send_buff[bytes_sent] = '\0';
    //    puts( send_buff );
    //  }

    close(sockfd); // is socket really done here?
    //printf("Write after close returns %d\n", write(sockfd, recv_buff, 1));
    return 0;
}