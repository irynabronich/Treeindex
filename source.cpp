#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>
using namespace std;


struct Record {
    int id;
    int key;
    int x;
    int y;
};

const int M = 4; 

struct BPlusNode {
    bool isLeaf;
    vector<int> keys;
    vector<BPlusNode*> children; 
    vector<Record> records;      
    BPlusNode* next;           

    BPlusNode(bool leaf) {
        isLeaf = leaf;
        next = nullptr;
    }
};

class BPlusTree {
private:
    BPlusNode* root;

    void splitChild(BPlusNode* parent, int i, BPlusNode* child) {
        BPlusNode* z = new BPlusNode(child->isLeaf);
        int mid = child->keys.size() / 2;

        if (child->isLeaf) {
            z->keys.assign(child->keys.begin() + mid, child->keys.end());
            z->records.assign(child->records.begin() + mid, child->records.end());
            child->keys.erase(child->keys.begin() + mid, child->keys.end());
            child->records.erase(child->records.begin() + mid, child->records.end());

            z->next = child->next;
            child->next = z;

            parent->keys.insert(parent->keys.begin() + i, z->keys[0]);
        }
        else {
            z->keys.assign(child->keys.begin() + mid + 1, child->keys.end());
            z->children.assign(child->children.begin() + mid + 1, child->children.end());

            int pushUpKey = child->keys[mid];

            child->keys.erase(child->keys.begin() + mid, child->keys.end());
            child->children.erase(child->children.begin() + mid + 1, child->children.end());

            parent->keys.insert(parent->keys.begin() + i, pushUpKey);
        }
        parent->children.insert(parent->children.begin() + i + 1, z);
    }

    void insertNonFull(BPlusNode* node, const Record& record) {
        if (node->isLeaf) {
            auto it = lower_bound(node->keys.begin(), node->keys.end(), record.key);
            int idx = distance(node->keys.begin(), it);
            node->keys.insert(it, record.key);
            node->records.insert(node->records.begin() + idx, record);
        }
        else {
            int i = node->keys.size() - 1;
            while (i >= 0 && record.key < node->keys[i]) {
                i--;
            }
            i++;
            if (node->children[i]->keys.size() == M) {
                splitChild(node, i, node->children[i]);
                if (record.key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], record);
        }
    }

public:
    BPlusTree() { root = new BPlusNode(true); }

    void insert(const Record& record) {
        BPlusNode* r = root;
        if (r->keys.size() == M) {
            BPlusNode* s = new BPlusNode(false);
            root = s;
            s->children.push_back(r);
            splitChild(s, 0, r);
            insertNonFull(s, record);
        }
        else {
            insertNonFull(r, record);
        }
    }

    void printTree() {
        if (!root) return;
        queue<BPlusNode*> q;
        q.push(root);
        int level = 0;
        while (!q.empty()) {
            int size = q.size();
            cout << "  Level " << level << ":\n";
            for (int i = 0; i < size; i++) {
                BPlusNode* curr = q.front();
                q.pop();
                cout << "    [Node Keys: ";
                for (int k : curr->keys) cout << k << " ";
                if (curr->isLeaf) {
                    cout << "| Points: ";
                    for (const auto& r : curr->records) cout << "(" << r.x << "," << r.y << ") ";
                }
                cout << "] ";
                if (!curr->isLeaf) {
                    for (BPlusNode* child : curr->children) {
                        q.push(child);
                    }
                }
            }
            cout << "\n";
            level++;
        }
    }

    vector<Record> rangeSearch(int K) {
        vector<Record> result;
        BPlusNode* curr = root;
        while (!curr->isLeaf) {
            int i = 0;
            while (i < curr->keys.size() && K >= curr->keys[i]) {
                i++;
            }
            curr = curr->children[i];
        }

        while (curr != nullptr) {
            for (const auto& r : curr->records) {
                if (r.key >= K && r.key <= K + 10) {
                    result.push_back(r);
                }
                if (r.key > K + 10) return result;
            }
            curr = curr->next;
        }
        return result;
    }
};

struct KDNode {
    Record record;
    int axis; 
    KDNode* left;
    KDNode* right;

    KDNode(Record r, int a) : record(r), axis(a), left(nullptr), right(nullptr) {}
};

class KDTree {
private:
    KDNode* root;

