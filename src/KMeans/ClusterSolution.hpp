#ifndef NREP_CLUSTER_SOLUTION_H
#define NREP_CLUSTER_SOLUTION_H

#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "ClusterSimple.hpp"
#include "StringUtils.hpp"


namespace nrep {


	using std::string;
	using std::vector;
	using std::ifstream;
	using std::ofstream;
	using std::cout;
	using std::endl;
	using std::unordered_map;

	class ClusterSolution {

	public:

		vector<string> terms;
		vector<std::shared_ptr<ClusterSimple>> clusters;

		void reduce(int maxSize) {

			vector<std::shared_ptr<ClusterSimple>> clustersTemp;

			for (auto cl : clusters) {
				if (cl->size() <= static_cast<size_t>(maxSize)) clustersTemp.push_back(cl);
			}

			clusters.clear();
			clusters = clustersTemp;
		}

		uint32_t numClusters() {
			return clusters.size();
		}

		void mergeSolution(ClusterSolution &csol) {

			// First need to set up term translation table
			std::unordered_map<string,int> tMap;
			int count = 0;
			for (auto s : terms) {
				tMap[s] = count;
				count++;
			}

			int idx;
			vector<int> idxs;
			for (auto s : csol.terms) {
				if (tMap.count(s) == 0) {
					idx = terms.size();
					tMap.insert({ s,idx });
					idxs.push_back(idx);
					terms.push_back(s);
				}
				else idxs.push_back(tMap[s]);
			}

			for (auto cl : csol.clusters) {
				auto cluster = std::make_shared<nrep::ClusterSimple>();
				for (auto member : cl->getMembers()) {
					idx = idxs[member]; // Convert to global id
					cluster->addMember(idx);
				}
				clusters.push_back(cluster);
			}
		}

		void addSimpleCluster(std::shared_ptr<ClusterSimple> cluster_) {
			clusters.push_back(cluster_);
		}

		void addClusters(vector<std::shared_ptr<ClusterSimple>> &clusters_) {

			for (size_t i = 0; i < clusters_.size(); i++) {
				clusters.push_back(clusters_[i]);
			}
		}

		void assignClusters(vector<std::shared_ptr<ClusterSimple>> &clusters_) {
			clusters.clear();
			clusters = clusters_;
		}

		void assignTerms(vector<string> &terms_) {
			terms.clear();
			terms = terms_;
		}


		// Write in Clusters
		void writeClusters(string fileName) {

			cout << endl << "Writing clusters ... ***" << endl;
			cout << endl << fileName << endl;

			cout << endl << terms.size() << endl;
			cout << endl << clusters.size() << endl;


			int idx;
			string s;

			std::ofstream fout;
			fout.open(fileName);

			for (size_t i = 0; i < terms.size(); i++) {
				fout << terms[i] << endl;
			}

			fout << endl;

			for (size_t i = 0; i < clusters.size(); i++) {

				auto members = clusters[i]->getMembers();
				if (members.size() < 1) continue;

				fout << members[0];
				for (size_t j = 1; j < members.size(); j++) {
					idx = members[j];
					fout << " " << idx;
				}
				fout << endl;
			}

			fout.close();
		}


		// Read in Clusters
		void readClusters(string fileName) {

			int elementCount;
			size_t idx;
			string str;
			int val;
			//bool first;
			const char *s;
			char c = ' ';
			const char *begin;

			std::ifstream fin(fileName);

			cout << endl << "Reading cluster ..." << endl;

			// First read terms
			string term;

			while (std::getline(fin, str)) {
				term = nrep::trim(str);
				if (term.size() == 0) break;

				terms.push_back(term);
			}

			// Now read cluster members
			while (std::getline(fin, str)) {

				auto cluster = std::make_shared<ClusterSimple>();

				elementCount = 0;
				s = str.c_str();
				idx = 0;
				//first = true;

				while (idx < str.size()) {
					begin = s;
					while (*s != c && idx < str.size()) {
						s++; idx++;
					}
					val = atoi(begin);
					cluster->addMember(val);
					elementCount++;
					s++;
					idx++;
				}
				cluster->id = clusters.size();
				clusters.push_back(cluster);
			}

			cout << "Num terms: " << terms.size() << endl;
			cout << "Num clusters: " << clusters.size() << endl;

			fin.close();
		}

	};


} // namespace


#endif
