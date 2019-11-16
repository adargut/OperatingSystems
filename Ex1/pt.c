#include "os.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "inttypes.h"
#include "string.h"
#include "limits.h"


void print_bin(uint64_t integer)
{
    int i = CHAR_BIT * sizeof (uint64_t); /* however many bits are in an integer */
    while(i--) {
        putchar('0' + ((integer >> i) & 1));
    }
}

void createOffsets(uint64_t vpn, uint64_t *offsets) {
    offsets[0] = (vpn >> 36) & 0x1ff;
    offsets[1] = (vpn >> 27) & 0x1ff;
    offsets[2] = (vpn >> 18) & 0x1ff;
    offsets[3] = (vpn >> 9) & 0x1ff;
    offsets[4] = (vpn) & 0x1ff;
}

// Create or destroy a mapping in the page table
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    int depth = 0;
    uint64_t curr_node = pt;

    uint64_t offsets[5], *node_data;
    createOffsets(vpn, offsets);

    while (depth < 5) {
        printf("depth is now %d\n", depth);
        curr_node = curr_node + offsets[depth++];
        node_data = (uint64_t *)(phys_to_virt(curr_node));

        curr_node = *node_data;
        // Check out valid bit
        if (node_data == NULL || 0 == (curr_node & 1)) {
            curr_node = alloc_page_frame();
        }
        curr_node |= 1;
    }

    // Set data for node as ppn
     *node_data = ppn  | 1;
    printf("node data is now %lx\n", *node_data);
}


// Query a vpn from page table: return ppn if exists, NO_MAPPING else
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    int depth = 0;
    uint64_t curr_node = pt;

    uint64_t offsets[5];
    createOffsets(vpn, offsets);

    while (depth < 5) {
        curr_node = curr_node + offsets[depth++];
        uint64_t *node_data = (uint64_t *)(phys_to_virt(curr_node));

        curr_node = *node_data;
        // Check out valid bit
        if (0 == (curr_node & 1)) {
            printf("in here \n");
            return NO_MAPPING;
        }
    }
    return curr_node;
}
