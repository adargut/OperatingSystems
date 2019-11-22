#include "os.h"
#include "string.h"
#include "limits.h"

void partition_vpn(uint64_t vpn, uint64_t *offsets) {
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
    uint64_t *node_pos = NULL;
    uint64_t curr_node = pt;

    uint64_t offsets[5], node_row;
    partition_vpn(vpn, offsets);

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
            curr_node = node_pos[offsets[depth]]; // Mapping exists, traverse to it
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
    partition_vpn(vpn, offsets);

    while (depth < 5) {
        node_pos = (uint64_t *)phys_to_virt(curr_node); // Traverse to entry
        node_row = node_pos[offsets[depth]]; // Move to relevant entry in page using 9 bits offset

        // Last level reached
        if (depth == 4) {
            // Format address correctly
            if (node_pos[offsets[depth]] != NO_MAPPING && node_pos[offsets[depth]] & 1 == 1) {
                return node_pos[offsets[depth]] >> 12;
            }
            else {
                return NO_MAPPING;
            }
        }
        // New frame needs to be allocated
        if (0 == (node_row & 1)) {
            return NO_MAPPING;
        }
        else {
            curr_node = node_pos[offsets[depth]]; // Mapping exists, traverse to it
        }
        depth++;
    }
}
