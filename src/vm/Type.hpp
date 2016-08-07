#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <iostream>

namespace ls {

/*
enum class RawType {
	UNKNOWN, NULLL, BOOLEAN, NUMBER, INTEGER, LONG, FLOAT, STRING, OBJECT, ARRAY, FUNCTION, CLASS
};
*/

enum class Nature {
	UNKNOWN, VALUE, POINTER, VOID
};

class BaseRawType {
public:
	virtual const std::string getName() const { return "?"; }
	virtual const std::string getClass() const { return "?"; }
	virtual const std::string getJsonName() const { return "?"; }
	virtual bool derived_from(const BaseRawType*) const { return false; }
};

class VoidRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "void"; }
	virtual const std::string getJsonName() const { return "void"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class NullRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "null"; }
	virtual const std::string getClass() const { return "Null"; }
	virtual const std::string getJsonName() const { return "null"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class BooleanRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "bool"; }
	virtual const std::string getClass() const { return "Boolean"; }
	virtual const std::string getJsonName() const { return "boolean"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class NumberRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "number"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class IntegerRawType : public NumberRawType {
public:
	virtual const std::string getName() const { return "int"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual bool operator > (const NumberRawType*) { return true; }
	virtual bool derived_from(const NumberRawType*) const { return true; }
};

class LongRawType : public NumberRawType {
public:
	virtual const std::string getName() const { return "long"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual bool derived_from(const NumberRawType*) const { return true; }
};

class FloatRawType : public NumberRawType {
public:
	virtual const std::string getName() const { return "real"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual bool derived_from(const NumberRawType*) const { return true; }
};

class StringRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "string"; }
	virtual const std::string getClass() const { return "String"; }
	virtual const std::string getJsonName() const { return "string"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class ArrayRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "array"; }
	virtual const std::string getClass() const { return "Array"; }
	virtual const std::string getJsonName() const { return "array"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class MapRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "map"; }
	virtual const std::string getClass() const { return "Map"; }
	virtual const std::string getJsonName() const { return "map"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class IntervalRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "interval"; }
	virtual const std::string getClass() const { return "Array"; }
	virtual const std::string getJsonName() const { return "array"; }
	virtual bool derived_from(const ArrayRawType*) const { return true; }
};

class ObjectRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "object"; }
	virtual const std::string getClass() const { return "Object"; }
	virtual const std::string getJsonName() const { return "object"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class FunctionRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "function"; }
	virtual const std::string getClass() const { return "Function"; }
	virtual const std::string getJsonName() const { return "function"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class ClassRawType : public BaseRawType {
public:
	virtual const std::string getName() const { return "class"; }
	virtual const std::string getClass() const { return "Class"; }
	virtual const std::string getJsonName() const { return "class"; }
	virtual bool derived_from(const BaseRawType*) const { return true; }
};

class RawType {
public:
	static const BaseRawType* const UNKNOWN;
	static const VoidRawType* const VOID;
	static const NullRawType* const NULLL;
	static const BooleanRawType* const BOOLEAN;
	static const NumberRawType* const NUMBER;
	static const IntegerRawType* const INTEGER;
	static const LongRawType* const LONG;
	static const FloatRawType* const FLOAT;
	static const StringRawType* const STRING;
	static const ArrayRawType* const ARRAY;
	static const MapRawType* const MAP;
	static const IntervalRawType* const INTERVAL;
	static const ObjectRawType* const OBJECT;
	static const FunctionRawType* const FUNCTION;
	static const ClassRawType* const CLASS;
};

class Type {
public:

	const BaseRawType* raw_type;
	Nature nature;
	bool native; // A C++ objects, memory management is done outside the language
	std::string clazz;
	std::vector<Type> element_type;
	std::vector<Type> return_types;
	std::vector<Type> arguments_types;

	Type();
	Type(const Type& type);
	Type(const BaseRawType* raw_type, Nature nature);
	Type(const BaseRawType* raw_type, Nature nature, bool native);
	Type(const BaseRawType* raw_type, Nature nature, const Type& elements_type);
	Type(const BaseRawType* raw_type, Nature nature, const Type& elements_type, bool native);
	Type(const BaseRawType* raw_type, Nature nature, const std::vector<Type>& element_types);

	bool must_manage_memory() const;

	Type getReturnType() const;
	void setReturnType(Type type);

	void addArgumentType(Type type);
	void setArgumentType(const unsigned int index, Type type);
	const Type getArgumentType(const unsigned int index) const;
	const std::vector<Type> getArgumentTypes() const;
	const Type getElementType(size_t i = 0) const;
	void setElementType(Type);
	bool isHomogeneous() const;

	std::string get_class_name() const;

	bool will_take(const int i, const Type& arg_type);
	bool will_take_element(const Type& arg_type);
	Type mix(const Type& x) const;
	bool compatible(const Type& type) const;

	bool operator == (const Type&) const;
	bool operator != (const Type&) const;

	void toJson(std::ostream&) const;

	bool isNumber() const;
	bool derived_from(const Type& type);

	/*
	 * Static part
	 */
	static const Type VOID;
	static const Type VALUE;
	static const Type POINTER;

	static const Type UNKNOWN;
	static const Type NULLL;
	static const Type BOOLEAN;
	static const Type BOOLEAN_P;
	static const Type NUMBER;
	static const Type INTEGER;
	static const Type INTEGER_P;
	static const Type LONG;
	static const Type FLOAT;
	static const Type FLOAT_P;
	static const Type STRING;
	static const Type OBJECT;
	static const Type ARRAY;
	static const Type INT_ARRAY;
	static const Type FLOAT_ARRAY;
	static const Type STRING_ARRAY;
	static const Type MAP;
	static const Type PTR_PTR_MAP;
	static const Type PTR_INT_MAP;
	static const Type PTR_FLOAT_MAP;
	static const Type INT_PTR_MAP;
	static const Type INT_INT_MAP;
	static const Type INT_FLOAT_MAP;
	static const Type INTERVAL;
	static const Type FUNCTION;
	static const Type FUNCTION_P;
	static const Type CLASS;

	static std::string get_nature_name(const Nature& nature);
	static std::string get_nature_symbol(const Nature& nature);
	static bool list_compatible(const std::vector<Type>& expected, const std::vector<Type>& actual);
	static bool list_more_specific(const std::vector<Type>& old, const std::vector<Type>& neww);
	static bool more_specific(const Type& old, const Type& neww);
	static Type get_compatible_type(const Type& t1, const Type& t2);
};

std::ostream& operator << (std::ostream&, const Type&);

}

#endif
