#include "../../compiler/value/Expression.hpp"

#include "../../compiler/value/ArrayAccess.hpp"
#include "../../compiler/value/Function.hpp"
#include "../../compiler/value/LeftValue.hpp"
#include "../../compiler/value/Number.hpp"
#include "../../compiler/value/VariableValue.hpp"
#include "../../vm/LSValue.hpp"
#include "../../vm/value/LSBoolean.hpp"
#include "../../vm/value/LSArray.hpp"

using namespace std;

namespace ls {

Expression::Expression() : Expression(nullptr) {}

Expression::Expression(Value* v) :
	v1(v), v2(nullptr), op(nullptr), ignorev2(false), no_op(false), operations(0)
{}

Expression::~Expression() {
	if (v1 != nullptr) {
		delete v1;
	}
	if (op != nullptr) {
		delete op;
	}
	if (v2 != nullptr) {
		delete v2;
	}
}

void Expression::append(Operator* op, Value* exp) {

	/*
	 * Single expression (2, 'hello', ...), just add the operator
	 */
	if (this->op == nullptr) {
		this->op = op;
		v2 = exp;
		return;
	}

	if (!parenthesis and (op->priority < this->op->priority
		   or (op->priority == this->op->priority and op->character == "="))) {
		/*
		 * We already have '5 + 2' for example,
		 * and try to add a operator with a higher priority,
		 * such as : '× 7' => '5 + (2 × 7)'
		 */
		Expression* ex = new Expression();
		ex->v1 = v2;
		ex->op = op;
		ex->v2 = exp;
		this->v2 = ex;

	} else {
		/*
		 * We already have '5 + 2' for example,
		 * and try to add a operator with a lower priority,
		 * such as : '< 7' => '(5 + 2) < 7'
		 */
		Expression* newV1 = new Expression();
		newV1->v1 = this->v1;
		newV1->op = this->op;
		newV1->v2 = this->v2;
		this->v1 = newV1;
		this->op = op;
		v2 = exp;
	}
}

void Expression::print(std::ostream& os) const {

	os << "Exp {";

	if (v1 != nullptr) {

		v1->print(os);

		if (op != nullptr) {
			os << " ";
			op->print(os);
			os << " ";
			v2->print(os);
		}
	}
	os << "}";
}

void Expression::analyse(SemanticAnalyser* analyser, const Type) {

	type = Type::VALUE;
	constant = true;
	operations = 1;

	if (op == nullptr) {
		v1->analyse(analyser, Type::NEUTRAL);
		type = v1->type;
		return;
	}

	Type v1_type = Type::NEUTRAL;
	Type v2_type = Type::NEUTRAL;

	if (op->type == TokenType::DIVIDE) {
		type.raw_type = RawType::FLOAT;
		v1_type = Type::FLOAT;
		v2_type = Type::FLOAT;
	}

	v1->analyse(analyser, v1_type);
	if (v1->type.nature == Nature::POINTER) type.nature = Nature::POINTER;
	if (v1->constant == false) constant = false;

	v2->analyse(analyser, v2_type);
	if (v2->type.nature == Nature::POINTER) type.nature = Nature::POINTER;
	if (v2->constant == false) constant = false;


	if (op->type == TokenType::GREATER or op->type == TokenType::DOUBLE_EQUAL or
		op->type == TokenType::LOWER or op->type == TokenType::LOWER_EQUALS or
		op->type == TokenType::GREATER_EQUALS or op->type == TokenType::TRIPLE_EQUAL or
		op->type == TokenType::DIFFERENT or op->type == TokenType::TRIPLE_DIFFERENT) {

		type = Type::BOOLEAN;
	}

	// Array += element
	if (op->type == TokenType::PLUS_EQUAL && v1->type.raw_type == RawType::ARRAY) {
		VariableValue* vv = dynamic_cast<VariableValue*>(v1);
		if (vv->type.raw_type == RawType::ARRAY) {
			vv->var->will_take_element(analyser, v2->type);
		}
	}

	if (op->type == TokenType::EQUAL or op->type == TokenType::PLUS_EQUAL
		or op->type == TokenType::PLUS or op->type == TokenType::TIMES
		or op->type == TokenType::MINUS) {

		type = v1->type.mix(v2->type);

		if (op->type == TokenType::EQUAL or op->type == TokenType::PLUS_EQUAL) {
			VariableValue* vv = dynamic_cast<VariableValue*>(v1);
			if (vv != nullptr and vv->var->value != nullptr) {
				// TODO not working
				//vv->var->must_be_pointer(analyser);
			}
		}
	}

	if (op->type == TokenType::TILDE_TILDE) {

		v2->will_take(analyser, 0, Type::POINTER);

		VariableValue* vv = dynamic_cast<VariableValue*>(v2);
		if (vv != nullptr and vv->var->value != nullptr) {

			vv->var->will_take(analyser, 0, Type::POINTER);
			v2->type.setReturnType(vv->var->type.getReturnType());
		}
		type = Type::ARRAY;
	}

	if (op->type == TokenType::DOUBLE_QUESTION_MARK) {
		type = Type::POINTER;
	}

	// Merge operations count
	// (2 + 3) × 4    ->  2 ops for the × directly
	if (op->type != TokenType::OR or op->type == TokenType::AND) {
		if (Expression* ex1 = dynamic_cast<Expression*>(v1)) {
			operations += ex1->operations;
			ex1->operations = 0;
		}
		if (Expression* ex2 = dynamic_cast<Expression*>(v2)) {
			operations += ex2->operations;
			ex2->operations = 0;
		}
	}
//	cout << "exp type " << type << endl;
}


LSValue* jit_add(LSValue* x, LSValue* y) {
	return y->operator + (x);
}
LSValue* jit_int_array_add(LSArray<int>* x, LSArray<int>* y) {
	return x->operator + (y);
}
LSValue* jit_sub(LSValue* x, LSValue* y) {
	return y->operator - (x);
}
LSValue* jit_mul(LSValue* x, LSValue* y) {
	return y->operator * (x);
}
LSValue* jit_div(LSValue* x, LSValue* y) {
	return y->operator / (x);
}
LSValue* jit_pow(LSValue* x, LSValue* y) {
	return y->poww(x);
}
LSValue* jit_mod(LSValue* x, LSValue* y) {
	return y->operator % (x);
}
LSValue* jit_and(LSValue* x, LSValue* y) {
	return LSBoolean::get(x->isTrue() and y->isTrue());
}
LSValue* jit_or(LSValue* x, LSValue* y) {
	return LSBoolean::get(x->isTrue() or y->isTrue());
}
LSValue* jit_xor(LSValue* x, LSValue* y) {
	return LSBoolean::get(x->isTrue() xor y->isTrue());
}
LSValue* jit_inc(LSValue* x) {
	return x->operator ++ (1);
}
LSValue* jit_dec(LSValue* x) {
	return x->operator -- (1);
}

LSValue* jit_equals(LSValue* x, LSValue* y) {
	return LSBoolean::get(x->operator == (y));
}
LSValue* jit_not_equals(LSValue* x, LSValue* y) {
	return LSBoolean::get(x->operator != (y));
}
LSValue* jit_lt(LSValue* x, LSValue* y) {
	return LSBoolean::get(y->operator < (x));
}
LSValue* jit_le(LSValue* x, LSValue* y) {
	return LSBoolean::get(y->operator <= (x));
}
LSValue* jit_gt(LSValue* x, LSValue* y) {
	return LSBoolean::get(y->operator > (x));
}
LSValue* jit_ge(LSValue* x, LSValue* y) {
	return LSBoolean::get(y->operator >= (x));
}

LSValue* jit_store(LSValue** x, LSValue* y) {
	y->refs++;
	return *x = y;
}

LSValue* jit_store_value(int* x, int y) {
	return LSNumber::get(*x = y);
}

LSValue* jit_swap(LSValue** x, LSValue** y) {
	LSValue* tmp = *x;
	*x = *y;
	*y = tmp;
	return *x;
}

LSValue* jit_add_equal(LSValue* x, LSValue* y) {
	return y->operator += (x);
}
LSValue* jit_add_equal_value(int* x, int y) {
	return LSNumber::get(*x += y);
}
LSValue* jit_sub_equal(LSValue* x, LSValue* y) {
	return y->operator -= (x);
}
LSValue* jit_mul_equal(LSValue* x, LSValue* y) {
	return y->operator *= (x);
}
LSValue* jit_div_equal(LSValue* x, LSValue* y) {
	return y->operator /= (x);
}
LSValue* jit_mod_equal(LSValue* x, LSValue* y) {
	return y->operator %= (x);
}
LSValue* jit_pow_equal(LSValue* x, LSValue* y) {
	return y->pow_eq(x);
}
int jit_array_add_value(LSArray<int>* x, int v) {
	x->push_clone(v);
	return v;
}

LSArray<LSValue*>* jit_tilde_tilde_int(LSArray<int>* array, LSFunction* fun) {
	return array->map(fun->function);
}

LSArray<LSValue*>* jit_tilde_tilde_pointer(LSArray<LSValue*>* array, LSFunction* fun) {
	return array->map(fun->function);
}

LSValue* jit_in(LSValue* x, LSValue* y) {
	return LSBoolean::get(y->in(x));
}

LSValue* jit_instanceof(LSValue* x, LSValue* y) {
	return LSBoolean::get(((LSClass*)x->getClass())->name == ((LSClass*) y)->name);
}

LSValue* jit_bit_and(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_and_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_or(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_or_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_xor(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_xor_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shl(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shl_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shr(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shr_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shr_unsigned(LSValue*, LSValue*) {
	return LSNull::null_var;
}
LSValue* jit_bit_shr_unsigned_equal(LSValue*, LSValue*) {
	return LSNull::null_var;
}
bool jit_is_null(LSValue* v) {
	return v->typeID() == 1;
}

jit_value_t Expression::compile_jit(Compiler& c, jit_function_t& F, Type req_type) const {

	if (op == nullptr) {
		return v1->compile_jit(c, F, req_type);
	}

	if (operations > 0) {
		VM::inc_ops(F, operations);
	}

//	cout << "v1 : " << v1->type << ", v2 : " << v2->type << endl;

	jit_value_t (*jit_func)(jit_function_t, jit_value_t, jit_value_t) = nullptr;
	void* ls_func;
	bool use_jit_func = v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE;
	vector<jit_value_t> args;
	Type conv_info = req_type;
	Type v1_conv = Type::POINTER;
	Type v2_conv = Type::POINTER;

	switch (op->type) {
		case TokenType::EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {

				if (ArrayAccess* l1 = dynamic_cast<ArrayAccess*>(v1)) {

					args.push_back(l1->compile_jit_l(c, F, Type::NEUTRAL));
					args.push_back(v2->compile_jit(c, F, Type::NEUTRAL));

					ls_func = (void*) &jit_store_value;
					v1_conv = Type::NEUTRAL;
					v2_conv = Type::NEUTRAL;
					use_jit_func = false;

				} else {

					jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
					jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
					jit_insn_store(F, x, y);
					if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
						return VM::value_to_pointer(F, y, req_type);
					}
					return y;
				}
			} else if (v1->type.nature == Nature::POINTER) {

				if (dynamic_cast<ArrayAccess*>(v1)) {

					args.push_back(((LeftValue*) v1)->compile_jit_l(c, F, Type::POINTER));
					args.push_back(v2->compile_jit(c, F, Type::POINTER));

					if (v1->type.must_manage_memory()) {
						VM::inc_refs(F, args[1]);
					}

					ls_func = (void*) &jit_store;

				} else if (dynamic_cast<VariableValue*>(v1)) {

					jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
					jit_value_t y = v2->compile_jit(c, F, Type::POINTER);

					if (v2->type.must_manage_memory()) {
						VM::inc_refs(F, y);
					}
					if (v2->type.must_manage_memory()) {
						VM::delete_obj(F, x);
					}
					jit_insn_store(F, x, y);
					return y;

				} else {
					args.push_back(((LeftValue*) v1)->compile_jit_l(c, F, Type::POINTER));
					args.push_back(v2->compile_jit(c, F, Type::POINTER));
					ls_func = (void*) &jit_store;
				}
			} else {
				throw new runtime_error("value = pointer !");
			}
			break;
		}
		case TokenType::SWAP: {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t t = jit_insn_load(F, x);
				jit_insn_store(F, x, y);
				jit_insn_store(F, y, t);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, x, req_type);
				}
				return x;
			} else {
				args.push_back(((LeftValue*) v1)->compile_jit_l(c, F, Type::POINTER));
				args.push_back(((LeftValue*) v2)->compile_jit_l(c, F, Type::POINTER));
				ls_func = (void*) &jit_swap;
			}
			break;
		}
		case TokenType::PLUS_EQUAL: {

			if (v1->type.raw_type == RawType::ARRAY) {
//				cout << "Array add " << endl;
				ls_func = (void*) jit_array_add_value;
				v2_conv = Type::NEUTRAL;
				break;
			}

			if (ArrayAccess* l1 = dynamic_cast<ArrayAccess*>(v1)) {

				args.push_back(l1->compile_jit_l(c, F, Type::NEUTRAL));
				args.push_back(v2->compile_jit(c, F, Type::NEUTRAL));
				ls_func = (void*) &jit_add_equal_value;
				v1_conv = Type::NEUTRAL;
				v2_conv = Type::NEUTRAL;
				use_jit_func = false;
				break;
			}

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t sum = jit_insn_add(F, x, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_add_equal;
			}
			break;
		}
		case TokenType::MINUS_EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t sum = jit_insn_sub(F, x, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_sub_equal;
			}
			break;
		}
		case TokenType::TIMES_EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t sum = jit_insn_mul(F, x, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_mul_equal;
			}
			break;
		}
		case TokenType::DIVIDE_EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t xf = jit_value_create(F, JIT_FLOAT);
				jit_insn_store(F, xf, x);
				jit_value_t sum = jit_insn_div(F, xf, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_div_equal;
			}
			break;
		}
		case TokenType::MODULO_EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t sum = jit_insn_rem(F, x, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_mod_equal;
			}
			break;
		}
		case TokenType::POWER_EQUAL: {

			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t sum = jit_insn_pow(F, x, y);
				jit_insn_store(F, x, sum);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, sum, req_type);
				}
				return sum;
			} else {
				ls_func = (void*) &jit_pow_equal;
			}
			break;
		}
		case TokenType::AND: {
			jit_func = &jit_insn_and;
			ls_func = (void*) &jit_and;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::OR: {
			jit_func = &jit_insn_or;
			ls_func = (void*) &jit_or;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::XOR: {
			if (use_jit_func) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				x = jit_insn_to_not_bool(F, x);
				y = jit_insn_to_not_bool(F, y);
				jit_value_t r = jit_insn_or(F,
					jit_insn_and(F, x, jit_insn_not(F, y)),
					jit_insn_and(F, y, jit_insn_not(F, x))
				);
				if (req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, r, Type::BOOLEAN);
				}
				return r;
			} else {
				ls_func = (void*) &jit_xor;
				conv_info = Type::BOOLEAN;
			}
			break;
		}
		case TokenType::PLUS: {
			jit_func = &jit_insn_add;
			if (v1->type == Type::INT_ARRAY and v2->type == Type::INT_ARRAY) {
				ls_func = (void*) &jit_int_array_add;
			} else {
				ls_func = (void*) &jit_add;
			}
			break;
		}
		case TokenType::MINUS: {
			jit_func = &jit_insn_sub;
			ls_func = (void*) &jit_sub;
			break;
		}
		case TokenType::TIMES: {
			jit_func = &jit_insn_mul;
			ls_func = (void*) &jit_mul;
			break;
		}
		case TokenType::DIVIDE: {
			jit_func = &jit_insn_div;
			ls_func = (void*) &jit_div;
			conv_info = Type::FLOAT;
			break;
		}
		case TokenType::MODULO: {
			jit_func = &jit_insn_rem;
			ls_func = (void*) &jit_mod;
			break;
		}
		case TokenType::POWER: {
			jit_func = &jit_insn_pow;
			ls_func = (void*) &jit_pow;
			break;
		}
		case TokenType::DOUBLE_EQUAL: {
			jit_func = &jit_insn_eq;
			ls_func = (void*) &jit_equals;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::DIFFERENT: {
			jit_func = &jit_insn_ne;
			ls_func = (void*) &jit_not_equals;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::LOWER: {
			jit_func = &jit_insn_lt;
			ls_func = (void*) &jit_lt;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::LOWER_EQUALS: {
			jit_func = &jit_insn_le;
			ls_func = (void*) &jit_le;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::GREATER: {
			jit_func = &jit_insn_gt;
			ls_func = (void*) &jit_gt;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::GREATER_EQUALS: {
			jit_func = &jit_insn_ge;
			ls_func = (void*) &jit_ge;
			conv_info = Type::BOOLEAN;
			break;
		}
		case TokenType::TILDE_TILDE: {
			if (v1->type.getElementType() == Type::INTEGER) {
				ls_func = (void*) &jit_tilde_tilde_int;
			} else {
				ls_func = (void*) &jit_tilde_tilde_pointer;
			}
			v2_conv = Type(RawType::FUNCTION, Nature::POINTER);
			break;
		}
		case TokenType::IN: {
			use_jit_func = false;
			ls_func = (void*) &jit_in;
			break;
		}
		case TokenType::INSTANCEOF: {
			use_jit_func = false;
			ls_func = (void*) &jit_instanceof;
			break;
		}
		case TokenType::BIT_AND: {
			jit_func = &jit_insn_and;
			ls_func = (void*) &jit_bit_and;
			break;
		}
		case TokenType::BIT_AND_EQUALS: {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t a = jit_insn_and(F, x, y);
				jit_insn_store(F, x, a);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, a, req_type);
				}
				return a;
			} else {
				ls_func = (void*) &jit_bit_and_equal;
			}
			break;
		}
		case TokenType::PIPE: {
			jit_func = &jit_insn_or;
			ls_func = (void*) &jit_bit_or;
			break;
		}
		case TokenType::BIT_OR_EQUALS: {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t o = jit_insn_or(F, x, y);
				jit_insn_store(F, x, o);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, o, req_type);
				}
				return o;
			} else {
				ls_func = (void*) &jit_bit_or_equal;
			}
			break;
		}
		case TokenType::BIT_XOR: {
			jit_func = &jit_insn_xor;
			ls_func = (void*) &jit_bit_xor;
			break;
		}
		case TokenType::BIT_XOR_EQUALS: {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t a = jit_insn_xor(F, x, y);
				jit_insn_store(F, x, a);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, a, req_type);
				}
				return a;
			} else {
				ls_func = (void*) &jit_bit_xor_equal;
			}
			break;
		}
		case TokenType::BIT_SHIFT_LEFT : {
			jit_func = &jit_insn_shl;
			ls_func = (void*) &jit_bit_shl;
			break;
		}
		case TokenType::BIT_SHIFT_LEFT_EQUALS : {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t a = jit_insn_shl(F, x, y);
				jit_insn_store(F, x, a);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, a, req_type);
				}
				return a;
			} else {
				ls_func = (void*) &jit_bit_shl_equal;
			}
			break;
		}
		case TokenType::BIT_SHIFT_RIGHT : {
			jit_func = &jit_insn_shr;
			ls_func = (void*) &jit_bit_shr;
			break;
		}
		case TokenType::BIT_SHIFT_RIGHT_EQUALS : {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t a = jit_insn_shr(F, x, y);
				jit_insn_store(F, x, a);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, a, req_type);
				}
				return a;
			} else {
				ls_func = (void*) &jit_bit_shr_equal;
			}
			break;
		}
		case TokenType::BIT_SHIFT_RIGHT_UNSIGNED : {
			jit_func = &jit_insn_ushr;
			ls_func = (void*) &jit_bit_shr_unsigned;
			break;
		}
		case TokenType::BIT_SHIFT_RIGHT_UNSIGNED_EQUALS : {
			if (v1->type.nature == Nature::VALUE and v2->type.nature == Nature::VALUE) {
				jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t a = jit_insn_ushr(F, x, y);
				jit_insn_store(F, x, a);
				if (v2->type.nature != Nature::POINTER and req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, a, req_type);
				}
				return a;
			} else {
				ls_func = (void*) &jit_bit_shr_unsigned_equal;
			}
			break;
		}
		case TokenType::DOUBLE_QUESTION_MARK: {

			// x ?? y ==> if (x == null) { y } else { x }

			jit_label_t label_end = jit_label_undefined;
			jit_label_t label_else = jit_label_undefined;
			jit_value_t v = jit_value_create(F, ls_jit_pointer);

			jit_type_t args_types[2] = {ls_jit_pointer};
			jit_value_t x = v1->compile_jit(c, F, Type::POINTER);
			jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, ls_jit_integer, args_types, 1, 0);
			jit_value_t r = jit_insn_call_native(F, "is_null", (void*) jit_is_null, sig, &x, 1, JIT_CALL_NOTHROW);

			jit_insn_branch_if_not(F, r, &label_else);

			jit_value_t y = v2->compile_jit(c, F, Type::POINTER);
			jit_insn_store(F, v, y);

			jit_insn_branch(F, &label_end);
			jit_insn_label(F, &label_else);

			jit_insn_store(F, v, x);

			jit_insn_label(F, &label_end);

			return v;
			break;
		}
		default: {
			throw new exception();
		}
		}

	if (use_jit_func) {

		jit_value_t x = v1->compile_jit(c, F, Type::NEUTRAL);
		jit_value_t y = v2->compile_jit(c, F, Type::NEUTRAL);

		jit_value_t r = jit_func(F, x, y);

		if (req_type.nature == Nature::POINTER) {
			return VM::value_to_pointer(F, r, conv_info);
		}
		return r;

	} else {

		jit_type_t args_types[2] = {JIT_POINTER, JIT_POINTER};
		jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, JIT_POINTER, args_types, 2, 0);

		if (args.size() == 0) {
			args.push_back(v1->compile_jit(c, F, v1_conv));
			args.push_back(v2->compile_jit(c, F, v2_conv));
		}
		jit_value_t v = jit_insn_call_native(F, "", ls_func, sig, args.data(), 2, JIT_CALL_NOTHROW);
		if (v1->type.nature == Nature::VALUE and op->type == TokenType::PLUS_EQUAL) {
			jit_insn_store(F, args[0], v);
		}

		// Delete operands
		if (v1_conv.nature == Nature::POINTER) {
			VM::delete_temporary(F, args[0]);
		}
		if (v2_conv.nature == Nature::POINTER) {
			VM::delete_temporary(F, args[1]);
		}

		// Convert to value
		if (type == Type::BOOLEAN and req_type == Type::BOOLEAN) {
			return VM::pointer_to_value(F, v, Type::BOOLEAN);
		}
		return v;
	}
}

}

