/*
 *	Simple File Management System
 *
 *	A simulation of Unix's file management system.
 *	Developed by "Prime Luo" and his buddy "Yupeng Hou"
 *
 *	Author1: Prime Luo (Sijie Luo)
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
	undefined_opt, opt0, opt1
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
		case opt0:			// help
			S.Help(); break;
		case opt1:			// exit
			flag = false; break;
		case undefined_opt:	// unknowned
			cout << "Unknowned option... ╮(￣▽￣"")╭" << endl; break;
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
	string_to_enum["help"] = opt0;
	string_to_enum["exit"] = opt1;
}

int main() {
	initialization();
	Menu();

	return 0;
}