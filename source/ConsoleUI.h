#pragma once
#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include <string>
#include <vector>
#include "FileSystem.h"


class ConsoleUI {
public:
    // Constants
	static const int REV_LEVEL = 4;
    static const char* memory_disk_symbol;
	static const char* DEFAULT_DISK_FILE;

	// Entry point
	int main_loop();
	static int about(int argc = 0, char** argv = NULL);
    
private:
	// Flags
	static const int SUCCESS = 0x0;
	static const int FAILED = 0x1;
	static const int NOT_EXIST = 0x2;
	static const int ALREADY_EXIST = 0x3;
	static const int INCOMPATIBLE = 0x4;
	static const int NOT_FILE = 0x5;
	static const int NOT_DIR = 0x6;

	static const int INVALID_COMMAND = 0x100;
	static const int INVALID_SYNTAX = 0x200;
	static const int INVALID_PATH = 0x300;
	static const int INVALID_NAME = 0x400;
	static const int INVALID_SIZE = 0x500;

	// Variables
	FileSystem virtual_disk;
    std::string disk_file;
	std::string PWD;

	// General functions
	std::vector<std::string> str2argv(std::string input_string);
	int str2int(std::string input_string);
	bool is_int(const std::string& input_string);

	// Functions
	int exec_cmd(std::string user_input);
    
	bool is_unix_path(std::string path);
	int resolve_path(std::string& path);
	int translate_storage_code(int exit_code);

	// Commands
    struct Command {
        int(ConsoleUI::*function)(int, char**) = NULL;
        std::string command;
        std::string syntax;
        std::string description;
        
        Command() {}
        
        Command(int(ConsoleUI::*function)(int, char**), string command, string syntax, string description) {
            this->function = function;
            this->description = description;
            this->command = command;
            this->syntax = syntax;
        }
    };
    
    static std::vector<Command> command_list;
    
	int display_help(int argc, char** argv);
	int display_version(int argc, char** argv);
	int clear_screen(int argc, char** argv);
	int exit_console(int argc, char** argv);
    
	int display_usage(int argc, char** argv);
	int save_vd(int argc, char** argv);
	int load_vd(int argc, char** argv);
	int create_vd(int argc, char** argv);

	int create_file(int argc, char** argv);
	int delete_file(int argc, char** argv);
	int display_file(int argc, char** argv);
	int copy_file(int argc, char** argv);

	int create_dir(int argc, char** argv);
	int delete_dir(int argc, char** argv);
	int list_dir(int argc, char** argv);
	int change_working_dir(int argc, char** argv);
    
};


#endif
