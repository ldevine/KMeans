#ifndef KTREE_H
#define KTREE_H

#include <cstdint>
#include <iostream>

#include "Node.h"
#include "Cluster.h"
//#include "NodeVisitor.h"


namespace lmw {

	using std::cout;
	using std::endl;

template <typename T>
struct SplitResult {
    bool isSplit;

    //T *_key1;
    T *_key2;

	// Always treat 1 as the existing node
    Node<T> *_child1;
    Node<T> *_child2;

    SplitResult() {
        isSplit = false;
    }

};

// KTree class
template <typename T, typename CLUSTERER, typename OPTIMIZER>
class KTree {
public:
    KTree(int order, int clustererMaxiters) : _clusterer(2) {
        _m = order;
        _root = new Node<T>(); // initial root is a leaf
        _clusterer.setMaxIters(clustererMaxiters);
        _clusterer.setEnforceNumClusters(true);
        _added = 0;
        _delayedUpdates = false;
        _updateDelay = 1000;
    }
    
    ~KTree() {
        delete _root;
    }

	void setOrder(int order) {
		_m = order;
	}

	// Extra methods
	void getLeafNodes(vector<Node<T>*> &nodes) {

		getLeafNodes(nodes, _root);
	}

	void getLeafNodes(vector<Node<T>*> &nodes, Node<T>* node) {

		if (!node->isLeaf()) {
			vector<Node<T>*> &children = node->getChildren();
			for (Node<T> *child : children) {
				
				if (child->isLeaf()) {
					nodes.push_back(child);					
				}
				else getLeafNodes(nodes, child);
			}
		}
	}

	/*
	void getLeafNodesAsClusters(vector<Cluster<vecType>*>& clusters) {

		vector<Node<T>*> nodes;
		getLeafNodes(nodes);

		/*
		if (!node->isLeaf()) {
			vector<Node<T>*> &children = node->getChildren();
			for (Node<T> *child : children) {

				if (child->isLeaf()) {
					nodes.push_back(child);
				}
				else getLeafNodes(nodes, child);
			}
		}
	}*/

    void setUpdateDelay(int updateDelay) {
        _updateDelay = updateDelay;
    }

    void setDelayedUpdates(bool delayedUpdates) {
        _delayedUpdates = delayedUpdates;
    }

    int getClusterCount() {
        return clusterCount(_root);
    }

    int getClusterCount(int depth) {
        return clusterCount(_root, depth);
    }

    int getEmptyClusterCount() {
        return emptyClusterCount(_root);
    }

    uint64_t getObjCount() {
        return objCount(_root);
    }

    int getLevelCount() {
        return levelCount(_root);
    }

    void printStats() {
        cout << "Number of objects: " << getObjCount() << endl;
        int levelCount = getLevelCount();
        cout << "Level count: " << getLevelCount() << endl;
        for (int level = 1; level < levelCount; ++level) {
                cout << "Cluster count level " << level << ": "
                        << getClusterCount(level) << endl;
        }
        int empty = getEmptyClusterCount();
        // Cluster counts no longer include empty clusters
        //cout << "Empty Cluster count: " << empty << endl;
        //cout << "Non-empty Cluster count: " << count - empty << endl;

        cout << "RMSE: " << getRMSE() << endl;
    }
	
    void add(T *obj) {

		//cout << "Adding...  Levels: " << getLevelCount() << endl;

        SplitResult<T> result = pushDown(_root, obj);

		// Handling the split result for _root is special.
		// _root has no associated key higher up in th tree.
		
		Node<T> *_newRoot;

        if (result.isSplit) {
			_newRoot = new Node<T>();
			T* newKey = new T(*result._key2); // Create new key

            //_root = new Node<T>();

			_newRoot->add(newKey, _root);
			_newRoot->add(result._key2, result._child2);

			updatePrototype(_root, newKey);
			updatePrototype(result._child2, result._key2);

			_newRoot->setOwnsKeys(true);

			_root = _newRoot;

			//if (_root->isLeaf()) cout << "Is leaf." << endl;
			//else {
			//	cout << "Not leaf." << endl;
			//}
        }
        ++_added;
    }

