#include "message_slot.h"
#include <sys/ioctl.h>  /* ioctl */
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    // Read params from command line
    char *file_path = argv[1], message = argv[3];
    int target_id = atoi(argv[2]), written_bytes;
    int file = open(file_path, O_RDWR);

    // Check for errors while opening file
    if (file < 0){
        fprintf(stderr,"Error while opening device file);
        exit(-1);
    }
    // Set channel id to target_id from command line
    ioctl(file, _IOW(MAJOR_NUM, 0, unsigned int), target_id), target_id);)
    // Write message to file
    written_bytes = write(file, message, strlen(message));
    // Check if message was actually written
    if (written_bytes < 0){
        printf("No message was written: %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    close(file); // Close the device file
    return SUCCESS;
}
