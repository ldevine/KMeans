#ifndef KMEANS_SER_H
#define KMEANS_SER_H

#include <random>
#include <algorithm>

#include "../lmw/Cluster.h"
#include "../lmw/Clusterer.h"
#include "../lmw/Seeder.h"
#include "../lmw/HUtils.h"


//#include "HParallel.h"


namespace lmwm  {

	using std::vector;

	using lmw::Cluster;
	using lmw::Clusterer;

	using lmw::HUtils;


	template <typename T, typename SEEDER, typename OPTIMIZER>
	class KMeansSer : public Clusterer<T> {
	public:

		KMeansSer(int numClusters) :
			_numClusters(numClusters),
			_seeder(new SEEDER())
		{

		}

		~KMeansSer() {
			// Need to clean up any created cluster objects
			HUtils::purge(_clusters);
		}

		/*
		private DenseVector getPointFromLargestVarianceCluster(ArrayList<Cluster> clusters) {

			double maxVariance = Double.NEGATIVE_INFINITY;
			Cluster selected = null;

			for (Cluster c : clusters) {
				if (!c.getPoints().isEmpty()) {

					// compute the distance variance of the current cluster
					DenseVector center = c.getCentroid();

					Variance stat = new Variance();
					for (DenseVector point : c.getPoints()) {
						stat.increment(point.euclideanDistanceSquared(center));
					}
					final double variance = stat.getResult();

					// select the cluster with the largest variance
					if (variance > maxVariance) {
						maxVariance = variance;
						selected = c;
					}

				}
			}

			// did we find at least one non-empty cluster ?
			if (selected == null) {
				//   ????
			}

			// extract a random point from the cluster
			ArrayList<DenseVector> selectedPoints = selected.getPoints();

			return selectedPoints.remove(random.nextInt(selectedPoints.size()));

		}
		*/


		void setNumClusters(size_t numClusters) {
			_numClusters = numClusters;
		}

		void setMaxIters(int maxIters) {
			_maxIters = maxIters;
		}

		void setEps(float eps) {
			_eps = eps;
		}

		void setEnforceNumClusters(bool enforceNumClusters) {
			_enforceNumClusters = enforceNumClusters;
		}

		int numClusters() {
			return _numClusters;
		}

		vector<float>& getRMSEs() {
			return rmses;
		}

		vector<Cluster<T>*>& cluster(vector<T*> &data) {

			HUtils::purge(_clusters);
			_clusters.clear();
			_finalClusters.clear();
			cluster(data, _numClusters);
			finalizeClusters(data);
			return _finalClusters;
		}

		/**
		 * pre: cluster() has been called
		 */
		double getRMSE() {
			double SSE = 0;
			size_t objects = 0;
			for (Cluster<T>* cluster : _clusters) {
				auto neighbours = cluster->getNearestList();
				objects += neighbours.size();
				SSE += _optimizer.sumSquaredError(cluster->getCentroid(), neighbours);
			}
			return sqrt(SSE / objects);
		}

	private:
		void finalizeClusters(vector<T*> &data) {
			// Create list of final clusters to return;
			bool emptyCluster = assignClusters(data);
			if (emptyCluster && _enforceNumClusters) {
				// randomly shuffle if k cluster were not created to enforce the number of clusters if required
				//std::cout << std::endl << "k-means is splitting randomly";
				vector<T*> shuffled(data);
				std::random_shuffle(shuffled.begin(), shuffled.end());
				{
					size_t step = shuffled.size(), i = 0, clusterIndex = 0;
					for (; i < shuffled.size(); i += step, ++clusterIndex) {
						for (size_t j = i; j < i + step && j < shuffled.size(); ++j) {
							_nearestCentroid[j] = clusterIndex;
						}
					}
				}
				vectorsToNearestCentroid(data);
				recalculateCentroids(data);
				assignClusters(data);
			}
		}

