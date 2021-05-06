#pragma once
#include <iostream>
#include <fstream>

class FileBinOp {
private:
	std::ifstream FileRead;
	std::ofstream FileWrite;

public:
	bool openFile(std::string path, char type);
	int getPosition(char type);
	int getSize();
	char* readBytes(int number);
	bool writeBytes(char* block, int number);
	bool isExist(std::string path);
	void closeFile();
};
