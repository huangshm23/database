#include"fptree/fptree.h"
#include<algorithm>
using namespace std;

// Initial the new InnerNode
InnerNode::InnerNode(const int& d, FPTree* const& t, bool _isRoot) {
    // TODO
    this->isRoot = _isRoot;
    this->nKeys = 0;
    this->nChild = 0;
    this->degree = d;
    this->isLeaf = false;
    this->tree = t;
    this->keys = new Key[2 * d + 1];
    this->childrens = new Node *[2 * d + 2];
}

// delete the InnerNode
InnerNode::~InnerNode() {
    // TODO
    delete [] this->keys;
    delete [] this->childrens;
}

// binary search the first key in the innernode larger than input key
int InnerNode::findIndex(const Key& k) {
    // TODO
    if (this->nKeys == 0)
        return 1;
    if (keys[this->nKeys - 1] < k)//如果K值大於所有，直接返回nkeys + 1；
        return nKeys + 1;
    int low, high, key;
    low = 0;
    high = this->nKeys - 1;
    while (low < high - 1) {   
        key = (low + high) / 2;
        if (keys[key] == k)
            return key;
        else if(keys[key] < k)
            low = key;
        else
        {
            high = key - 1;
        }        
    }
    if (keys[low] < k)
        low = low + 1;
    return low + 1;
}

// insert the node that is assumed not full
// insert format:
// ======================
// | key | node pointer |
// ======================
// WARNING: can not insert when it has no entry
void InnerNode::insertNonFull(const Key& k, Node* const& node) {
    // TODO
    if (node == NULL)
        exit(1);
    else {
        if (this->getChildNum == 0) { //插入第一個節點
            this->childrens[0] = node;
            this->nChild ++;
        }
        else {
            int index = this->findIndex(k);
            int num = index;
            for (int i = 0; i <= this->nKeys - num; ++ i) {
                this->keys[this->nKeys - i] = this->keys[this->nKeys - i - 1];
                this->childrens[this->nChild - i] = this->childrens[this->nChild - i - 1];
            }
            this->childrens[num - 1] = this->childrens[num];
            this->keys[num - 1] = k;
            this->childrens[num] = node;
            ++ this->nKeys;
            ++ this->nChild;
        }
    }
}

// insert func
// return value is not NULL if split, returning the new child and a key to insert
KeyNode* InnerNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;

    // 1.insertion to the first leaf(only one leaf)
    if (this->isRoot && this->nKeys == 0) {

        // TODO
        LeafNode *le = new LeafNode(this->tree);
        le->insert(k, v);
        this->insertNonFull(k, le);
        return newChild;
    }
    
    // 2.recursive insertion

    // TODO
    int index = this->findIndex(k);
    if (this->nChild == 0) {            //不用檢查是否分割
        LeafNode *le = new LeafNode(this->tree);
        le->insert(k, v);
        this->insertNonFull(k, le);
        return newChild;
    }
    if (this->childrens[index]->ifLeaf()) {
        LeafNode *le = (LeafNode *) this->childrens[index];
        newChild = le->insert(k, v);
        if (newChild != NULL) {
            this->insertNonFull(k, le);
            if (this->nChild == 2 * this->degree + 2) {
                newChild = this->split();
                return newChild;
            }
            return NULL;
        }
    }
    else {
        InnerNode *next = (InnerNode *)this->childrens[index];
        newChild = next->insert(k, v);
        if (newChild != NULL) {
            this->insertNonFull(newChild->key, newChild->node);
            if (this->nChild == 2 * this->degree + 2) {
                newChild = this->split();
                return newChild;
            }
        }
        return newChild;
    }
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) {
    KeyNode* newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0) {
        // TODO:
        return newChild;
    }
    
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO:

    // next level is leaf, insert to childrens array
    // TODO:

    return newChild;
}

KeyNode* InnerNode::split() {
    KeyNode* newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node. 

    // TODO
    InnerNode *newC = new InnerNode(this->degree, this->tree, this->isRoot);
    return newChild;
}

