#include "LSNull.hpp"
#include "LSClass.hpp"
#include "LSNumber.hpp"

using namespace std;

namespace ls {

LSValue* LSNull::null_var = new LSNull();
LSClass* LSNull::null_class = new LSClass("Null");

LSValue* LSNull::get() {
	return null_var;
}

LSNull::LSNull() {
	refs = 1;
	native = true;
}

LSNull::~LSNull() {}

bool LSNull::isTrue() const {
	return false;
}

bool LSNull::eq(const LSNull*) const {
	return true;
}

bool LSNull::lt(const LSNull*) const {
	return false;
}

std::ostream& LSNull::dump(std::ostream& os) const {
	os << "null";
	return os;
}

LSValue* LSNull::getClass() const {
	return LSNull::null_class;
}

int LSNull::typeID() const {
	return 1;
}

}
