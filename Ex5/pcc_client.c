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

#include <limits.h>

#define UNUSED(x)(void)(x)

// Util
off_t get_file_length(FILE * file) {
    fpos_t position; // fpos_t may be a struct and store multibyte info
    off_t length; // off_t is integral type, perhaps long long

    fgetpos(file, & position); // save previous position in file

    fseeko(file, 0, SEEK_END); // seek to end
    length = ftello(file); // determine offset of end

    fsetpos(file, & position); // restore position

    return length;
}

int main(int argc, char * argv[]) {
    int totalsent = 0;
    int sockfd = -1;
    int nread = -1;
    int bytes_sent = 0;
    uint32_t N = 0;
    int port = atoi(argv[2]);
    char * ip = argv[1];
    char * path = argv[3];
    char * send_buff;
    uint32_t N_buff[10];
    char serv_resp[1024] = {
            0
    };
    FILE * file;

    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr; // where we actually connected through
    struct sockaddr_in peer_addr; // where we actually connected to
    socklen_t addrsize = sizeof(struct sockaddr_in);
    UNUSED(peer_addr);

    if (argc != 4) {
        printf("Error : Incorrect args count \n");
        return 1;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        printf("Error : Opening file failed \n");
        return 1;
    }

    N = get_file_length(file);
    N_buff[0] = N;
    send_buff = malloc(N);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    // print socket details
    getsockname(sockfd,
                (struct sockaddr * ) & my_addr, & addrsize);

    memset( & serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Note: htons for endiannes
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    // connect socket to the target address
    if (connect(sockfd,
                (struct sockaddr * ) & serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed. %s \n", strerror(errno));
        return 1;
    }

    int notwritten = N;

    // Write N from client to server
    bytes_sent = write(sockfd,
                       N_buff,
                       4);
    char * query = malloc(CHAR_MAX);
    int j = 0;
    while (fgets(send_buff, 1024, file) != NULL) {
        for (int i = 0; i < strlen(send_buff); i++) {
            query[j++] = send_buff[i];
        }
    }

    // Write message from client to server
    while (notwritten > 0) {
        // notwritten = how much we have left to write
        // totalsent  = how much we've written so far
        // nsent = how much we've written in last write() call */
        bytes_sent = write(sockfd,
                           query + totalsent,
                           notwritten);
        // check if error occured (client closed connection?)
        assert(bytes_sent >= 0);
        totalsent += bytes_sent;
        notwritten -= bytes_sent;
    }

    // Read response from the server
    while (true) {
        nread = read(sockfd,
                     serv_resp,
                     sizeof(serv_resp) - 1);
        if (nread > 0) break;
    }

    printf("# of printable characters: %u\n", serv_resp[0]);

    close(sockfd);
    fclose(file);
    return 0;
}