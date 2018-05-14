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
	undefined_opt, _help, _exit, _create, _delete
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
	bool flag = true;
	switch(string_to_enum[opt[0]]) {
		case _help:			// help
			S.Help(); break;
		case _exit:			// exit
			flag = false; break;
        case _create:       // touch
            // create a new file
        case _delete:       // rm
            // delete a file
		case undefined_opt:	// unknowned
			cout << "Unknown option... ╮(￣▽￣"")╭" << endl; break;
		default: break;
	}
	return flag;
}

void Menu() {				// get opts and execute them
	FileSystem S;
	vector<string> opt;
	do {
		cout << ">>> ";
		opt.clear();
		get_opt(opt);
	} while(exec_opt(S, opt));
}

void initialization() {
	string_to_enum["help"] = _help;
	string_to_enum["exit"] = _exit;
    string_to_enum["touch"] = _create;
    string_to_enum["rm"] = _delete;
}

int main() {
	initialization();
	Menu();

	return 0;
}