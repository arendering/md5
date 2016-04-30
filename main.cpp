#include <iostream>
#include "bitset_arithmetic.hpp"
#include "md5.hpp"

int main()
{
    const unsigned int passwd_len = 8;
    const unsigned int byte = 8;
    const unsigned int bits_len = 512;
    
    std::string inp("abcdefgh");
    std::bitset<passwd_len * byte> inp_bitset = GetBitsetFromString<passwd_len * byte>(inp);
    std::bitset<bits_len> outp = Adjustment<passwd_len * byte>(inp_bitset);
    std::cout << "Prehash string hex: ";
    std::string out_string = outp.to_string();
    PrintInHex(out_string);
    Hash<bits_len>(outp);
    
    return 0;
}
