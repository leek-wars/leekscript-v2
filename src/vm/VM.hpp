#ifndef VM_HPP
#define VM_HPP

#include <vector>
#include <string>
#include <jit/jit.h>
#include <gmp.h>
#include <gmpxx.h>

#include "Context.hpp"
#include "../compiler/lexical/LexicalError.hpp"
#include "../compiler/syntaxic/SyntaxicalError.hpp"
#include "../compiler/semantic/SemanticError.hpp"
#include "../compiler/Compiler.hpp"
#include "../vm/value/LSNumber.hpp"

#define OPERATION_LIMIT 10000000

#define LS_VOID jit_type_void
#define LS_INTEGER jit_type_int
#define LS_LONG jit_type_long
#define LS_REAL jit_type_float64
#define LS_BOOLEAN jit_type_sys_bool
#define LS_POINTER jit_type_void_ptr
#define LS_STRING LS_POINTER
#define LS_NUMBER LS_POINTER

#define LS_CREATE_INTEGER(F, X) jit_value_create_nint_constant((F), LS_INTEGER, (X))
#define LS_CREATE_BOOLEAN(F, X) jit_value_create_nint_constant((F), LS_INTEGER, (X))
#define LS_CREATE_LONG(F, X) jit_value_create_long_constant((F), LS_LONG, (X))
#define LS_CREATE_REAL(F, X) jit_value_create_float64_constant((F), LS_REAL, (X))

struct jit_stack_trace {
	unsigned int size;
	void* items[1];
};

namespace ls {

class Type;
class Module;
class Program;
class LSValue;
class LexicalError;
class Compiler;
class SemanticVar;

class VM {
public:

	enum Exception {
		DIVISION_BY_ZERO = -2,
		NO_EXCEPTION = 0,
		EXCEPTION = 1,
		OPERATION_LIMIT_EXCEEDED = 2,
		NUMBER_OVERFLOW = 3,
		NO_SUCH_OPERATOR = 4,
		ARRAY_OUT_OF_BOUNDS = 5,
		ARRAY_KEY_IS_NOT_NUMBER = 6,
		CANT_MODIFY_READONLY_OBJECT = 7,
		NO_SUCH_ATTRIBUTE = 8
	};

	struct ExceptionObj {
		Exception type;
		std::vector<int> lines;
		std::vector<std::string> files;
		std::vector<std::string> functions;
		std::vector<void*> pcs;
		std::vector<void*> frames;
		ExceptionObj(Exception type) : type(type) {}
	};

	static const unsigned long int DEFAULT_OPERATION_LIMIT;
	static jit_type_t mpz_type;
	static VM* current_vm;

	struct Result {
		bool compilation_success = false;
		bool execution_success = false;
		std::vector<LexicalError> lexical_errors;
		std::vector<SyntaxicalError> syntaxical_errors;
		std::vector<SemanticError> semantical_errors;
		ExceptionObj* exception = nullptr;
		std::string program = "";
		std::string value = "";
		std::string context = "";
		long compilation_time = 0;
		long compilation_time_ms = 0;
		long execution_time = 0;
		long execution_time_ms = 0;
		long operations = 0;
		int objects_created = 0;
		int objects_deleted = 0;
		int mpz_objects_created = 0;
		int mpz_objects_deleted = 0;
	};

	std::vector<Module*> modules;
	std::map<std::string, LSValue*> system_vars;
	std::map<std::string, std::shared_ptr<SemanticVar>> internal_vars;
	std::map<std::string, jit_value_t> internals;
	Compiler compiler;
	LSNull* null_value;
	LSBoolean* true_value;
	LSBoolean* false_value;
	unsigned int operations = 0;
	bool enable_operations = true;
	unsigned long int operation_limit;
	std::ostream* output = &std::cout;
	long mpz_created = 0;
	long mpz_deleted = 0;
	ExceptionObj* last_exception = nullptr;
	jit_stack_trace_t stack_trace;
	jit_context_t jit_context;
	std::string file_name;

	VM(bool v1 = false);
	virtual ~VM();

	static VM* current();

	/** Main execution function **/
	Result execute(const std::string code, std::string ctx, std::string file_name, bool debug = false, bool ops = true);

	/** Add a module **/
	void add_module(Module* m);
	void add_internal_var(std::string name, Type type);
	static jit_type_t get_jit_type(const Type& type);

	/** Value creation **/
	static jit_value_t create_array(jit_function_t F, const Type& element_type,
		int cap = 0);

	/** Conversions **/
	static jit_value_t value_to_pointer(jit_function_t, jit_value_t, Type);
	static jit_value_t pointer_to_value(jit_function_t, jit_value_t, Type);
	static jit_value_t int_to_real(jit_function_t F, jit_value_t v);

	/** Ref counting and memory management **/
	static jit_value_t get_refs(jit_function_t F, jit_value_t obj);
	static void inc_refs_if_not_temp(jit_function_t F, jit_value_t obj);
	static void dec_refs(jit_function_t F, jit_value_t obj);
	static void inc_mpz_counter(jit_function_t F);

	/** Utilities **/
	static void print_mpz_int(jit_function_t F, jit_value_t val);
	void store_exception(jit_function_t F, jit_value_t ex);
	static std::string exception_message(VM::Exception expected);

	template <unsigned int level>
	inline static void* get_exception_object(int obj) {
		auto ex = new VM::ExceptionObj((VM::Exception) obj);
		auto frame = __builtin_frame_address(level);
		auto pc = jit_get_return_address(frame);
		auto trace = (jit_stack_trace_t) jit_malloc(sizeof(struct jit_stack_trace));
		trace->size = 1;
		auto context = VM::current()->jit_context;
		while (true) {
			ex->pcs.push_back(pc);
			ex->frames.push_back(frame);
			trace->items[0] = pc;
			unsigned int line = jit_stack_trace_get_offset(context, trace, 0);
			if (ex->lines.size() > 1 && line == JIT_NO_OFFSET) break;
			ex->lines.push_back(line);
			auto function = jit_function_from_pc(context, pc, nullptr);
			auto name = function ? (std::string*) jit_function_get_meta(function, 12) : nullptr;
			auto file = function ? (std::string*) jit_function_get_meta(function, 13) : nullptr;
			ex->files.push_back(file == nullptr ? "?" : *file);
			ex->functions.push_back(name == nullptr ? "?" : *name);
			frame = jit_get_next_frame_address(frame);
			pc = jit_get_return_address(frame);
		}
		jit_free(trace);
		return ex;
	}
};

}

#endif
