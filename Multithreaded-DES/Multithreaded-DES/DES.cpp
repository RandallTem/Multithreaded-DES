#include "DES.h"


unsigned long long DES::Permutate(unsigned long long block, short array[], int length, int size) {
	unsigned long long new_block = 0;
	for (int i = 0; i < length-1; i++) {
		new_block += ((block >> (array[i]-1)) & 0x01);
		new_block <<= 1;
	}
	new_block += ((block >> (array[length-1] - 1)) & 0x01);
	return new_block;
} 

unsigned long long DES::Permutation(unsigned long long block, short type) {
	unsigned long long new_block = 0;
	switch (type) {
	case 1:
		new_block = Permutate(block, ip_table, 64, 64);
		break;
	case 2:
		new_block = Permutate(block, back_ip_table, 64, 64);
		break;
	case 3:
		new_block = Permutate(block, expand_table, 48, 32);
		break;
	case 4:
		new_block = Permutate(block, key56_table, 64, 64);
		break;
	case 5:
		new_block = Permutate(block, key48_table, 48, 64);
		break;
	case 6:
		new_block = Permutate(block, p_table, 32, 32);
		break;
	}
	return new_block;
}

unsigned long DES::SPermutation(unsigned long long expanded_block) {
	unsigned long block = 0;
	short line = 0, column = 0, bits_block = 0;
	for (int i = 0; i < 8; i++) {
		bits_block = ((expanded_block & 0x0000FC0000000000) >> 42);
		expanded_block <<= 6;
		line = ((bits_block & 0x20) >> 4) + (bits_block & 0x01);
		column = ((bits_block & 0x1E) >> 1);
		block += s_table[i][line][column];
		if (i != 7) block <<= 4;
	}
	return block;
}

bool DES::ExpandedKeyGetNumberOfOnes(short byte) {
	short counter = 0;
	bool res = false;
	for (int i = 0; i < 7; i++) {
		if ((byte & 0x0040) == 0x0040) counter++;
		byte <<= 1;
	}
	if ((counter % 2) == 0) res = true;
	return res;
}

unsigned long long DES::GetExpandedKeyForFeistelFunction(unsigned long long key) {
	unsigned long long expanded_key = 0;
	short byte = 0;
	for (int i = 0; i < 8; i++) {
		byte = ((key & 0x00FE000000000000LL) >> 49);
		if (ExpandedKeyGetNumberOfOnes(byte)) byte += 0x80;
		expanded_key = (expanded_key << 8) + byte;
		key <<= 7;
	}
	expanded_key = Permutation(expanded_key, 4);
	return expanded_key;
}

unsigned long long DES::KeyCyclicShift(unsigned long long expanded_key, int number_of_shifts, char direction) {
	short bit = 0;
	unsigned long c = (expanded_key >> 28), d = (expanded_key & 0x0FFFFFFF);
	if (direction == 'r') {
		for (int i = 0; i < number_of_shifts; i++) {
			bit = (c & 0x01);
			c = (c >> 1) + (0x80000000 * bit);
			bit = (d & 0x01);
			d = (d >> 1) + (0x80000000 * bit);
		}
	}
	else {
		for (int i = 0; i < number_of_shifts; i++) {
			bit = ((c & 0x80000000) >> 31);
			c = (c << 1) + bit;
			bit = ((d & 0x80000000) >> 31);
			d = (d << 1) + bit;
		}
	}
	return expanded_key;
}

unsigned long DES::FeistelFunction(unsigned long block, unsigned long long round_key, char mode) {
	unsigned long shrinked_block = 0;
	unsigned long long expanded_block = Permutation(block, 3);
	expanded_block ^= round_key;
	shrinked_block = SPermutation(expanded_block);
	shrinked_block = (Permutation(shrinked_block, 6) & 0xFFFFFFFF);
	return shrinked_block;
}

unsigned long long DES::Feistel(unsigned long long block, unsigned long long expanded_key, char direction) {
	unsigned long left_block = (block >> 32), right_block = block & 0xFFFFFFFF, buf = 0;
	unsigned long long res_block = 0, round_key = 0;
	if (direction == 'f') {
		for (int i = 1; i <= 16; i++) {
			if (i == 1 || i == 2 || i == 9 || i == 16) expanded_key = KeyCyclicShift(expanded_key, 1, 'l');
			else expanded_key = KeyCyclicShift(expanded_key, 2, 'l');
			round_key = Permutation(expanded_key, 5);
			buf = left_block;
			left_block = right_block;
			right_block = FeistelFunction(right_block, round_key, 'e') ^ buf;
		}
	}
	else {
		for (int i = 16; i >= 1; i--) {
			if (i == 1 || i == 2 || i == 9 || i == 16) expanded_key = KeyCyclicShift(expanded_key, 1, 'r');
			else expanded_key = KeyCyclicShift(expanded_key, 2, 'r');
			round_key = Permutation(expanded_key, 5);
			buf = right_block;
			right_block = left_block;
			left_block = FeistelFunction(left_block, round_key, 'd') ^ buf;
		}
	}
	res_block += left_block;
	res_block = (res_block << 32) + right_block;
	return res_block;
}


unsigned long long DES::DESRound(unsigned long long block, unsigned long long key, char dir) {
	unsigned long long expanded_key = GetExpandedKeyForFeistelFunction(key);
	block = Permutation(block, 1);
	if (dir == 'e') block = Feistel(block, expanded_key, 'f');
	else block = Feistel(block, expanded_key, 'b');
	block = Permutation(block, 2);
	return block;
}




