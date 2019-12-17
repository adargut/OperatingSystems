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
    if (argc != 4) {
        fprintf(stderr, "Incorrect count of params given to sender\n");
        exit(-1);
    }

    // Read params from command line
    char *message = argv[3];
    int target_id = atoi(argv[2]), valid;
    int file = open(argv[1], O_RDWR);

    // Check for errors while opening file
    if (file < 0){
        printf("Error while opening device file, %s\n", strerror(errno));
        exit(-1);
    }

    // Set channel id to target_id from command line
    valid = ioctl(file, MSG_SLOT_CHANNEL, target_id);
    if (valid)
    {
        printf("Error occurred during ioctl in sender, %s\n", strerror(errno));
        exit(-1);
    }
    // Write message to file
    int length;
    length = write(file, message, strlen(message));
    // Check if message was actually written
    if (length < 0){
        printf("Error: no bytes written, %s\n", strerror(errno));
        exit(-1);
    }
    close(file); // Close the device file
    return SUCCESS;
}
