#pragma once

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

class Disk {
private:
	const string disk_file;
	int BLKsize;		// size of a single block
public:
	Disk();
	~Disk();

	bool Getblk(char buf[], int blk_num);
	bool Putblk(char buf[], int blk_num);
	int Get_BLKsize();
};