#pragma once
#include <iostream>
#include "disk.h"
using namespace std;

class FileSystem {
private:
	Disk D;			// disk
	const int BLKsize;
public:
	FileSystem();
	~FileSystem();
	void Help();
    int Get_actual_BLKnumber(int n);
    bool CreateFile(const string &filename);
    bool DeleteFile(const string &filename);
};