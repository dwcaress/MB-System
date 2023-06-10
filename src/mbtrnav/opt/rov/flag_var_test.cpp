#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <iomanip>
#include "flag_utils.hpp"

using namespace std;

static void test_flag()
{
    flag_var<uint32_t> x(0xcafebabe);
    assert(x == 0xcafebabe);

    x = 0xcafedead;
    assert(x == 0xcafedead);

    x >>= 16;
    assert(x == (uint32_t)0xcafe);

    x <<= 16;
    assert(x == 0xcafe0000);

    x &= 0x0;
    assert(x == (uint32_t)0);

    x = 0xcafe0000;
    assert(x == 0xcafe0000);

    x |= 0xabcd;
    assert(x == 0xcafeabcd);

    assert( (x >> 16) == (uint32_t)0xcafe);
    assert(x > (uint32_t)123);
    assert(x < 3.5e9);

    flag_var<uint32_t> y(0x3);
    assert(y == (uint32_t)0x3);

    y = x;
    assert(y == x);

    x = 0x1234abcd;
    y = 0xffff0000;
    y |= x;
    assert(y == 0xffffabcd);

    x = 0x1234abcd;
    y = 0xffff0000;
    y &= x;
    assert(y == (uint32_t)0x12340000);
    assert(y != x);
    assert(y < x);
    assert(x > y);

    std::cerr << std::hex << std::setw(8) << std::setfill('0');
    std::cerr << "       x : " << x <<"\n";
    std::cerr << "  x << 4 : " << (x << 4) <<"\n";
    std::cerr << "  x >> 8 : " << (x >> 8) <<"\n";
    std::cerr << "x | 0xff : " << (x | 0xff) <<"\n";
    std::cerr << "x & 0xff : " << (x & 0xff) <<"\n";

    flag_var<uint8_t> wc;
    flag_var<uint8_t> zc(0x3);
    assert(wc == 0);
    assert(zc == 0x3);

    zc = wc;
    assert(zc == wc);

    wc = 0x12;
    zc = 0xf0;
    zc |= wc;
    assert(zc == 0xf2);

    wc = 0x12;
    zc = 0xf0;
    zc &= wc;
    assert(zc == 0x10);
    assert(zc != wc);
    assert(zc < wc);
    assert(wc > zc);

    flag_var<uint16_t> wh;
    flag_var<uint16_t> zh(0x1234);
    assert(wh == 0);
    assert(zh == 0x1234);

    zh = wh;
    assert(zh == wh);

    wh = 0x12;
    zh = 0xf0;
    zh |= wh;
    assert(zh == 0xf2);

    wh = 0x12;
    zh = 0xf0;
    zh &= wh;
    assert(zh == 0x10);
    assert(zh != wh);
    assert(zh < wh);
    assert(wh > zh);

}

static void test_ref(int &x)
{
    x++;
    return;
}

int main(int argc, char **argv)
{

    test_flag();

    int x = 1;
    std::cerr << "before " << x;
    test_ref(x);
    std::cerr << " after " << x << "\n";

    return 0;

}
