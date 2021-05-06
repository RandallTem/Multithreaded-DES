#include "FileBinOp.h"

bool FileBinOp::openFile(std::string path, char type) {
	if (type == 'r') {
		FileRead.open(path, std::ios::in | std::ios::binary);
		return (FileRead.is_open());
	}
	else {
		FileWrite.open(path, std::ios::out | std::ios::binary);
		return (FileWrite.is_open());
	}
}

int FileBinOp::getPosition(char type) {
	if (type == 'r') return FileRead.tellg();
	else return FileWrite.tellp();
}

int FileBinOp::getSize() {
	int pos = FileRead.tellg(), size = 0;
	FileRead.seekg(0, std::ios_base::end);
	size = FileRead.tellg();
	FileRead.seekg(pos);
	return size;
}

char* FileBinOp::readBytes(int number) {
	char* block = new char[number + 1];
	block[number] = '\0';
	int size = getSize();
	FileRead.read(block, number);
	return block;
}

bool FileBinOp::writeBytes(char* block, int number) {
	FileWrite.write(block, number);
	return true;
}

bool FileBinOp::isExist(std::string path) {
	FileBinOp file;
	if (file.openFile(path, 'r')) return true;
	else return false;
}

void FileBinOp::closeFile() {
	FileRead.close();
}
