#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>
#include <stdbool.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

typedef struct message_channel
{
    int channel_id;
    char the_message[MSG_LEN];
    bool msg_exists;
    int msg_length;
    struct message_channel *next_channel;
} channel;

// Definition for message slot, a linked list
typedef struct message_slot
{
    int minor_num;
    channel *channels;
} slot;

// Number of devices files bounded by 256
static slot *devices_files[256] = {NULL};

// Find channel by channel id from slot
static channel *find_channel(int channel_id, slot *the_slot) {
    channel *curr_channel = the_slot->channels;
    while (curr_channel != NULL) {
        // Check if we found necessary channel
        if (channel_id == curr_channel->channel_id) return curr_channel;
        curr_channel = curr_channel->next_channel;
    }
    return NULL;
}


// Current channel we're using
static int the_channel;

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
    slot *open_slot;
    // Minor number is computed using inode
    int minor_num = iminor(inode);
    // Check if minor number exists in array of minors
    if (NULL == devices_files[minor_num]) {
        // No channel was set yet, start as -100
        file->private_data = (void *)-100;
        // Minor does not exist in array, need to allocate
        open_slot = kmalloc(sizeof(struct message_slot), GFP_KERNEL);
        // Set its minor number to minor we have
        open_slot->minor_num = minor_num;
        // Its channels start out as null
        open_slot->channels = NULL;
        // Place the new slot in the array of slots
        devices_files[minor_num] = open_slot;
    }
    return SUCCESS;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
    // Check for errors
    if (ioctl_param == 0 || file == NULL){
        return -EINVAL;
    }
    // Set channel id to ioctl parameter
    file->private_data = (void *)ioctl_param;
    return SUCCESS;
}

//---------------------------------------------------------------
// Read a message from a channel
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
    int channel_id, minor, i;
    slot *slot;
    struct message_channel *ch;
    if (buffer == NULL || file == NULL){ // Check for errors
        return -EINVAL;
    }
    channel_id = (int)file->private_data;
    if (-100 == channel_id){ // No channel set yet
        return -EINVAL;
    }
    minor = iminor(file_inode(file));
    slot = devices_files[minor];
    if(NULL == slot) { // Slot not allocated yet in array
        return -EFAULT;
    }
    // Search for channel in message slot
    ch = find_channel(channel_id, slot);
    if (NULL == ch){ // Channel not allocated yet in slot
        return -EFAULT;
    }
    if (length < ch->msg_length){ // Message is longer than buffer
        return -ENOSPC;
    }
    if (!(ch->msg_exists)){ // Message does not exist in channel
        return -EWOULDBLOCK;
    }
    i = 0;
    while (i < length && i < MSG_LEN && i < ch->msg_length) { // Read message
        if (put_user(ch->the_message[i], &buffer[i])){
            return -EFAULT;
        }
        i++;
    }
    return i;
}

//---------------------------------------------------------------
