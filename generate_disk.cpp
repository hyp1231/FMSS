#include <iostream>
#include <fstream>
using namespace std;

int main() {
    ofstream ofile("./diskdata");				// create a new file

    // super block
    short basic_size = 1024;					// basic size of a block
    ofile.write((char*)&basic_size, sizeof(basic_size));

    char tmp = 0;
    for(int i = 0; i < 1024 - 2; ++i) {
	    ofile.write((char*)&tmp, sizeof(tmp));	// fill the file with '\0'
	}

	// first inode
	short num = 1;
	ofile.write((char*)&num, sizeof(num));
	for(int i = 0; i < 7; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}
	tmp = '/';
	ofile.write((char*)&tmp, sizeof(tmp));
	tmp = 0;
	
	// rest of inode table
	for(int i = 0; i < 5 * 1024 - 10; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}

	// first data block
	for(int i = 0; i < 7; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}
	tmp = '.';
	ofile.write((char*)&tmp, sizeof(tmp));
	tmp = 0;

	ofile.write((char*)&num, sizeof(num));

	for(int i = 0; i < 6 + 2; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}
	tmp = '.';
	for(int i = 0; i < 2; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}
	tmp = 0;

	ofile.write((char*)&num, sizeof(num));

	// rest of data block
	for(int i = 0; i < 1024 - 12 - 10 + (1024 - 7) * 1024; ++i) {
		ofile.write((char*)&tmp, sizeof(tmp));
	}

	ofile.close();

    return 0;
}
