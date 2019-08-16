#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>


namespace SOFTWARE {
    const char* const NAME = "UNIX File System - Console Edition";
    const std::vector<const char*> AUTHORS = {
        "wthe22",
    };
    const char* const BUILD_DATE = __DATE__;
    const char* const BUILD_TIME = __TIME__;
    const std::vector<int> VERSION = {0, 2, 0, 0, 0};

    #if defined(__GNUC__) || defined(__GNUG__)
    const char* const COMPILER = "GNU";
    #elif defined(_MSC_VER)
    const char* const COMPILER = "MSVC";
    #else
    const char* const COMPILER = "OTHER";
    #endif
    
    std::string VERSION_string() {
        std::string version_str;
        for (size_t i = 0; i < VERSION.size(); i++) {
            if (i != 0 && VERSION[i] == 0)
                break;
            version_str += "." + std::to_string(SOFTWARE::VERSION[i]);
        }
        version_str = version_str.substr(1);
        if (VERSION[3] == 3 && VERSION[4] == 0)
            return version_str;
        
        std::string pre_release;
        switch(VERSION[3]) {
        case 0: pre_release = "a"; break;
        case 1: pre_release = "b"; break;
        case 2: pre_release = "rc"; break;
        case 3: pre_release = "r"; break;
        }
        version_str += "-" + pre_release;
        if (VERSION[4] != 0)
            version_str += "." + std::to_string(VERSION[4]);
        return version_str;
    }
}


#endif
