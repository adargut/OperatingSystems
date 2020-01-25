#include "chardev.h"    

#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>

int main()
{
  int file_desc;
  int ret_val;

  file_desc = open( "/dev/"DEVICE_FILE_NAME, O_RDWR );
  if( file_desc < 0 ) 
  {
    printf ("Can't open device file: %s\n", 
            DEVICE_FILE_NAME);
    exit(-1);
  }

  ret_val = ioctl( file_desc, IOCTL_SET_ENC, 0);
  ret_val = write( file_desc, "Hello", 5);
  ret_val = read(  file_desc, NULL, 100 );
  ret_val = ioctl( file_desc, IOCTL_SET_ENC, 1);
  ret_val = write( file_desc, "Hello", 5);
  ret_val = read(  file_desc, NULL, 100 );
 
  close(file_desc); 
  return 0;
}
