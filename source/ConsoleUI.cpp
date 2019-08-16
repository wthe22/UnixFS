#include <iostream>
#include <sstream>
#include <iomanip>
#include "Config.h"
#include "ConsoleUI.h"
#include "FileSystem.h"
#include "Inode.h"
using namespace std;

const char* ConsoleUI::DEFAULT_DISK_FILE = "vdisk.dat";
const char* ConsoleUI::memory_disk_symbol = "#:memory:#";

vector<ConsoleUI::Command> ConsoleUI::command_list = {
    Command(&display_help,
        "help", "[command]",
        "Display help"),
	Command(&display_version,
        "ver", "",
        "Display version information"),
	Command(&clear_screen,
        "clear", "",
        "Clear screen"),
	Command(&exit_console,
        "exit", "",
        "Quits the program"),
        
    Command(&save_vd,
        "image-save", "[filename]",
        "Save disk image to file."),
	Command(&load_vd,
        "image-load", "",
        "Load disk image from file"),
	Command(&create_vd,
        "image-create", "<filename> <disk_size> <block_size>",
        "Create a new disk image file"),
        
	Command(&display_usage,
        "sum", "",
        "Print properties of the current disk"),
        
	Command(&create_file,
        "newfile", "<name> <size>",
        "Create a new file"),
	Command(&delete_file,
        "rm", "<name>",
        "Remove a file"),
	Command(&display_file,
        "cat", "<name>",
        "Print contents of a file"),
	Command(&copy_file,
        "cp", "<source_file> <destination_file>",
        "Copy contents of a file"),
	Command(&list_dir,
        "ls", "[path]",
        "List contents of a directory"),
	Command(&create_dir,
        "mkdir", "<name>",
        "Make a new directory"),
	Command(&delete_dir,
        "rmdir", "<name>",
        "Remove a directory"),
	Command(&change_working_dir,
        "cd", "<path>",
        "Change current working directory"),
};


int ConsoleUI::about(int argc, char** argv) {
	cout << SOFTWARE::NAME << " " << SOFTWARE::VERSION_string() << endl;
    
    if (SOFTWARE::AUTHORS.size() > 1) {
        cout << "(c) 2019 " << endl;
        for (size_t i = 0; i < SOFTWARE::AUTHORS.size(); i++)
            cout << SOFTWARE::AUTHORS[i] << endl;
        cout << "All rights reserved.\n";
    } else {
        cout << "(c) 2019 " << SOFTWARE::AUTHORS[0] << ". All rights reserved.\n";
    }
	cout << endl;
	cout << SOFTWARE::NAME << " comes with ABSOLUTELY NO WARRANTY, to the\n";
	cout << "extent permitted by applicable law.\n";
	cout << endl;
	return SUCCESS;
}

int ConsoleUI::main_loop() {
	disk_file = DEFAULT_DISK_FILE;
	virtual_disk = FileSystem();
    if (virtual_disk.load(DEFAULT_DISK_FILE) != SUCCESS) {
        virtual_disk.init(16 * 0x100000, 1024);
        disk_file = memory_disk_symbol;
    }
	PWD = "/";
    
    about();
	while (true) {
        string user_input;
		cout << disk_file << ":" << PWD << " $ ";
        getline(cin, user_input);
        
		this->exec_cmd(user_input);
		cout << endl;
	};
	cout << endl;
	return 0;
}

int ConsoleUI::exec_cmd(string user_input) {
	vector<string> args = str2argv(user_input);
	if (args.size() == 0)
		return SUCCESS;
    
    Command cmd;
    for (size_t i = 0; i < command_list.size(); i++) {
        if (args[0] == command_list[i].command)
            cmd = command_list[i];
    }
    if (cmd.function == NULL) {
		cout << args[0] << ": command not found\n";
		return INVALID_COMMAND;
	}

	std::vector<char*> cstrings;
	cstrings.reserve(args.size());
	for (size_t i = 1; i < args.size(); i++)
		cstrings.push_back(const_cast<char*>(args[i].c_str()));
	char** argv_ptr = NULL;
	if (cstrings.size() > 0)
		argv_ptr = &cstrings[0];

	int exit_code = (this->*(cmd.function))(cstrings.size(), argv_ptr);
	switch (exit_code)
	{
	case FAILED: cout << "Command failed.\n"; break;
	case NOT_EXIST: cout << "File does not exist.\n"; break;
	case ALREADY_EXIST: cout << "File already exist.\n"; break;
	case INCOMPATIBLE: cout << "Incompatible feeatures detected.\n"; break;
	case NOT_FILE: cout << "The path entered is not a file.\n"; break;
	case NOT_DIR: cout << "The path entered is not a directory.\n"; break;

	case INVALID_SYNTAX: cout << "Syntax of comamnd is incorrect.\n"; break;
	case INVALID_PATH: cout << "Invalid path.\n"; break;
	case INVALID_NAME: cout << "Invalid file name.\n"; break;
	case INVALID_SIZE: cout << "Invalid file size.\n"; break;
	default: break;
	}
    if (exit_code == INVALID_SYNTAX)
        cout << "usage: " << cmd.command << "   " << cmd.syntax << endl;
	return exit_code;
}


