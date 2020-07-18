/**
 * \class Options
 *
 * This options for AFMFind

 */

#ifndef NREP_OPTIONS_HPP
#define NREP_OPTIONS_HPP


#include <iostream>
#include <string>

using std::string;


namespace nrep {


	struct Options {

		bool normalize;

		int maxIters;
		int numClusters;
		int numThreads;

		string vectors;
		string clustersFile;

		int argPos(const char *str, int argc, const char **argv) {
			int a;
			for (a = 1; a < argc; a++) {
				if (!strcmp(str, argv[a])) {
					if (a == argc - 1) {
						std::cout << "Argument missing for " << str << std::endl;
						exit(1);
					}
					return a;
				}
			}
			return -1;
		}

		void parseOptions(int argc, const char** argv) {

			int i;

			// Inputs
			if ((i = argPos((char *)"-vectors", argc, argv)) > 0) vectors = argv[i + 1];
			if ((i = argPos((char *)"-output", argc, argv)) > 0) clustersFile = argv[i + 1];

			if ((i = argPos((char *)"-clusters", argc, argv)) > 0) numClusters = static_cast<int>(atoi(argv[i + 1]));
			if ((i = argPos((char*)"-max-iters", argc, argv)) > 0) maxIters = static_cast<int>(atoi(argv[i + 1]));
			if ((i = argPos((char*)"-threads", argc, argv)) > 0) numThreads = static_cast<int>(atoi(argv[i + 1]));

			if ((i = argPos((char*)"-normalize", argc, argv)) > 0) normalize = static_cast<int>(atoi(argv[i + 1]));

		}

		Options() {
			
			normalize = true;

			maxIters = 10;
			numClusters = 10;
			numThreads = 4;

			vectors = "";
			clustersFile = "";
		}

		void print() {

		}
	};

} // namespace

#endif




