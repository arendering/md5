# -*- coding: utf-8 -*-

from z3 import *
import time

def get_input_string(inp):
    """
    Из int'a inp получаем исходное представление прообраза в виде нормальной строки,
    например "abcdefgh" или "12345678", а не в виде бит, или огромного int'a
    """
    byte = 8
    bin_string = bin(inp)
    bin_string = bin_string[2:]
    additive = byte - len(bin_string) % byte 
    if additive != 8:
        tmp_str = str()
        for i in range(additive):
            tmp_str += '0'
    bin_string = tmp_str + bin_string
    output_lst = list()
    for i in range( len(bin_string) / byte):
        tmp_bin = bin_string[i*8 : i*8+8]
        tmp_int = int('0b' + tmp_bin, 2)
        tmp_ch = str(unichr(tmp_int))
        output_lst.append(tmp_ch)
    output = ''.join(output_lst)
    return output

def get_right_len(n):
    """
    По длине сообщения получаем правильно перевернутую строку
    16-ых чисел, которую преобразуем в int
    """
    n_hex_str = hex(n)
    n_hex_str = n_hex_str[2:]
    if len(n_hex_str) < 16:
        pre_str = '0' * (16 - len(n_hex_str))
        n_hex_str = pre_str + n_hex_str
    reverse_n_hex_str = n_hex_str[::-1]
    ret_hex_str = ''.join([reverse_n_hex_str[i:i+2][::-1] for i in range(0, len(reverse_n_hex_str), 2) ])
    ret = int(ret_hex_str, 16)
    return ret


def find_all_results(s, inp):
    t1 = time.time()
    result = []
    while True:
        if s.check() == sat:
            m = s.model()
            print get_input_string(m[inp].as_long())
            result.append(m)
            block = []
            for d in m:
                if d.arity() > 0:
                    raise Z3Exception("uninterpreted functions are not supported")
                c = d()
                if is_array(c) or c.sort().kind() == Z3_UNINTERPRETED_SORT:
                    raise Z3Exception("arrays and uninterpreted sorts aren't supported")
                block.append(c != m[d])
            s.add(Or(block))
        else:
            print "total results = ", len(result)
            print "time: {:.3f} sec".format(time.time() - t1)
            break

input_len = 8 # input string length in bytes
byte = 8

inp_list = list()
for i in range(input_len):
    s = 'tmp' + str(i)
    tmp = BitVec(s, 8)
    inp_list.append(tmp)

s = Solver()
for elem in inp_list:
    s.add(elem >= 0x30)
    s.add(elem <= 0x39) # check only digits
    #s.add(Or(And(x >= 0x30, x <= 0x39), And(x >= 0x61, x <= 0x7A))) check digits and low case letters

one = BitVec('one', 1)
zeros_len = 512 - 64 - 1 - input_len * byte
zeros = BitVecVal(0, zeros_len)
right_len_of_inp_str = get_right_len(byte * input_len)
len_in_bits = BitVecVal(right_len_of_inp_str, 64)

inp_list.append(one)
inp_list.append(zeros)
inp_list.append(len_in_bits)

inp_str = BitVec('inp_str', 512)
s.add( inp_str == Concat(inp_list) ) # concatenate input symbols-bits

#find_all_results(s,inp_str)


