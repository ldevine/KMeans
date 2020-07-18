#ifndef HVECTOR_H
#define HVECTOR_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>




namespace lmwm {

	using std::string;
	using std::vector;
	using std::cout;
	using std::endl;

template <class T>
class HVector {

//protected:
public:

	bool isOwner;
	T* _data;
	size_t _length;
	uint32_t _intId;
	string _id;

public:
	
	HVector(size_t length) {
		_length = length;
		_data = new T[_length];
		isOwner = true;
	}

	HVector(T *data, size_t length) {
		_length = length;
		_data = data;
		isOwner = false;
		//std::cout << "$";
	}

	HVector(HVector<T> &other) {
		_length = other._length;
		_data = new T[_length];
		for (size_t i = 0; i < _length; i++) {
			_data[i] = other._data[i];
		}
		isOwner = true;
	}

	~HVector() {
		if (isOwner) delete[] _data;
	}

	typedef T * iterator;
	typedef const T * const_iterator;

	iterator begin() {
		return &_data[0];
	}

	iterator end() {
		return &_data[_length];
	}

	T& operator[](size_t i) {
		return at(i);
	}

	T& at(size_t i) {
		return _data[i];
	}

	void setID(string &id) {
		_id = id;
	}

	string getID() {
		return _id;
	}

	void set(size_t i, T val) {
		_data[i] = val;
	}

	void setAll(T a) {
		// Maybe use memset here instead
		for (size_t i = 0; i < _length; i++) {
			_data[i] = a;
		}
	}


	T* getData() {
		return _data;
	}


	void add(HVector &other) {

		for (size_t i = 0; i < _length; i++) {
			_data[i] = _data[i] + other._data[i];
		}
	}

	void addMult(HVector &other, float coef) {

		for (size_t i = 0; i < _length; i++) {
			_data[i] = _data[i] + other._data[i] * coef;
		}
	}

	float norm() {
		float _norm = 0.0f;
		for (size_t i = 0; i < _length; i++) {
			_norm = _data[i] * _data[i];
		}
		_norm = sqrt(_norm);
		return _norm;
	}

	void normalize() {
		float _norm = norm();
		if (_norm == 0) return;
		scale(1.0f / _norm);
	}

	void scale(T val) {

		for (size_t i = 0; i < _length; i++) {
			_data[i] = _data[i] * val;
		}
	}

	size_t size() {
		return _length;
	}

	void print() {
		for (size_t i = 0; i < _length; i++) {
			std::cout << _data[i] << " ";
		}
	}

};


}

#endif

