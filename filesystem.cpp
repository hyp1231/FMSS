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

// return -1 if not found
int FileSystem::Get_file_inodeNum_from_dir(int dir_inodeNum, const string& filename) {
	char buf[BLKsize];
	//D.Getblk(buf, )
	/*
		TBA
	*/
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

int FileSystem::Get_cur_dir_inodeNum() {
    return cur_dir_inodeNum;
}