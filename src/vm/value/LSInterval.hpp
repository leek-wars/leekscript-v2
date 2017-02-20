#ifndef VM_VALUE_LSINTERVAL_HPP_
#define VM_VALUE_LSINTERVAL_HPP_

#include "LSArray.hpp"

namespace ls {

class LSInterval : public LSValue {
public:

	static LSValue* clazz;

	int a = 0;
	int b = 0;

	LSInterval();
	virtual ~LSInterval();

	/*
	 * Array methods
	 */
	LSArray<int>* ls_filter(LSFunction<bool>* function);

	/*
	 * LSValue methods
	 */
	virtual bool isTrue() const;
	bool in_v(int) const;

	int atv(const int key) const;
	virtual LSValue* at(const LSValue* key) const override;
	LSValue* range(int start, int end) const override;
	virtual int abso() const override;

	virtual LSValue* clone() const override;
	virtual std::ostream& dump(std::ostream& os) const override;
	LSValue* getClass() const override;
};

}

#endif
