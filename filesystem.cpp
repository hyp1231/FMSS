#include "filesystem.h"

/*
    --------------------- Tool Functions ---------------------
*/

// untested
// decide if bytes in buffer( buf[begin, end) ) equals to string(str)
bool byteEQUstring(const string& str, char buf[], int begin, int end) {
    if (str.length() != end - begin)
        return false;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] != buf[begin + i])
            return false;
    }
    return true;
}

// untested
// convert bytes( buf[begin, end) ) to int
int byte2int(char buf[], int begin, int end) {
    if (end - begin > 4) {
        cout << "Too many bytes to convert them to int!" << endl;
        return -1;
    }

    int ret = 0;
    for (int i = 0; i < end - begin; i++) {
        ret += (int)buf[begin + i] << (i * 8);   // little endian
    }
    return ret;
}

// find '/' and change a path to dir_names
// return False if illegal
bool Decomposition_path(string ori_path, vector<string>& path) {
    if(ori_path.empty()) { return false; }

    // if start of root
    if(ori_path[0] == '/') {
        path.push_back("/");
        ori_path = ori_path.substr(1);
    }

    // rest
    int p;
    while((p = ori_path.find('/')) != -1) {
        // start of '/', illegal
        if(p < 1) { return false; }

        path.push_back(ori_path.substr(0, p));

        ori_path = ori_path.substr(p + 1);
    }

    // the last one
    if(!ori_path.empty()) { 
        path.push_back(ori_path);
    }

    return true;
}

/*
    ------------------- End of Tool Functions -----------------
*/

FileSystem::FileSystem() : D(), BLKsize(1024) {
	cout << "Welcome to use our system!" << endl;
	cout << "If you need help, just type \"help\", and hit [enter]." << endl;
	cout << "Input \"exit\" to exit the ststem." << endl;
	cout << "Hope you enjoy yourself~ (´▽｀)" << endl;

    // set BLK_No. of root directory to 0 
    // (the implementation of diskdata has done this work)

    // get super block
    char buf[BLKsize];
    D.Getblk(buf, 0);

    // get root path inode
    root_dir_inode = cur_dir_inode = buf[2];

    // get inode bitmap
    for(int i = 3; i <= 12; ++i) {
        inode_bitmap[i - 3] = buf[i];
    }

    // get data block bitmap
    for(int i = 13; i <= 92; ++i) {
        data_block_bitmap[i - 13] = buf[i];
    }
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

// count from zero
int FileSystem::Get_actual_dataBLKnumber(int n) {
    return n + 86;
}

// untested
// get directory block(dir) and directory filename_map block(dir_map)
// void getDirInfo(char dir[], char dir_map[], int blk_num) {
//     Get(dir, Get_actual_dataBLKnumber(blk_num));
//     int map_blk_number = dir[0] + (dir[1] << 8);
//     Get(dir_map, Get_actual_dataBLKnumber(map_blk_number));
// }

// untested
// decide if filename(str) is in directory map block(dir_map)
//      if it's in the map, return the inode number; otherwise, return -1
// int file_in_DirMap(const string &str, char dir_map[]) {
//     int map_size = 12;
//     for (int i = 0; i < 80; i++) {
//         int end;    // the end position of filename (0 - 7)
//         for (int j = 0; j < 8; j++) {
//             if (dir_map[i * map_size + j] == 0) {
//                 end = j;
//                 break;
//             }
//         }
//         if (end == 0) break;                    // filename = '', meaning this pair is empty
//         if (byteEQUstring(str, dir_map, i * map_size, i * map_size + end))
//             return byte2int(dir_map, i * map_size + 10, i * map_size + 12);
//     }
//     return -1;
// }

int FileSystem::Get_dir_inode(vector<string>& path) {
    return 0;
}

// return false if cannot create file
bool FileSystem::CreateFile(const string &filepath) {
    /* 
        step 1: find the right directory
    */
    //char blk[BLKsize], dir[BLKsize], dir_map[BLKsize];
    //getDirInfo(dir, dir_map, 0);                       // get root directory and its map block

    // int dir_token_pos = 0;
    // while (true) {
    //     int curPos = filename.find('/', dir_token_pos);
    //     if (curPos == -1) break;

    //     int next_Inode_num = file_in_DirMap(filename.substr(dir_token_pos + 1, curPos), dir_map);
    //     if (next_Inode_num == -1)
    //         return false;
        

    //     dir_token_pos = curPos;
    // }

    vector<string> path;
    string filename;
    if(!Decomposition_path(filepath, path) || (int)path.size() < 2) {
        cout << "illegal path!" << endl;
        return false;
    }

    filename = path[path.size() - 1];
    path.pop_back();

    if(path.empty())

    int dir_inode = Get_dir_inode(path);

    /*
        step 2: make sure no files have the same name; otherwise, return FALSE
    */

    /*  
        step 3: search for empty index_node and set corresponding attributes
    */

    /*
        step 4: create new pair of mapping in its directory's data block
    */

    /*
        step 5: search the block_bitmap for empty blocks and set corresponding bits to TRUE
    */
    return true;
}

int FileSystem::Get_cur_dir_inode() {
    return cur_dir_inode;
}