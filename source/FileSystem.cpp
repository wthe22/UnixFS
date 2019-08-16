#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
#include "FileSystem.h"
using namespace std;


int FileSystem::init(int disk_size, int block_size) {
    // ! Warning: calculations needs to be verified
	sb.blocks_count = disk_size/block_size;
	sb.inodes_count = sb.blocks_count / 2; // ! Estimated value. Closer to blocks_count is better
	sb.block_size = block_size;
	sb.rev_level = REV_LEVEL;
	sb.first_inode = inode_first;

	sb.block_bitmap = (sizeof(Superblock)-1) / sb.block_size + 1;
	sb.inode_bitmap = sb.block_bitmap + (sb.blocks_count-1) / sb.block_size + 1;
	sb.inode_table = sb.inode_bitmap + (sb.inodes_count-1) / sb.block_size + 1;

    sb.disk_size = disk_size;

	int first_data_block = sb.inode_table + (sb.inodes_count * sizeof(Inode) - 1) / sb.block_size + 1;

	// Allocate virtual disk
	disk = make_unique<char[]>(disk_size);
	object_write(0, sb);

	// Mark bitmaps
	for (int i = 0; i < first_data_block; i++)
		bit_write(sb.block_bitmap * sb.block_size, i, USED);
	for (int i = first_data_block; i < sb.blocks_count; i++)
		bit_write(sb.block_bitmap * sb.block_size, i, UNUSED);
	for (int i = 0; i < sb.first_inode; i++)
		bit_write(sb.inode_bitmap * sb.block_size, i, USED);
	for (int i = sb.first_inode; i < sb.inodes_count; i++)
		bit_write(sb.inode_bitmap * sb.block_size, i, UNUSED);

    // Initialize root directory
    bit_write(sb.inode_bitmap, inode_root, USED);
	object_write(sb.inode_table * sb.block_size + inode_root * sizeof(Inode), Inode(Inode::DIRECTORY));
    dir_entry_add(inode_root, DirEntry(2, "."));
    dir_entry_add(inode_root, DirEntry(2, ".."));

	return SUCCESS;
}


int FileSystem::display_properties() {
	int first_data_block = sb.inode_table + (sb.inodes_count * sizeof(Inode) - 1) / sb.block_size + 1;

	int used_inodes_count = 0;
	for (int i = 0; i < sb.inodes_count; i++)
		used_inodes_count += bit_read(sb.inode_bitmap * sb.block_size, i);

	int used_blocks_count = 0;
	for (int i = 0; i < sb.blocks_count; i++)
		used_blocks_count += bit_read(sb.block_bitmap * sb.block_size, i);

	cout << "Used inodes: " << used_inodes_count << "/" << sb.inodes_count << " (" << (used_inodes_count * 100 / sb.inodes_count) << "%)" << endl;
	cout << "Used blocks: " << used_blocks_count << "/" << sb.blocks_count << " (" << (used_blocks_count * 100 / sb.blocks_count) << "%)" << endl;
    cout << endl;
	cout << "Disk size: " << sb.disk_size << endl;
	cout << "Block size: " << sb.block_size << endl;
    cout << endl;
	cout << "File system revision: " << sb.rev_level << endl;
    cout << endl;
	cout << "Block Bitmap: " << sb.block_bitmap << endl;
	cout << "Inode Bitmap: " << sb.inode_bitmap << endl;
	cout << "Inode Table: " << sb.inode_table << endl;
	cout << endl;
	cout << "First data block: " << first_data_block << endl;
	return SUCCESS;
}


bool FileSystem::bit_read(int byte_offset, int bit_offset) {
	char value = 0;
	object_read(byte_offset + bit_offset / 8, &value);
	int index = 8 - bit_offset % 8 - 1;
	return bool((value >> index) & 1);
}

bool FileSystem::bit_write(int byte_offset, int bit_offset, bool is_used) {
	char value = 0;
	object_read(byte_offset + bit_offset / 8, &value);
	int index = 8 - bit_offset % 8 - 1;
    value &= ~(1 << index);
    value |= is_used << index;
	object_write(byte_offset + bit_offset / 8, value);
	return true;
}

