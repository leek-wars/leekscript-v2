#ifndef LS_MAP_TCC
#define LS_MAP_TCC

#include "LSNumber.hpp"
#include "LSArray.hpp"

#include <exception>

namespace ls {

template <typename K>
inline bool lsmap_less<K>::operator()(K lhs, K rhs) const {
	return lhs < rhs;
}

template <>
inline bool lsmap_less<LSValue*>::operator()(LSValue* lhs, LSValue* rhs) const {
	return *lhs < *rhs;
}

template <class K, class T>
LSValue* LSMap<K, T>::clazz;

template <class K, class T>
LSMap<K, T>::LSMap() : LSValue(MAP) {}

template <class K, class V>
LSMap<K, V>::~LSMap() {
	for (auto it = this->begin(); it != this->end(); ++it) {
		ls::unref(it->first);
		ls::unref(it->second);
	}
}

/*
 * Map methods
 */
template <class K, class V>
bool LSMap<K, V>::ls_insert(K key, V value) {
	auto it = this->lower_bound(key);
	if (it == this->end() || !ls::equals(it->first, key)) {
		this->emplace_hint(it, ls::move_inc(key), ls::move_inc(value));
		if (refs == 0) delete this;
		return true;
	}
	ls::release(key);
	ls::release(value);
	if (refs == 0) delete this;
	return false;
}

template <class K, class V>
LSMap<K, V>* LSMap<K, V>::ls_clear() {
	for (auto it = this->begin(); it != this->end(); ++it) {
		ls::unref(it->first);
		ls::unref(it->second);
	}
	this->clear();
	return this;
}

template <class K, class V>
bool LSMap<K, V>::ls_erase(K key) {
	auto it = this->find(key);
	ls::release(key);
	if (it != this->end()) {
		ls::unref(it->first);
		ls::unref(it->second);
		this->erase(it);
		LSValue::delete_temporary(this);
		return true;
	}
	LSValue::delete_temporary(this);
	return false;
}

template <class K, class V>
V LSMap<K, V>::ls_look(K key, V def) {
	auto it = this->find(key);
	ls::release(key);
	if (it != this->end()) {
		ls::release(def);
		if (refs == 0) {
			V r = ls::clone(it->second);
			LSValue::delete_temporary(this);
			return r;
		}
		return it->second;
	}
	LSValue::delete_temporary(this);
	return def;
}

template <typename K, typename V>
LSArray<V>* LSMap<K, V>::values() const {
	LSArray<V>* array = new LSArray<V>();
	for (auto i : *this) {
		array->push_clone(i.second);
	}
	LSValue::delete_temporary(this);
	return array;
}

/*
 * LSValue methods
 */
template <class K, class T>
bool LSMap<K, T>::to_bool() const {
	return not this->empty();
}

template <class K, class T>
LSValue* LSMap<K, T>::ls_not() {
	bool r = this->size() == 0;
	if (refs == 0) delete this;
	return LSBoolean::get(r);
}

template <class K, class T>
bool LSMap<K, T>::in(K key) const {
	bool r = this->find(key) != this->end();
	LSValue::delete_temporary(this);
	ls::release(key);
	return r;
}

template <class K, class T>
template <class K2, class T2>
bool LSMap<K, T>::map_equals(const LSMap<K2, T2>* map) const {
	if (this->size() != map->size()) {
		return false;
	}
	auto i = this->begin();
	auto j = map->begin();
	while (i != this->end()) {
		if (j == map->end()) return false; // TODO to remove
		if (!ls::equals(i->first, j->first)) return false;
		if (!ls::equals(i->second, j->second)) return false;
		++j;
		++i;
	}
	return j == map->end();
}

template <class K, class T>
bool LSMap<K, T>::eq(const LSValue* v) const {
	if (auto map = dynamic_cast<const LSMap<LSValue*, LSValue*>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<LSValue*, double>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<LSValue*, int>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<int, LSValue*>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<double, LSValue*>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<int, double>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<int, int>*>(v))
		return map_equals(map);
	if (auto map = dynamic_cast<const LSMap<double, double>*>(v))
		return map_equals(map);
	return false;
}

template <class K, class T>
template <class K2, class T2>
bool LSMap<K, T>::map_lt(const LSMap<K2, T2>* map) const {
	auto i = this->begin();
	auto j = map->begin();
	for (; i != this->end(); ++i, ++j) {
		if (j == map->end()) return false;
		if (ls::lt(i->first, j->first)) return true;
		if (ls::lt(j->first, i->first)) return false;
		if (ls::lt(i->second, j->second)) return true;
		if (ls::lt(j->second, i->second)) return false;
	}
	return j != map->end();
}

template <class K, class T>
bool LSMap<K, T>::lt(const LSValue* v) const {
	if (auto map = dynamic_cast<const LSMap<LSValue*, LSValue*>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<LSValue*, double>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<LSValue*, int>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<int, LSValue*>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<double, LSValue*>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<int, double>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<int, int>*>(v))
		return map_lt(map);
	if (auto map = dynamic_cast<const LSMap<double, double>*>(v))
		return map_lt(map);
	return LSValue::lt(v);
}

template <class K, class V>
V LSMap<K, V>::at(const K key) const {
	try {
		auto map = (std::map<K, V, lsmap_less<K>>*) this;
		return map->at(key);
	} catch (std::exception&) {
		return V();
	}
}

template <typename K, typename T>
inline LSValue** LSMap<K, T>::atL(const LSValue*) {
	return nullptr;
}

template <>
inline LSValue** LSMap<LSValue*, LSValue*>::atL(const LSValue* key) {
	try {
		return (LSValue**) &((std::map<LSValue*, LSValue*, lsmap_less<LSValue*>>*) this)->at((LSValue*) key);
	} catch (std::exception&) {
		return nullptr;
	}
}

template <>
inline LSValue** LSMap<int, LSValue*>::atL(const LSValue* key) {

	if (const LSNumber* n = dynamic_cast<const LSNumber*>(key)) {
		try {
			return (LSValue**) &((std::map<int,LSValue*>*) this)->at((int) n->value);
		} catch (std::exception&) {
			return nullptr;
		}
	}
	return nullptr;
}

template <typename K, typename T>
inline int* LSMap<K, T>::atLv(const LSValue*) const {
	return nullptr;
}
template <>
inline int* LSMap<LSValue*, int>::atLv(const LSValue* key) const {
	try {
		auto map = (std::map<LSValue*, int, lsmap_less<LSValue*>>*) this;
		return &map->at((LSValue*) key);
	} catch (std::exception&) {
		return nullptr;
	}
}

template <class K, class V>
std::ostream& LSMap<K, V>::dump(std::ostream& os) const {
	os << "[";
	for (auto it = this->begin(); it != this->end(); ++it) {
		if (it != this->begin()) os << ", ";
		os << it->first << ": " << it->second;
	}
	if (this->empty()) os << ':';
	return os << "]";
}

template <class K, class V>
std::string LSMap<K, V>::json() const {
	std::string res = "{";
	for (auto it = this->begin(); it != this->end(); ++it) {
		if (it != this->begin()) res += ",";
		res += "\"" + ls::print(it->first) + "\"";
		res += ": ";
		res += ls::to_json(it->second);
	}
	return res + "}";
}

template <class K, class V>
LSValue* LSMap<K, V>::clone() const {
	LSMap<K, V>* map = new LSMap<K, V>();
	for (auto it = this->begin(); it != this->end(); ++it) {
		map->emplace(ls::clone_inc(it->first), ls::clone_inc(it->second));
	}
	return map;
}

template <typename K, typename T>
inline LSValue* LSMap<K, T>::getClass() const {
	return LSMap<K, T>::clazz;
}

}

#endif
