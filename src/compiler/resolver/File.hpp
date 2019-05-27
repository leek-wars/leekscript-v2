#ifndef FILE_HPP
#define FILE_HPP

#include "FileContext.hpp"

namespace ls {

class File {
public:
	std::string path;
	std::string code;
	FileContext* context;

	File(std::string path, std::string code, FileContext* context) {
		this->path = path;
		this->code = code;
		this->context = context;
	}
};

}

#endif