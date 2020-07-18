#ifndef NREP_FILE_SYSTEM_UTILS_HPP
#define NREP_FILE_SYSTEM_UTILS_HPP

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "StringUtils.hpp"


#ifdef _MSC_VER
#include "misc/dirent_win.h"
#else
#include <sys/types.h>
#include <sys/stat.h>
#include "dirent.h"
#endif


namespace nrep {

	using std::string;
	using namespace std;

	int isDirectory(std::string & path) {
		struct stat statbuf;
		if (stat(path.c_str(), &statbuf) != 0)
			return 0;
		return S_ISDIR(statbuf.st_mode);
	}

	int isRegularFile(std::string & path) {
        struct stat path_stat;
        stat(path.c_str(), &path_stat);
        return S_ISREG(path_stat.st_mode);
    }


	int64_t getFileLength(std::string fileName) {
		int64_t length = -1;
		std::ifstream is(fileName, std::ifstream::binary);

		if (is) {
			// get length of file:
			is.seekg(0, is.end);
			length = is.tellg();
			is.seekg(0, is.beg);
		}

		return length;
	}

	// Adds a file name to the vector if it matches the prefix and suffix
	void addFile(std::vector<std::string> &files, string file, string prefix, string suffix) {

		if (prefix.length()==0 && suffix.length()==0) {
			files.push_back(file);
		}
		else if (prefix.length()==0) {
			if (nrep::str_endsWith(file, suffix)) files.push_back(file);
		}
		else if (nrep::str_beginsWith(file, prefix)) files.push_back(file);
	}



	// Get all files in directory with prefix and/or suffix
	void getFiles(std::string dirName, std::vector<std::string> &files, string prefix = "", string suffix = "") {
		DIR *dir;
		struct dirent *ent;

		/* Open directory stream */
		dir = opendir(dirName.c_str());

		string name, path;
		if (dir != NULL) {
			/* Print all files and directories within the directory */
			while ((ent = readdir(dir)) != NULL) {

                name = ent->d_name;
                if (name=="." || name == "..") continue;
                path = dirName + "/" + name;

                if (isRegularFile(path)) {
                    addFile(files, path, prefix, suffix);
                }
			}
			closedir(dir);
		}
		else {
			/* Could not open directory */
			std::cout << "Cannot open directory" << dirName;
			exit(EXIT_FAILURE);
		}
	}

	// Get all files in directory with prefix and/or suffix
	void getFilesRecursive(std::string dirName, std::vector<std::string> &files, string prefix = "", string suffix = "") {
		DIR *dir;
		struct dirent *ent;

		/* Open directory stream */
		dir = opendir(dirName.c_str());

		string name, path;
		if (dir != NULL) {
			/* Print all files and directories within the directory */
			while ((ent = readdir(dir)) != NULL) {

                name = ent->d_name;
                if (name=="." || name == "..") continue;
                path = dirName + "/" + name;

                if (isRegularFile(path)) {
                    addFile(files, path, prefix, suffix);
                }
                else if (isDirectory(path)) {

                    //cout << endl << ent->d_name;
                    getFilesRecursive(path, files, prefix, suffix);
                }
			}
			closedir(dir);
		}
		else {
			/* Could not open directory */
			std::cout << "Cannot open directory" << dirName;
			exit(EXIT_FAILURE);
		}
	}
}


#endif
