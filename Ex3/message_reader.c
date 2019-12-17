#include "message_slot.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    // Check correct amount of parameters given
    if (argc != 3){
        fprintf(stderr,"Incorrect number of parameters given\n");
        exit(-1);
    }

    // Read params from command line
    char buffer[MSG_LEN];
    int target_id = atoi(argv[2]);
    int file = open(argv[1], O_RDWR);
    int valid, length;

    // Set channel id to target id
    valid = ioctl(file, MSG_SLOT_CHANNEL, target_id);
    if (valid)
    {
        printf("Error occurred during ioctl in reader: %s\n", strerror(errno));
        exit(-1);
    }

    // Read message from channel
    length = read(file, buffer, MSG_LEN);
    // Check if any bytes were read
    if (length < 0) {
        printf("Error: failed to read message, %s\n", strerror(errno));
        exit(-1);
    }
    // Check for buffer overflow
    if (length > MSG_LEN) {
        fprintf(stderr, "Error: message is too long for buffer\n");
        exit(-1);
    }
    // Close file
    close(file);
    // Write message to stdout
    valid = write(STDOUT_FILENO, buffer, length);
    if (valid < -1) {
        printf("Error: failed to print message to stdout, %s\n", strerror(errno));
    }
    return SUCCESS;
}