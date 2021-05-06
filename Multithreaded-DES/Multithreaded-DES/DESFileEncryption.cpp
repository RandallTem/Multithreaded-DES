#include "DES.h"
#include "FileBinOp.h"
#include <iostream>
#include <vector>
#include <thread>
#include <ctime>


unsigned long long* arrayToBinBlocks(char* file_bytes, int number_of_threads) {
	unsigned long long* bin_block = new unsigned long long[number_of_threads]();
	for (int i = 0; i < number_of_threads; i++) {
		bin_block[i] ^= ((file_bytes[i * 8 + 0]) & 0xFF);
		for (int j = 1; j < 8; j++) {
			bin_block[i] <<= 8;
			bin_block[i] ^= ((file_bytes[i * 8 + j]) & 0xFF);
		}
	}
	return bin_block;
}

char* binBlocksToArray(unsigned long long* bin_block, int number_of_threads) {
	char* file_bytes = new char[(number_of_threads * 8) + 1]();
	file_bytes[(number_of_threads * 8)] = '\0';
	for (int i = 0; i < number_of_threads; i++) {
		for (int j = 0; j < 8; j++) {
			file_bytes[(i * 8) + j] ^= ((bin_block[i] & 0xFF00000000000000) >> 56);
			bin_block[i] <<= 8;
		}
	}
	return file_bytes;
}

void cryptBlock(unsigned long long block, unsigned long long key, char dir, unsigned long long* new_block) {
	DES cryptor;
	*new_block = cryptor.DESRound(block, key, dir);
}

void Encrypt(std::string path, unsigned long long key, int number_of_threads) {
	FileBinOp file_read, file_write;
	std::vector<std::thread> threads;
	char* file_bytes;
	unsigned long file_size;
	unsigned long long* plaintext;
	unsigned long long* ciphertext;
	int pos;
	if (file_read.openFile(path, 'r')) {
		std::string res = path + ".crptd";
		file_write.openFile(res, 'w');
		file_size = file_read.getSize();
		ciphertext = new unsigned long long[number_of_threads]();
		for (int blocks_count = 0; blocks_count < (file_size / (number_of_threads * 8)); blocks_count++) {
			file_bytes = file_read.readBytes(number_of_threads * 8);
			plaintext = arrayToBinBlocks(file_bytes, number_of_threads);
			for (int i = 0; i < number_of_threads; i++) {
				threads.push_back(std::thread(cryptBlock, plaintext[i], key, 'e', &ciphertext[i]));
				threads.at(i);
			}
			for (int i = 0; i < number_of_threads; i++) {
				if (threads.at(i).joinable()) threads.at(i).join();
			}
			threads.clear();
			file_bytes = binBlocksToArray(ciphertext, number_of_threads);
			file_write.writeBytes(file_bytes, (8 * number_of_threads));
		}
		int leftover = (file_size - file_read.getPosition('r'));
		if (leftover / 8 > 0) {
			file_bytes = file_read.readBytes((leftover / 8) * 8);
			plaintext = arrayToBinBlocks(file_bytes, (leftover / 8));
			for (int i = 0; i < (leftover / 8); i++) {
				threads.push_back(std::thread(cryptBlock, plaintext[i], key, 'e', &ciphertext[i]));
				threads.at(i);
			}
			for (int i = 0; i < (leftover / 8); i++) {
				if (threads.at(i).joinable()) threads.at(i).join();
			}
			threads.clear();
			file_bytes = binBlocksToArray(ciphertext, (leftover / 8));
			file_write.writeBytes(file_bytes, (8 * (leftover / 8)));
		}
		if (file_size % 8 != 0) {
			file_bytes = file_read.readBytes((file_size % 8));
			char* last_block = new char[8]();
			last_block[file_size % 8] = 0x80;
			for (int i = 0; i < (file_size % 8); i++) {
				last_block[i] = file_bytes[i];
			}
			plaintext = arrayToBinBlocks(last_block, 1);
			cryptBlock(plaintext[0], key, 'e', &ciphertext[0]);
			file_bytes = binBlocksToArray(ciphertext, 1);
			file_write.writeBytes(file_bytes, (8 * 1));
		}
		file_read.closeFile();
		file_write.closeFile();	
	}
	else { std::cout << "Couldn't open the file" << std::endl; }
}

