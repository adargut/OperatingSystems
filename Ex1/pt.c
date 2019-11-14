#include "os.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "inttypes.h"

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

// Util function to convert Hex address to binary


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
    char vpn_buff[20];
    sprintf(vpn_buff, "%" PRIu64, vpn);
    printf("got bin vpn %s\n", vpn_buff);
//    HexToBin((char *)vpn);
//    printf("got vpn %lx\n", vpn);
}