// remove the target entry
// return TRUE if the children node is deleted after removement.
// the InnerNode need to be redistributed or merged after deleting one of its children node.
bool InnerNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // only have one leaf
    // TODO:
    
    // recursive remove
    // TODO:
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int& index, InnerNode* const& parent, InnerNode* &leftBro, InnerNode* &rightBro) {

    // TODO
    if (index > parent->nChild)
        exit(1);
    if (index > 1) {
        leftBro = (InnerNode *)parent->childrens[index - 2];
        if (index <= parent->nChild - 1)
            rightBro = (InnerNode *)parent->childrens[index];
    }

}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode* const& parent, InnerNode* const& leftBro) {
    // TODO:
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode* const& parent, InnerNode* const& rightBro) {
    // TODO:
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int& index, InnerNode* const& leftBro, InnerNode* const& parent) {
    // TODO:
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int& index, InnerNode* const& rightBro, InnerNode* const& parent) {
    // TODO:
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode* const& leftBro, const Key& k) {
    // TODO:
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode* const& rightBro, const Key& k) {
    // TODO:
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int& keyIdx, const int& childIdx) {
    // TODO:
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key& k, const Value& v) {
    // TODO:
    return false;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key& k) {
    // TODO:
    return MAX_VALUE;
}

// get the children node of this InnerNode
Node* InnerNode::getChild(const int& idx) {

    // TODO
    if (idx <= this->nChild)
        return this->childrens[idx - 1];
    else
        return NULL;
}

// get the key of this InnerNode
Key InnerNode::getKey(const int& idx) {
    if (idx < this->nKeys) {
        return this->keys[idx];
    } else {
        return MAX_KEY;
    }
}

// print the InnerNode
void InnerNode::printNode() {
    cout << "||#|";
    for (int i = 0; i < this->nKeys; i++) {
        cout << " " << this->keys[i] << " |#|";
    }
    cout << "|" << "    ";
}

// print the LeafNode
void LeafNode::printNode() {
    cout << "||";
    for (int i = 0; i < 2 * this->degree; i++) {
        if (this->getBit(i)) {
            cout << " " << this->kv[i].k << " : " << this->kv[i].v << " |";
        }
    }
    cout << "|" << " ====>> ";
}

// new a empty leaf and set the valuable of the LeafNode
LeafNode::LeafNode(FPTree* t) {
    // TODO:
    PAllocator* p_allocator = PAllocator::getAllocator();
    PPointer ppointer;
    char *pmem_addr;
    p_allocator->getLeaf(ppointer, pmem_addr);
    this->pmem_addr = pmem_addr;
    this->pPointer = ppointer; 
    this->filePath = DATA_DIR + to_string(ppointer.fileId);//above three are gotten from getLeaf
    this->n = 0;
    this->bitmap = this->fingerprints = NULL;
    this->pNext = NULL;
    this->kv = NULL;
    this->prev = this->next = NULL;
    this->tree = t;
    this->isLeaf = true;
    this->degree = 56;
}

// reload the leaf with the specific Persistent Pointer
// need to call the PAllocator
LeafNode::LeafNode(PPointer p, FPTree* t) {
    // TODO:
    PAllocator* p_allocator = PAllocator::getAllocator();
    char* pmem_addr = p_allocator->getLeafPmemAddr(p);
    this->pmem_addr = pmem_addr;
    this->pPointer = p; 
    this->filePath = DATA_DIR + to_string(p.fileId);//above three are gotten from getLeaf
    this->n = 0;
    this->bitmap = this->fingerprints = NULL;
    this->pNext = NULL;
    this->kv = NULL;
    this->prev = this->next = NULL;
    this->tree = t;
    this->isLeaf = true;
    this->degree = 56;
}

LeafNode::~LeafNode() {
    // TODO:
}