		bool assignClusters(vector<T*> &data) {
			// Create list of final clusters to return;
			bool emptyCluster = false;
			for (Cluster<T>* c : _clusters) {
				if (!c->getNearestList().empty()) {
					_finalClusters.push_back(c);
				}
				else {
					emptyCluster = true;
				}
			}
			return emptyCluster;
		}

		// Randomly allocates data to centroids
		void shuffleData(vector<T*> &data) {

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, _numClusters-1);

			// Clear the nearest vectors in each cluster
			for (Cluster<T> *c : _clusters) {
				c->clearNearest();
			}

			size_t nearest;
			// Accumlate into clusters
			for (size_t i = 0; i < data.size(); i++) {
				_nearestCentroid[i] = dis(gen);
				_clusters[_nearestCentroid[i]]->addNearest(data[i]);
				//cout << endl << _nearestCentroid[i];
			}
		}


		/**
		 * @param vectors       the vectors to form clusters for
		 * @param clusters      the number of clusters to find (i.e. k)
		 */
		void cluster(vector<T*> &data, size_t clusters) {

			//cout << endl << "Clustering ...";

			// Setup initial state.
			_iterCount = 0;
			_numClusters = clusters;
			_nearestCentroid.resize(data.size());
			_seeder->seed(data, _centroids, _numClusters);

			//cout << endl << "Number of centroids ..." << _centroids.size() << endl;

			float rmseOld, rmseCurr;

			// Create as many cluster objects as there are centroids
			for (T* c : _centroids) {
				_clusters.push_back(new Cluster<T>(c));
			}

			// First iteration
			//cout << endl << "First iteration ...";

			//cout << endl << _numClusters;

			vectorsToNearestCentroid(data);
			assignCentroids(data);

			if (_maxIters == 0) {

				while (hasEmptyClusters()) {
					shuffleData(data);
					//vectorsToNearestCentroid(data);
					//assignCentroids(data);
				}				

				return;
			}

			recalculateCentroids(data);

			rmseCurr = getRMSE();
			rmseOld = rmseCurr;
			rmses.push_back(rmseCurr);

			//cout << endl << 1 << "  " << rmseCurr;

			if (_maxIters == 1) {

				return;
			}

			_converged = false;
			_iterCount = 1;
			//_saIterCount = 0;

			// Do standard k-means
			innerLoop(data);

		}

		void innerLoop(vector<T*> &data)  {

			float rmseOld, rmseCurr;

			rmseCurr = getRMSE();
			rmseOld = rmseCurr;

			int innerIterCount = 0;

			_converged = false;

			while (!_converged) {

				vectorsToNearestCentroid(data);
				assignCentroids(data);
				recalculateCentroids(data);

				_iterCount++;
				innerIterCount++;

				rmseCurr = getRMSE();
				rmses.push_back(rmseCurr);
				//cout << endl << _iterCount << "  " << getRMSE();

				if ((rmseOld - rmseCurr) < _eps) break;
				rmseOld = rmseCurr;

				if (innerIterCount >= _maxIters && innerIterCount >= 0) {
					break;
				}
			}
		}

		bool hasEmptyClusters() {

			for (Cluster<T> *c : _clusters) {
				if (c->size() == 0) return true;
			}
			return false;
		}

		void assignCentroids(vector<T*> &data) {

			// Serial
			// Clear the nearest vectors in each cluster
			for (Cluster<T> *c : _clusters) {
				c->clearNearest();
			}

			// Accumlate into clusters
			for (size_t i = 0; i < data.size(); i++) {
				size_t nearest = _nearestCentroid[i];
				_clusters[nearest]->addNearest(data[i]);
				//cout << endl << nearest;
			}
		}


