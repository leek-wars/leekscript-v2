#include "Test.hpp"
#include "../src/type/Ty.hpp"

void Test::test_types() {

	header("Types");
	// Print a list of types
	std::vector<ls::Type> types { ls::Type::INTEGER, ls::Type::STRING, ls::Type::REAL_ARRAY };
	std::cout << types << std::endl;

	// Print type names
	assert(ls::Type::LONG.raw_type->getJsonName() == "number");
	assert(ls::Type::MAP.raw_type->getJsonName() == "map");
	assert(ls::Type::INTERVAL.raw_type->getJsonName() == "interval");
	assert(ls::Type::OBJECT.raw_type->getJsonName() == "object");
	assert(ls::Type::CLASS.raw_type->getJsonName() == "class");
	assert(ls::Type::SET.raw_type->getJsonName() == "set");
	assert(ls::Type::NULLL.raw_type->getJsonName() == "null");
	assert(ls::Type::ANY.raw_type->getName() == "any");
	assert(ls::Type::FUNCTION.raw_type->getName() == "function");

	// Type::more_specific
	assert(ls::Type::more_specific(ls::Type::PTR_ARRAY, ls::Type::INT_ARRAY));
	assert(ls::Type::more_specific(ls::Type::PTR_PTR_MAP, ls::Type::INT_INT_MAP));
	assert(ls::Type::more_specific(ls::Type::PTR_PTR_MAP, ls::Type::REAL_REAL_MAP));

	// New type
	// code("12").type(ls::Ty::get_int());
	// code("12.5").type(ls::Ty::get_double());
	// code("'salut'").type(ls::Ty::get_string());
	// code("[]").type(ls::Ty::array());
	// code("[1]").type(ls::Ty::array(ls::Ty::get_int()));
	// code("[1, 2.5]").type(ls::Ty::array(ls::Ty::get_int()));
	// code("[[1]]").type(ls::Ty::array(ls::Ty::array(ls::Ty::get_int())));

	// std::cout << ls::Ty::get_int().get_compatible(ls::Ty::get_long()) << std::endl;
	// std::cout << ls::Ty::get_int().get_compatible(ls::Ty::get_integer()) << std::endl;
	// std::cout << ls::Ty::get_int().get_compatible(ls::Ty::array()) << std::endl;
}