int FileSystem::bit_unused(int byte_offset, int size) {
	for (int i = 0; i < size; i++) {
		if (bit_read(byte_offset, i) == UNUSED)
			return i;
	};
	return -1;
}


int FileSystem::save(string filepath) {
	fstream file(filepath, ios::out | ios::binary);
    if (!file)
        return FAILED;
	file.write(disk.get(), sb.disk_size);
	file.close();
    
	return SUCCESS;
}

int FileSystem::load(string filepath) {
	fstream file(filepath, ios::in | ios::binary);
    if (!file)
        return NOT_EXIST;
    file.seekg (0, file.end);
    sb.disk_size = (int) file.tellg();
    
    file.seekg (0, file.beg);
    file.read((char*) &sb, sizeof(Superblock));

    file.seekg (0, file.beg);
	disk = make_unique<char[]>(sb.disk_size);
	file.read(disk.get(), sb.disk_size);
	
	object_read(0, &sb);

	if (sb.rev_level != REV_LEVEL)
		return INCOMPATIBLE;
	return SUCCESS;
}


int FileSystem::dir_entry_find(int inode_num, const char* name, int indirect) {
	Inode dir_inode;
	object_read(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), &dir_inode);
	int max_entries = sb.block_size / sizeof(DirEntry);

	if (indirect == 0) {
		for (int i = 0; i < Inode::direct_blocks_count; i++) {
			int block_num = dir_inode.direct_blocks[i];
			if (block_num == 0 && strcmp(name, DirEntry().name) == 0) {
				block_num = bit_unused(sb.block_bitmap * sb.block_size, sb.blocks_count);
				bit_write(sb.block_bitmap * sb.block_size, block_num, USED);

				dir_inode.direct_blocks[i] = block_num;
				object_write(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), dir_inode);

				return block_num * sb.block_size;
			}
			for (int j = 0; j < max_entries; j++) {
				DirEntry current;
				object_read(block_num * sb.block_size + j * sizeof(DirEntry), &current);
				if (strcmp(name, current.name) == 0)
					return (block_num * sb.block_size + j * sizeof(DirEntry));
			}
		}
	}
	// ! Indirect block support
	return 0;
}

int FileSystem::dir_entry_add(int inode_num, DirEntry entry) {
	Inode dir_inode;
	object_read(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), &dir_inode);

	int entry_offset = dir_entry_find(inode_num, "");
	if (entry_offset == 0)
		return FAILED;
	object_write(entry_offset, entry);
	return SUCCESS;
}

int FileSystem::inode_of(string path, int parent_inode) {
	string filename = path;
	string next = "";
	if (path.rfind("/") != string::npos) {
		filename = path.substr(0, path.find("/"));
		next = path.substr(path.find("/") + 1);
	}

	if (filename == "") {
		if (next == "")
			return inode_root;
		else
			return inode_of(next, inode_root);
	}

	if (bit_read(sb.inode_bitmap * sb.block_size, parent_inode) == UNUSED)
		return 0;

	int entry_offset = dir_entry_find(parent_inode, filename.c_str());
	if (entry_offset == 0)
		return 0;

	DirEntry entry;
	object_read(entry_offset, &entry);
	if (next == "")
		return entry.inode;
	return inode_of(next, entry.inode);
}

int FileSystem::blocks_free_all(int inode_num, int indirect) {
	Inode inode;
	object_read(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), &inode);

	if (indirect == 0) {
		for (int i = 0; i < Inode::direct_blocks_count; i++) {
			int block_num = inode.direct_blocks[i];
			if (block_num != 0)
				bit_write(sb.block_bitmap * sb.block_size, block_num, UNUSED);
		}
	}
	return SUCCESS;
}


