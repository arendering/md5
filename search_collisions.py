# -*- coding: utf-8 -*-

from z3 import *
import time
import math

hash_value = 0x25d55ad283aa400af464c76d713c07ad # it's 12345678

def F(x,y,z):
    return (x & y) | (~x & z)

def G(x,y,z):
    return (x & z) | (y & ~z)

def H(x,y,z):
    return x ^ y ^ z

def I(x,y,z):
    return y ^ (x | ~z)


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

def get_T_array():
    T = list()
    for i in range(64):
        n = 4294967296.0 * abs(math.sin(i + 1))
        #if i <= 9:
        #    print 'T[%d]: %d' % (i, n)
        val = BitVecVal(n, 32)
        T.append(val)
    return T

def get_X_array():
    global solver
    global inp_str

    X = list()
    temp_for_X = list()

    for i in range(16):
        s = 'X' + str(i)
        tmp = BitVec(s, 32)
        X.append(tmp)

    for i in range(64):
        s = 'temp_for_X' + str(i)
        tmp = BitVec(s, 8)
        temp_for_X.append(tmp)

    high = 511
    low = 504
    k = 0
    for j in range(0, 64):
        if j != 0 and j % 4 == 0:
            solver.add(X[k] == Concat(temp_for_X[-1], temp_for_X[-2], temp_for_X[-3], temp_for_X[-4]))
            k += 1
        solver.add(temp_for_X[j] == Extract(high, low, inp_str))
        high -= byte
        low -= byte

    solver.add(X[k] == Concat(temp_for_X[-1],temp_for_X[-2], temp_for_X[-3], temp_for_X[-4]))
    return X

def set_right_order_and_check_hash(A, B, C, D):
    """
    Из итоговых вариантов A, B, C и D меняем порядок байт в словах и собираем итоговую строку
    """
    global solver
    global hash_value
    tmp_lst_for_final = list()
    for i in range(16):
        s = 'tmp_lst_for_final' + str(i)
        tmp = BitVec(s, 8)
        tmp_lst_for_final.append(tmp)
    abcd = BitVec('abcd', 128)
    solver.add(abcd == Concat(A, B, C, D))
    high = 127
    low = 120
    for i in range(16):
        solver.add(tmp_lst_for_final[i] == Extract(high, low, abcd))
        high -= byte
        low -= byte

    tmp_right_order = list()
    for i in range(4):
        s = 'tmp_right_order' + str(i)
        tmp = BitVec(s, 32)
        tmp_right_order.append(tmp)

    k = 0
    for i in range(0, 16, 4):
        solver.add(tmp_right_order[k] == Concat(tmp_lst_for_final[i + 3], tmp_lst_for_final[i + 2], tmp_lst_for_final[i + 1], tmp_lst_for_final[i]))
        k += 1
    hash_string = BitVec('hash_string', 128)
    solver.add(hash_string == Concat(tmp_right_order[0], tmp_right_order[1], tmp_right_order[2], tmp_right_order[3]))
    solver.add(hash_string == hash_value)
    


        
    

def find_all_results(solver, inp):
    t1 = time.time()
    result = []
    while True:
        if solver.check() == sat:
            m = solver.model()
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
            solver.add(Or(block))
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

solver = Solver()
for elem in inp_list:
    solver.add(elem >= 0x30)
    solver.add(elem <= 0x39) # check only digits
    #solver.add(Or(And(x >= 0x30, x <= 0x39), And(x >= 0x61, x <= 0x7A))) check digits and low case letters

one = BitVec('one', 1)
zeros_len = 512 - 64 - 1 - input_len * byte
zeros = BitVecVal(0, zeros_len)
right_len_of_inp_str = get_right_len(byte * input_len)
len_in_bits = BitVecVal(right_len_of_inp_str, 64)

inp_list.append(one)
inp_list.append(zeros)
inp_list.append(len_in_bits)

inp_str = BitVec('inp_str', 512)
solver.add( inp_str == Concat(inp_list) ) # concatenate input symbols-bits

A = BitVecVal(0x67452301, 32)
B = BitVecVal(0xEFCDAB89, 32)
C = BitVecVal(0x98BADCFE, 32)
D = BitVecVal(0x10325476, 32)
T = get_T_array() # bit_vec<32> T[64]
X = get_X_array() # bit_vec<32> X[16]

rounds_tmp = list()
for i in range(68):
    if i == 0:
        tmp = BitVecVal(0x67452301, 32) # A
    elif i == 1:
        tmp = BitVecVal(0x10325476, 32) # D
    elif i == 2:
        tmp = BitVecVal(0x98BADCFE, 32) # C
    elif i == 3:
        tmp = BitVecVal(0xEFCDAB89, 32) # B
    else:
        s = 'rounds_tmp' + str(i)
        tmp = BitVec(s, 32)
    rounds_tmp.append(tmp)

