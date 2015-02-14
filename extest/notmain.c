#define NULL	((void *)0)
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void PUT32 ( unsigned int, unsigned int );
unsigned int GET32 ( unsigned int );
extern void uart_init ( void );
extern void uart_puts(char *str);
extern void hexstring ( unsigned int d );
extern void hexstrings ( unsigned int d );
extern void start_l1cache ( void );
extern void stop_l1cache ( void );
extern void start_mmu ( unsigned int, unsigned int );
extern unsigned int LDREX(unsigned int addr);
extern unsigned int STREX(unsigned int addr, unsigned int val);
extern unsigned int EXTEST(unsigned int addr, unsigned int val);

#define MMUTABLEBASE 0x00100000
#define TEST_BASE    0x01000000
#define TEST_STEP	(1 << 20)

#define	PTE_C		0x00008
#define PTE_B		0x00004
#define PTE_S		0x10000
#define PTE_TEX1	0x01000
#define PTE_TEX2	0x02000
#define PTE_TEX3	0x03000

struct test {
	unsigned int prot;
	char *desc;
};

#define TEST(p)						\
{							\
	.prot = (p),					\
	.desc = #p "\n",				\
}

const struct test tests[] = {
	/* illumos */
	TEST(PTE_C | PTE_B | PTE_S | PTE_TEX1),

	/* original extest? */
	TEST(PTE_C | PTE_B),

	/* FreeBSD non-SMP */
	// TEST(0),					/* fails to test */
	// TEST(PTE_TEX2),				/* fails to store */
	TEST(PTE_B),
	TEST(PTE_TEX1),
	TEST(PTE_C),
	//TEST(PTE_C | PTE_B),				/* duplicate */
	TEST(PTE_C | PTE_B | PTE_TEX1),

	/* FreeBSD SMP */
	// TEST(0),					/* fails to test */
	// TEST(PTE_TEX2),				/* fails to store */
	// TEST(PTE_B),					/* duplicate */
	TEST(PTE_TEX1 | PTE_S),
	TEST(PTE_C | PTE_S),
	TEST(PTE_C | PTE_B | PTE_S),
	// TEST(PTE_C | PTE_B | PTE_S | PTE_TEX1),	/* duplicate */

	{ 0, NULL },
};

//-------------------------------------------------------------------
unsigned int mmu_section ( unsigned int add, unsigned int flags )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    ra=add>>20;
    rb=MMUTABLEBASE|(ra<<2);
    rc=(ra<<20)|flags|2;
    PUT32(rb,rc);
    return(0);
}
//-------------------------------------------------------------------------
static void dotest(char *desc, unsigned int addr)
{
    unsigned int ra,rb,rc;

    uart_puts(desc);
    ra=LDREX(addr);
    rb=STREX(addr, 0xABCD);
    rc=EXTEST(addr, 0x1234);
    hexstrings(addr); hexstrings(ra); hexstrings(rb); hexstring(rc);
}

static void test()
{
	unsigned int addr, i;

	for (i = 0; tests[i].desc; i++) {
		addr = TEST_BASE + (TEST_STEP * i);

		dotest(tests[i].desc, addr);
	}
}

static void init_vars()
{
	unsigned int addr, i;

	for (i = 0; tests[i].desc; i++) {
		addr = TEST_BASE + (TEST_STEP * i);

		hexstring(addr);
		PUT32(addr, addr | 0x1234);
	}
}

int notmain ( void )
{
    unsigned int ra;

    uart_init();

    uart_puts("extest\n");
    hexstring(0x12345678);

    //ram used by the stack and the program
    if(mmu_section(0x00000000, PTE_C | PTE_B)) return(1);
    //Memory mapped I/O used by the uart, etc, not cached
    if(mmu_section(0x20000000, 0x0000)) return(1);
    if(mmu_section(0x20100000, 0x0000)) return(1);
    if(mmu_section(0x20200000, 0x0000)) return(1);

    /* now map various test pages */
    for (ra = 0; tests[ra].desc; ra++) {
        unsigned int addr = TEST_BASE + (TEST_STEP * ra);

	uart_puts(tests[ra].desc);
	hexstrings(addr);
	hexstring(tests[ra].prot);

        if (mmu_section(addr, tests[ra].prot))
            return (1);
    }

    /**********/

    // not enabling data cache just yet.
    start_mmu(MMUTABLEBASE,0x00800001);
    hexstring(0x9abcdef0);

    init_vars();

    uart_puts("MMU on, cache off, try 1\n");
    test();
    uart_puts("MMU on, cache off, try 2\n");
    test();

    start_l1cache();
    // is this really necessary? GET32(0x00009000);
    uart_puts("MMU on, cache on, try 1\n");
    test();
    uart_puts("MMU on, cache on, try 2\n");
    test();
    stop_l1cache();

    uart_puts("MMU on, cache off, try 3\n");
    test();

    /**********/

    start_mmu(MMUTABLEBASE,0x00800009); /* enable W */
    hexstring(0x12345678);

    uart_puts("MMU on, cache off, W on, try 1\n");
    test();

    start_l1cache();
    // is this really necessary? GET32(0x00009000);
    uart_puts("MMU on, cache on, W on, try 1\n");
    test();
    stop_l1cache();

    uart_puts("MMU on, cache off, W on, try 2\n");
    test();

    hexstring(0xffffffff);
    return(0);
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------
