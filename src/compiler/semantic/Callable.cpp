#include "Callable.hpp"
#include "../semantic/SemanticAnalyzer.hpp"
#include "../../vm/Module.hpp"
#include "CallableVersion.hpp"

namespace ls {

void Callable::add_version(const CallableVersion* v) {
	versions.push_back(v);
}

const CallableVersion* Callable::resolve(SemanticAnalyzer* analyzer, std::vector<const Type*> arguments) const {
	// std::cout << "Callable::resolve(" << arguments << ")" << std::endl;
	const CallableVersion* best = nullptr;
	int best_score = std::numeric_limits<int>::max();
	for (auto& version : versions) {
		auto result = version->get_score(analyzer, arguments);
		if (best == nullptr or result.first <= best_score) {
			best_score = result.first;
			best = result.second;
		}
	}
	return best;
}

bool Callable::is_compatible(int argument_count) {
	for (const auto& version : versions) {
		if (version->type->arguments().size() == (size_t) argument_count) return true;
	}
	return false;
}

void CallableVersion::apply_mutators(SemanticAnalyzer* analyzer, std::vector<Value*> values) const {
	// std::cout << "CallableVersion::apply_mutators() mutators : " << mutators.size() << std::endl;
	for (const auto& mutator : mutators) {
		mutator->apply(analyzer, values);
	}
}

}

namespace std {
	std::ostream& operator << (std::ostream& os, const ls::Callable* callable) {
		os << "[" << std::endl;
		for (const auto& v : callable->versions) {
			os << "    " << *v << std::endl;
		}
		os << "]";
		return os;
	}
}