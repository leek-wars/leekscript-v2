#include <sstream>
#include <algorithm>
#include "LSString.hpp"
#include "LSNull.hpp"
#include "LSBoolean.hpp"
#include "LSClass.hpp"
#include "LSNumber.hpp"
#include "LSArray.hpp"
#include <string.h>
#include "../../../lib/utf8.h"

using namespace std;

namespace ls {

LSValue* LSString::string_class(new LSClass("String", 1));

LSString::LSString() {}
LSString::LSString(const char value) : string(string(1, value)) {}
LSString::LSString(const char* value) : string(value) {}
LSString::LSString(std::string value) : string(value) {}
LSString::LSString(Json& json) : string(json.get<std::string>()) {}

LSString::~LSString() {}

LSString* LSString::charAt(int index) const {
	return new LSString(this->operator[] (index));
}

int LSString::unicode_length() const {
	return u8_strlen(this->c_str());
}

/*
 * LSValue methods
 */
bool LSString::isTrue() const {
	return size() > 0;
}

LSValue* LSString::operator ! () const {
	return LSBoolean::get(size() == 0);
}

LSValue* LSString::operator ~ () const {

	// reverse(copy.begin(), copy.end());

	char buff[5];
	char* string_chars = (char*) this->c_str();
	int i = 0;
	int l = strlen(string_chars);
	string reversed = "";
	while (i < l) {
		u_int32_t c = u8_nextchar(string_chars, &i);

		u8_toutf8(buff, 5, &c, 1);
		reversed = buff + reversed;
	}
	return new LSString(reversed);
}

LSValue* LSString::operator + (const LSValue* v) const {
	return v->operator + (this);
}
LSValue* LSString::operator + (const LSNull*) const {
	return new LSString(*this + "null");
}
LSValue* LSString::operator + (const LSBoolean* boolean) const {
	return new LSString(*this + (boolean->value ? "true" : "false"));
}
LSValue* LSString::operator + (const LSNumber* value) const {
	return new LSString(*this + value->toString());
}
LSValue* LSString::operator + (const LSString* string) const {
	return new LSString(*this + *string);
}
LSValue* LSString::operator + (const LSArray<LSValue*>*) const {
	return new LSString(*this + "<array>");
}
LSValue* LSString::operator + (const LSArray<int>*) const {
	return new LSString(*this + "<array>");
}
LSValue* LSString::operator + (const LSObject* ) const {
	return new LSString(*this + "<object>");
}
LSValue* LSString::operator + (const LSFunction*) const {
	return new LSString(*this + "<function>");
}
LSValue* LSString::operator + (const LSClass*) const {
	return new LSString(*this + "<class>");
}

LSValue* LSString::operator += (LSValue* value) {
	return value->operator += (this);
}
LSValue* LSString::operator += (const LSNull*) {
	((std::string*) this)->operator += ("null");
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSBoolean* boolean) {
	((std::string*) this)->operator += (boolean->value ? "true" : "false");
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSNumber* value) {
	((std::string*) this)->operator += (value->toString());
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSString* string) {
	((std::string*) this)->operator += (*string);
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSArray<LSValue*>*) {
	((std::string*) this)->operator += ("<array>");
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSObject*) {
	((std::string*) this)->operator += ("<object>");
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSFunction*) {
	((std::string*) this)->operator += ("<function>");
	return LSNull::null_var;
}
LSValue* LSString::operator += (const LSClass*) {
	((std::string*) this)->operator += ("<class>");
	return LSNull::null_var;
}

LSValue* LSString::operator - (const LSValue* value) const {
	return value->operator - (this);
}
LSValue* LSString::operator - (const LSNumber* value) const {
	return new LSString(value->toString() + *this);
}
LSValue* LSString::operator - (const LSString* string) const {
	return new LSString(*string + *this);
}

LSValue* LSString::operator -= (LSValue* value) {
	return value->operator -= (this);
}

LSValue* LSString::operator * (const LSValue* value) const {
	return value->operator * (this);
}
LSValue* LSString::operator * (const LSNumber* value) const {
	string res = "";
	for (int i = 0; i < value->value; ++i) {
		res += *this;
	}
	return new LSString(res);
}

LSValue* LSString::operator *= (LSValue* value) {
	return value->operator *= (this);
}

LSValue* LSString::operator / (const LSValue* value) const {
	return value->operator / (this);
}

LSValue* LSString::operator / (const LSString* s) const {

	char buff[5];
	char* string_chars = (char*) this->c_str();

	LSArray<LSValue*>* array = new LSArray<LSValue*>();

	if (s->size() == 0) {
//		for (char c : *this) {
//			array->push_no_clone(new LSString(c));
//		}
		int i = 0;
		int l = strlen(string_chars);
		while (i < l) {
			u_int32_t c = u8_nextchar(string_chars, &i);
			u8_toutf8(buff, 5, &c, 1);
			LSString* ch = new LSString(buff);
			array->push_no_clone(ch);
		}
 	} else {

 		u_int32_t separator = u8_char_at((char*) s->c_str(), 0);
		int i = 0;
		int l = strlen(string_chars);
		std::string item = "";
		while (i < l) {
			u_int32_t c = u8_nextchar(string_chars, &i);
			if (c == separator) {
				array->push_no_clone(new LSString(item));
				item = "";
			} else {
				u8_toutf8(buff, 5, &c, 1);
				item += buff;
			}
		}
		if (item.size() > 0) {
			array->push_no_clone(new LSString(item));
		}
//		stringstream ss(*this);
//		string item;
//		while (getline(ss, item, s->operator[] (0))) {
//			array->push_no_clone(new LSString(item));
//		}

 	}
	return array;
}

LSValue* LSString::operator /= (LSValue* value) {
	return value->operator /= (this);
}
LSValue* LSString::poww(const LSValue*) const {
	return LSNull::null_var;
}
LSValue* LSString::pow_eq(LSValue* value) {
	return value->operator *= (this);
}
LSValue* LSString::operator % (const LSValue* value) const {
	return value->operator % (this);
}
LSValue* LSString::operator %= (LSValue* value) {
	return value->operator %= (this);
}

bool LSString::operator == (const LSValue* v) const {
	return v->operator == (this);
}
bool LSString::operator == (const LSString* v) const {
	return *this == *v;
}

bool LSString::operator < (const LSValue* v) const {
	return v->operator < (this);
}
bool LSString::operator < (const LSNull*) const {
	return false;
}
bool LSString::operator < (const LSBoolean*) const {
	return false;
}
bool LSString::operator < (const LSNumber*) const {
	return false;
}
bool LSString::operator < (const LSString* v) const {
	return *this < *v;
}
bool LSString::operator < (const LSArray<LSValue*>*) const {
	return true;
}
bool LSString::operator < (const LSArray<int>*) const {
	return true;
}
bool LSString::operator < (const LSArray<double>*) const {
	return true;
}
bool LSString::operator < (const LSObject*) const {
	return true;
}
bool LSString::operator < (const LSFunction*) const {
	return true;
}
bool LSString::operator < (const LSClass*) const {
	return true;
}

bool LSString::operator > (const LSValue* v) const {
	return v->operator > (this);
}
bool LSString::operator > (const LSNull*) const {
	return true;
}
bool LSString::operator > (const LSBoolean*) const {
	return true;
}
bool LSString::operator > (const LSNumber*) const {
	return true;
}
bool LSString::operator > (const LSString* v) const {
	return *this > *v;
}
bool LSString::operator > (const LSArray<LSValue*>*) const {
	return false;
}
bool LSString::operator > (const LSArray<int>*) const {
	return false;
}
bool LSString::operator > (const LSArray<double>*) const {
	return false;
}
bool LSString::operator > (const LSObject*) const {
	return false;
}
bool LSString::operator > (const LSFunction*) const {
	return false;
}
bool LSString::operator > (const LSClass*) const {
	return false;
}

bool LSString::operator <= (const LSValue* v) const {
	return v->operator <= (this);
}
bool LSString::operator <= (const LSNull*) const {
	return false;
}
bool LSString::operator <= (const LSBoolean*) const {
	return false;
}
bool LSString::operator <= (const LSNumber*) const {
	return false;
}
bool LSString::operator <= (const LSString* v) const {
	return *this <= *v;
}
bool LSString::operator <= (const LSArray<LSValue*>*) const {
	return true;
}
bool LSString::operator <= (const LSArray<int>*) const {
	return true;
}
bool LSString::operator <= (const LSArray<double>*) const {
	return true;
}
bool LSString::operator <= (const LSObject*) const {
	return true;
}
bool LSString::operator <= (const LSFunction*) const {
	return true;
}
bool LSString::operator <= (const LSClass*) const {
	return true;
}

bool LSString::operator >= (const LSValue* v) const {
	return v->operator >= (this);
}
bool LSString::operator >= (const LSNull*) const {
	return true;
}
bool LSString::operator >= (const LSBoolean*) const {
	return true;
}
bool LSString::operator >= (const LSNumber*) const {
	return true;
}
bool LSString::operator >= (const LSString* v) const {
	return *this >= *v;
}
bool LSString::operator >= (const LSArray<LSValue*>*) const {
	return false;
}
bool LSString::operator >= (const LSArray<int>*) const {
	return false;
}
bool LSString::operator >= (const LSArray<double>*) const {
	return false;
}
bool LSString::operator >= (const LSObject*) const {
	return false;
}
bool LSString::operator >= (const LSFunction*) const {
	return false;
}
bool LSString::operator >= (const LSClass*) const {
	return false;
}

bool LSString::in(const LSValue*) const {
	return false;
}

u_int32_t LSString::u8_char_at(char* s, int pos) {
	int i = 0;
	int j = 0;
	int l = strlen(s);
	while (i < l) {
		u_int32_t c = u8_nextchar(s, &i);
		if (j++ == pos) return c;
	}
	return 0;
}

LSValue* LSString::at(const LSValue* key) const {
	if (const LSNumber* n = dynamic_cast<const LSNumber*>(key)) {
//		return new LSString(this->operator[] ((int) n->value));
		char buff[5];
		u_int32_t c = u8_char_at((char*) this->c_str(), (int) n->value);
		u8_toutf8(buff, 5, &c, 1);
		return new LSString(buff);
	}
	return LSNull::null_var;
}

LSValue** LSString::atL(const LSValue*) {
	// TODO
	return &LSNull::null_var;
}

LSValue* LSString::range(int start, int end) const {

	char buff[5];
	char* string_chars = (char*) this->c_str();
	std::string new_string;

	int i = 0;
	int j = 0;
	int l = strlen(string_chars);
	while (i < l) {
		j++;
		u_int32_t c = u8_nextchar(string_chars, &i);
		if (j <= start) continue;
		u8_toutf8(buff, 5, &c, 1);
		new_string += buff;
		if (j > end) break;
	}
	return new LSString(new_string);
//	return new LSString(this->substr(start, end - start + 1));
}
LSValue* LSString::rangeL(int, int) {
	// TODO
	return this;
}

LSValue* LSString::attr(const LSValue* key) const {
	if (*((LSString*) key) == "class") {
		return getClass();
	}
	return LSNull::null_var;
}

LSValue** LSString::attrL(const LSValue*) {
	return &LSNull::null_var;
}

LSValue* LSString::abso() const {
	return LSNumber::get(unicode_length());
}

std::ostream& LSString::print(std::ostream& os) const {
	os << "'" << *this << "'";
	return os;
}
string LSString::json() const {
	return "\"" + *this + "\"";
}

LSValue* LSString::clone() const {
	return new LSString((std::string) *this);
}

std::ostream& operator << (std::ostream& os, const LSString& obj) {
	os << obj;
	return os;
}

LSValue* LSString::getClass() const {
	return LSString::string_class;
}

int LSString::typeID() const {
	return 4;
}

const BaseRawType* LSString::getRawType() const {
	return RawType::STRING;
}

}
