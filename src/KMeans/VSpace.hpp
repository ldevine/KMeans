#ifndef NREP_VSPACE_H
#define NREP_VSPACE_H

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "NearestList.hpp"
#include "MathUtils.hpp"
#include "IOUtils.hpp"
#include "Projection.hpp"
#include "Dictionary.hpp"

#include "lmwm/BitVector.hpp"

#include "ClusterSolution.hpp"


namespace nrep {


	class VSpace;

	using lmwm::BitVector;


	struct Task_NN {

		std::shared_ptr<nrep::VSpace> vs;
		nrep::NearestList nl;

		int nns; // num nearest neighbours
		int endId;
		int startId;

		Eigen::VectorXf query;

		Task_NN(std::shared_ptr<nrep::VSpace> &vs_, int nns_) : vs(vs_), nns(nns_) {
			nl.resize(nns + 2);
		}
	};


	class VSpace {


	public:

		Dictionary vStore;

		Eigen::MatrixXf mat;
		Eigen::MatrixXf matCen;
		Eigen::MatrixXi matNN;

		vector<BitVector*> bitVecs;

		Eigen::VectorXf query;

		bool bitVectorOwner;

        nrep::ClusterSolution csol;


		VSpace() {
			bitVectorOwner = true;
		}

		~VSpace() {
			if (bitVectorOwner) {
				for (auto b : bitVecs) {
					delete b;
				}
			}
		}

		void copy(VSpace & vs) {

			mat = vs.mat;
			vector<string> terms_;
			vs.vStore.getTerms(terms_);

			//cout << endl << "Number of terms in dictionary: " << terms_.size() << endl;
			vStore.removeAll(); // Clear any existing data
			for (auto t : terms_) vStore.addTerm(t);
		}

		int size() {
			return vStore.size();
		}

		int realVectorDimension() {
			return mat.rows();
		}

		int bitVectorDimension() {
			if (bitVecs.size() == 0) return 0;
			return bitVecs[0]->size();
		}

		void setIsBitVectorOwner(bool isOwner) {
			bitVectorOwner = isOwner;
		}


		uint32_t numClusters() {
			return csol.numClusters();
		}

		void loadClusterSolution(string fileName) {

			csol.readClusters(fileName);
			initializeRealCentroids();
		}

		void initializeRealCentroids() {

			if (mat.cols() == 0) {
				cout << endl << "No vectors for initializing centroids." << endl;
				return;
			}

			// Resize centroid matrix
			matCen.resize(mat.rows(), csol.clusters.size());

			int count = 0;
			for (auto c : csol.clusters) {
				matCen.col(count).setZero();
				for (auto i : c->getMembers()) {
					matCen.col(count) += mat.col(i);
				}
				matCen.col(count).normalize();
				count++;
			}
		}

		void setVectors(Eigen::MatrixXf &m, vector<string> &terms_) {

			vStore.removeAll();

			vector<string> terms;

			terms = terms_;
			mat = m;

			nrep::normalizeColWiseSafe(mat);

			int count = 0;
			for (string s : terms) {
				if (vStore.contains(s)) {
					cout << endl << s;
				}
				auto v = vStore.addTerm(s);
				v->id = count;
				count++;
			}
		}

		void normalizeVectors() {
			nrep::normalizeColWiseSafe(mat);
		}

		void loadVectors(string &fileName, int maxTerms=-1) {

			vector<string> terms;

			if (maxTerms == -1) {
				readVectorsBinary(fileName, mat, terms, true, maxTerms);
				//nrep::readVectorsBinary(fileName, mat, terms);
			}
			else {
				readVectorsBinary(fileName, mat, terms, true, maxTerms);
				//readVectorsBinary(fileName, mat, terms, maxTerms);
			}

			cout << endl << "Number of terms: " << terms.size();

			// Don't normalize
			//nrep::normalizeColWiseSafe(mat);

			cout << endl << mat.cols();

			int count = 0;
			for (string s : terms) {
				if (vStore.contains(s)) {
					cout << endl << s;
				}
				auto v = vStore.addTerm(s);
				v->id = count;
				count++;
			}

			cout << endl << "VStore: number of terms: " << vStore.size();

			query.resize(mat.rows());
		}

		void saveVectors(string fileName) {

			vector<string> terms;
			vStore.getTerms(terms);

			// Need to write vectors
			nrep::writeVectorsBinary(fileName, mat, terms);
		}

		void clearVectors() {

			mat.resize(0, 0);
		}


