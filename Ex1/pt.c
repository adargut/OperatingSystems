#include "os.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "inttypes.h"
#include "string.h"

// Define a node in a trie
typedef struct trie_node
{
    uint64_t key; // 9 bit key by which we perform search
    uint64_t children [512]; // 2^9 possibilities for node children
    uint64_t ppn; // ppn mapping found in leaf node
    bool isLeaf; // flag to distinguish leaf nodes from inner nodes
} TrieNode;

// Store a list of all base addresses for pts
typedef struct base_list
{
    uint64_t base_addr;
    TrieNode *base_ptr;
    struct base_list *next;
} BaseList;

static BaseList *root_list;

// Util function to reverse string
char *strrev(char *str){
    char c, *front, *back;

    if(!str || !*str)
        return str;
    for(front=str,back=str+strlen(str)-1;front < back;front++,back--){
        c=*front;*front=*back;*back=c;
    }
    return str;
}

// Util function to convert decimal to binary
char *decToBinary(long n)
{
    long decimal, tempDecimal;
    char *binary = malloc(65);
    int index = 0;

    decimal = n;


    /* Copies decimal value to temp variable */
    tempDecimal = decimal;

    while(tempDecimal!=0)
    {
        /* Finds decimal%2 and adds to the binary value */
        binary[index] = (tempDecimal % 2) + '0';

        tempDecimal /= 2;
        index++;
    }
    binary[index] = '\0';

    /* Reverse the binary value found */
    strrev(binary);

    printf("\nDecimal value = %ld\n", decimal);
    printf("Binary value of decimal = %s\n", binary);

    return binary;
}


// Util function to fetch root address from list of roots
TrieNode *getRoot(uint64_t root_addr) {
    BaseList *head = root_list;

    while (head != NULL) {
        if (head->base_addr == root_addr) {
            return head->base_ptr;
        }
        head = head->next;
    }
    fprintf(stderr, "root for base page table register not found");
}

void addRoot(uint64_t root_addr) {
    BaseList *head = root_list;

    while (head->next != NULL) {
        head = head->next;
    }
//    head->next = malloc(sizeof())
}

// Create or destroy a mapping in the page table
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    printf("hello\n");
    printf("\n");
}

// Query a vpn from page table: return ppn if exists, NO_MAPPING else
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    char vpn_buff[45], *ptr;
    int i = 0;
//    sprintf(vpn_buff, "%" PRIu64, vpn);
//    printf("got bin vpn %s\n", vpn_buff);
//    uint64_t x = strtol(vpn_buff, &ptr, 10);
//    printf("x is now %lx\n", x);
    ptr = decToBinary(vpn);
    printf("res is %s\n", ptr);
    printf("length of res is %d\n", strlen(ptr));
    while (i <= 44) {
        vpn_buff[i++] = '0';
    }
    vpn_buff[45] = '\0';
    vpn_buff[0] = '0';
    printf("vpn buff is %s\n", vpn_buff);
    printf("vpn[0] is %c\n", vpn_buff[0]);
    int start_idx = strlen(vpn_buff) - strlen(ptr);
    printf("start from %d\n", start_idx);
    int j = 0;
    for (int i = start_idx; i < 45; i++) {
        printf("hey, ptr is %d\n", ptr[j]);
        vpn_buff[i] = ptr[j];
        j++;
    }
}
