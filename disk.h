#pragma once

class Disk {
private:

public:
	Disk();
	~Disk();

	void Getblk(char buf[], int blk_num);
	void Putblk(char buf[], int blk_num);
};