string FileSystem::path_abspath(string fullpath) {
	int inode_num = inode_of(fullpath);
	if (inode_num == 0)
		return "";

	vector<int> inode_nums = {inode_root};
	vector<string> dir_names = {""};
	string filename;
	string next = fullpath;
	do {
		if (next.rfind("/") == string::npos) {
			filename = next;
			next = "";
		} else {
			filename = next.substr(0, next.find("/"));
			next = next.substr(next.find("/") + 1);
		}
		if (filename == "")
			continue;

		int current_inode_num = inode_of(filename, inode_nums.back());
		bool match = false;
		for (size_t i = 0; i < inode_nums.size(); i++) {
			if (inode_nums[i] == current_inode_num) {
				inode_nums.erase(inode_nums.begin() + i + 1, inode_nums.end());
				dir_names.erase(dir_names.begin() + i + 1, dir_names.end());
				match = true;
				break;
			}
		}
		if (!match) {
			inode_nums.push_back(current_inode_num);
			dir_names.push_back(filename);
		}
	} while (next != "");

	string abspath = "";
	for (size_t i = 0; i < dir_names.size(); i++)
		abspath += "/" + dir_names[i];
	if (abspath.substr(0, 2) == "//")
		abspath.erase(0, 1);

	return abspath;
}

int FileSystem::type_of(string fullpath) {
	int inode_num = inode_of(fullpath);
	if (inode_num == 0) 
		return Inode::UNKNOWN;

	Inode inode;
	object_read(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), &inode);
	return inode.file_type;
}

int FileSystem::dir_list(string fullpath) {
	int inode_num = inode_of(fullpath);
	if (inode_num == 0) 
		return NOT_EXIST;

	Inode dir_inode;
	object_read(sb.inode_table * sb.block_size + inode_num * sizeof(Inode), &dir_inode);
	if (dir_inode.file_type != Inode::DIRECTORY)
		return NOT_DIR;

    cout << " " << right << setw(5) << "Inode"
        << "   " << left << setw(28) << "Name"
        << "   " << left << setw(10) << "Type"
        << "   " << left << setw(14) << "Modified Time"
        << endl;
	int max_entries = sb.block_size / sizeof(DirEntry);
	for (int i = 0; i < Inode::direct_blocks_count; i++) {
		int block_num = dir_inode.direct_blocks[i];
		if (block_num == 0)
			continue;

		for (int j = 0; j < max_entries; j++) {
			DirEntry entry;
			object_read(block_num * sb.block_size + j * sizeof(DirEntry), &entry);
			if (entry.inode == 0)
				continue;
            
			Inode inode;
			object_read(sb.inode_table * sb.block_size + entry.inode * sizeof(Inode), &inode);
            string file_type;
            switch(inode.file_type) {
            case Inode::UNKNOWN: file_type = "Unknown"; break;
            case Inode::FILE: file_type = "File"; break;
            case Inode::DIRECTORY: file_type = "Directory"; break;
            default: file_type = "Unknown"; break;
            }
			cout << " " << right << setw(5) << entry.inode
                << "   " << left << setw(28) << entry.name
                << "   " << left << setw(10) << file_type
                << "   " << left << setw(14) << inode.mod_time
                << endl;
		}
	}
	return SUCCESS;
}

int FileSystem::dir_create(string path, string name) {
	int path_inode_num = inode_of(path);
	if (path_inode_num == 0)
		return NOT_EXIST;
	
	int new_inode_num = inode_of(name, path_inode_num);
	if (new_inode_num != 0)
		return ALREADY_EXIST;

	new_inode_num = bit_unused(sb.inode_bitmap * sb.block_size, sb.inodes_count);
	bit_write(sb.inode_bitmap * sb.block_size, new_inode_num, USED);
	object_write(sb.inode_table * sb.block_size + new_inode_num * sizeof(Inode), Inode(Inode::DIRECTORY));
	dir_entry_add(new_inode_num, DirEntry(new_inode_num, "."));
	dir_entry_add(new_inode_num, DirEntry(path_inode_num, ".."));

	dir_entry_add(path_inode_num, DirEntry(new_inode_num, name.c_str()));

	return SUCCESS;
}

