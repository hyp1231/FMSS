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

bool byteEQUstring(char buf[], int begin, int end, const string& str) {
	return byteEQUstring(str, buf, begin, end);
}

// untested
// convert bytes( buf[begin, end) ) to int
int byte2int(char buf[], int begin, int end) {
    if (end - begin > 4) {
        cout << "[Error] Too many bytes to convert them to int!" << endl;
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

    // get root path inodeNum and cur dir inodeNum
    root_dir_inodeNum = cur_dir_inodeNum = buf[2];

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

// count from zero
int FileSystem::Get_actual_inodeBLKnumber(int n) {
	return 1 + n / 16;
}


// return -1 if illegal
int FileSystem::Get_dir_inodeNum_from_path(vector<string>& path) {
    int p = 0, inode_num;

    if(path.empty()) {
    	cout << "[Error] empty path!" << endl;
        return -1;
    }

    // initialization
    if(path[0] == "/") {
        ++p;
        inode_num = root_dir_inodeNum;
    } else {
        inode_num = cur_dir_inodeNum;
    }

    while(p < (int)path.size()) {
    	inode_num = Get_file_inodeNum_from_dir(inode_num, path[p]);

    	// if not found
    	if(inode_num == -1) {
    		cout << "[Error] Cannot find file: " << path[p] << endl;
    		return -1;
    	}
    	++p;
    }

    return inode_num;
}

/*
	Make sure that the inode points to a directory
	Find the inodeNum according to the filename

	return -1 if not found
*/
int FileSystem::Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename) {
	char inode_buf[64], buf[BLKsize];
	
	if(!Get_inode_from_inodeNum(inode_buf, dir_inodeNum)) {
		cout << "[Error] Cannot get inode: " << dir_inodeNum << endl;
		return -1;
	}

	// if inode doesn't point to a directory
	if(byte2int(inode_buf, 0, 2) == 0) {
		cout << "[Error] Inode " << dir_inodeNum << " is not a dir" << endl;
		return -1;
	}

	// get the directory data block
	D.Getblk(buf, byte2int(inode_buf, 42, 44));

	for(int i = 0; i < 1024; i += 12) {
		// check if filename EQU
		if(byteEQUstring(buf + i, 0, 8, filename)) {
			// check if is valid
			if(byte2int(buf + i, 8, 10) == 1) {
				// return inodeNum
				return byte2int(buf + i, 10, 12);
			}
		}
	}

	// not found
	return -1;
}

// return false if cannot get inode
bool FileSystem::Get_inode_from_inodeNum(char buf[], int inodeNum) {
	char BLKbuf[BLKsize];

	// get inode table
	D.Getblk(BLKbuf, Get_actual_inodeBLKnumber(inodeNum));

	// get inode
	for(int i = 0; i < 64; ++i) {
		buf[i] = BLKbuf[i + (inodeNum % 16) * 64];
	}

	return true;
}

// find next empty inode number, scan map
// return -1 if no space
int FileSystem::Find_empty_inodeNum() {
	for(int i = 0; i < 10; ++i) {
		// find a byte
		if((int)inode_bitmap[i] != -1) {
			// m -> mask
			for(int j = 0, m = 1; j < 8; ++j, m <<= 1) {
				// find a bit
				if((inode_bitmap[i] & m) == 0) {
					return i * 8 + j;
				}
			}
		}
	}
	return -1;
}

// find next empty data block number, scan map
// return -1 if no space
int FileSystem::Find_empty_dataBLKNum() {
	for(int i = 0; i < 80; ++i) {
		// find a byte
		if((int)data_block_bitmap[i] != -1) {
			// m -> mask
			for(int j = 0, m = 1; j < 8; ++j, m <<= 1) {
				// find a bit
				if((data_block_bitmap[i] & m) == 0) {
					return i * 8 + j;
				}
			}
		}
	}
	return -1;
}

// return false if cannot create file
bool FileSystem::CreateFile(const string &filepath) {
    /* 
        step 1: find the right directory
    */

    vector<string> path;
    string filename;
    if(!Decomposition_path(filepath, path) || (int)path.size() < 2) {
        cout << "[Error] Illegal path!" << endl;
        return false;
    }

    filename = path[path.size() - 1];
    path.pop_back();

    if(path.empty()) {
    	cout << "[Error] Empty path!" << endl;
    	return false;
    }

    int dir_inodeNum = Get_dir_inodeNum_from_path(path);

    /*
        step 2: make sure no files have the same name; otherwise, return FALSE
    */

	if(Get_file_inodeNum_from_dir(dir_inodeNum, filename) != -1) {
		cout << "[Error] File already exist" << endl;
		return false;
	}

    /*  
        step 3: search for empty index_node
    */

	int next_inodeNum = Find_empty_inodeNum();
	if(next_inodeNum == -1) {
		cout << "[Error] Cannot find an empty inode" << endl;
		return false;
	}

	/*
        step 4: search the block_bitmap for empty blocks
    */

    int next_dataBLKNum = Find_empty_dataBLKNum();
    if(next_dataBLKNum == -1) {
    	cout << "[Error] Cannot find an empty data block" << endl;
    	return false;
    }

    /*
        step 5: create new pair of mapping in its directory's data block
    */

    /*
    	step 6: initialize the first data block ( set the first block's first byte = 0xFF(EOF) )
    */

    /*
    	step 7: change inode bitmap and data block bitmap, save the super block
    */



    return true;
}

int FileSystem::Get_cur_dir_inodeNum() {
    return cur_dir_inodeNum;
}