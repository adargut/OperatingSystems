#include "message_slot.h"
#include "message_slot.c"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
    // Read params from command line
    int target_id = atoi(argv[2]);
    char *file_path = argv[1], *the_message = argv[3];
    char buffer[MSG_LEN];
    int file = fopen(file_path, O_RDWR);
    int bytes_read = read(file, buffer, MSG_LEN);

    // Set channel id to target id
    ioctl(file, MSG_SLOT_CHANNEL, target_id);
    // Check if any bytes were read
    if (bytes_read < 0) {
        printf("No message to read...\n");
        exit(EXIT_FAILURE);
    }
    if (bytes_read > MSG_LEN) {
        printf("Message is too long for buffer...\n")
        exit(EXIT_FAILURE);
    }
    // Set terminating nul character
    if (bytes_read < MSG_LEN) {
        buffer[bytes_read] = '\0';
    }
    close(file);
    return SUCCESS;
}