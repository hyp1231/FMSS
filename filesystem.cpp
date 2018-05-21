#include "filesystem.h"

/*
    --------------------- Tool Functions ---------------------
*/

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
	for(int i = begin, t = 0; i < end && t < (int)str.size(); ++i, ++t) {
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
    cout << "[mkdir]:   create a new directory" << endl;
    cout << "[cd]:      go to the aimed directory" << endl;
    cout << "[mv]:      move file from inside/outside to inside/outside" << endl;
   	cout << "           path starting with ':' indicates outside path" << endl;
   	cout << "[cat]:     print file" << endl;
    cout << "[echo]:    write something into file" << endl;
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

// create a directory in dir "dir_inodeNum", and new dir_inode will be stored in dir_inodeNum
//      if already exists, return FALSE; otherwise, return TRUE
bool FileSystem::CreateInCurDir(int &dir_inodeNum, const string &filename, bool isFile) {
    /*
        step 1: make sure filename isn't too large; otherwise, return FALSE
    */

    if((int)filename.size() > 8) {
        cout << "[Error] filename is too long" << endl;
        return false;
    }

    /*
        step 2: make sure no files have the same name; otherwise, return FALSE
    */

    int new_dir_inodeNum = Get_file_inodeNum_from_dir(dir_inodeNum, filename);
    if(new_dir_inodeNum != -1) {
        dir_inodeNum = new_dir_inodeNum;
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
    if (isFile)
        Fill_byte_by_num(buf, st, st + 2, 0);
    else
        Fill_byte_by_num(buf, st, st + 2, 1);
    Fill_byte_by_str(buf, st + 2, st + 2 + (int)filename.size(), filename);
    Fill_byte_by_str(buf, st + 10, st + (int)curTime.size() + 10, curTime);
    Fill_byte_by_str(buf, st + 26, st + (int)curTime.size() + 26, curTime);
    Fill_byte_by_num(buf, st + 42, st + 44, next_dataBLKNum);

    D.Putblk(buf, Get_actual_inodeBLKnumber(next_inodeNum));

    // modify next data block
    D.Getblk(buf, Get_actual_dataBLKnumber(next_dataBLKNum));
    if (isFile) {
        // EOF
        Fill_byte_by_num(buf, 0, 1, -1);  
    }
    else {
        // set index of "."
        Fill_byte_by_str(buf, 0, 8, ".");
        Fill_byte_by_num(buf, 8, 10, 1);
        Fill_byte_by_num(buf, 10, 12, next_inodeNum);
        // set index of ".."
        Fill_byte_by_str(buf, 12, 20, "..");
        Fill_byte_by_num(buf, 20, 22, 1);
        Fill_byte_by_num(buf, 22, 24, dir_inodeNum);
    }
    D.Putblk(buf, Get_actual_dataBLKnumber(next_dataBLKNum));

    /*
        step 7: change inode bitmap and data block bitmap, save the super block
    */

    Fill_inode_bitmap(next_inodeNum, true);
    Fill_data_block_bitmap(next_dataBLKNum, true);
    Save_superBLK();

    dir_inodeNum = next_inodeNum;
    return true;
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

    if (path[0] == "/") {
        dir_inodeNum = root_dir_inodeNum;
        path.erase(path.begin());   // erase the root dir "/"
    }
    else
        dir_inodeNum = cur_dir_inodeNum;

    // apart from the last file, all other directories need to be created
    for (int i = 0; i < (int)path.size() - 1; i++) {
        // get to the next directory (dir_inodeNum points to its inode)
        // if dir exists, nothing happens; otherwise, create a new dir and return false
        CreateInCurDir(dir_inodeNum, path[i], false);
    }
    if (!CreateInCurDir(dir_inodeNum, path[path.size() - 1], true)) {
        // file already exists
        cout << "[Error] " << filepath << " already exists!" << endl;
        return false;
    }
    return true;
}

// return false if cannot delete file
bool FileSystem::DeleteFile(const string &filepath) {
    /* 
        step 1: find the right directory
    */

	string filename;
    int dir_inodeNum = Analysis_path(filepath, filename);
    if(dir_inodeNum == -1) {
    	return false;
    }

	if(filename == "." || filename == "..") {
        cout << "[Error] '.' or '..' cannot be removed" << endl;
        return false;
    }

    /*
        step 2: make sure file exists; otherwise, return FALSE
    */

    int file_inodeNum = Get_file_inodeNum_from_dir(dir_inodeNum, filename, false);
    if (file_inodeNum == -1) {
        cout << "[Error] File doesn't exist (cannot delete file \""
        << filename << "\")" << endl;
        return false;
    }

    /*
        step 3: delete data in file's data_BLK
        (set corresponding bit in data bitmap to 0)
    */

    bool isFile;
    char file_buf[64];
    Get_inode_from_inodeNum(file_buf, file_inodeNum);
    // judge if inode is a file_inode
    if (byte2int(file_buf, 0, 2) == 0)
        isFile = true;
    else 
        isFile = false;

    if (!isFile) {  // if it's dir, data should be deleted recursively
        char fileBLK[BLKsize];
        int dataNum = byte2int(file_buf, 42, 44);
        Fill_data_block_bitmap(dataNum, false);
        D.Getblk(fileBLK, Get_actual_dataBLKnumber(dataNum));
        for (int i = 24; i + 12 < BLKsize; i += 12) {
            if (byte2int(fileBLK, i + 8, i + 10) == 0) 
                continue;
            string tmp_filename = byte2string(fileBLK, i, i + 8);
            // delete all files in dir recursively
            DeleteFile(filepath + '/' + tmp_filename);
        }
    } else {    // if it's a file, just delete data block one by one
        for (int i = 42; i < 58; i += 2) {
            char fileBLK[BLKsize];
            int dataNum = byte2int(file_buf, i, i + 2);
            Fill_data_block_bitmap(dataNum, false);
            // if EOF, break
            D.Getblk(fileBLK, Get_actual_dataBLKnumber(dataNum));
            if (FindEOF(fileBLK) != -1)
                break;
        }
    }

    /*
        step 4: modify dir's data block to remove this file's inode
        (set valid to 0x0)
     */
    Get_file_inodeNum_from_dir(dir_inodeNum, filename, true);

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

    if (param == "-l") {
        cout << "Name      ";       // name
        cout << "       ";          // <DIR>
        cout << "     Create Time  "; // create_time
        cout << "Last Modified Time"; // modify_time
        cout << endl;
    }

    int cnt_per_line = 0;
    int max_per_line = 4;
    for (int i = 0; i + 12 < BLKsize; i += 12) {
        if (byte2int(dir_buf, i + 8, i + 10) != 0) {
            string name = byte2string(dir_buf, i, i + 8);
            // if not show hidden files and filename "." or ".."
            if((param != "-a" && param != "-l") && (name == "." || name == "..")) {
                continue;
            }
            else if (param == "-l") {
                char buf[64];
                int file_inodeNum = byte2int(dir_buf, i + 10, i + 12);
                Get_inode_from_inodeNum(buf, Get_actual_inodeBLKnumber(file_inodeNum));

                string create_time = byte2string(buf, 10, 26);
                string modify_time = byte2string(buf, 26, 42);
                cout << std::left << setw(10) << name << std::right;
                if (byte2int(buf, 0, 2) == 0)
                    cout << "     " << "  ";
                else
                    cout << "<DIR>" << "  ";
                cout << create_time << "    ";
                cout << modify_time << "    ";
                cout << endl;

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

inline bool FileSystem::isOutside_path(const string& path) {
	return path[0] == ':';
}

// store data in ifile to str
// return false if too large
bool FileSystem::Outside_file2str(ifstream& ifile, string& str) {
	string tmp;
	str.clear();	// start from empty
	while(getline(ifile, tmp)) {
		str += tmp + '\n';
		if((int)str.length() > 8 * BLKsize) {
			cout << "[Error] File is too large" << endl;
			return false;
		}
	}
	return true;
}

int FileSystem::Analysis_path(const string& filepath, string& filename) {
	vector<string> path;
    int dir_inodeNum;
    if(!Decomposition_path(filepath, path)) {
        cout << "[Error] Illegal path!" << endl;
        return -1;
    }

    if(path.empty()) {
        cout << "[Error] Empty path!" << endl;
        return -1;
    }

    if((int)path.size() >= 2) {
        filename = path[path.size() - 1];
    } else {
        filename = path[0]; // omit path
    }
    path.pop_back();
    
    if((int)filename.size() > 8) {
    	cout << "[Error] filename is too long" << endl;
    	return -1;
    }

    if((int)path.size() >= 1) {
        dir_inodeNum = Get_dir_inodeNum_from_path(path);
    } else {
        dir_inodeNum = cur_dir_inodeNum;    // omit path -> cur dir
    }

    return dir_inodeNum;
}
  
bool FileSystem::CreateDir(const string &filepath) {
    /* 
        step 1: find the right directory
    */

    vector<string> path;
    string dirname;
    int dir_inodeNum;
    if(!Decomposition_path(filepath, path)) {
        cout << "[Error] Illegal path!" << endl;
        return false;
    }

    if(path.empty()) {
        cout << "[Error] Empty path!" << endl;
        return false;
    }

    if (path[0] == "/") {
        dir_inodeNum = root_dir_inodeNum;
        path.erase(path.begin());   // erase the root dir "/"
    }
    else
        dir_inodeNum = cur_dir_inodeNum;

    // apart from the last file, all other directories need to be created
    for (int i = 0; i < (int)path.size() - 1; i++) {
        // get to the next directory (dir_inodeNum points to its inode)
        // if dir exists, nothing happens; otherwise, create a new dir and return false
        CreateInCurDir(dir_inodeNum, path[i], false);
    }
    if (!CreateInCurDir(dir_inodeNum, path[path.size() - 1], false)) {
        // dir already exists
        cout << "[Error] \"" << filepath << "\" already exists!" << endl;
        return false;
    }
    return true;
}

void FileSystem::OpenDir(const string &dirpath) {
    /*
        step 1: get to parent dir
     */

    vector<string> path;
    string dirname;
    int dir_inodeNum;
    if(!Decomposition_path(dirpath, path)) {
        cout << "[Error] Illegal path!" << endl;
        return ;
    }

    if(path.empty()) {
        cout << "[Error] Empty path!" << endl;
        return ;
    }

    if((int)path.size() >= 2) {
        dirname = path[path.size() - 1];
    } else {
        // if path is "/" (root directory), return
        if (path[0] == "/" && path.size() == 1) {
            cur_dir_inodeNum = root_dir_inodeNum;
            cur_path = path;
            return ;
        }
        dirname = path[0]; // omit path
    }
    path.pop_back();

    if((int)path.size() >= 1) {
        dir_inodeNum = Get_dir_inodeNum_from_path(path);
    } else {
        dir_inodeNum = cur_dir_inodeNum;    // omit path -> cur dir
    }
  
  if (dir_inodeNum == -1) {
        cout << "[Error] Directory \"" << dirpath << "\" doesn't exist!" << endl;
        return;
    }

    /*
        step 2: open dir
     */

    char inode_buf[64], dir_buf[BLKsize];
    Get_inode_from_inodeNum(inode_buf, dir_inodeNum);
    // get dir data block
    D.Getblk(dir_buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));

    for (int i = 0; i + 12 < BLKsize; i += 12) {
        if (byte2int(dir_buf, i + 8, i + 10) != 0) {
            // find file that has "dirname" in dir_buf
            if (byte2string(dir_buf, i, i + 8) == dirname) {
                char tmp_buf[64];
                int tmp_inodeNum = byte2int(dir_buf, i + 10, i + 12);
                Get_inode_from_inodeNum(tmp_buf, tmp_inodeNum);

                // make sure the found file is a directory
                if (byte2int(tmp_buf, 0, 2) == 0) {
                    cout << "[Error] \"" << dirpath << "\" is not a directory" << endl;
                    return;
                }

                // the found file is a directory
                //      modify cur_path
                path.push_back(dirname);
                // if path starts with '/', it's a complete path
                if (path[0] == "/") {
                    cur_dir_inodeNum = root_dir_inodeNum;
                    int cur_path_size = cur_path.size();
                    for (int i = 0; i < cur_path_size - 1; i++)
                        cur_path.pop_back();
                    path.erase(path.begin());
                }
                for (int i = 0; i < (int)path.size(); i++) {
                    if (path[i] == ".") continue;
                    else if (path[i] == "..") {
                        if (cur_dir_inodeNum != root_dir_inodeNum)
                            cur_path.pop_back();
                    }                            
                    else
                        cur_path.push_back(path[i]);
                }

                //      modify cur_dir_inodeNum
                cur_dir_inodeNum = tmp_inodeNum;

                return ;
            }
        }
    }

    cout << "[Error] Directory \"" << dirpath << "\" doesn't exist!" << endl;
    return;
}

// save inode
void FileSystem::PutInode(char buf[], int inodeNum) {
	char large_buf[BLKsize];
	D.Getblk(large_buf, Get_actual_inodeBLKnumber(inodeNum));
	int st = 64 * (inodeNum % 16);
	for(int i = 0; i < 64; ++i) {
		large_buf[i + st] = buf[i];
	}
	D.Putblk(large_buf, Get_actual_inodeBLKnumber(inodeNum));
}

bool FileSystem::str2Inside_file(string& file_data, const string& filepath) {
	/* 
        step 1: find the right directory
    */

	string filename;
    int dir_inodeNum = Analysis_path(filepath, filename);
    if(dir_inodeNum == -1) {
    	return false;
    }

    /*
        step 2: test if there is file having the same name
    */

    // exist file, delete to clear bitmap
	if(Get_file_inodeNum_from_dir(dir_inodeNum, filename) != -1) {
		if(!DeleteFile(filepath)) {
			cout << "[Error] Cannot delete file " << filename;
			return false;
		}
	}
	CreateFile(filepath);
	int file_inodeNum = Get_file_inodeNum_from_dir(dir_inodeNum, filename);

	/*
		step 3: allocate data blocks
	*/

	// EOF
	int BLKnum = (file_data.length() + (file_data.length() != 8 * BLKsize)) / BLKsize;
	char _EOF = -1;
	if(file_data.length() != 8 * BLKsize) {
		file_data += _EOF;
	}

	char buf[BLKsize], inode_buf[64];

	// first BLK
	Get_inode_from_inodeNum(inode_buf, file_inodeNum);
	Fill_byte_by_str(buf, 0, min(1024, (int)file_data.length()), file_data);
	file_data = file_data.substr(min(1024, (int)file_data.length()));
	D.Putblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42, 44)));

	for(int i = 1; i < BLKnum; ++i) {
		// find an empty data block
		int next_dataBLKNum = Find_empty_dataBLKNum();
		// set data block bitmap
		Fill_data_block_bitmap(next_dataBLKNum, true);
		// change inode.data_block_number
		Fill_byte_by_num(inode_buf, 42 + 2 * i, 44 + 2 * i, next_dataBLKNum);
		// write data
		Fill_byte_by_str(buf, 0, min(1024, (int)file_data.length()), file_data);
		// cut str
		file_data = file_data.substr(min(1024, (int)file_data.length()));
		D.Putblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, 42 + 2 * i, 44 + 2 * i)));
	}

	PutInode(inode_buf, file_inodeNum);
	Save_superBLK();

	return true;
}