		/**
		 * Assign vectors to nearest centroid.
		 * Pre: seedCentroids() OR recalculateCentroids() has been called
		 * Post: nearestCentroid contains all the indexes into centroids for the
		 *       nearest centroid for the vector (nearestCentroid and vectors are
		 *       aligned).
		 * @return boolean indicating if there were any changes, i.e. was there
		 *                 convergence
		 */
		void vectorsToNearestCentroid(vector<T*> &data) {
			// Clear the nearest vectors in each cluster
			for (Cluster<T> *c : _clusters) {
				c->clearNearest();
			}
			_converged = true;


			//ThreadPool tPool(4);
			/*
			auto func = [=](int i) {
				auto nearest = _optimizer.nearest(data[i], _centroids);
				if (nearest.index != _nearestCentroid[i]) {
					_converged = false;
				}
				_nearestCentroid[i] = nearest.index;
			};*/

			//parallel_for(tPool, 0, data.size(), 10000, func);

			// make sure all memory writes are visible on all CPUs
			//atomic_thread_fence(std::memory_order_release);

			
			for (size_t i = 0; i != data.size(); ++i) {
				//size_t nearest = nearestObj(data[i], _centroids);
				auto nearest = _optimizer.nearest(data[i], _centroids);
				//cout << endl << "-  " << nearest.index;
				if (nearest.index != _nearestCentroid[i]) {
					_converged = false;
				}
				_nearestCentroid[i] = nearest.index;
			}

			//cout << endl << "End of vecs to nearest centroid.";
		}

		/**
		 * Recalculate centroids after vectors have been moved to there nearest
		 * centroid.
		 * Pre: vectorsToNearestCentroid() has been called
		 * Post: centroids has been updated with new vector data
		 */
		void recalculateCentroids(vector<T*> &data) {

			//_numEmptyClusters = 0;

			/*
			auto func = [=](int i) {
				Cluster<T>* c = _clusters[i];
				if (c->size() > 0) {
					_optimizer.updatePrototype(c->getCentroid(), c->getNearestList(), _weights);
				}
				else {
					cout << endl << "EMPTY CLUSTER !!!" << endl;
				}
			};

			parallel_for(tPool, 0, _clusters.size(), 10000, func);

			atomic_thread_fence(std::memory_order_release);
			*/
						
			for (size_t i = 0; i != _clusters.size(); ++i) {
				Cluster<T>* c = _clusters[i];
				if (c->size() > 0) {
					//cout << endl << "\n---\n++ " << c->getNearestList().size();
					_optimizer.updatePrototype(c->getCentroid(), c->getNearestList(), _weights);
				}
				else {
					//_numEmptyClusters++;
					cout << endl << "EMPTY CLUSTER !!!" << endl;
					//exit(0);
				}
			}

		}

		/*
		void print() {

			hammingDistance _distance;
			cout << endl << endl;

			// Iterate over cluster2

			for (int i = 0; i < 5; i++)  {
				Cluster<T> *c = _clusters[i];
				T *cent = c->getCentroid();
				cent->print();

				vector<T*>& members = c->getNearestList();

				for (T *v : members) {
					cout << endl;
					v->print();
					cout << "  " << _distance(cent, v);
				}

				cout << endl << endl;
			}
		}
		*/


		// For simulated annealing
		std::mt19937 rng;
		std::uniform_int_distribution<uint32_t> uint_dist;
		std::bernoulli_distribution bern_dist;

		SEEDER *_seeder;
		OPTIMIZER _optimizer;

		// enforce the number of clusters required
		// if less than k clusters are produced, shuffle vectors randomly and split into k cluster
		bool _enforceNumClusters = false;

		// present number of iterations
		int _iterCount = 0;

		// maximum number of iterations
		// -1 - run until complete convergence
		// 0 - only assign nearest neighbors after seeding
		// >= 1 - perform this many iterations
		int _maxIters = 100;

		// How many clusters should be found? i.e. k
		int _numClusters = 0;

		vector<T*> _centroids;
		vector<Cluster<T>*> _clusters;
		vector<Cluster<T>*> _finalClusters;

		// The centroid index for each vector. Aligned with vectors member variable.
		vector<size_t> _nearestCentroid;

		// Weights for prototype function (we don't have to use these)
		vector<int> _weights;

		// Residual for convergence
		float _eps = 0.00001f;

		// has the clustering converged
		bool _converged;

		// the number of empty clusters
		int  _numEmptyClusters;

		vector<float> rmses;
	};

}

#endif	/* KMEANS_H */