    double getRMSE() {
        return RMSE();
    }

	/*
    void visit(NodeVisitor<Node<T> > &visitor) {
        visit(visitor, _root);
    }

    void visit(NodeVisitor<Node<T> > &visitor, int depth) {
        visit(visitor, _root, depth);
    }*/

	void EMStep() {

		rearrange();
		int pruned = 1;
		while (pruned > 0) {
			pruned = prune();
		}
		rebuildInternal();
	}

private:

    double RMSE() {
        double RMSE = sumSquaredError(NULL, _root);
        uint64_t size = getObjCount();
        RMSE /= size;
        RMSE = sqrt(RMSE);
        return RMSE;
    }

	/*
    void visit(NodeVisitor<Node<T> > &visitor, Node<T> *node) {

        visitor.accept(node);

        if (!node->isLeaf()) {
            vector<Node<T>*> &children = node->getChildren();
            for (Node<T> *child : children) {
                visit(visitor, child);
            }
        }
    }

    void visit(NodeVisitor<Node<T> > &visitor, Node<T> *node, int depth) {

        if (depth == 1) visitor.accept(node);
        else if (!node->isLeaf()) {
            vector<Node<T>*> &children = node->getChildren();
            for (Node<T> *child : children) {
                visit(visitor, child, depth - 1);
            }
        }
    }
	*/

    double sumSquaredError(T* parentKey, Node<T> *child) {
        double distance = 0.0;
        if (child->isLeaf()) {
            distance += _optimizer.sumSquaredError(parentKey, child->getKeys());
        } else {
            vector<T*> &keys = child->getKeys();
            vector<Node<T>*> &children = child->getChildren();
            for (int i = 0; i < child->size(); i++) {
                distance += sumSquaredError(keys[i], children[i]);
            }
        }
        return distance;
    }

    uint64_t objCount(Node<T>* current) {
        if (current->isLeaf()) {

			//cout << "LEAF  " << current->size() << endl;
            return current->size();
        } else {
            uint64_t localCount = 0;
            vector<Node<T>*>& children = current->getChildren();
			//cout << endl << "Number of children ... " << children.size() << endl;
            for (Node<T> *child : children) {

				//cout << child->size() << "  ";

                localCount += objCount(child);

            }
            return localCount;
        }
    }

    int clusterCount(Node<T>* current) {
        if (current->isLeaf()) {
            if (current->isEmpty()) {
                return 0;
            } else {
                return 1;
            }
        } else {
            int localCount = 0;
            vector<Node<T>*>& children = current->getChildren();
            for (Node<T> *child : children) {
                localCount += clusterCount(child);
            }
            return localCount;
        }
    }

    int clusterCount(Node<T>* current, int depth) {
        if (depth == 1) {
            int localCount = 0;
            for (Node<T>* child : current->getChildren()) {
                if (!child->isEmpty()) {
                    ++localCount;
                }
            }
            return localCount;
        } else {
            int localCount = 0;
            vector<Node<T>*>& children = current->getChildren();
            for (Node<T> *child : children) {
                localCount += clusterCount(child, depth - 1);
            }
            return localCount;
        }
    }


    int emptyClusterCount(Node<T>* current) {
        if (current->isLeaf()) {
            if (current->size() == 0) return 1;
            else return 0;
        } else {
            int localCount = 0;
            vector<Node<T>*>& children = current->getChildren();
            for (Node<T> *child : children) {
                localCount += emptyClusterCount(child);
            }
            return localCount;
        }
    }

    int levelCount(Node<T>* current) {
        if (current->isLeaf()) {
            return 1;
        } else {
            return levelCount(current->getChild(0)) + 1;
        }
    }