// move file from source to destination
bool FileSystem::MoveFile(string& S_filepath, string& D_filepath) {
	if(isOutside_path(S_filepath)) {		// souce is outside path
		if(isOutside_path(D_filepath)) {	// both outside path
			cout << "[Error] Cannot move an outside file to another outside place" << endl;
			return false;
		} else {	// move from outside to inside
			/*
				step 1: test if can open the outside file, and change file to string (attention length)
			*/

			S_filepath = S_filepath.substr(1);

			ifstream ifile(S_filepath);
			if(!ifile) {	// cannot open
				cout << "[Error] Illegal outside path:" << S_filepath << endl;
				return false;
			}

			// store the data in outside file
			string file_data;

			if(!Outside_file2str(ifile, file_data)) {
				ifile.close();
				return false;
			}

			ifile.close();

			if(!str2Inside_file(file_data, D_filepath)) {
				return false;
			}
		}
	} else {		// souce is inside path
		if(isOutside_path(D_filepath)) {	// move from inside to outside
			D_filepath = D_filepath.substr(1);

			// convert inside file to string
			string file_data;
			if(!Inside_file2str(S_filepath, file_data)) {
				return false;
			}

			ofstream ofile(D_filepath);
			if(!ofile) {
				cout << "[Error] Cannot open file: " << D_filepath << endl;
				return false;
			}

			ofile << file_data;
			ofile.close();
		} else {	// both inside
			// convert inside file to string
			string file_data;
			if(!Inside_file2str(S_filepath, file_data)) {
				return false;
			}

			str2Inside_file(file_data, D_filepath);
		}
	}

	return true;
}