		void loadBitVectors(string fileName) {

			vector <string> terms_;
			cout << endl << "Reading binary vectors ... " << fileName << endl;
			
			//nrep::readBitVectors(fileName, bitVecs, terms_);
			nrep::readBitVectors(fileName, bitVecs, terms_, true);
			
			cout << endl << "Number of bit vectors: " << bitVecs.size() << endl;

			// Need to do more checks - need to reconcile
			if (terms_.size() !=vStore.size()) {
				cout << endl << "Number of real vectors and binary vectors do not match." << endl;
			}
		}

		void saveBitVectors(string fileName) {
			vector<string> terms;
			vStore.getTerms(terms);

			// Save vectors
			nrep::writeBitVectors(fileName, bitVecs, terms);
		}

		void clearBitVectors() {

			// Need to clear bitvecs
			for (size_t i = 0; i < bitVecs.size(); i++) {
				delete bitVecs[i];
			}
			bitVecs.clear();
		}

		void nearestApproxClusters(Eigen::VectorXf &rVec, NearestList &nl, NearestList &nlc, float clusterRatio) {

			if (matCen.cols() == 0) {
				cout << endl << "No clusters." << endl;
				return;
			}

			int id;
			float score;
			size_t maxClusters = clusterRatio * matCen.cols();

			nl.reset();

			getClusters(rVec, nlc);

			// Examine clusters
			auto & scores = nlc.getResults();

			for (size_t i = 0; i < scores.size() && i < maxClusters; i++) {
				id = scores[i]->_id;
				auto c = csol.clusters[id];
				for (int idx : c->getMembers()) {
					score = mat.col(idx).dot(rVec);
					nl.pushScore(idx, score);
				}
			}
		}

		void getClusters(Eigen::VectorXf &rVec, NearestList &nlc) {

			nlc.reset();

			float score;
			// Iterate over all real centroids
			for (int i = 0; i < matCen.cols(); i++) {
				score = matCen.col(i).dot(rVec);
				//cout << endl << score;
				nlc.pushScore(i, score);
			}
		}

		// Retrieves vectors which are closest to vector with id
		void nearestApproxBits(int id, NearestList &nl, NearestList &nlb) {

			query = mat.col(id);
			nearestBits(bitVecs[id], nlb);
			reRank(query, nl, nlb);
		}

		void nearestBits(BitVector* bVec, NearestList &nl) {

			nl.reset();

			float dim = bVec->size();
			float score;

			for (size_t i = 0; i < bitVecs.size(); i++) {
				auto dVec = bitVecs[i];
				score = (dim - bVec->hammingDistance(*dVec)) / dim;
				nl.pushScore(i, score);
			}
		}

		void reRank(Eigen::VectorXf &rVec, NearestList &nlr, NearestList &nlb) {

			nlr.reset();

			int id;
			float score;
			auto & scores = nlb.getResults();

			for (size_t i = 0; i < scores.size(); i++) {
				id = scores[i]->_id;
				score = rVec.dot(mat.col(id));
				nlr.pushScore(id, score);
			}
		}

		void randomProject(nrep::Projection &proj, Eigen::MatrixXf &mat1_, Eigen::MatrixXf &mat2_) {

			uint32_t dim = proj.numCols();

			mat2_.resize(dim, mat1_.cols());
			proj.projectMatrix(mat1_, mat2_);
			nrep::normalizeColWiseSafe(mat2_);
		}

		// Standard 1 bit quantization
		void quantizeVectors(Eigen::MatrixXf &m, vector<BitVector*> &bitVecs) {

			int dim = m.rows();

			cout << endl << " -- " << m.rows();
			cout << endl << " -- " << m.cols();

			BitVector *v;
			Eigen::VectorXf vec(m.rows());

			// Now iterate over vectors
			for (int i = 0; i < m.cols(); i++) {

				v = new BitVector(dim);
				v->setAllBlocks(0);
				bitVecs.push_back(v);

				for (int j = 0; j < dim; j++) {
					if (m(j, i) >= 0.0f) v->set(j);
				}
			}

			cout << endl << " -- " << bitVecs.size();
			cout << endl << " -- " << vStore.size();
		}

		void makeBitVecs(int vecDim) {

			clearBitVectors();

			Eigen::MatrixXf mProj;

			nrep::Projection proj;
			proj.init(mat.rows(), vecDim);

			cout << endl << "Random projecting ... " << endl;

			mProj.resize(vecDim, mat.cols());
			randomProject(proj, mat, mProj);
			quantizeVectors(mProj, bitVecs);
		}

	};

}


#endif





