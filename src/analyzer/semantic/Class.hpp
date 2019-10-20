#ifndef CLASS_HPP
#define CLASS_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include "../../constants.h"
#include "Call.hpp"
#include "Callable.hpp"
#include "CallableVersion.hpp"
#if COMPILER
#include "../../compiler/Compiler.hpp"
#endif

namespace ls {

class Callable;
class CallableVersion;
class Type;
class Environment;

class Class {
public:

	class field {
	public:
		std::string name;
		const Type* type;
		void* native_fun = nullptr;
		void* addr = nullptr;
		#if COMPILER
		LSValue* value = nullptr;
		std::function<Compiler::value(Compiler&, Compiler::value)> fun = nullptr;
		std::function<Compiler::value(Compiler&)> static_fun = nullptr;
		LSValue* default_value = nullptr;
		#endif
		field(std::string name, const Type* type) : name(name), type(type) {}
		field(std::string name, const Type* type, void* fun);
		#if COMPILER
		field(std::string name, const Type* type, std::function<Compiler::value(Compiler&, Compiler::value)> fun, LSValue* default_value) : name(name), type(type), fun(fun), default_value(default_value) {}
		field(std::string name, const Type* type, std::function<Compiler::value(Compiler&)> static_fun) : name(name), type(type), static_fun(static_fun) {}
		field(std::string name, const Type* type, LSValue* value) : name(name), type(type), value(value) {}
		#endif
	};

	Environment& env;
	std::string name;
	Class* parent;
	std::unordered_map<std::string, field> fields;
	std::unordered_map<std::string, field> static_fields;
	std::unordered_map<std::string, Callable> methods;
	std::unordered_map<std::string, Callable> operators;
	std::unordered_map<std::string, Call> operators_callables;

	Class(Environment& env, std::string name);
	~Class();

	void addMethod(std::string, std::initializer_list<CallableVersion>, std::vector<const Type*> templates = {}, int flags = 0, bool legacy = false);
	#if COMPILER
	void addField(std::string, const Type*, std::function<Compiler::value(Compiler&, Compiler::value)> fun);
	#endif
	void addField(std::string, const Type*, void* fun);
	void addStaticField(field f);
	void addOperator(std::string name, std::initializer_list<CallableVersion>, std::vector<const Type*> templates = {}, bool legacy = false);
	const Call& getOperator(SemanticAnalyzer* analyzer, std::string& name);
	#if COMPILER
	LSFunction* getDefaultMethod(const std::string& name);
	#endif
};

}

#endif