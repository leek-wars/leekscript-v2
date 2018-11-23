#include "Set.hpp"
#include "../../vm/value/LSSet.hpp"
#include "../../vm/VM.hpp"

using namespace std;

namespace ls {

Set::Set() {}

Set::~Set() {
	for (auto ex : expressions) delete ex;
}

void Set::print(ostream& os, int indent, bool debug, bool condensed) const {
	os << "<";
	for (size_t i = 0; i < expressions.size(); ++i) {
		if (i > 0) os << ", ";
		expressions[i]->print(os, indent + 1, debug);
	}
	os << ">";
	if (debug) os << " " << type;
}

Location Set::location() const {
	return {{0, 0, 0}, {0, 0, 0}}; // TODO
}

void Set::analyse(SemanticAnalyser* analyser, const Type&) {

	Type element_type = Type::ANY;

	for (auto& ex : expressions) {
		ex->analyse(analyser, Type::ANY);
		element_type = Type::get_compatible_type(element_type, ex->type);
	}

	if (element_type.nature == Nature::VALUE) {
		if (element_type != Type::INTEGER && element_type != Type::REAL) {
			element_type = Type::POINTER;
		}
	} else if (element_type.nature == Nature::ANY) {
		element_type.nature = Nature::POINTER;
	}

	constant = true;
	for (auto& ex : expressions) {
		ex->analyse(analyser, element_type);
		constant = constant && ex->constant;
	}

	type = Type(RawType::SET, Nature::POINTER, element_type);
}

bool Set::will_store(SemanticAnalyser* analyser, const Type& type) {

	Type added_type = type;
	if (added_type.raw_type == RawType::ARRAY or added_type.raw_type == RawType::SET) {
		added_type = added_type.getElementType();
	}
	Type current_type = this->type.getElementType();
	if (expressions.size() == 0) {
		this->type.setElementType(added_type);
	} else {
		this->type.setElementType(Type::get_compatible_type(current_type, added_type));
	}
	// Re-analyze expressions with the new type
	for (size_t i = 0; i < expressions.size(); ++i) {
		expressions[i]->analyse(analyser, this->type.getElementType());
	}
	this->types = type;
	return false;
}

LSSet<LSValue*>* Set_create_ptr() { return new LSSet<LSValue*>(); }
LSSet<int>* Set_create_int()      { return new LSSet<int>();      }
LSSet<double>* Set_create_float() { return new LSSet<double>();   }

void Set_insert_ptr(LSSet<LSValue*>* set, LSValue* value) {
	auto it = set->lower_bound(value);
	if (it == set->end() || (**it != *value)) {
		set->insert(it, value->move_inc());
	}
	LSValue::delete_temporary(value);
}
void Set_insert_int(LSSet<int>* set, int value) {
	set->insert(value);
}
void Set_insert_float(LSSet<double>* set, double value) {
	set->insert(value);
}

Compiler::value Set::compile(Compiler& c) const {
	void* create = type.getElementType() == Type::INTEGER ? (void*) Set_create_int :
				   type.getElementType() == Type::REAL   ? (void*) Set_create_float :
															(void*) Set_create_ptr;
	void* insert = type.getElementType() == Type::INTEGER ? (void*) Set_insert_int :
				   type.getElementType() == Type::REAL   ? (void*) Set_insert_float :
															(void*) Set_insert_ptr;

	unsigned ops = 1;
	auto s = c.insn_call(type, {}, (void*) create);

	double i = 0;
	for (Value* ex : expressions) {
		auto v = ex->compile(c);
		ex->compile_end(c);
		c.insn_call(Type::VOID, {s, v}, (void*) insert);
		ops += std::log2(++i);
	}
	c.inc_ops(ops);
	return s;
}

Value* Set::clone() const {
	auto s = new Set();
	for (const auto& v : expressions) {
		s->expressions.push_back(v->clone());
	}
	return s;
}

}
