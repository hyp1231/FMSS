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

// convert bytes( buf[begin, end) ) to string
//   conversion stops at END or '0x0' byte
string byte2string(char buf[], int begin, int end) {
    string ret = "";
    for (int i = begin; i < end; i++) {
        int tmp = byte2int(buf, i, i + 1);
        if (tmp == 0)
            break;
        ret += buf[i];
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
    while((p = (int)ori_path.find('/')) != -1) {
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

void Fill_byte_by_num(char buf[], int begin, int end, int num) {
	for(int i = begin; i < end; ++i) {
		buf[i] = num % 256;
		num /= 256;
	}
}

void Fill_byte_by_str(char buf[], int begin, int end, const string& str) {
	for(int i = begin, t = 0; i < end; ++i, ++t) {
		buf[i] = str[t];
	}
}

string getTime() {
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M",localtime(&timep) );
    return tmp;
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

    cur_path.push_back("/");
}

FileSystem::~FileSystem() {
	cout << "Thanks for using! See you later~ ♪───Ｏ（≧∇≦）Ｏ────♪" << endl;
}

void FileSystem::Help() {
	cout << "* ------------------------------------ *" << endl;
	cout << "[help]:    get help list" << endl;
	cout << "[exit]:    exit the system" << endl;
    cout << "[touch]:   create file" << endl;
    cout << "[rm]:      delete file" << endl;
    cout << "[ls]:      list files and directories in cur directory" << endl;
    cout << "           [-a] to show hidden files" << endl;
	cout << "* ------------------------------------ *" << endl;
}

// count from zero
inline int FileSystem::Get_actual_dataBLKnumber(int n) {
    return n + 6;
}

// count from zero
inline int FileSystem::Get_actual_inodeBLKnumber(int n) {
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
    if isDelete is TRUE, after finding the inodeNum, set valid byte to ZERO

	return -1 if not found
*/
int FileSystem::Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename, bool isDelete) {
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
	D.Getblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));

	for(int i = 0; i + 12 <= BLKsize; i += 12) {
		// check if filename EQU
		if(byteEQUstring(buf, i, i + (int)filename.size(), filename)) {
			// check if is used
			if(byte2int(buf, i + 8, i + 10) == 1) {
				// return inodeNum
                if (isDelete) {
                    Fill_byte_by_num(buf, i + 8, i + 10, 0);
                    D.Putblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));
                }
				return byte2int(buf, i + 10, i + 12);
                // int file_inodeNum = byte2int(buf, i + 10, i + 12);
                // if (isDelete) {
                //     Fill_inode_bitmap(file_inodeNum,false);
                // }
                // return file_inodeNum;
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

void FileSystem::Fill_inode_bitmap(int inodeNum, bool flag) {
	if(flag) {
		inode_bitmap[inodeNum / 10] |= (1 << (inodeNum % 10));
	} else {
		inode_bitmap[inodeNum / 10] &= ~(1 << (inodeNum % 10));
	}
}

void FileSystem::Fill_data_block_bitmap(int blockNum, bool flag) {
	if(flag) {
		data_block_bitmap[blockNum / 80] |= (1 << (blockNum % 80));
	} else {
		data_block_bitmap[blockNum / 80] &= ~(1 << (blockNum % 80));
	}
}

void FileSystem::Save_superBLK() {
	char buf[BLKsize];
	D.Getblk(buf, 0);

	// save inode bitmap
    for(int i = 3; i <= 12; ++i) {
        buf[i] = inode_bitmap[i - 3];
    }

    // save data block bitmap
    for(int i = 13; i <= 92; ++i) {
        buf[i] = data_block_bitmap[i - 13];
    }

    D.Putblk(buf, 0);
}

// find EOF in a file, return the byte_num if EOF in this block
//    else return -1
int FileSystem::FindEOF(char buf[]) {
    for (int i = 0; i < BLKsize - 1; i++) {
        int num = byte2int(buf, i, i + 2);
        if (num == -1)
            return i;
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
    int dir_inodeNum;
    if(!Decomposition_path(filepath, path)) {
        cout << "[Error] Illegal path!" << endl;
        return false;
    }

    if(path.empty()) {
        cout << "[Error] Empty path!" << endl;
        return false;
    }

    if((int)path.size() >= 2) {
        filename = path[path.size() - 1];
    } else {
        filename = path[0]; // omit path
    }
    path.pop_back();
    
    if((int)filename.size() > 8) {
    	cout << "[Error] filename is too long" << endl;
    	return false;
    }

    if((int)path.size() >= 2) {
        dir_inodeNum = Get_dir_inodeNum_from_path(path);
    } else {
        dir_inodeNum = cur_dir_inodeNum;    // omit path -> cur dir
    }

    /*
        further plan: mkdir
    */

    /*
        step 2: make sure no files have the same name; otherwise, return FALSE
    */

	if(Get_file_inodeNum_from_dir(dir_inodeNum, filename) != -1) {
		cout << "[Error] File already exists" << endl;
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

    char inode_buf[64], buf[BLKsize];
    Get_inode_from_inodeNum(inode_buf, dir_inodeNum);

    // modify directory data block
    D.Getblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));
    for(int i = 0; i + 12 <= BLKsize; i += 12) {
    	// find used = 0
    	if(byte2int(buf, i + 8, i + 10) == 0) {
    		// set used = 1;
    		Fill_byte_by_num(buf, i + 8, i + 10, 1);
    		// set filename
    		Fill_byte_by_str(buf, i, i + (int)filename.size(), filename);
    		// zero end of str
    		if((int)filename.size() < 8) {
    			buf[i + filename.size()] = 0;
    		}

    		// set next inodeNum
    		Fill_byte_by_num(buf, i + 10, i + 12, next_inodeNum);

    		break;
    	} 
    }
    D.Putblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));

    /*
    	step 6: initialize the inode and the first data block ( set the first block's first byte = 0xFF(EOF) )
    */

    // modify next inode
    D.Getblk(buf, Get_actual_inodeBLKnumber(next_inodeNum));
    int st = 64 * (next_inodeNum % 16);

    string curTime = getTime();
    // set flag
    Fill_byte_by_num(buf, st, st + 2, 0);
    Fill_byte_by_str(buf, st + 2, st + 2 + (int)filename.size(), filename);
    Fill_byte_by_str(buf, st + 10, st + (int)curTime.size() + 10, curTime);
    Fill_byte_by_str(buf, st + 26, st + (int)curTime.size() + 26, curTime);
    Fill_byte_by_num(buf, st + 42, st + 44, next_dataBLKNum);

    D.Putblk(buf, Get_actual_inodeBLKnumber(next_inodeNum));

    // modify next data block
    D.Getblk(buf, Get_actual_dataBLKnumber(next_dataBLKNum));
    // EOF
    Fill_byte_by_num(buf, 0, 1, -1);
    D.Putblk(buf, Get_actual_dataBLKnumber(next_dataBLKNum));

    /*
    	step 7: change inode bitmap and data block bitmap, save the super block
    */

    Fill_inode_bitmap(next_inodeNum, true);
    Fill_data_block_bitmap(next_dataBLKNum, true);
    Save_superBLK();

    return true;
}

// return false if cannot delete file
bool FileSystem::DeleteFile(const string &filepath) {
    /* 
        step 1: find the right directory
    */

    vector<string> path;
    string filename;
    int dir_inodeNum;
    if(!Decomposition_path(filepath, path)) {
        cout << "[Error] Illegal path!" << endl;
        return false;
    }

    if(path.empty()) {
        cout << "[Error] Empty path!" << endl;
        return false;
    }

    if((int)path.size() >= 2) {
        filename = path[path.size() - 1];
    } else {
        filename = path[0]; // omit path
    }
    path.pop_back();

    if((int)filename.size() > 8) {
        cout << "[Error] filename is too long" << endl;
        return false;
    }

    if(filename == "." || filename == "..") {
        cout << "[Error] '.' or '..' cannot be removed" << endl;
        return false;
    }

    if((int)path.size() >= 2) {
        dir_inodeNum = Get_dir_inodeNum_from_path(path);
    } else {
        dir_inodeNum = cur_dir_inodeNum;    // omit path -> cur dir
    }

    /*
        step 2: make sure file exists; otherwise, return FALSE
        step 3: modify dir's data block to remove this file's inode
        (set valid to 0x0)
    */

    int file_inodeNum = Get_file_inodeNum_from_dir(dir_inodeNum, filename, true);
    if (file_inodeNum == -1) {
        cout << "[Error] File doesn't exist (cannot delete file \""
        << filename << "\")" << endl;
        return false;
    }

    /*
        step 4: delete data in file's data_BLK
        (set corresponding bit in data bitmap to 0)
    */

    char file_buf[64];
    Get_inode_from_inodeNum(file_buf, file_inodeNum);
    for (int i = 42; i < 58; i += 2) {
        char fileBLK[BLKsize];
        int dataNum = byte2int(file_buf, i, i + 2);
        Fill_data_block_bitmap(dataNum, false);
        // if EOF, break
        D.Getblk(fileBLK, Get_actual_dataBLKnumber(dataNum));
        if (FindEOF(fileBLK) != -1)
            break;
    }

    /*
        step 5: remove file's inode
        (set corresponding bit in inode bitmap to 0)
    */

    Fill_inode_bitmap(file_inodeNum, false);

    /*
        step 6: save the super block
    */

    Save_superBLK();

    return true;
}

void FileSystem::ListFile(const string param) {
    /* 
        step 1: get current dir block
    */

    char inode_buf[64], dir_buf[BLKsize];
    Get_inode_from_inodeNum(inode_buf, cur_dir_inodeNum);
    // get dir data block
    D.Getblk(dir_buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));

    /* 
        step 2: list all files in the dir
    */

    int cnt_per_line = 0;
    int max_per_line = 4;
    // here is a bug: BLKsize = 0 ????
    for (int i = 0; i + 12 <= BLKsize; i += 12) {
        if (byte2int(dir_buf, i + 8, i + 10) != 0) {
            string name = byte2string(dir_buf, i, i + 8);
            // if not show hidden files and filename start with '.'
            if(param != "-a" && name[0] == '.') {
                continue;
            }

            cout << setw(10) << name;
            cnt_per_line++;
            if (cnt_per_line == max_per_line) {
                cnt_per_line = 0;
                cout << endl;
            }
        }
    }
    if(cnt_per_line != 0) {
        cout << endl;
    }
}

int FileSystem::Get_cur_dir_inodeNum() {
    return cur_dir_inodeNum;
}

void FileSystem::Print_cur_path() {
	for(int i = 0; i < (int)cur_path.size(); ++i) {
		if(i != 0 && i != (int)cur_path.size() - 1) {
			cout << "/";
		}
		cout << cur_path[i];
	}
}