std::string getOriginalExtension(std::string path) {
	int pos = path.length();
	while (path[pos--] != '.') {}
	return path.substr(0, pos+1);
}

void Decrypt(std::string path, unsigned long long key, int number_of_threads) {
	FileBinOp file_read, file_write;
	std::vector<std::thread> threads;
	char* file_bytes;
	unsigned long file_size;
	unsigned long long* plaintext;
	unsigned long long* ciphertext;
	if (file_read.openFile(path, 'r')) {
		std::string res = getOriginalExtension(path);
		file_write.openFile(res, 'w');
		file_size = file_read.getSize() - 8;
		plaintext = new unsigned long long[number_of_threads]();
		for (int blocks_count = 0; blocks_count < (file_size / (number_of_threads * 8)); blocks_count++) {
			file_bytes = file_read.readBytes(number_of_threads * 8);
			ciphertext = arrayToBinBlocks(file_bytes, number_of_threads);
			for (int i = 0; i < number_of_threads; i++) {
				threads.push_back(std::thread(cryptBlock, ciphertext[i], key, 'd', &plaintext[i]));
				threads.at(i);
			}
			for (int i = 0; i < number_of_threads; i++) {
				if (threads.at(i).joinable()) threads.at(i).join();
			}
			threads.clear();
			file_bytes = binBlocksToArray(plaintext, number_of_threads);
			file_write.writeBytes(file_bytes, (8 * number_of_threads));
		}
		int leftover = (file_size - file_read.getPosition('r'));
		if (leftover / 8 > 0) {
			file_bytes = file_read.readBytes((leftover / 8) * 8);
			ciphertext = arrayToBinBlocks(file_bytes, (leftover / 8));
			for (int i = 0; i < (leftover / 8); i++) {
				threads.push_back(std::thread(cryptBlock, ciphertext[i], key, 'd', &plaintext[i]));
				threads.at(i);
			}
			for (int i = 0; i < (leftover / 8); i++) {
				if (threads.at(i).joinable()) threads.at(i).join();
			}
			threads.clear();
			file_bytes = binBlocksToArray(plaintext, (leftover / 8));
			file_write.writeBytes(file_bytes, (8 * (leftover / 8)));
		}
		file_bytes = file_read.readBytes(8);
		ciphertext = arrayToBinBlocks(file_bytes, 1);
		cryptBlock(ciphertext[0], key, 'd', &plaintext[0]);
		file_bytes = binBlocksToArray(plaintext, 1);
		int cut = 0, index = 7;
		while (file_bytes[index] == 0x00) {
			index--;
			cut++;
		}
		if (file_bytes[index] == (char)0x80)
			cut++;
		else
			cut = 0;
		file_write.writeBytes(file_bytes, 8-cut);
		file_read.closeFile();
		file_write.closeFile();
	} else { std::cout << "Couldn't open the file" << std::endl; }
}

unsigned long long getKeyFromString(std::string string_key) {
	unsigned long long key = 0;
	for (int i = 0; i < string_key.length(); i++) {
		key = (key << 8) + string_key[i];
	}
	return key;
}

int main() {
	std::string path;
	std::string string_key;
	unsigned long long key;
	int number_of_threads, start_time, finish_time;
	std::cout << "Enter path: ";
	std::cin >> path;
	std::cout << "Enter key: ";
	std::cin >> string_key;
	key = getKeyFromString(string_key);
	std::cout << "Enter number of threads: ";
	std::cin >> number_of_threads;
	while (std::cin.fail() || number_of_threads < 1) {
		std::cout << "Enter number of threads correctly: ";
		std::cin.clear();
		std::cin.ignore(100, '\n');
		std::cin >> number_of_threads;
	}
	if (path.length() > 6 && path.substr(path.length() - 6, path.length()) == ".crptd") {
		start_time = clock();
		Decrypt(path, key, number_of_threads);
		finish_time = clock();
		std::cout << "Decryption done in " << (finish_time - start_time) * 1.0 / 1000 << " seconds" << std::endl;
	}
	else {
		start_time = clock();
		Encrypt(path, key, number_of_threads);
		finish_time = clock();
		std::cout << "Encryption done in " << (finish_time - start_time) * 1.0 / 1000 << " seconds" <<  std::endl;
	}	
}