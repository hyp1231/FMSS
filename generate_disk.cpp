#include <iostream>
#include <fstream>
using namespace std;

int main() {
    ofstream ofile("./diskdata");				// create a new file

    short basic_size = 1024;					// basic size of a block
    ofile.write((char*)&basic_size, sizeof(basic_size));

    char tmp = 0;
    for(int i = 0; i < 1024 * 1024 - 2; ++i) {
	    ofile.write((char*)&tmp, sizeof(tmp));	// fill the file with '\0'
	}

    return 0;
}
