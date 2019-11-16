#include "os.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "inttypes.h"
#include "string.h"
#include "limits.h"


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

void print_bin(uint64_t integer)
{
    int i = CHAR_BIT * sizeof (uint64_t); /* however many bits are in an integer */
    while(i--) {
        putchar('0' + ((integer >> i) & 1));
    }
}

// Create or destroy a mapping in the page table
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
//    void *base = pto_virt(pt);
    printf("at base");
}

/* Function to reverse bits of num */
uint64_t reverseBits(uint64_t num)
{
    unsigned int  NO_OF_BITS = sizeof(num) * 8;
    uint64_t reverse_num = 0, i, temp;

    for (i = 0; i < NO_OF_BITS; i++)
    {
        temp = (num & (1 << i));
        if(temp)
            reverse_num |= (1 << ((NO_OF_BITS - 1) - i));
    }

    return reverse_num;
}

// Query a vpn from page table: return ppn if exists, NO_MAPPING else
uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t mask = 0b0000000000000000000111111111000000000000000000000000000000000000;
    print_bin(mask);
    printf("\n");
    int depth = 0;
    uint64_t *node_ptr = (uint64_t *)phys_to_virt(pt);

    while (++depth < 5) {
        uint64_t page_offset = mask & vpn;

    }
}
