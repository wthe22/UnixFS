#pragma once
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>
#include <memory>
#include "Inode.h"
using std::string;


struct Superblock {
	int inodes_count;
	int blocks_count;
	int block_size;
	int rev_level;
	int first_inode;

	int block_bitmap;
	int inode_bitmap;
	int inode_table;
    
	int disk_size;

	//char reserved[8] = { 0 };
};


class FileSystem {
public:
    // Constants
	static const int REV_LEVEL = 3;

    // Functions
	int init(int disk_size, int block_size);
    int display_properties();
	std::string path_abspath(std::string fullpath);
	int type_of(std::string fullpath);

	int save(std::string filepath);
	int load(std::string filepath);

	int dir_list(std::string fullpath);
	int dir_create(std::string path, std::string name);
	int dir_remove(std::string path, std::string name); // !

	int file_create(std::string path, std::string name, int size); // !
	int file_remove(std::string path, std::string name);
	int file_display(std::string fullpath); // !
	int file_copy(std::string source_file, std::string dest_dir, string dest_name); // !

	// Return codes
	static const int SUCCESS = 0x0;
	static const int FAILED = 0x1;
	static const int NOT_EXIST = 0x2;
	static const int ALREADY_EXIST = 0x3;
	static const int INCOMPATIBLE = 0x4;
	static const int NOT_FILE = 0x5;
	static const int NOT_DIR = 0x6;
	
private:
	// Constants
	static const int inode_root = 2;
	static const int inode_first = 11;

    // Flags
    static const bool USED = true;
    static const bool UNUSED = false;

	// Variables
	std::unique_ptr<char[]> disk;
	Superblock sb;

	// Read/write operations
	template<typename T> bool object_write(int byte_offset, T data);
	template<typename T> bool object_read(int byte_offset, T* data);
	template<typename T> bool block_write(int block_offset, T data);
	template<typename T> bool block_read(int block_offset, T* data);

	// Bitmap functions
	bool bit_read(int byte_offset, int bit_offset);
	bool bit_write(int byte_offset, int bit_offset, bool is_used);
	int bit_unused(int byte_offset, int size);
    
    // Blocks operations
    // ! Implement generator function
	int dir_entry_find(int inode_num, const char* name, int indirect = 0); // ! Implement indirect block support
	int dir_entry_add(int inode_num, DirEntry entry);
	int inode_of(std::string fullpath, int parent_inode = 0);
	int blocks_free_all(int inode_num, int indirect = 0);
};


template<typename T>
bool FileSystem::object_write(int byte_offset, T data) {
	*((T*)(disk.get() + byte_offset)) = data;
	return true;
}

template<typename T>
bool FileSystem::object_read(int byte_offset, T* data) {
	*data = *((T*)(disk.get() + byte_offset));
	return true;
}

template<typename T>
bool FileSystem::block_write(int block_offset, T data) {
	memcpy(disk.get() + block_offset * sb.block_size, data, sb.block_size);
	return true;
}

template<typename T>
bool FileSystem::block_read(int block_offset, T* data) {
	memcpy(data, disk.get() + block_offset * sb.block_size, sb.block_size);
	return true;
}

#endif
