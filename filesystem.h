#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <algorithm>
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
    int Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename, bool isDelete = false);
    bool Get_inode_from_inodeNum(char buf[], int inodeNum);
    int Find_empty_inodeNum();
    int Find_empty_dataBLKNum();
    void Fill_inode_bitmap(int inodeNum, bool flag);
    void Fill_data_block_bitmap(int blockNum, bool flag);
    void Save_superBLK();
    int FindEOF(char buf[]);
    inline bool isOutside_path(const string& path);
    bool Outside_file2str(ifstream& ifile, string& str);
    bool Inside_file2str(const string& filepath, string& str);
    bool str2Inside_file(string& file_data, const string& filepath);
    int Analysis_path(const string& filepath, string& filename);
    void PutInode(char buf[], int inodeNum);

public:
	FileSystem();
	~FileSystem();
	void Help();
    bool CreateFile(const string &filepath);
    bool DeleteFile(const string &filepath);
    void ListFile(const string param = "none");
    bool MoveFile(string& S_filepath, string& D_filepath);
    bool PrintFile(const string& filepath);
    int Get_cur_dir_inodeNum();
    void Print_cur_path();
};