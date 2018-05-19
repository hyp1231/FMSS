#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include "disk.h"
using namespace std;

class FileSystem {
private:
	const int BLKsize;

	Disk D;						// disk
	int root_dir_inodeNum;		// inodeNum of root dir
	int cur_dir_inodeNum;		// inodeNum of cur dir
	vector<string> cur_path;

	/*
		bitmap: count from low to high, 0 - 31
		eg. 0x80 means block 7 is not empty
	*/
	char inode_bitmap[10];
	char data_block_bitmap[80];

    inline int Get_actual_dataBLKnumber(int n);
    inline int Get_actual_inodeBLKnumber(int n);
    int Get_dir_inodeNum_from_path(vector<string>& path);
    int Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename);
    bool Get_inode_from_inodeNum(char buf[], int inodeNum);
    int Find_empty_inodeNum();
    int Find_empty_dataBLKNum();
    void Fill_inode_bitmap(int inodeNum, bool flag);
    void Fill_data_block_bitmap(int blockNum, bool flag);
    void Save_superBLK();

public:
	FileSystem();
	~FileSystem();
	void Help();
    bool CreateFile(const string &filepath);
    bool DeleteFile(const string &filepath);
    void ListFile();
    int Get_cur_dir_inodeNum();
    void Print_cur_path();
};