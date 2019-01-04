#ifndef CONTINUE_HPP
#define CONTINUE_HPP

#include "Instruction.hpp"

namespace ls {

class Continue : public Instruction {
public:

	int deepness;

	Continue();
	virtual ~Continue();

	virtual void print(std::ostream&, int indent, bool debug) const override;
	virtual Location location() const override;

	virtual void analyse(SemanticAnalyser*, const Type& req_type) override;

	virtual Compiler::value compile(Compiler&) const override;

	virtual Instruction* clone() const override;
};

}

#endif