    KDNode* insertRec(KDNode* node, const Record& r, int depth) {
        if (!node) return new KDNode(r, depth % 2);

        int cd = node->axis;
        if (cd == 0) {
            if (r.x < node->record.x)
                node->left = insertRec(node->left, r, depth + 1);
            else
                node->right = insertRec(node->right, r, depth + 1);
        }
        else {
            if (r.y < node->record.y)
                node->left = insertRec(node->left, r, depth + 1);
            else
                node->right = insertRec(node->right, r, depth + 1);
        }
        return node;
    }

    void rangeSearchRec(KDNode* node, int xMin, int xMax, int yMin, int yMax, vector<Record>& result) {
        if (!node) return;

        if (node->record.x >= xMin && node->record.x <= xMax &&
            node->record.y >= yMin && node->record.y <= yMax) {
            result.push_back(node->record);
        }

        int cd = node->axis;
        if (cd == 0) {
            if (xMin <= node->record.x) rangeSearchRec(node->left, xMin, xMax, yMin, yMax, result);
            if (xMax >= node->record.x) rangeSearchRec(node->right, xMin, xMax, yMin, yMax, result);
        }
        else {
            if (yMin <= node->record.y) rangeSearchRec(node->left, xMin, xMax, yMin, yMax, result);
            if (yMax >= node->record.y) rangeSearchRec(node->right, xMin, xMax, yMin, yMax, result);
        }
    }

    KDNode* searchPointRec(KDNode* node, int qx, int qy) {
        if (!node) return nullptr;
        if (node->record.x == qx && node->record.y == qy) return node;

        int cd = node->axis;
        if (cd == 0) {
            if (qx < node->record.x) return searchPointRec(node->left, qx, qy);
            return searchPointRec(node->right, qx, qy);
        }
        else {
            if (qy < node->record.y) return searchPointRec(node->left, qx, qy);
            return searchPointRec(node->right, qx, qy);
        }
    }

public:
    KDTree() { root = nullptr; }

    void insert(const Record& r) {
        root = insertRec(root, r, 0);
    }

    void printTree() {
        if (!root) return;
        queue<KDNode*> q;
        q.push(root);
        int level = 0;
        while (!q.empty()) {
            int size = q.size();
            cout << "  Level " << level << ":\n";
            for (int i = 0; i < size; i++) {
                KDNode* curr = q.front();
                q.pop();
                cout << "    [ID:" << curr->record.id << " Key:" << curr->record.key
                    << " Pt:(" << curr->record.x << "," << curr->record.y << ") Split:"
                    << (curr->axis == 0 ? "X" : "Y") << "] ";
                if (curr->left) q.push(curr->left);
                if (curr->right) q.push(curr->right);
            }
            cout << "\n";
            level++;
        }
    }

    vector<Record> rangeSearch(int x, int y) {
        vector<Record> result;
        rangeSearchRec(root, x, x + 20, y, y + 20, result);
        return result;
    }

    KDNode* searchPoint(int qx, int qy) {
        return searchPointRec(root, qx, qy);
    }
};

int main() {
    const int numRecords = 25;
    vector<Record> dataset;
    set<pair<int, int>> uniquePoints;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937 gen(seed);
    uniform_int_distribution<> keyDist(1, 100);
    uniform_int_distribution<> coordDist(0, 100);

    for (int i = 1; i <= numRecords; i++) {
        Record r;
        r.id = i;
        r.key = keyDist(gen);

        int rx, ry;
        do {
            rx = coordDist(gen);
            ry = coordDist(gen);
        } while (uniquePoints.count({ rx, ry }));

        uniquePoints.insert({ rx, ry });
        r.x = rx;
        r.y = ry;
        dataset.push_back(r);
    }

    cout << "--- GENERATED DATASET (25 RECORDS) ---\n";
    cout << "ID\tKey\tPoint(X, Y)\n";
    for (const auto& r : dataset) {
        cout << r.id << "\t" << r.key << "\t(" << r.x << ", " << r.y << ")\n";
    }
    cout << "\n";

    BPlusTree bPlusTree;
    KDTree kdTree;

    for (const auto& r : dataset) {
        bPlusTree.insert(r);
        kdTree.insert(r);
    }

    cout << "--- B+ TREE STRUCTURE (m = 4) BY LEVELS ---\n";
    bPlusTree.printTree();
    cout << "\n";

}