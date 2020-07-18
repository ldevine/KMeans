#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Eigen/Dense"

#include "Model.hpp"
#include "Clock.hpp"


#include "lmw/Optimizer.h"
#include "lmw/RandomSeeder.h"
#include "lmw/DSquaredSeeder2.h"

#include "lmwm/HVector.h"
#include "lmwm/Distance.hpp"
#include "lmwm/Prototype.h"
#include "lmwm/KMeans.h"
#include "lmw/Cluster.h"

#include "misc/tinyformat.h"

#include "ClusterSolution.hpp"



using lmwm::HVector;
using lmw::Cluster;

typedef HVector<float> vecType;
typedef lmw::RandomSeeder<vecType> RandomSeeder_t;
typedef lmw::DSquaredSeeder2<vecType, lmwm::euclideanDistanceSq<vecType> > DSSeeder_t;
typedef lmw::Optimizer<vecType, lmwm::euclideanDistanceSq<vecType>, lmw::Minimize, lmwm::meanPrototype<vecType> > OPTIMIZER;
typedef lmwm::KMeans<vecType, DSSeeder_t, OPTIMIZER> KMeans_t;



void writeClustersMembers(std::shared_ptr<nrep::Model> m, string path, vector<Cluster<vecType>*>& clusters) {

	cout << endl << "Writing cluster members ..." << endl;

	int idx;
	string s;
	std::ofstream fout;
	fout.open(path);

	for (int i = 0; i < clusters.size(); i++) {

		auto clusVec = clusters[i]->getNearestList();
		for (int j = 0; j < clusVec.size(); j++) {
			idx = clusVec[j]->_intId;
			fout << endl << m->vs->vStore.getTermByIndex(idx)->word;
		}

		fout << endl << "-------------";
	}

	fout.close();
}

void printVector(vector<HVector<float>*> &data) {

	std::cout << endl;
	auto vec = data[0];
	int dim = vec->size();

	std::cout << std::endl << "============" << std::endl;

	for (int i = 0; i < dim; i++) {
		std::cout << vec->at(i) << " ";
	}
	std::cout << endl;

	/*
	for (int i = 0; i < data.size(); i++) {
		vec = data[i];
		std::cout << vec->norm() << " ";
		//if (vec->norm() == 0.0f) {
			//std::cout << "*****" << " ";
		//}		
	}
	*/
	std::cout << std::endl << "============" << std::endl;
}


void run(std::shared_ptr<nrep::Model> m) {

	// Vectors to cluster
	vector<std::shared_ptr< HVector<float>>> data1;
	vector<HVector<float>*> data2;

	// 1.
	//
	// Need to prepare vectors for clustering
	int dim = m->vs->mat.rows();

	//std::cout << "Dimensions: " << dim << std::endl;
	//std::cout << "Vectors: " << m->vs->vStore.size() << std::endl;

	for (int i = 0; i < m->vs->vStore.size(); i++) {
		auto vec = std::make_shared<HVector<float>>( m->vs->mat.col(i).data(), dim);
		vec->_intId = i;
		auto vp = &(*vec);
		data1.push_back(vec);
		data2.push_back(vp);
	}
	//m.data() + n * j;

	//  2
	//
	// Cluster vectors
	KMeans_t clusterer(m->options.numClusters, m->options.numThreads);
	clusterer.setMaxIters(m->options.maxIters);
	clusterer.setEps(0.000f);

	//for (int i = 0; i < 100; i++) {
	//	std::cout << m->vs->mat(0,i) << " ";
	//}
	

	//std::cout << "++++++++++++" << std::endl;
	//std::cout << "Max iters: " << m->options.maxIters << std::endl;
	//std::cout << "Num vectors: " << data2.size() << std::endl;
	//std::cout << "Num clusters: " << m->options.numClusters << std::endl;

	//printVector(data);

	nrep::Clock c;
	vector<Cluster<vecType>*>& clusters = clusterer.cluster(data2);


	//  3
	//
	// Write out clusters
	writeClustersMembers(m, m->options.clustersFile, clusters);
}


int main(int argc, const char** argv) {

	if (argc == 1) {
		return 0;
	}

	// Parsing options
	cout << endl << "Parsing options ... " << endl;
	nrep::Options options;
	options.parseOptions(argc, argv);

	// Model
	auto model = std::make_shared<nrep::Model>(options);
	model->init();

	run(model);

}