    int prune(Node<T>* n) {
        if (n->isLeaf()) {
            return 0; // non-empty leaf node
        } else {
            int pruned = 0;
            vector<Node<T>*> &children = n->getChildren();
            for (int i = 0; i < children.size(); i++) {
                if (children[i]->isEmpty()) {
                    n->remove(i);
                    pruned++;
                } else {
                    pruned += prune(children[i]);
                }
            }
            n->finalizeRemovals();
            return pruned;
        }
    }

    SplitResult<T> pushDown(Node<T> *n, T *vec) {
        //std::cout << "\n\tPushing down ...";
        SplitResult<T> result;
        if (n->isLeaf()) {
            if (n->size() >= _m) {
                // split this node and pass new node to parent to insert
                result = splitLeafNode(n, vec);
            } else {
                n->add(vec); // Finished
            }
        } else { // It is an internal node.
            // recurse via nearest neighbour cluster
            vector<T*>& keys = n->getKeys();
            auto nearest = _optimizer.nearest(vec, keys);
            
			auto child = n->getChild(nearest.index);
			auto childKey = n->getKey(nearest.index);

			result = pushDown(child, vec);

            if (result.isSplit) {
                updatePrototype(child, childKey);
                updatePrototype(result._child2, result._key2);

                // add new node
                if (n->size() >= _m) {
                    // split this node and pass new node to parent to insert
                    result = splitInternalNode(n, result._child2, result._key2);
                } else {
                    // insert new entry
                    n->add(result._key2, result._child2);
                    result.isSplit = false;
                }
            } else {
                //if (!_delayedUpdates || (_delayedUpdates && _added % _updateDelay == 0)) {
				updatePrototype(child, childKey);
                //}
            }
        }
        return result;
    }


    // Perform a binary split of the child node

    SplitResult<T> splitInternalNode(Node<T>* parent, Node<T>* child, T* obj) {

        //cout << "\nSplitting internal node ...";

        SplitResult<T> result;

        // Create a 2nd node
        Node<T>* node2 = new Node<T>();
        node2->setOwnsKeys(true);

        // Copy child nodes into a temp storage vector

        tempKeys = parent->getKeys();
        tempKeys.push_back(obj);

		// Keep track of relationship to corresponding nodes
		int kCount = 0;
		for (T* t : tempKeys) {
			t->_intId = kCount;
			kCount++;
		}
		
        tempChildren = parent->getChildren();
        tempChildren.push_back(child);

        // DetachRemove children from child node
        parent->clearKeysAndChildren();

        // At this point we have 2 clear nodes: "parent" node (node 1) and "node2"

        // Cluster keys into 2 groups
        _clusterer.setNumClusters(2);
        //_clusterer.cluster(tempKeys);
        vector<Cluster<T>*>& clusters = _clusterer.cluster(tempKeys);
        //std::cout << "clusters found = " << clusters.size() << std::flush;

        // Get nearest centroids after clustering
        for (auto key : clusters[0]->getNearestList()) {
            parent->add(key, tempChildren[key->_intId]);
        }
        for (auto key : clusters[1]->getNearestList()) {
			node2->add(key, tempChildren[key->_intId]);
        }        

        // Now make our split result
        result.isSplit = true;
        //result._child1 = parent;
        result._child2 = node2;

		// Don't create new key for this one.
        //result._key1 = new T(*(clusters[0]->getCentroid()));
		
		// Create new key
		result._key2 = new T(*(clusters[1]->getCentroid()));

        return result;
    }


    // Perform a binary split of the child node

