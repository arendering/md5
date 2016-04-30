#ifndef BITSET_ARITHMETIC_HPP
#define BITSET_ARITHMETIC_HPP

#include <stdexcept>
#include <bitset>

bool fullAdder(bool b1, bool b2, bool& carry)
{
    bool sum = (b1 ^ b2) ^ carry;
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
    return sum;
}

template<unsigned int N>
void BitsetAdd(std::bitset<N> &x, const std::bitset<N> &y)
{
    bool carry = false;
    for (size_t i = 0; i < N; ++i) {
        x[i] = fullAdder(x[i], y[i], carry);
    }
}

template<unsigned int N>
void BitsetSubtract(std::bitset<N> &x, const std::bitset<N> &y)
{
    bool borrow = false;
    for(int i = 0; i < N; ++i) {
        if(borrow) {
            if(x[i]) {
                x[i] = y[i];
                borrow = y[i];
            } else {
                x[i] = !y[i];
                borrow = true;
            }
        } else {
            if(x[i]) {
                x[i] = !y[i];
                borrow = false;
            } else {
                x[i] = y[i];
                borrow = y[i];
            }
        }
    }
}

template<unsigned int N>
void BitsetMultiply(std::bitset<N> &x, const std::bitset<N> &y)
{
    std::bitset<N> tmp = x;
    x.reset();
    if(tmp.count() < y.count()) {
        for(int i = 0; i < N; ++i) 
            if(tmp[i])
                BitsetAdd(x, y << i);
    } else {
        for(int i = 0; i < N; ++i) {
            if(y[i])
                BitsetAdd(x, tmp << i);
        }
    }
}

template<unsigned int N>
bool BitsetLt(const std::bitset<N> &x, const std::bitset<N> &y)
{
    for(int i = N - 1; i >= 0; --i) {
        if(x[i] && !y[i])
            return false;
        if(!x[i] && y[i])
            return true;
    }
    return false;
}

template<unsigned int N>
bool BitsetLtEq(const std::bitset<N> &x, const std::bitset<N> &y)
{
    for(int i = N - 1; i >= 0; --i) {
        if(x[i] && !y[i]) 
            return false;
        if(!x[i] && y[i])
            return true;
    }
    return true;
}

template<unsigned int N>
void BitsetDivide(std::bitset<N> x, std::bitset<N> y,
                  std::bitset<N> &q, std::bitset<N> &r)
{
    if(y.none())
        throw std::domain_error("division by zero!");
    q.reset();
    r.reset();
    if(x.none())
        return;
    if(x == y) {
        q[0] = 1;
        return;
    }
    r = x;
    if(BitsetLt(x, y))
        return;
    unsigned int sig_x;
    for(int i = N - 1; i >= 0; --i) {
        sig_x = i;
        if(x[i])
            break;
    }
    unsigned int sig_y;
    for(int i = N - 1; i >= 0; --i) {
        sig_y = i;
        if(y[i])
            break;
    }
    unsigned int n = sig_x - sig_y;
    y <<= n;
    n++;
    while(n--) {
       if(BitsetLtEq(y,r)) {
           q[n] = true;
           BitsetSubtract(r, y);
       } 
       y >>= 1;
    }
}

template<unsigned int N>
void RotLeft(std::bitset<N> &bitset, unsigned long count)
{
    std::string substr(bitset.to_string());
    substr = substr.substr(0, count);
    std::bitset<N> rotated(substr);
    bitset <<= count;
    bitset |= rotated;
}

template<unsigned int N>
void RotRight(std::bitset<N> &bitset, unsigned long count)
{
    std::string substr(bitset.to_string());
    size_t len = substr.length();
    substr = substr.substr(len - count, count);
    std::bitset<N> rotated(substr);
    rotated <<= len - count;
    bitset >>= count;
    bitset |= rotated;
}



template<unsigned int N>
std::bitset<N> GetBitsetFromString(std::string &str)
{
    std::bitset<8> b(str[0]);
    std::bitset<N> bitset(str[0]);
    for(auto it = str.begin() + 1; it != str.end(); ++it) {
        std::bitset<N> b(*it);
        bitset <<= 8;
        bitset |= b;
    }
    return bitset;
}

template<unsigned int N>
std::string GetStringFromBitset(std::bitset<N> &bitset)
{
    std::bitset<N> mask(255);
    std::string output("");
    unsigned byte = 8;
    unsigned round_times = N / byte;;
    for(size_t i = 0; i < round_times; ++i) {
        std::bitset<N> copy_bitset(bitset.to_string());
        copy_bitset >>= i * byte;
        copy_bitset &= mask;
        char ch = copy_bitset.to_ulong();        
        output.push_back(ch);
    }

    output = std::string(output.rbegin(), output.rend());
    return output;
}

#endif





