/**
 * \class Model
 *
 * This class is the primary class for holding the algorithm state.
 * Multiple threads will read from this object.

 */

#ifndef NREP_MODEL_H
#define NREP_MODEL_H

#include <random>


#include "VSpace.hpp"
#include "Options.hpp"


namespace nrep {


	class Model {


	public:

		std::shared_ptr<nrep::VSpace> vs;
		
		Options &options;
		
		Model(Options &opts) : options(opts) {

	
		}

		~Model() {

		}


		void init() {
			
			if (options.vectors != "") {
				vs = std::make_shared<VSpace>();
				vs->loadVectors(options.vectors);
				if (options.normalize) vs->normalizeVectors();
			}
		
			cout << endl;
			cout << "Vector dimensionality: " << vs->mat.rows() << endl;

		}


	};

}

#endif


