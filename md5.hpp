#ifndef MD5_HPP
#define MD5_HPP

#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include "bitset_arithmetic.hpp"


static const unsigned int prelength = 448;
static const unsigned int length = 512;
static const unsigned int word = 32;

enum funcs {F_func, G_func, H_func, I_func};

std::bitset<64> GetLengthBitset(long long len)
{
    std::bitset<8> bs_arr[8];
    for(int i = 0; i < 8; ++i) {
        unsigned char ch = (unsigned char)(len >> i * 8);
        std::bitset<8> tmp_bs(ch);
        bs_arr[i] = tmp_bs;
    }

    std::string ret_str(64, '0');
    for (int i = 0; i < 8; i++) {
        std::string tmp = bs_arr[i].to_string();
        std::copy(tmp.begin(), tmp.end(), ret_str.begin() + 8 * i);
    }
    std::bitset<64> bs_ret(ret_str);
    return bs_ret;
}

template<unsigned int N>
std::bitset<N> SwapHalfs(std::bitset<N> bs)
{
    std::string tmp = bs.to_string();
    std::string ret_str(N, '0');
    std::copy(tmp.begin() + N / 2, tmp.end(), ret_str.begin());
    std::copy(tmp.begin(), tmp.begin() + N / 2, ret_str.begin() + N / 2);
    std::bitset<N> ret_bs(ret_str);
    return ret_bs;
}

void PrintInHex(std::string bit_string)
{
    int str_len = bit_string.length();
    for(int i = 0; i < str_len / 4; ++i) {
        std::string tmp_str(4, '0');
        std::copy(bit_string.begin() + i*4, bit_string.begin() + 4 + i*4, tmp_str.begin());
        std::bitset<4> tmp_bs(tmp_str);
        std::cout << std::hex << tmp_bs.to_ulong();
    }
    std::cout << std::endl;
}

// 1st and 2nd steps: Adjustment
template<unsigned int N>
std::bitset<length> Adjustment(std::bitset<N> bs)
{
    // 1
    std::bitset<prelength> pre_val(bs.to_string());
    pre_val <<= prelength - N;
    std::string pre_val_string = pre_val.to_string();
    pre_val_string[N] = '1';
    // 2
    std::bitset<length> ret_val(pre_val_string);
    ret_val <<= length - prelength;

    const long long len = 64; // because input string have 8 symbols
    std::bitset<64> tail_bs = GetLengthBitset(len);
    std::bitset<length> extended_tail_bs(tail_bs.to_string());
    ret_val |= extended_tail_bs;
    return ret_val;
}

std::bitset<word> F(std::bitset<word> x, std::bitset<word> y, std::bitset<word> z)
{
    return (x & y) | (~x & z);
}

std::bitset<word> G(std::bitset<word> x, std::bitset<word> y, std::bitset<word> z)
{
    return (x & z) | (y & ~z);
}

std::bitset<word> H(std::bitset<word> x, std::bitset<word> y, std::bitset<word> z)
{
    return x ^ y ^ z;
}

std::bitset<word> I(std::bitset<word> x, std::bitset<word> y, std::bitset<word> z)
{
    return y ^ (x | ~z);
}

std::bitset<word> ReplaceX(std::bitset<word> X)
{
    std::string X_string = X.to_string();
    std::string a(8, '0');
    std::copy(X_string.begin(), X_string.begin() + 8, a.begin());
    std::string b(8, '0');
    std::copy(X_string.begin() + 8, X_string.begin() + 16, b.begin());
    std::string c(8, '0');
    std::copy(X_string.begin() + 16, X_string.begin() + 24, c.begin());
    std::string d(8, '0');
    std::copy(X_string.begin() + 24, X_string.end(), d.begin());
    std::string new_X_string = d + c + b + a;
    std::bitset<word> new_X(new_X_string);
    return new_X;

}

void Set(std::bitset<word> &A, std::bitset<word> B, std::bitset<word> C, std::bitset<word> D,
         std::bitset<word> * X, std::bitset<word> * T, unsigned int k, unsigned int s, unsigned int i, funcs foo)
{
    std::bitset<word> func_tmp("0");
    if(foo == F_func) 
        func_tmp = F(B, C, D);
    if(foo == G_func) 
        func_tmp = G(B, C, D);
    if(foo == H_func) 
        func_tmp = H(B, C, D);
    if(foo == I_func) 
        func_tmp = I(B, C, D);

    BitsetAdd<word>(A, func_tmp);
    /*
    std::bitset<word> new_X = ReplaceX(X[k]);
    if(k == 1) {
        std::cout << "X[" << k << "]:";
        PrintInHex(new_X.to_string());
    }
    BitsetAdd<word>(A, new_X);
    */
    BitsetAdd<word>(A, X[k]);
    BitsetAdd<word>(A, T[i]);
    RotLeft<word>(A, s);
    BitsetAdd<word>(A, B);
}