int FileSystem::dir_remove(string path, string name) {
	if (name == "." || name == "..")
		return FAILED;

	int path_inode_num = inode_of(path);
	if (path_inode_num == 0)
		return NOT_EXIST;

	int target_inode_num = inode_of(name, path_inode_num);
	if (target_inode_num == 0)
		return NOT_EXIST;

	Inode path_inode;
	object_read(sb.inode_table * sb.block_size + target_inode_num * sizeof(Inode), &path_inode);
	if (path_inode.file_type != Inode::DIRECTORY)
		return NOT_DIR;

	int entry_offset = dir_entry_find(path_inode_num, name.c_str());
	if (entry_offset == 0)
		return FAILED;
	object_write(entry_offset, DirEntry());
	bit_write(sb.inode_bitmap * sb.block_size, target_inode_num, UNUSED);

	// ! Free up blocks if blocks does not contain any entry

	return SUCCESS;
}

int FileSystem::file_create(string path, string name, int size) {
	int path_inode_num = inode_of(path);
	if (path_inode_num == 0)
		return NOT_EXIST;

	int new_inode_num = inode_of(name, path_inode_num);
	if (new_inode_num != 0)
		return ALREADY_EXIST;

	new_inode_num = bit_unused(sb.inode_bitmap * sb.block_size, sb.inodes_count);
	bit_write(sb.inode_bitmap * sb.block_size, new_inode_num, USED);
    Inode new_inode;
    new_inode.file_type = Inode::FILE;
    new_inode.size = size;
    new_inode.mod_time = (int) time(0);

	// ! Fill file with random values
    
	object_write(sb.inode_table * sb.block_size + new_inode_num * sizeof(Inode), new_inode);
	dir_entry_add(path_inode_num, DirEntry(new_inode_num, name.c_str()));

	return SUCCESS;
}

int FileSystem::file_remove(string path, string name) {
	int path_inode_num = inode_of(path);
	if (path_inode_num == 0)
		return NOT_EXIST;

	int target_inode_num = inode_of(name, path_inode_num);
	if (target_inode_num == 0)
		return NOT_EXIST;

	Inode path_inode;
	object_read(sb.inode_table * sb.block_size + target_inode_num * sizeof(Inode), &path_inode);
	if (path_inode.file_type != Inode::FILE)
		return NOT_FILE;

	blocks_free_all(target_inode_num);
	bit_write(sb.inode_bitmap * sb.block_size, target_inode_num, UNUSED);

	int entry_offset = dir_entry_find(path_inode_num, name.c_str());
	if (entry_offset == 0)
		return FAILED;
	object_write(entry_offset, DirEntry());

	return SUCCESS;
}

int FileSystem::file_display(string fullpath) {
	int file_inode_num = inode_of(fullpath);
	if (file_inode_num == 0)
		return NOT_EXIST;

	Inode file_inode;
	object_read(sb.inode_table * sb.block_size + file_inode_num * sizeof(Inode), &file_inode);
	if (file_inode.file_type != Inode::FILE)
		return NOT_FILE;

	// ! Display file content!!!
	// ! WARNING: THIS IS BLACK MAGIC
	srand(file_inode.mod_time);
	for (int i = 0; i < file_inode.size; i++)
		cout << (rand() % 10);
	cout << endl;

	return SUCCESS;
}

int FileSystem::file_copy(string source, string dest_dir, string dest_name) {
	int source_inode_num = inode_of(source);
	if (source_inode_num == 0)
		return NOT_EXIST;

	int dest_inode_num = inode_of(dest_dir);
	if (dest_inode_num == 0)
		return NOT_EXIST;

	int new_inode_num = inode_of(dest_name, dest_inode_num);
	if (new_inode_num != 0)
		return ALREADY_EXIST;

	Inode source_inode;
	object_read(sb.inode_table * sb.block_size + source_inode_num * sizeof(Inode), &source_inode);
	new_inode_num = bit_unused(sb.inode_bitmap * sb.block_size, sb.inodes_count);
	bit_write(sb.inode_bitmap * sb.block_size, new_inode_num, USED);
	object_write(sb.inode_table * sb.block_size + new_inode_num * sizeof(Inode), source_inode);

	dir_entry_add(dest_inode_num, DirEntry(new_inode_num, dest_name.c_str()));

	// ! Copy file contents

	return SUCCESS;
}
