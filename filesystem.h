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
	int root_dir_inodeNum;		// inodeNum of root dir
	int cur_dir_inodeNum;		// inodeNum of cur dir
	char inode_bitmap[10];
	char data_block_bitmap[80];

    int Get_actual_dataBLKnumber(int n);
    int Get_dir_inodeNum_from_path(vector<string>& path);
    int Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename);
public:
	FileSystem();
	~FileSystem();
	void Help();
    bool CreateFile(const string &filepath);
    //bool DeleteFile(const string &filepath);
    int Get_cur_dir_inodeNum();
};