#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "../compiler/value/Function.hpp"

namespace ls {

class Program {
private:

	std::string code; // The program code
	std::string file_name;
	void* closure;

public:

	Function* main;
	std::vector<Function*> functions;

	Program(const std::string& code, const std::string& file_name);
	virtual ~Program();

	void analyse(SemanticAnalyser* analyser);

	/*
	 * Compile the program with a VM and a context (json)
	 */
	VM::Result compile(VM& vm, const std::string& context);

	/*
	 * Execute the program and get a std::string result
	 */
	std::string execute(VM& vm);

	void print(std::ostream& os, bool debug = false) const;
};

std::ostream& operator << (std::ostream& os, const Program* program);

}

#endif