    SplitResult<T> splitLeafNode(Node<T>* child, T* obj) {

        //cout << "\nSplitting leaf node ...";

        SplitResult<T> result;

        // Create a 2nd node
        Node<T>* node2 = new Node<T>();

        // Copy child nodes into a temp storage vector

        tempKeys = child->getKeys();
        tempKeys.push_back(obj);

        // DetachRemove children from child node
        child->clearKeysAndChildren();

        // At this point we have 2 clear nodes: "child" node (node 1) and "node2"

        // Cluster keys into 2 groups
        _clusterer.setNumClusters(2);
        //_clusterer.cluster(tempKeys);
        vector<Cluster<T>*>& clusters = _clusterer.cluster(tempKeys);
        //std::cout << "clusters found = " << clusters.size() << std::flush;

        // Get nearest centroids after clustering
        for (auto key : clusters[0]->getNearestList()) {
            child->add(key);
        }
        for (auto key : clusters[1]->getNearestList()) {
            node2->add(key);
        }

        // Now make our split result
        result.isSplit = true;
        //result._child1 = child;
        result._child2 = node2;

		// Don't create new key for this one.
		//result._key1 = new T(*(clusters[0]->getCentroid()));

		// Create new key
		result._key2 = new T(*(clusters[1]->getCentroid()));

        //result._key1 = clusters[0]->getCentroid();
        //result._key2 = clusters[1]->getCentroid();

        return result;
    }

    // Update the protype parentKey

    void updatePrototype(Node<T> *child, T* parentKey) {

        //cout << "\nUpdating mean ...";

        //int[] weights = new int[count];
        weights.clear();

        if (!child->isLeaf()) {
            vector<Node<T>*>& children = child->getChildren();

            for (size_t i = 0; i < children.size(); i++) {
                weights.push_back(objCount(children[i]));
            }
        }

        _optimizer.updatePrototype(parentKey, child->getKeys(), weights);
    }

	void rearrange() {

		removeData(_root, removed);

		for (int i = 0; i < removed.size(); i++) {
			pushDownNoUpdate(_root, removed[i]);
		}

		removed.clear();
	}

	int prune() {
		return prune(_root);
	}

	void rebuildInternal() {
		// rebuild starting with above leaf level (bottom up)
		for (int depth = getLevelCount() - 1; depth >= 1; --depth) {
			rebuildInternal(_root, depth);
		}
	}

	void pushDownNoUpdate(Node<T> *n, T *vec) {
		//std::cout << "\n\tPushing down (no update) ...";
		if (n->isLeaf()) {
			n->add(vec); // Finished
		}
		else { // It is an internal node.
			// recurse via nearest neighbour cluster
			auto nearest = _optimizer.nearest(vec, n->getKeys());
			pushDownNoUpdate(n->getChild(nearest.index), vec);
		}
	}

	void rebuildInternal(Node<T> *n, int depth) {

		if (n->isLeaf()) return;

		vector<Node<T>*> &children = n->getChildren();

		if (depth == 1) {
			vector<T*> &keys = n->getKeys();
			for (int i = 0; i < children.size(); ++i) {
				Node<T>* child = children[i];
				T* key = keys[i];
				updatePrototype(child, key);
			}
		}
		else {
			for (int i = 0; i < children.size(); ++i) {
				rebuildInternal(children[i], depth - 1);
			}
		}
	}

	void removeData(Node<T> *n, vector<T*> &data) {

		if (n->isLeaf()) {
			n->removeData(data);
		}
		else {
			vector<Node<T>*>& children = n->getChildren();

			for (int i = 0; i < children.size(); i++) {
				removeData(children[i], data);
			}
		}
	}



    // The order of this tree
    int _m;

    // The root of the tree.
    Node<T> *_root;

    CLUSTERER _clusterer;

    OPTIMIZER _optimizer;

    // Use these containers so as not to create new ones
    // every time we split
    vector<T*> tempKeys;
    vector<Node<T>*> tempChildren;
    vector<size_t> tempNearCentroids;

    vector<T*> removed;

    // Weights for prototype function (we don't have to use these)
    vector<int> weights;

    // How many vectors have been inserted into the tree.
    size_t _added;

    // Use delayed updates?
    bool _delayedUpdates;

    // Update along insertion path every _updateDelay insertions.
    int _updateDelay;
};

} // namespace lmw

#endif






