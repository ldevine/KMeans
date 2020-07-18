#ifndef NREP_DISTANCE_HPP
#define NREP_DISTANCE_HPP


#include <vector>
#include <cstring>

//#include "Eigen/Dense"

#include "BitVector.hpp"


namespace lmwm {


	struct hammingDistance {
		double operator()(BitVector *v1, BitVector *v2) const {
			return BitVector::hammingDistance(*v1, *v2);
		}

		double squared(BitVector *v1, BitVector *v2) const {
			double distance = operator()(v1, v2);
			return distance * distance;
		}
	};

	template <typename T>
	struct euclideanDistanceSq {
		double operator()(T *t1, T *t2) const {
			typename T::iterator it1 = t1->begin();
			typename T::iterator it2 = t2->begin();
			double d, sum = 0.0f;
			for (it1 = t1->begin(), it2 = t2->begin(); it1 != t1->end(), it2 != t2->end(); it1++, it2++) {
				d = *it1 - *it2;
				sum = sum + (d * d);
			}
			return sum;
		}

		double squared(T *t1, T *t2) const {
			return operator()(t1, t2);
		}
	};

	template <typename T>
	struct euclideanDistance {
		double operator()(T *t1, T *t2) const {
			euclideanDistanceSq<T> squared;
			return sqrt(_squared(t1, t2));
		}

		double squared(T *t1, T *t2) const {
			return _squared(t1, t2);
		}

		euclideanDistanceSq<T> _squared;
	};


}


#endif
