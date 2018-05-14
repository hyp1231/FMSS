#include "disk.h"

Disk::Disk() : disk_file("diskdata") {
	ifstream ifile(disk_file);
	BLKsize = 0;
	if(!ifile) { cout << "Disk initialization error! ！(◎_◎;)" << endl; return; }
	char tmp = 0;
	for(int i = 0; i < 2; ++i) {		// read the basic size of a block in the disk
		ifile.read((char*)&tmp, sizeof(tmp));
		BLKsize += tmp << (8 * i);		// Little endian
	}
	ifile.close();
}

Disk::~Disk() {
	;
}

bool Disk::Getblk(char buf[], int blk_num) {
	ifstream ifile(disk_file);
	if(!ifile) {	// cannot open file
		cout << "Not found diskfile... (´Д` )" << endl;
		return false;
	}
	ifile.seekg(ifile.beg + BLKsize * blk_num);
	for(int i = 0; i < BLKsize; ++i) {
		ifile.read((char*)&buf[i], sizeof(char));
	}
	ifile.close();
	return true;
}

bool Disk::Putblk(char buf[], int blk_num) {
	ofstream ofile(disk_file, ios::out | ios::in);
	if(!ofile) {	// cannot open file
		cout << "Not found diskfile... (´Д` )" << endl;
		return false;
	}
	ofile.seekp(ofile.beg + BLKsize * blk_num);
	for(int i = 0; i < BLKsize; ++i) {
		ofile.write((char*)&buf[i], sizeof(char));
	}
	ofile.close();
	return true;
}