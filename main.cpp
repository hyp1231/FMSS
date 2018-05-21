/*
 *	Simple File Management System
 *
 *	A simulation of Unix's file management system.
 *	Developed by "Prime Luo" and his buddy "Yupeng Hou"
 *
 *	Author1: Sijie Luo
 * 	ID:		 2016202186
 *
 *	Author2: Yupeng Hou
 *	ID:		 2016202190
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include "filesystem.h"
using namespace std;

enum enum_opts {
	undefined_opt, _help, _exit, _create, _delete, _list, _mkdir,
    _cd
};

static map<string, enum_opts> string_to_enum;

void get_opt(vector<string>& opt) {
	string line, tmp;
	getline(cin, line);
	stringstream ss(line);
	while(ss >> tmp) {
		opt.push_back(tmp);
	}
}

bool exec_opt(FileSystem& S, vector<string>& opt) {
	if(opt.empty()) { return true; }
	bool flag = true; string str;
	switch(string_to_enum[opt[0]]) {
		case _help:			// help
			S.Help(); break;
		case _exit:			// exit
			flag = false; break;
        case _create:       // touch
            S.CreateFile(opt[1]); break;
        case _delete:       // rm
            S.DeleteFile(opt[1]); break;
        case _list:         // ls
            if ((int)opt.size() > 2) {
                cout << "Too many parameters in command \"ls\"" << endl;
                S.Help();
            } else if((int)opt.size() == 2) {	// -a
            	S.ListFile(opt[1]);
            } else {					// none
            	S.ListFile();
            } 
            break;
        case _mkdir:        // mkdir
            S.CreateDir(opt[1]); break;
        case _cd:           // cd
            if ((int)opt.size() < 2) {
                cout << "Directory path should be provided" << endl;
                cout << "cd [dirpath]" << endl;
                break;
            }
            S.OpenDir(opt[1]); break;
		case undefined_opt:	// unknowned
			cout << "Unknown option..." << endl; break;
		default: break;
	}
	return flag;
}

void Menu() {				// get opts and execute them
	FileSystem S;
	vector<string> opt;
	do {
		cout << "[";
		S.Print_cur_path();
		cout << "]>>> ";
		opt.clear();
		get_opt(opt);
	} while(exec_opt(S, opt));
}

void initialization() {
	string_to_enum["help"] = _help;
	string_to_enum["exit"] = _exit;
    string_to_enum["touch"] = _create;
    string_to_enum["rm"] = _delete;
    string_to_enum["ls"] = _list;
    string_to_enum["mkdir"] = _mkdir;
    string_to_enum["cd"] = _cd;
}

int main() {
	initialization();
	Menu();

	return 0;
}
