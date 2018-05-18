#include "filesystem.h"

FileSystem::FileSystem() : D(), BLKsize(D.Get_BLKsize()) {
	cout << "Welcome to use our system!" << endl;
	cout << "If you need help, just type \"help\", and hit [enter]." << endl;
	cout << "Input \"exit\" to exit the ststem." << endl;
	cout << "Hope you enjoy yourself~ (´▽｀)" << endl;

    // set BLK_No. of root directory to 0 
    // (the implementation of diskdata has done this work)
}

FileSystem::~FileSystem() {
	cout << "Thanks for using! See you later~ ♪───Ｏ（≧∇≦）Ｏ────♪" << endl;
}

void FileSystem::Help() {
	cout << "* ------------------------------------ *" << endl;
	cout << "[help]: get help list" << endl;
	cout << "[exit]: exit the system" << endl;
	cout << "* ------------------------------------ *" << endl;
}

int FileSystem::Get_actual_dataBLKnumber(int n) {
    return n + 86;
}

// untested
// get directory block(dir) and directory filename_map block(dir_map)
void getDirInfo(char dir[], char dir_map[], int blk_num) {
    Get(dir, Get_actual_dataBLKnumber(blk_num));
    int map_blk_number = dir[0] + (dir[1] << 8);
    Get(dir_map, Get_actual_dataBLKnumber(map_blk_number));
}

// untested
// decide if bytes in buffer( buf[begin, end) ) equals to string(str)
bool byteEQUstring(const string& str, char buf[], int begin, int end) {
    if (str.length() != end - begin)
        return false;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] != buf[beg + i])
            return false;
    }
    return true;
}

// untested
// convert bytes( buf[begin, end) ) to int
int byte2int(char buf[], int begin, int end) {
    if (end - begin > 8) {
        cout << "Too many bytes to convert them to int!" << endl;
        return -1;
    }

    int ret = 0;
    for (int i = 0; i < end - begin; i++) {
        ret += buf[begin + i] << (i * 8);   // little endian
    }
    return ret;
}

// untested
// decide if filename(str) is in directory map block(dir_map)
//      if it's in the map, return the inode number; otherwise, return -1
int file_in_DirMap(const string &str, char dir_map[]) {
    int map_size = 12;
    for (int i = 0; i < 80; i++) {
        int end;    // the end position of filename (0 - 7)
        for (int j = 0; j < 8; j++) {
            if (dir_map[i * map_size + j] == 0) {
                end = j;
                break;
            }
        }
        if (end == 0) break;                    // filename = '', meaning this pair is empty
        if (byteEQUstring(str, dir_map, i * map_size, i * map_size + end))
            return byte2int(dir_map, i * map_size + 10, i * map_size + 12);
    }
    return -1;
}

bool FileSystem::CreateFile(const string &filename) {
    /* 
        step 1: get to the right directory
    */
    char blk[BLKsize], dir[BLKsize], dir_map[BLKsize];
    getDirInfo(dir, dir_map, 0);                       // get root directory and its map block

    int dir_token_pos = 0;
    while (true) {
        int curPos = filename.find('/', dir_token_pos);
        if (curPos == -1) break;

        int next_Inode_num = file_in_DirMap(filename.substr(dir_token_pos + 1, curPos), dir_map);
        if (next_Inode_num == -1)
            return false;
        

        dir_token_pos = curPos;
    }

    /*
        step 2: make sure no files have the same name; otherwise, return FALSE
                create new pair of mapping in last directory's file_name_index block
    */

    /*  
        step 3: search for empty index_node and set corresponding attributes
    */

    /*
        step 4: search the block_bitmap for empty blocks and set corresponding bits to TRUE
    */
}