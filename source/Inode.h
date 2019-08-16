#pragma once
#ifndef INODE_H
#define INODE_H

#include <ctime>
#include <iostream>
#include <string>
#include "Support.h"


struct DirEntry {
	int inode = 0;
	char name[60] = "";
    
	DirEntry() {}
    
	DirEntry(int inode_num, const char* file_name) {
		inode = inode_num;
		strcpy_s(name, file_name);
	}
};


struct Inode {
	// Constants
	static const int direct_blocks_count = 10;

	// Flags
	static const int UNKNOWN = 0x0;   // Unknown
	static const int FILE = 0x1;   // Regular file
	static const int DIRECTORY = 0x2;   // Directory

	int file_type = 0;
	int size = 0;
	int mod_time = (int) time(0);
	int direct_blocks[direct_blocks_count] = {0};
	int indirect_block = 0;
	char reserved[8] = { 0 };
    
	Inode() {}
    
	Inode(int file_type, int size = 0) {
		this->file_type = file_type;
		this->size = size;
	}
    
	static std::string strof_file_type(int file_type) {
        switch(file_type) {
        case Inode::UNKNOWN: return "Unknown"; break;
        case Inode::FILE: return "File"; break;
        case Inode::DIRECTORY: return "Directory"; break;
        default: return "Unknown"; break;
        }
	}
};

#endif