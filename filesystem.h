#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "disk.h"
using namespace std;

class FileSystem {
private:
	const int BLKsize;

	Disk D;					// disk
	int root_dir_inode;		// inode of root dir
	int cur_dir_inode;		// inode of cur dir
	char inode_bitmap[10];
	char data_block_bitmap[80];

    int Get_actual_dataBLKnumber(int n);
    int Get_dir_inode(vector<string>& path);
public:
	FileSystem();
	~FileSystem();
	void Help();
    bool CreateFile(const string &filepath);
    //bool DeleteFile(const string &filepath);
    int Get_cur_dir_inode();
};