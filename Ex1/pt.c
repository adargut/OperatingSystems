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
    // If vpn is already mapped to ppn, we first need to remove the mapping

    int depth = 0;
    uint64_t *node_pos = NULL;
    uint64_t curr_node = pt;

    uint64_t offsets[5], node_row;
    createOffsets(vpn, offsets);

    while (depth < 5) {
        node_pos = (uint64_t *)phys_to_virt(curr_node); // Traverse to entry
        node_row = node_pos[offsets[depth]]; // Move to relevant entry in page using 9 bits offset

        // Last level reached
        if (depth == 4) {
            // Format address correctly
            if (ppn != NO_MAPPING) {
                ppn <<= 12;
                ppn |= 1;
                // Set row to formatted address and return
                node_pos[offsets[depth]] = ppn;
            }
            else {
                printf("in here \n");
                node_pos[offsets[depth]] &= 0;
            }
            return;
        }

        // New frame needs to be allocated
        if (0 == (node_row & 1)) {
            uint64_t new_frame = alloc_page_frame();
            new_frame <<= 12; // Shift frame to correct position
            new_frame |= 1; // Set valid bit of frame to 1
            node_pos[offsets[depth]] = new_frame; // Put new frame in the correct row
            curr_node = new_frame; // Traverse to new frame
        }
        else {
            curr_node = node_pos[offsets[depth]] >> 12; // Mapping exists, traverse to it
        }
        depth++;
    }
}


// Query a vpn from page table: return ppn if exists, NO_MAPPING else
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    int depth = 0;
    uint64_t *node_pos = NULL;
    uint64_t curr_node = pt;

    uint64_t offsets[5], node_row;
    createOffsets(vpn, offsets);

    while (depth < 5) {

        printf("hello \n");
        printf("at depth %d\n", depth);
        node_pos = (uint64_t *)phys_to_virt(curr_node); // Traverse to entry
        printf("got entry %lx\n", node_pos);
        node_row = node_pos[offsets[depth]]; // Move to relevant entry in page using 9 bits offset

        // No mapping exists
        if (0 == (node_row & 1)) {
            return NO_MAPPING;
        }
        // Reached end, mapping exists
        else if (depth == 4) {
            return node_pos[offsets[depth]] >> 12;
        }
        // Need to traverse further
        else {
            curr_node = node_pos[offsets[depth]];
        }
        depth++;
    }
}