void Print_ABCD(std::bitset<word> a, std::bitset<word> b, std::bitset<word> c, std::bitset<word> d)
{
    std::cout << "a: ";
    PrintInHex(a.to_string());
    std::cout << "b: ";
    PrintInHex(b.to_string());
    std::cout << "c: ";
    PrintInHex(c.to_string());
    std::cout << "d: ";
    PrintInHex(d.to_string());

}

template<unsigned int length>
void Hash(std::bitset<length> bs)
{
    std::bitset<word> A(0x67452301);
    std::bitset<word> B(0xEFCDAB89);
    std::bitset<word> C(0x98BADCFE);
    std::bitset<word> D(0x10325476);

    // Initialize T array
    const unsigned int size = 64;
    unsigned long pre_T[size];
    for(unsigned int i = 0; i < size; ++i) {
        pre_T[i] = (unsigned long)(4294967296.0 * std::fabs(std::sin(double(i) + 1)));
    }
    std::bitset<word> T[size];
    for(unsigned int i = 0; i < size; ++i) {
        T[i] = std::bitset<word>(pre_T[i]);
    }

    // Initialize X array
    std::bitset<word> X[16];
    std::string tmp_bs = bs.to_string();
    for(unsigned int i = 0; i < 16; ++i) {
        std::string buffer(word, '0'); 
        std::copy(tmp_bs.begin() + i * word, tmp_bs.begin() + word + i * word, buffer.begin());
        std::bitset<word> before_reverse(buffer);
        X[i] = ReplaceX(before_reverse);
    }
    
    std::bitset<word> AA(A.to_string());
    std::bitset<word> BB(B.to_string());
    std::bitset<word> CC(C.to_string());
    std::bitset<word> DD(D.to_string());

    funcs f_func = F_func;
    funcs g_func = G_func;
    funcs h_func = H_func;
    funcs i_func = I_func;

    /*
    Print_ABCD(A,B,C,D);
    std::cout << "X[0]:";
    PrintInHex(X[0].to_string());
    std::cout << "T[1]:";
    PrintInHex(T[1].to_string());
    */

    // 1st round
    // [abcd k s i] --> a = b + (( a + F(b,c,d) + X[k] + T[i]) <<< s)    
    Set(A, B, C, D, X, T, 0, 7, 0, f_func);
    Set(D, A, B, C, X, T, 1, 12, 1, f_func);
    Set(C, D, A, B, X, T, 2, 17, 2, f_func);
    Set(B, C, D, A, X, T, 3, 22, 3, f_func);
    Set(A, B, C, D, X, T, 4, 7, 4, f_func);
    Set(D, A, B, C, X, T, 5, 12, 5, f_func);
    Set(C, D, A, B, X, T, 6, 17, 6, f_func);
    Set(B, C, D, A, X, T, 7, 22, 7, f_func);
    Set(A, B, C, D, X, T, 8, 7, 8, f_func);
    Set(D, A, B, C, X, T, 9, 12, 9, f_func);
    Set(C, D, A, B, X, T, 10, 17, 10, f_func);
    Set(B, C, D, A, X, T, 11, 22, 11, f_func);
    Set(A, B, C, D, X, T, 12, 7, 12, f_func);
    Set(D, A, B, C, X, T, 13, 12, 13, f_func);
    Set(C, D, A, B, X, T, 14, 17, 14, f_func);
    Set(B, C, D, A, X, T, 15, 22, 15, f_func);

    // 2nd round
    // [abcd k s i] --> a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s)
    Set(A, B, C, D, X, T, 1, 5, 16, g_func);
    Set(D, A, B, C, X, T, 6, 9, 17, g_func);
    Set(C, D, A, B, X, T, 11, 14, 18, g_func);
    Set(B, C, D, A, X, T, 0, 20, 19, g_func);
    Set(A, B, C, D, X, T, 5, 5, 20, g_func);
    Set(D, A, B, C, X, T, 10, 9, 21, g_func);
    Set(C, D, A, B, X, T, 15, 14, 22, g_func);
    Set(B, C, D, A, X, T, 4, 20, 23, g_func);
    Set(A, B, C, D, X, T, 9, 5, 24, g_func);
    Set(D, A, B, C, X, T, 14, 9, 25, g_func);
    Set(C, D, A, B, X, T, 3, 14, 26, g_func);
    Set(B, C, D, A, X, T, 8, 20, 27, g_func);
    Set(A, B, C, D, X, T, 13, 5, 28, g_func);
    Set(D, A, B, C, X, T, 2, 9, 29, g_func);
    Set(C, D, A, B, X, T, 7, 14, 30, g_func);
    Set(B, C, D, A, X, T, 12, 20, 31, g_func);

    // 3rd round
    // [abcd k s i] --> a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s)
    Set(A, B, C, D, X, T, 5, 4, 32, h_func);
    Set(D, A, B, C, X, T, 8, 11, 33, h_func);
    Set(C, D, A, B, X, T, 11, 16, 34, h_func);
    Set(B, C, D, A, X, T, 14, 23, 35, h_func);
    Set(A, B, C, D, X, T, 1, 4, 36, h_func);
    Set(D, A, B, C, X, T, 4, 11, 37, h_func);
    Set(C, D, A, B, X, T, 7, 16, 38, h_func);
    Set(B, C, D, A, X, T, 10, 23, 39, h_func);
    Set(A, B, C, D, X, T, 13, 4, 40, h_func);
    Set(D, A, B, C, X, T, 0, 11, 41, h_func);
    Set(C, D, A, B, X, T, 3, 16, 42, h_func);
    Set(B, C, D, A, X, T, 6, 23, 43, h_func);
    Set(A, B, C, D, X, T, 9, 4, 44, h_func);
    Set(D, A, B, C, X, T, 12, 11, 45, h_func);
    Set(C, D, A, B, X, T, 15, 16, 46, h_func);
    Set(B, C, D, A, X, T, 2, 23, 47, h_func);

    // 4th round
    // [abcd k s i] --> a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s
    Set(A, B, C, D, X, T, 0, 6, 48, i_func);
    Set(D, A, B, C, X, T, 7, 10, 49, i_func);
    Set(C, D, A, B, X, T, 14, 15, 50, i_func);
    Set(B, C, D, A, X, T, 5, 21, 51, i_func);
    Set(A, B, C, D, X, T, 12, 6, 52, i_func);
    Set(D, A, B, C, X, T, 3, 10, 53, i_func);
    Set(C, D, A, B, X, T, 10, 15, 54, i_func);
    Set(B, C, D, A, X, T, 1, 21, 55, i_func);
    Set(A, B, C, D, X, T, 8, 6, 56, i_func);
    Set(D, A, B, C, X, T, 15, 10, 57, i_func);
    Set(C, D, A, B, X, T, 6, 15, 58, i_func);
    Set(B, C, D, A, X, T, 13, 21, 59, i_func);
    Set(A, B, C, D, X, T, 4, 6, 60, i_func);
    Set(D, A, B, C, X, T, 11, 10, 61, i_func);
    Set(C, D, A, B, X, T, 2, 15, 62, i_func);
    Set(B, C, D, A, X, T, 9, 21, 63, i_func);

    BitsetAdd<word>(A, AA);
    BitsetAdd<word>(B, BB);
    BitsetAdd<word>(C, CC);
    BitsetAdd<word>(D, DD);
    Print_ABCD(A,B,C,D);

    
    std::bitset<word> A_replace = ReplaceX(A);
    std::bitset<word> B_replace = ReplaceX(B);
    std::bitset<word> C_replace = ReplaceX(C);
    std::bitset<word> D_replace = ReplaceX(D);

    std::string A_str = A_replace.to_string();
    std::string B_str = B_replace.to_string();
    std::string C_str = C_replace.to_string();
    std::string D_str = D_replace.to_string();
    std::string answer_str = A_str + B_str + C_str + D_str;
    /*
    std::copy(A_str.begin(), A_str.end(), answer_str.begin());
    std::copy(B_str.begin(), B_str.end(), answer_str.begin() + word);
    std::copy(C_str.begin(), C_str.end(), answer_str.begin() + word * 2);
    std::copy(D_str.begin(), D_str.end(), answer_str.begin() + word * 3);
    */


    PrintInHex(answer_str);
}

#endif
