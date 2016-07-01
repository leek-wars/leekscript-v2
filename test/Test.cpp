#include "Test.hpp"

Test::Test() {
	total = 0;
	success_count = 0;
	exeTime = 0;
	obj_deleted = 0;
	obj_created = 0;
}

Test::~Test() {}

int Test::all() {

	clock_t begin = clock();
	exeTime = 0;

	test_general();
	test_operators();
	test_references();
	test_operations();
	test_system();
	test_objects();

	double elapsed_secs = double(clock() - begin) / CLOCKS_PER_SEC;
	int errors = (total - success_count);

	std::cout << std::endl;
	std::cout << "------------------------------------------------" << std::endl;
	std::cout << "Total : " << total << ", succès : " << success_count << ", erreurs : " << errors << std::endl;
	std::cout << "Total time : " << elapsed_secs * 1000 << " ms, execution time : " << (exeTime / CLOCKS_PER_SEC) * 1000 << " ms" << std::endl;
	std::cout << "Objects destroyed : " << obj_deleted << " / " << obj_created << " (" << (obj_created - obj_deleted) << " leaked)" << std::endl;
	std::cout << "------------------------------------------------" << std::endl;

	return errors;
}

void Test::header(std::string text) {
	std::cout << "----------------" << std::endl;
	std::cout << text << std::endl;
	std::cout << "----------------" << std::endl;
}

void Test::success(std::string code, std::string expected) {

	total++;

	std::string res = vm.execute(code, "{}", ls::ExecMode::TEST);
	obj_created += ls::LSValue::obj_count;
	obj_deleted += ls::LSValue::obj_deleted;

	if (res != expected) {
		std::cout << "FAUX : " << code << "  =/=>  " << expected << "  got  " << res << std::endl;
	} else {
		std::cout << "OK   : " << code << "  ===>  " << expected << std::endl;
		success_count++;
	}
}

void Test::ops(std::string code, int expected) {

	total++;

	std::string res = vm.execute(code, "{}", ls::ExecMode::TEST_OPS);
	obj_created += ls::LSValue::obj_count;
	obj_deleted += ls::LSValue::obj_deleted;

	int ops = std::stoi(res);
	if (ops != expected) {
		std::cout << "FAUX : " << code << "  =/=>  " << expected << "  got  " << ops << std::endl;
	} else {
		std::cout << "OK   : " << code << "  ===>  " << expected << std::endl;
		success_count++;
	}
}

void Test::sem_err(std::string code, ls::SemanticException::Type expected_type, std::string token) {

	total++;

	try {
		vm.execute(code, "{}", ls::ExecMode::TEST);

	} catch (ls::SemanticException& e) {

		if (expected_type != e.type or token != e.token->content) {

			std::string expected_message = ls::SemanticException::build_message(expected_type, token);
			std::cout << "FAUX : " << code << "  =/=> " << expected_message << "  got  " << e.message() << std::endl;

		} else {
			std::cout << "OK   : " << code << "  ===> " << e.message() <<  std::endl;
			success_count++;
		}
	}

	obj_created += ls::LSValue::obj_count;
	obj_deleted += ls::LSValue::obj_deleted;
}
