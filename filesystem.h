#pragma once
#include <iostream>
#include "disk.h"
using namespace std;

class FileSystem {
private:
	Disk D;			// disk
public:
	FileSystem();
	~FileSystem();
	void Help();
};