int ConsoleUI::display_help(int argc, char** argv) {
    if (argc == 1) {
        Command cmd;
        for (size_t i = 0; i < command_list.size(); i++) {
            if (argv[0] == command_list[i].command)
                cmd = command_list[i];
        }
        if (cmd.function != NULL) {
            cout << cmd.description << endl;
            cout << endl;
            cout << cmd.command << "   " << cmd.syntax << endl;
            return SUCCESS;
        }
    }
    cout << "List of commands:\n";
    cout << endl;
    for (size_t i = 0; i < command_list.size(); i++)
        cout << left << setw(16) << command_list[i].command << command_list[i].description << "\n";
    cout << endl;
    cout << "For more information on a command: \n";
    cout << "help   <command>\n";
    return SUCCESS;
}

int ConsoleUI::display_version(int argc, char** argv) {
    const int width = 24;
    cout << left << setw(width) << "Software version" << ": " << SOFTWARE::VERSION_string() << endl;
    cout << left << setw(width) << "File system revision" << ": " << FileSystem::REV_LEVEL << endl;
    cout << left << setw(width) << "Console UI revision" << ": " << ConsoleUI::REV_LEVEL << endl;
    cout << endl;
    cout << left << setw(width) << "Compiler" << ": " << SOFTWARE::COMPILER << endl;
    cout << left << setw(width) << "Build time" << ": " << SOFTWARE::BUILD_DATE << " " << SOFTWARE::BUILD_TIME << endl;
    
    return SUCCESS;
}

int ConsoleUI::clear_screen(int argc, char** argv) {
	if (argc > 0)
		return INVALID_SYNTAX;
    
    system("clear || cls 2> nul");
    return SUCCESS;
}

int ConsoleUI::exit_console(int argc, char** argv) {
    exit(EXIT_SUCCESS);
    return SUCCESS;
}

 
int ConsoleUI::display_usage(int argc, char** argv) {
	int exit_code = virtual_disk.display_properties();
	return translate_storage_code(exit_code);
}

int ConsoleUI::save_vd(int argc, char** argv) {
	if (argc > 1)
		return INVALID_SYNTAX;
    
    if (argc == 1)
        disk_file = argv[0];
    
	int exit_code = virtual_disk.save(disk_file);
	return translate_storage_code(exit_code);
}

int ConsoleUI::load_vd(int argc, char** argv) {
	if (argc > 1)
		return INVALID_SYNTAX;

    if (argc == 1)
        disk_file = argv[0];
    
    int exit_code = virtual_disk.load(disk_file);
    if (exit_code == FileSystem::SUCCESS)
        PWD = "/";
    
	return translate_storage_code(exit_code);
}

int ConsoleUI::create_vd(int argc, char** argv) {
	if (argc != 3)
		return INVALID_SYNTAX;

    disk_file = argv[0];
    int disk_size = str2int(argv[1]);
    int block_size = str2int(argv[2]);
    
    int exit_code = virtual_disk.init(disk_size * 0x400, block_size);
    if (exit_code == FileSystem::SUCCESS)
        PWD = "/";
    
	return translate_storage_code(exit_code);
}

int ConsoleUI::create_file(int argc, char** argv) {
	if (argc != 2)
		return INVALID_SYNTAX;

	if (!is_unix_path(argv[0]))
		return INVALID_PATH;

	if (!is_int(argv[1]))
		return INVALID_SIZE;

	string name = argv[0];
	if (name.rfind("/") != string::npos)
		return INVALID_NAME;

	int exit_code = virtual_disk.file_create(PWD, argv[0], str2int(argv[1]));
	return translate_storage_code(exit_code);
}

int ConsoleUI::delete_file(int argc, char** argv) {
	if (argc != 1)
		return INVALID_SYNTAX;

	if (!is_unix_path(argv[0]))
		return INVALID_PATH;

	string name = argv[0];
	if (name.rfind("/") != string::npos)
		return INVALID_NAME;

	int exit_code = virtual_disk.file_remove(PWD, argv[0]);
	return translate_storage_code(exit_code);
}

int ConsoleUI::display_file(int argc, char** argv) {
	if (argc != 1)
		return INVALID_SYNTAX;

	int exit_code = SUCCESS;
	string file_path = argv[0];
	exit_code = resolve_path(file_path);
	if (exit_code != SUCCESS)
		return exit_code;

	exit_code = virtual_disk.file_display(file_path);
	return translate_storage_code(exit_code);
}

