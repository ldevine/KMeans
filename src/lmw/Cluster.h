#ifndef CLUSTER_H
#define CLUSTER_H


// Cluster is a container for the cluster centroid and
// the nearest vectors of the centroid. It does not
// assume ownership of the contained vectors. 

#include <vector>


namespace lmw {

	using std::vector;

	template <typename T>
	class Cluster {

	private:

		T* _centroid;
		vector<T*> _nearest; // nearest to centroid

	public:

		Cluster(T* centroid) {
			_centroid = centroid;
		}

		vector<T*>& getNearestList() {
			return _nearest;
		}

		void addNearest(T* neighbour) {
			_nearest.push_back(neighbour);
		}

		T* getCentroid() {
			return _centroid;
		}

		void clearNearest() {
			return _nearest.clear();
		}

		size_t size() {
			return _nearest.size();
		}

	};

}

#endif	/* CLUSTER_H */



