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
static slot *devices_files[MAX_MINORS] = {NULL};

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

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
    slot *open_slot;
    // Check if minor number exists in array of minors
    if (NULL == devices_files[iminor(inode)]) {
        // No channel was set yet, start as -100
        file->private_data = (void *)-100;
        // Minor does not exist in array, need to allocate
        open_slot = kmalloc(sizeof(struct message_slot), GFP_KERNEL);
        // Set its minor number to minor we have
        open_slot->minor_num = iminor(inode);
        // Its channels start out as null
        open_slot->channels = NULL;
        // Place the new slot in the array of slots
        devices_files[iminor(inode)] = open_slot;
    }
    return SUCCESS;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
    // Check for errors
    if (0 == ioctl_param || NULL == file || ioctl_command_id != MSG_SLOT_CHANNEL){
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
    slot *curr_slot;
    struct message_channel *ch;
    if (buffer == NULL || file == NULL){ // Check for errors
        return -EINVAL;
    }
    channel_id = (uintptr_t)file->private_data;
    if (-100 == channel_id){ // No channel set yet
        return -EINVAL;
    }
    minor = iminor(file_inode(file));
    curr_slot = devices_files[minor];
    if(NULL == curr_slot) { // Slot not allocated yet in array
        return -EFAULT;
    }
    // Search for channel in message slot
    ch = find_channel(channel_id, curr_slot);
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
// Write a message to a channel
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
    int channel_id, i;
    channel *ch;
    slot *slot;
    // Check for errors
    if (buffer == NULL || file == NULL){
        return -EINVAL;
    }
    channel_id = (uintptr_t)file->private_data;
    if (-100 == channel_id) {
        return -EINVAL;
    }
    slot = devices_files[iminor(file_inode(file))];
    if (NULL == slot) { // Slot not allocated yet
        return -EFAULT;
    }
    // Search for channel in message slot
    ch = find_channel(channel_id, slot);
    if (NULL == ch) {
        // Channel not allocated yet, insert it
        ch = kmalloc(sizeof(channel), GFP_KERNEL);
        ch->channel_id = channel_id;
        ch->msg_length = -100;
        ch->msg_exists = false;
        ch->next_channel = slot->channels;
        slot->channels = ch;
    }
    if (ch->msg_exists) {
        // Remove existing message
        memset(&ch->the_message[0], 0, sizeof(ch->the_message));
        ch->msg_length = -100;
    }
    i = 0;
    // Write message to channel
    while (i < length && i < MSG_LEN) {
        // Check for error while writing message
        if (get_user(ch->the_message[i], &buffer[i])){
            return -EFAULT;
        }
        i++;
    }
    ch->msg_exists = true;
    ch->msg_length = length;
    // Return number of bytes written
    return i;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops =
        {
                .unlocked_ioctl = device_ioctl,
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .owner          = THIS_MODULE,
        };

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
    // Register the driver
    int rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
    // Check for errors
    if( rc < 0 )
    {
        printk( KERN_ERR "%s failed to register for  %d\n", DEVICE_FILE_NAME, MAJOR_NUM );
        return rc;
    }
    printk(KERN_INFO "message_slot: registered major number %d\n", MAJOR_NUM);
    return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
    channel *head;
    channel *next;
    int i = 0;
    // Free memory allocated
    for (; i < MAX_MINORS; i++) {
        if (NULL != devices_files[i]) {
            head = devices_files[i]->channels;
            while (NULL != head) {
                next = head->next_channel;
                kfree(head);
                head = next;
            }
            kfree(devices_files[i]);
        }
    }
    // Unregister the char device
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);
//========================= END OF FILE =========================