int ConsoleUI::copy_file(int argc, char** argv) {
	if (argc != 2)
		return INVALID_SYNTAX;

	int exit_code = SUCCESS;
	string source_file = argv[0];
	exit_code = resolve_path(source_file);
	if (exit_code != SUCCESS)
		return exit_code;
	string dest_file = argv[1];
	if (dest_file.back() == '/')
		return INVALID_PATH;
	string dest_path = dest_file.substr(0, dest_file.rfind("/") + 1);
	string dest_name = dest_file.substr(dest_file.rfind("/") + 1);
	exit_code = resolve_path(dest_path);
	if (exit_code != SUCCESS)
		return exit_code;

	exit_code = virtual_disk.file_copy(source_file, dest_path, dest_name);
	return translate_storage_code(exit_code);
}

int ConsoleUI::create_dir(int argc, char** argv) {
	if (argc != 1)
		return INVALID_SYNTAX;

	if (!is_unix_path(argv[0]))
		return INVALID_PATH;

	string name = argv[0];
	if (name.rfind("/") != string::npos)
		return INVALID_NAME;

	int exit_code = virtual_disk.dir_create(PWD, argv[0]);
	return translate_storage_code(exit_code);
}

int ConsoleUI::delete_dir(int argc, char** argv) {
	if (argc != 1)
		return INVALID_SYNTAX;

	if (!is_unix_path(argv[0]))
		return INVALID_PATH;

	string name = argv[0];
	if (name.rfind("/") != string::npos)
		return INVALID_NAME;

	int exit_code = virtual_disk.dir_remove(PWD, argv[0]);
	return translate_storage_code(exit_code);
}

int ConsoleUI::list_dir(int argc, char** argv) {
	if (argc > 1)
		return INVALID_SYNTAX;

	int exit_code = SUCCESS;
	string target_dir = "";
	if (argc == 1)
		target_dir = argv[0];
	exit_code = resolve_path(target_dir);
	if (exit_code != SUCCESS)
		return exit_code;

	exit_code = virtual_disk.dir_list(target_dir);
	return translate_storage_code(exit_code);
}

int ConsoleUI::change_working_dir(int argc, char** argv) {
	if (argc != 1)
		return INVALID_SYNTAX;

	int exit_code = SUCCESS;
	string target_dir = argv[0];
	exit_code = resolve_path(target_dir);
	if (exit_code != SUCCESS)
		return exit_code;
    
    if (virtual_disk.type_of(target_dir) != Inode::DIRECTORY)
        return NOT_DIR;

	PWD = target_dir;
	return SUCCESS;
}

bool ConsoleUI::is_unix_path(string path) {
	if (path == "")
		return false;

	string invalid_characters = "\\:*\"<>|";
	for (size_t i = 0; i < path.size(); i++)
		for (size_t j = 0; j < invalid_characters.size(); j++)
			if (path[i] == invalid_characters[j])
				return false;
	return true;
}

int ConsoleUI::resolve_path(string& path) {
	if (path[0] != '/')
		path = PWD + '/' + path;

	if (!is_unix_path(path))
		return INVALID_PATH;

	path = virtual_disk.path_abspath(path);

	if (!is_unix_path(path))
		return NOT_EXIST;

	return SUCCESS;
}

int ConsoleUI::translate_storage_code(int exit_code) {
	switch (exit_code) {
	case FileSystem::SUCCESS: return SUCCESS;
	case FileSystem::FAILED: return FAILED;
	case FileSystem::NOT_EXIST: return NOT_EXIST;
	case FileSystem::ALREADY_EXIST: return ALREADY_EXIST;
	case FileSystem::INCOMPATIBLE: return INCOMPATIBLE;
	case FileSystem::NOT_FILE: return NOT_FILE;
	case FileSystem::NOT_DIR: return NOT_DIR;
	default: return FAILED;  break;
	}
}


vector<string> ConsoleUI::str2argv(string input_string) {
	vector<string> argv;

	bool is_quoted = false;
	string current_arg = "";

	for (size_t i = 0; i < input_string.size(); i++) {
		bool is_delim = false;

		if (input_string[i] == ' ' && !is_quoted)
			is_delim = true;

		if (input_string[i] == '"') {
			is_quoted = !is_quoted;
			is_delim = true;
		}

		if (is_delim) {
			if (current_arg != "") {
				argv.push_back(current_arg);
				current_arg = "";
			}
			continue;
		}

		current_arg += input_string[i];
	}
	if (current_arg != "") {
		argv.push_back(current_arg);
		current_arg = "";
	}
	return argv;
}

int ConsoleUI::str2int(string input_string) {
	int number = 0;
	stringstream convert_stream(input_string);
	convert_stream >> number;
	return number;
}

bool ConsoleUI::is_int(const string& input_string) {
	string::const_iterator it = input_string.begin();
	while (it != input_string.end() && isdigit(*it)) ++it;
	return !input_string.empty() && it == input_string.end();
}
