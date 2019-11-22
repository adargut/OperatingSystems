
#define _GNU_SOURCE

#include <assert.h>
#include <time.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

#include "math.h"

/* 2^20 pages ought to be enough for anybody */
#define NPAGES	(1024*1024)

#define VPN_MASK 0x1FFFFFFFFFFF
#define PPN_MASK 0xFFFFFFFFFFFFF

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

// TESTS FUNCTIONS

void assert_equal(uint64_t received, uint64_t expected) {
    static int counter = 0;

    if (expected != received) {
        printf("\n\033[0;31mFailed test!\nExpected \033[0m\033[0;32m%llX\033[0m\033[0;31m and received \033[0m\033[0;33m%llX\033[0m\033[0;31m.\033[0m\n", expected, received);

        void* callstack[128];
        int i, frames = backtrace(callstack, 128);
        char** strs = backtrace_symbols(callstack, frames);
        printf("\033[0;36m(Almost readable) stacktrace\n");
        for (i = 0; i < frames; ++i) {
            printf("%s\n", strs[i]);
        }
        printf("\033[0m\n");
        free(strs);

        assert(0);
    }

    if (counter % 500 == 0)
        printf("\033[0;32m.\033[0m");
    counter++;
}

uint64_t get_random(uint64_t mask) {
    return rand() & mask;
}


int in_array(uint64_t *arr, int size, uint64_t value) {
    for (int i = 0; i < size; i++)
        if (arr[i] == value)
            return 1;
    return 0;
}


void get_random_list(uint64_t **out, int size, uint64_t mask) {
    *out = calloc(size, sizeof(uint64_t));
    uint64_t *arr = *out;
    int count = 0;
    uint64_t val;

    while (count < size) {
        val = get_random(mask);

        if (!in_array(arr, count, val)) {
            arr[count] = val;
            count++;
        }
    }
}

uint64_t get_random_vpn() {
    return get_random(VPN_MASK);
}

uint64_t get_random_ppn() {
    return get_random(VPN_MASK);
}

void update_random_and_check(uint64_t pt) {
    uint64_t vpn = get_random_vpn();
    uint64_t ppn = get_random_ppn();

    if (rand() % 10 < 3)
        ppn = NO_MAPPING;

    page_table_update(pt, vpn, ppn);
    assert_equal(page_table_query(pt, vpn), ppn);
}

void update_many_with_prefix(uint64_t pt) {
    int prefix = (rand() % 45) + 1;
    uint64_t mask = pow(2, prefix + 1) - 1;
    uint64_t vpn_mask = pow(2, (45 - prefix) + 1) - 1;
    int amount = (rand() % 20) + 2;

    if (amount > vpn_mask / 2)
        amount = vpn_mask / 2;

    uint64_t block = get_random(mask) << prefix;
    uint64_t* vpn_arr;
    uint64_t* ppn_arr = malloc(sizeof(uint64_t) * amount);

    get_random_list(&vpn_arr, amount, vpn_mask);
    for (int i = 0; i < amount; i++) {
        vpn_arr[i] = block + vpn_arr[i];
        ppn_arr[i] = get_random_ppn();

        page_table_update(pt, vpn_arr[i], ppn_arr[i]);
        assert_equal(page_table_query(pt, vpn_arr[i]), ppn_arr[i]);
    }

    for (int i = 0; i < amount; i++) {
        uint64_t value = page_table_query(pt, vpn_arr[i]);
        uint64_t expected = ppn_arr[i];
        if (value != expected) {
            printf("Set values:\n");
            for (int j = 0; j < amount; j++)
                printf("page_table[%llX] = %llX\n", vpn_arr[j], ppn_arr[j]);
            printf("\nFailed on index %d,\ngot value %llX instead of %llX\n", i, value, expected);
            assert(0);
        }
    }

    free(vpn_arr);
    free(ppn_arr);
}

void perform_random_move(uint64_t pt) {
    int option = rand() % 2;

    switch (option) {
        case 0:
            update_random_and_check(pt);
            break;
        case 1:
            update_many_with_prefix(pt);
            break;
    }

}

int main(int argc, char **argv)
{
    srand(time(NULL));
    uint64_t pt = alloc_page_frame();

    assert_equal(page_table_query(pt, 0xcafe), NO_MAPPING);
    page_table_update(pt, 0xcafe, 0xf00d);
    assert_equal(page_table_query(pt, 0xcafe), 0xf00d);
    page_table_update(pt, 0xcafe, NO_MAPPING);
    assert_equal(page_table_query(pt, 0xcafe), NO_MAPPING);

    for (int i = 0; i < pow(2, 14); i++) {
        perform_random_move(pt);
        printf("passed %d tests \n", i);
    }
    printf("\nAll tests passed!\n");

    return 0;
}

