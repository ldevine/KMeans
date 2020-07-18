#ifndef RANDOM_SEEDER_H
#define RANDOM_SEEDER_H

#include <algorithm>
#include <random>

#include "Seeder.h"

namespace lmw {

	template <typename T>
	class RandomSeeder : public Seeder<T> {

	private:


	public:

		RandomSeeder() {

		}

		// Pre: The centroids vector is empty
		void seed(vector<T*> &data, vector<T*> &centroids, size_t numCentres) {

			//cout << endl << data.size();
			//cout << endl << numCentres;

			centroids.clear();

			vector<int> indices;
			//indices.reserve(data.size());
			for (int i = 0; i < data.size(); i++) indices.push_back(i);

			std::mt19937 gen{ std::random_device{}() };

			//cout << endl << "Shuffling ..." << endl;
			
			std::shuffle(indices.begin(), indices.end(), gen);

			//cout << endl << "End of shuffling." << endl;


			T *vec;

			for (int i = 0; i < numCentres && i < data.size(); i++) {
				
				//cout << endl << "Centroid: " << indices[i] << endl;
				//cout << endl << data[indices[i]]->_intId << endl;

				vec = new T(*data[indices[i]]);

				//cout << endl << "Centroid: " << indices[i] << " " << vec->size() << endl;
				//cout << endl << vec->at(0) << "  " << vec->at(1) << endl;
				
				centroids.push_back(vec);				
			}

			//std::cout << "....";
		}

	};

}

#endif