// insert an entry into the leaf, need to split it if it is full
KeyNode* LeafNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;
    // TODO:
    insertNonFull(k,v);
    if(this->n >= this->degree * 2){ // sort before split
        newChild = split();
    }
    return newChild; 
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key& k, const Value& v) {
    // TODO:
    KeyValue kv;// no need to sort before split
    kv.k = k;
    kv.v = v;
    this->kv[this->n -1] = kv;
    this->bitmap[this->n - 1] = true;
    ++this->n;
}


// split the leaf node
KeyNode* LeafNode::split() {
    KeyNode* newChild = new KeyNode();
    // TODO:
    for(int i = 0 ; i < this->n - 1; ++i){//sort
        KeyValue kvi = kv[i];
        for(int j = i ; j < this->n; ++j){ // n=112 bubble sort may be quicker than other method
            if(kv[j].k < kvi.k){
                kv[i] = kv[j];
                kv[j] = kvi;
                kvi = kv[i];
            }
        }
    }
    //split
    KeyValue medium = kv[this->n/2];
    newChild->key = medium.k;
    newChild->node = new LeafNode(this->tree);
    LeafNode* newNode = (LeafNode *)newChild->node;
    LeafNode* next = this->next;
    //modify the old leafNode
    for(int i = this->n / 2; i < this->n; ++i)
        this->bitmap[i] = false;
    this->n /= 2;
    this->next = newNode;
    //modify the new LeafNode
    newNode->n = this->n;
    for(int i = 0; i < newNode->n ; ++i){
        newNode->bitmap[i] = true;
        newNode->kv[i]= this->kv[this->n + i];
    }
    newNode->prev = this;
    newNode->next = next;
    return newChild;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
Key LeafNode::findSplitKey() {
    Key midKey = 0;
    // TODO:
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int& idx) {
    // TODO:
    return 0;
}

Key LeafNode::getKey(const int& idx) {
    return this->kv[idx].k;
}

Value LeafNode::getValue(const int& idx) {
    return this->kv[idx].v;
}

PPointer LeafNode::getPPointer() {
    return this->pPointer;
}

// remove an entry from the leaf
// if it has no entry after removement return TRUE to indicate outer func to delete this leaf.
// need to call PAllocator to set this leaf free and reuse it
bool LeafNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    // TODO:
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key& k, const Value& v) {
    bool ifUpdate = false;
    // TODO:
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    // TODO:
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    // TODO:
    return -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    // TODO:
}

// call by the ~FPTree(), delete the whole tree
void FPTree::recursiveDelete(Node* n) {
    if (n->isLeaf) {
        delete n;
    } else {
        for (int i = 0; i < ((InnerNode*)n)->nChild; i++) {
            recursiveDelete(((InnerNode*)n)->childrens[i]);
        }
        delete n;
    }
}

FPTree::FPTree(uint64_t t_degree) {
    FPTree* temp = this;
    this->root = new InnerNode(t_degree, temp, true);
    this->degree = t_degree;
    bulkLoading();
}

FPTree::~FPTree() {
    recursiveDelete(this->root);
}

// get the root node of the tree
InnerNode* FPTree::getRoot() {
    return this->root;
}

// change the root of the tree
void FPTree::changeRoot(InnerNode* newRoot) {
    this->root = newRoot;
}

void FPTree::insert(Key k, Value v) {
    if (root != NULL) {
        root->insert(k, v);
    }
}

bool FPTree::remove(Key k) {
    if (root != NULL) {
        bool ifDelete = false;
        InnerNode* temp = NULL;
        return root->remove(k, -1, temp, ifDelete);
    }
    return false;
}

bool FPTree::update(Key k, Value v) {
    if (root != NULL) {
        return root->update(k, v);
    }
    return false;
}

Value FPTree::find(Key k) {
    if (root != NULL) {
        return root->find(k);
    }
}

// call the InnerNode and LeafNode print func to print the whole tree
// TIPS: use Queue
void FPTree::printTree() {
    // TODO:
}

// bulkLoading the leaf files and reload the tree
// need to traverse leaves chain
// if no tree is reloaded, return FALSE
// need to call the PALlocator
bool FPTree::bulkLoading() {
    // TODO:
    return false;
}
