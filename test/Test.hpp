#ifndef TEST_HPP_
#define TEST_HPP_

#include <iostream>
#include <string>
#include "../leekscript.h"
#include "../src/compiler/semantic/SemanticException.hpp"

class Test {

	ls::VM vm;
	int total;
	int success_count;
	double exeTime;
	int obj_created;
	int obj_deleted;

public:

	Test();
	virtual ~Test();

	int all();
	void header(std::string);

	void success(std::string code, std::string result);
	void sem_err(std::string code, ls::SemanticException::Type type, std::string token);
	void ops(std::string code, int operations);

	void test_general();
	void test_operations();
	void test_operators();
	void test_references();
	void test_system();
	void test_objects();
};

#endif
