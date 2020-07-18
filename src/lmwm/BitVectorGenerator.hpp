/**

Based on implementation from LMW-tree


*/

#ifndef NREP_BIT_VECTOR_HPP
#define NREP_BIT_VECTOR_HPP


#include <cstdint>
#include <string>
#include <iostream>


template <>
class VectorGenerator<RND_BERN_GEN_01, SVector<bool> > {

public:

	VectorGenerator() {};
	virtual ~VectorGenerator() {};

	// Fills one vector with random values
	static SVector<bool>* genVector(RND_BERN_GEN_01 &rndGen, size_t size) {
		SVector<bool>* v = new SVector<bool>(size);
		fillVector( v, rndGen);
		return v;
	}

	static void genVectors(vector<SVector<bool>*> &vecs, int numVecs, RND_BERN_GEN_01 &rndGen, size_t size) {
		for (int i=0; i<numVecs; i++) {
			vecs.push_back( new SVector<bool>(size) );
		}
		fillVectors(vecs, numVecs, rndGen);
	}

	static void fillVector(SVector<bool> *v, RND_BERN_GEN_01 &rndGen) {
		int value;
		for (int i=0; i<v->size(); i++) {
			value = rndGen();
			if (value) v->set(i);
		}
	}

	static void fillVectors(vector<SVector<bool>*> &vecs, int numVecs, RND_BERN_GEN_01 &rndGen) {
		for (SVector<bool>* v : vecs) {
			fillVector(v, rndGen);
		}
	}

};

}


#endif