bool FileSystem::Inside_file2str(const string& filepath, string& str) {
	/*
		step 1: get dir inode number and filename
	*/
	string filename;
	int dir_inodeNum = Analysis_path(filepath, filename);
	if(dir_inodeNum == -1) {
		return false;
	}

	/*
		step 2: get file inode number and get the inode
	*/

	int file_inodeNum = Get_file_inodeNum_from_dir(dir_inodeNum, filename);
	if(file_inodeNum == -1) {
		return false;
	}

	char inode_buf[64];
	Get_inode_from_inodeNum(inode_buf, file_inodeNum);

	/*
		step 3: travel each data block, add to string until find EOF
	*/

	str.clear();
	char buf[BLKsize];
	for(int i = 42; i < 58; i += 2) {
		D.Getblk(buf, Get_actual_dataBLKnumber(byte2int(inode_buf, i, i + 2)));
		int pos = FindEOF(buf);
		if(pos == -1) {
			for(int j = 0; j < BLKsize; ++j) {
				str += buf[j];
			}
		} else {
			for(int j = 0; j < pos; ++j) {
				str += buf[j];
			}
			break;
		}
	}
	return true;
}

bool FileSystem::PrintFile(const string& filepath) {
	string file_data;
	Inside_file2str(filepath, file_data);
	cout << file_data;
	return true;
}

bool FileSystem::WriteFile(string& str, const string& filepath) {
    str2Inside_file(str, filepath);
    return true;
}

void FileSystem::Print_cur_path() {
    for(int i = 0; i < (int)cur_path.size(); ++i) {
        cout << cur_path[i];
        if(i != 0 && i != (int)cur_path.size() - 1) {
            cout << "/";
        }
    }
}