def Set(A, B, C, D, *args):
    global X
    global T
    k = args[0]
    s = args[1]
    i = args[2]
    func_const = args[3]

    if func_const == 'F':
        return B + RotateLeft( (A | F(B,C,D) | X[k] | T[i] ), s) 
    if func_const == 'G':
        return B + RotateLeft( (A | G(B,C,D) | X[k] | T[i] ), s) 
    if func_const == 'H':
        return B + RotateLeft( (A | H(B,C,D) | X[k] | T[i] ), s) 
    if func_const == 'I':
        return B + RotateLeft( (A | I(B,C,D) | X[k] | T[i] ), s) 

list_of_constants = list()
list_of_constants.append([0, 7, 0, 'F'])
list_of_constants.append([1, 12, 1, 'F'])
list_of_constants.append([2, 17, 2, 'F'])
list_of_constants.append([3, 22, 3, 'F'])
list_of_constants.append([4, 7, 4, 'F'])
list_of_constants.append([5, 12, 5, 'F'])
list_of_constants.append([6, 17, 6, 'F'])
list_of_constants.append([7, 22, 7, 'F'])
list_of_constants.append([8, 7, 8, 'F'])
list_of_constants.append([9, 12, 9, 'F'])
list_of_constants.append([10, 17, 10, 'F'])
list_of_constants.append([11, 22, 11, 'F'])
list_of_constants.append([12, 7, 12, 'F'])
list_of_constants.append([13, 12, 13, 'F'])
list_of_constants.append([14, 17, 14, 'F'])
list_of_constants.append([15, 22, 15, 'F'])
list_of_constants.append([1, 5, 16, 'G'])
list_of_constants.append([6, 9, 17, 'G'])
list_of_constants.append([11, 14, 18, 'G'])
list_of_constants.append([0, 20, 19, 'G'])
list_of_constants.append([5, 5, 20, 'G'])
list_of_constants.append([10, 9, 21, 'G'])
list_of_constants.append([15, 14, 22, 'G'])
list_of_constants.append([4, 20, 23, 'G'])
list_of_constants.append([9, 5, 24, 'G'])
list_of_constants.append([14, 9, 25, 'G'])
list_of_constants.append([3, 14, 26, 'G'])
list_of_constants.append([8, 20, 27, 'G'])
list_of_constants.append([13, 5, 28, 'G'])
list_of_constants.append([2, 9, 29, 'G'])
list_of_constants.append([7, 14, 30, 'G'])
list_of_constants.append([12, 20, 31, 'G'])
list_of_constants.append([5, 4, 32, 'H'])
list_of_constants.append([8, 11, 33, 'H'])
list_of_constants.append([11, 16, 34, 'H'])
list_of_constants.append([14, 23, 35, 'H'])
list_of_constants.append([1, 4, 36, 'H'])
list_of_constants.append([4, 11, 37, 'H'])
list_of_constants.append([7, 16, 38, 'H'])
list_of_constants.append([10, 23, 39, 'H'])
list_of_constants.append([13, 4, 40, 'H'])
list_of_constants.append([0, 11, 41, 'H'])
list_of_constants.append([3, 16, 42, 'H'])
list_of_constants.append([6, 23, 43, 'H'])
list_of_constants.append([9, 4, 44, 'H'])
list_of_constants.append([12, 11, 45, 'H'])
list_of_constants.append([15, 16, 46, 'H'])
list_of_constants.append([2, 23, 47, 'H'])
list_of_constants.append([0, 6, 48, 'I'])
list_of_constants.append([7, 10, 49, 'I'])
list_of_constants.append([14, 15, 50, 'I'])
list_of_constants.append([5, 21, 51, 'I'])
list_of_constants.append([12, 6, 52, 'I'])
list_of_constants.append([3, 10, 53, 'I'])
list_of_constants.append([10, 15, 54, 'I'])
list_of_constants.append([1, 21, 55, 'I'])
list_of_constants.append([8, 6, 56, 'I'])
list_of_constants.append([15, 10, 57, 'I'])
list_of_constants.append([6, 15, 58, 'I'])
list_of_constants.append([13, 21, 59, 'I'])
list_of_constants.append([4, 6, 60, 'I'])
list_of_constants.append([11, 10, 61, 'I'])
list_of_constants.append([2, 15, 62, 'I'])
list_of_constants.append([9, 21, 63, 'I'])
    
for i in range(64):
    solver.add( rounds_tmp[i + 4] == Set(rounds_tmp[i], rounds_tmp[i + 3], rounds_tmp[i + 2], rounds_tmp[i + 1], *(list_of_constants[i])))
        
AA, BB, CC, DD = BitVecs('AA BB CC DD', 32)

solver.add(AA == A | rounds_tmp[64])
solver.add(BB == B | rounds_tmp[67])
solver.add(CC == C | rounds_tmp[66])
solver.add(DD == D | rounds_tmp[65])

set_right_order_and_check_hash(AA, BB, CC, DD)

#find_all_results(solver,inp_str)


