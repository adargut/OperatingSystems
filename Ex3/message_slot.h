// We use hard coded major number set to 240
#define MAJOR_NUM 240
#define MAX_MINORS 256
#define SUCCESS 0
#define MSG_LEN 128
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "my_char_dev"