#ifndef VM_STANDARD_ARRAYSTD_HPP_
#define VM_STANDARD_ARRAYSTD_HPP_

#include "../Module.hpp"

namespace ls {

class ArraySTD : public Module {
public:
	ArraySTD();

	static Compiler::value in(Compiler& c, std::vector<Compiler::value> args);
	static Compiler::value array_add_eq(Compiler& c, std::vector<Compiler::value> args);
	static Compiler::value lt(Compiler&, std::vector<Compiler::value>);

	static Compiler::value contains(Compiler&, std::vector<Compiler::value>);
	static Compiler::value size(Compiler&, std::vector<Compiler::value>);

	static LSArray<LSValue*>* chunk_1_ptr(LSArray<LSValue*>* array);
	static LSArray<LSValue*>* chunk_1_int(LSArray<int>* array);
	static LSArray<LSValue*>* chunk_1_float(LSArray<double>* array);

	static LSValue* sub(LSArray<LSValue*>* array, int begin, int end);

	static Compiler::value search_any(Compiler&, std::vector<Compiler::value>);
	static Compiler::value search_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value search_int(Compiler&, std::vector<Compiler::value>);

	static Compiler::value fill_any(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fill_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fill_int(Compiler&, std::vector<Compiler::value>);

	static Compiler::value fold_left_ptr(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_left_clo_ptr(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_left_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_left_clo_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_left_int(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_left_clo_int(Compiler&, std::vector<Compiler::value>);

	static Compiler::value fold_right_ptr(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_right_clo_ptr(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_right_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_right_clo_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_right_int(Compiler&, std::vector<Compiler::value>);
	static Compiler::value fold_right_clo_int(Compiler&, std::vector<Compiler::value>);

	static Compiler::value remove_element_any(Compiler&, std::vector<Compiler::value>);
	static Compiler::value remove_element_real(Compiler&, std::vector<Compiler::value>);
	static Compiler::value remove_element_int(Compiler&, std::vector<Compiler::value>);

	static Compiler::value partition(Compiler&, std::vector<Compiler::value>);
	static Compiler::value map(Compiler&, std::vector<Compiler::value>);
};

}

#endif
