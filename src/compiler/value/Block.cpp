#include "Block.hpp"
#include "../../vm/value/LSNull.hpp"
#include "../../vm/value/LSNumber.hpp"
#include "../instruction/Return.hpp"
#include "../instruction/Throw.hpp"
#include "../instruction/VariableDeclaration.hpp"
#include "Function.hpp"
#include "../semantic/FunctionVersion.hpp"

namespace ls {

Block::Block(bool is_function_block) : is_function_block(is_function_block) {}

void Block::print(std::ostream& os, int indent, bool debug, bool condensed) const {
	if (!condensed) {
		os << "{";
		os << std::endl;
	}
	for (auto& instruction : instructions) {
		if (!condensed) os << tabs(indent + 1);
		instruction->print(os, indent + 1, debug, condensed);
		if (!condensed) os << std::endl;
	}
	if (!condensed) os << tabs(indent) << "}";
	if (debug) {
		os << " " << type;
		if (may_return) os << " ==>" << return_type;
	}
}

Location Block::location() const {
	assert(instructions.size());
	auto start = instructions.at(0)->location().start;
	auto end = instructions.back()->location().end;
	return {instructions.at(0)->location().file, start, end};
}

void Block::pre_analyze(SemanticAnalyzer* analyzer) {
	analyzer->enter_block();
	for (const auto& instruction : instructions) {
		instruction->pre_analyze(analyzer);
	}
	analyzer->leave_block();
}

void Block::analyze(SemanticAnalyzer* analyzer) {
	// std::cout << "Block::analyze() " << is_void << std::endl;

	analyzer->enter_block();
	throws = false;

	type = Type::void_;

	for (unsigned i = 0; i < instructions.size(); ++i) {
		const auto& instruction = instructions.at(i);
		if (i < instructions.size() - 1 or is_void) { // Not the last instruction
			instruction->is_void = true;
		}
		instruction->analyze(analyzer);
		if (i == instructions.size() - 1 and not is_void) { // Last instruction
			type = instruction->type;
		}
		if (instruction->may_return) may_return = true;
		if (instruction->throws) throws = true;
		return_type = return_type->operator + (instruction->return_type);
		if (instruction->returning) {
			returning = true;
			break; // no need to analyze after a return
		}
	}

	analyzer->leave_block();

	type = type->not_constant();

	// std::cout << "Block type " << type << std::endl;

	if (type == Type::mpz) {
		type = Type::tmp_mpz;
	} else if (type == Type::tmp_mpz) {
		temporary_mpz = true;
	} else if (type == Type::tmp_mpz_ptr) {
		type = Type::tmp_mpz;
		temporary_mpz = true;
		mpz_pointer = true;
	} else if (type == Type::mpz_ptr) {
		type = Type::tmp_mpz;
		mpz_pointer = true;
	}
}

Compiler::value Block::compile(Compiler& c) const {

	// std::cout << "Compile block " << type << std::endl;

	c.enter_block();

	for (unsigned i = 0; i < instructions.size(); ++i) {

		auto val = instructions[i]->compile(c);

		if (instructions[i]->returning) {
			// no need to compile after a return
			c.leave_block(false); // Variables already deleted by the return instruction
			if (is_function_block) {
				c.fun->compile_return(c, {});
			}
			return {};
		}
		if (i < instructions.size() - 1) {
			if (val.v != nullptr && !instructions[i]->type->is_void()) {
				c.insn_delete_temporary(val);
			}
		} else {
			auto return_value = [&]() {
				if (not val.v) {
					return val;
				} else if (type->must_manage_memory() and val.v != nullptr) {
					return c.insn_move(val);
				} else if (mpz_pointer) {
					return c.insn_load(temporary_mpz ? val : c.insn_clone_mpz(val));
				} else if (type->is_mpz()) {
					return temporary_mpz ? val : c.insn_clone_mpz(val);
				} else {
					return val;
				}
			}();
			if (is_function_block and c.vm->context) {
				c.fun->parent->export_context(c);
			}
			c.leave_block();
			if (is_function_block) {
				c.fun->compile_return(c, return_value);
			}
			return return_value;
		}
	}
	c.leave_block();
	if (is_function_block) {
		c.fun->compile_return(c, {});
	}
	return {};
}

std::unique_ptr<Value> Block::clone() const {
	auto b = std::make_unique<Block>(is_function_block);
	for (const auto& i : instructions) {
		b->instructions.push_back(i->clone());
	}
	return b;
}

}
