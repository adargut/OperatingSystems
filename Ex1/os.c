
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

#include "math.h"

/* 2^20 pages ought to be enough for anybody */
#define NPAGES	(1024*1024)

static void* pages[NPAGES];

uint64_t alloc_page_frame(void)
{
    static uint64_t nalloc;
    uint64_t ppn;
    void* va;

    if (nalloc == NPAGES)
        errx(1, "out of physical memory");

    /* OS memory management isn't really this simple */
    ppn = nalloc;
    nalloc++;

    va = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (va == MAP_FAILED)
        err(1, "mmap failed");

    pages[ppn] = va;
    return ppn;
}

void* phys_to_virt(uint64_t phys_addr)
{
    uint64_t ppn = phys_addr >> 12;
    uint64_t off = phys_addr & 0xfff;
    void* va = NULL;

    if (ppn < NPAGES)
        va = pages[ppn] + off;

    return va;
}

uint64_t get_random_vpn() {
    return rand() & 0x1FFFFFFFFFFF; // 45 bits
}

uint64_t get_random_ppn() {
    return rand() & 0xFFFFFFFFFFFFF; // 13 'F's
}

void update_and_check(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    page_table_update(pt, vpn, ppn);
    assert(page_table_query(pt, vpn) == ppn);
}

void perform_random_move(uint64_t pt) {
    uint64_t vpn = get_random_vpn();
    uint64_t ppn = get_random_ppn();

    if (rand() % 10 < 3)
        ppn = NO_MAPPING;

    update_and_check(pt, vpn, ppn);
}

int main(int argc, char **argv)
{
    uint64_t pt = alloc_page_frame();

    assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
    page_table_update(pt, 0xcafe, 0xf00d);
    assert(page_table_query(pt, 0xcafe) == 0xf00d);
    page_table_update(pt, 0xcafe, 0xfaab);
    assert(page_table_query(pt, 0xcafe) == 0xfaab);

    for (int i = 0; i < pow(2, 15); i++)
        perform_random_move(pt);

    printf("finished testing\n");

    return 0;
}

