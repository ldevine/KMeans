#ifndef SEEDER_H
#define SEEDER_H


namespace lmw {

	template <typename T>
	class Seeder {


	public:

		Seeder() { }

		virtual ~Seeder() { }

		// Pre: The centroids vector is empty
		virtual void seed(vector<T*> &data, vector<T*> &centroids, size_t numCentres) = 0;
	};


}


#endif



