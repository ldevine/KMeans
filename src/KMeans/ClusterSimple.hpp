#ifndef NREP_CLUSTER_SIMPLE_H
#define NREP_CLUSTER_SIMPLE_H

#include <vector>


namespace nrep {

	using std::vector;


	struct ClusterSimple {

		int id;

		vector<int> members;

		vector<int>& getMembers() {
			return members;
		}

		void addMember(int id) {
			members.push_back(id);
		}

		void clearMembers() {
			members.clear();
		}

		uint32_t size() {
			return members.size();
		}
	};


} // namespace


#endif

