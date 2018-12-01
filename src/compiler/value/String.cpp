#include "String.hpp"
#include "../../vm/value/LSString.hpp"
#include "../semantic/SemanticAnalyser.hpp"
#include "../../type/RawType.hpp"

using namespace std;

namespace ls {

String::String(std::shared_ptr<Token> token) : token(token) {
	type = Type::STRING;
	type.temporary = true;
	constant = true;
	ls_string = new LSString(token->content);
}

String::~String() {
	delete ls_string;
}

void String::print(ostream& os, int, bool debug, bool condensed) const {
	os << "'" << token->content << "'";
	if (debug) {
		os << " " << type;
	}
}

Location String::location() const {
	return token->location;
}

bool String::will_store(SemanticAnalyser* analyser, const Type& type) {
	if (!type.is_string()) {
		analyser->add_error({SemanticError::Type::NO_SUCH_OPERATOR, location(), location(), {}});
	}
	return false;
}

Compiler::value String::compile(Compiler& c) const {
	c.add_literal(ls_string, std::string("'") + *ls_string + std::string("'"));
	auto base = c.new_pointer(ls_string);
	return c.insn_call(Type::STRING_TMP, {base}, (void*) +[](LSString* s) {
		return s->clone();
	});
}

Value* String::clone() const {
	return new String(token);
}

}
