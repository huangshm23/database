#include"fptree/fptree.h"
#include<algorithm>
using namespace std;


struct Leaf_Unit{
    Key key;
    Value value;
};
struct Leaf{
    Byte*      bitmap;         // bitmap of the KV slots
    PPointer*  pNext;          // next leafnode
    Byte*      fingerprints;   // the fingerprint of the keys array
    Leaf_Unit* unit;
    Leaf() {
        this->bitmap = new Byte[14];
        for (int i = 0; i < 14; i ++) {
            this->bitmap[i] &= 0x00;
        }
        this->fingerprints = new Byte[112];
        this->unit = new Leaf_Unit[112];
    }
};
struct LeafGroup{
    uint64_t usedNum;
    bool is_used[16];
    Leaf leaf[16];
    LeafGroup () {
        for (int i = 0; i < 16; i ++) {
            this->is_used[i] = 0;
        }
        usedNum = 0;
    }
};

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
    return upper_bound(this->keys, this->keys + this->nKeys, k) - this->keys;
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
        if (this->getChildNum() == 0) { //插入第一個節點
            this->childrens[0] = node;
            this->nChild ++;
        }
        else {
            int index = this->findIndex(k);
            int num = index + 1;
            for (int i = 0; i <= this->nKeys - num; ++ i) {
                this->keys[this->nKeys - i] = this->keys[this->nKeys - i - 1];
                this->childrens[this->nChild - i] = this->childrens[this->nChild - i - 1];
            }
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
    KeyNode* right = NULL;
    KeyNode* next = NULL;

    // 1.insertion to the first leaf(only one leaf)
    if (this->isRoot && this->nKeys == 0) {

        // TODO
        if (this->nChild == 1) {
            LeafNode *le = (LeafNode *) this->childrens[0];
            next = le->insert(k, v);
            if (next != NULL) {
                this->insertNonFull(next->key, next->node);
            }
        }
        else {
            LeafNode *le = new LeafNode(this->tree);
            le->insert(k, v);
            this->insertNonFull(k, le);
        }
        return newChild;
    }
    
    // 2.recursive insertion
    // TODO
    int index = this->findIndex(k);
    if (this->nChild == 0) {            //不用檢查是否分割和下一層
        LeafNode *le = new LeafNode(this->tree);
        le->insert(k, v);
        this->insertNonFull(k, le);
        return newChild;
    }
    if (this->childrens[index]->ifLeaf()) {
        LeafNode *le = (LeafNode *) this->childrens[index];
        next = le->insert(k, v);
        if (next != NULL) {
            int index = this->findIndex(next->key);
            if (this->nChild == 2 * this->degree + 2) {
                right = this->split();
                if (index < this->degree)
                    this->insertNonFull(next->key, next->node);
                else {
                    InnerNode* te = (InnerNode *) right->node;
                    te->insertNonFull(next->key, next->node);
                    newChild = right;
                }
                if (this->isRoot) {
                    InnerNode * newRoot = new InnerNode(this->degree, this->tree, true);
                    this->isRoot = false;
                    newRoot->insertNonFull(this->keys[this->nKeys - 1], this);
                    newRoot->insertNonFull(right->key, right->node);
                    this->tree->changeRoot(newRoot);
                }
            }
            else {
                this->insertNonFull(next->key, next->node);
            }
        }
    }
    else {
        InnerNode *le = (InnerNode *)this->childrens[index];
        next = le->insert(k, v);
        if (next != NULL) {
            int index = this->findIndex(next->key);
            if (this->nChild == 2 * this->degree + 2) {
                right = this->split();
                if (index < this->degree)
                    this->insertNonFull(next->key, next->node);
                else {
                    InnerNode* te = (InnerNode *) right->node;
                    te->insertNonFull(next->key, next->node);
                    newChild = right;
                }
                if (this->isRoot) {
                    InnerNode * newRoot = new InnerNode(this->degree, this->tree, true);
                    this->isRoot = false;
                    newRoot->insertNonFull(this->keys[this->nKeys - 1], this);
                    newRoot->insertNonFull(right->key, right->node);
                    this->tree->changeRoot(newRoot);
                }
            }
            else {
                this->insertNonFull(next->key, next->node);
            }
        }
    }
    return newChild;
}

// ensure that the leaves inserted are ordered
// used by the bulkLoading func
// inserted data: | minKey of leaf | LeafNode* |
KeyNode* InnerNode::insertLeaf(const KeyNode& leaf) {
    KeyNode* newChild = NULL;
    // first and second leaf insertion into the tree
    if (this->isRoot && this->nKeys == 0) {
        // TODO:
        this->insertNonFull(leaf.key, leaf.node);
        return newChild;
    }
    
    // recursive insert
    // Tip: please judge whether this InnerNode is full
    // next level is not leaf, just insertLeaf
    // TODO:
    KeyNode* right = NULL;
    KeyNode* next = NULL;
    int index = this->findIndex(leaf.key);
    if (!this->childrens[0]->ifLeaf()) { //如果爲扉頁，進入下一層
        InnerNode* nextIn = (InnerNode *) this->childrens[index];
        next = nextIn->insertLeaf(leaf);
        if (next != NULL) {
            index = this->findIndex(next->key);
            if (this->nKeys == this->degree * 2 + 1) {
                right = this->split();
                if (index < this->degree)
                    this->insertNonFull(next->key, next->node);
                else {
                    InnerNode* te = (InnerNode *) right->node;
                    te->insertNonFull(next->key, next->node);
                    newChild = right;
                }
                if (this->isRoot) {
                    InnerNode * newRoot = new InnerNode(this->degree, this->tree, true);
                    this->isRoot = false;
                    newRoot->insertNonFull(this->keys[this->nKeys - 1], this);
                    newRoot->insertNonFull(right->key, right->node);
                    this->tree->changeRoot(newRoot);
                }
            }
            else {
                this->insertNonFull(next->key, next->node);
            }
        }
    }
    // next level is leaf, insert to childrens array
    // TODO:
    else {
        index = this->findIndex(leaf.key);
        if (this->nKeys != this->degree * 2 + 1) {
            this->insertNonFull(leaf.key, leaf.node);
        }
        else {
            right = this->split();
            if (index < this->degree)
                this->insertNonFull(leaf.key, leaf.node);
            else {
                InnerNode* te = (InnerNode *) right->node;
                te->insertNonFull(leaf.key, leaf.node);
                newChild = right;
            }
            if (this->isRoot) {
                    InnerNode * newRoot = new InnerNode(this->degree, this->tree, true);
                    this->isRoot = false;
                    newRoot->insertNonFull(this->keys[this->nKeys - 1], this);
                    newRoot->insertNonFull(right->key, right->node);
                    this->tree->changeRoot(newRoot);
                }
        }
    }
    return newChild;
}

KeyNode* InnerNode::split() {
    KeyNode* newChild = new KeyNode();
    // right half entries of old node to the new node, others to the old node. 

    // TODO
    InnerNode *newC = new InnerNode(this->degree, this->tree, false);
    for (int i = 0; i <= this->degree; ++ i) {
        newC->insertNonFull(this->keys[this->degree + i], this->childrens[this->degree + 1 + i]);
    }
    this->nKeys = this->degree;
    this->nChild = this->degree + 1;
    newChild->key = this->keys[this->degree];
    newChild->node = newC;
    return newChild;
}

// remove the target entry
// return TRUE if the children node is deleted after removement.
// the InnerNode need to be redistributed or merged after deleting one of its children node.
bool InnerNode::remove(const Key& k, const int& index, InnerNode* const& parent, bool &ifDelete) {
    bool ifRemove = false;
    bool te = false;
    // only have one leaf
    // TODO:
    int idx = this->findIndex(k);
    if (this->nChild == 1 && this->childrens[0]->ifLeaf()) {
        LeafNode *le = (LeafNode *) this->childrens[0];
        if (le->remove(k, 0, this, te)) {
            ifRemove = true;
        }
        if (te == true) {
            this->removeChild(k, 0);
        }
    }
    // recursive remove
    // TODO:
    else if (this->childrens[idx]->ifLeaf()) {
        LeafNode *le = (LeafNode *) this->childrens[idx];
        if (le->remove(k, idx, this, te)) {
            ifRemove = true;
        }
        if (te == true) {
            this->removeChild(k, idx);
            if (this->nKeys < this->degree) {
                InnerNode *ri = NULL;
                InnerNode *lef = NULL;
                this->getBrother(index, parent, lef, ri);
                if (ri != NULL && ri->getKeyNum() > this->degree) {
                    this->redistributeRight(index, ri, parent);
                    ifDelete = NULL;
                    return ifRemove;
                }
                if (lef != NULL && lef->getKeyNum() > this->degree) {
                    this->redistributeLeft(index, lef, parent);
                    ifDelete = NULL;
                    return ifRemove;
                }
                if (ri != NULL) {
                    if (parent->getIsRoot() && parent->getChildNum() == 2) {
                        this->mergeParentRight(parent, ri);
                        return ifRemove;
                    }
                    else {
                        this->mergeRight(ri, parent->getKey(index + 1));
                        parent->removeChild(index + 1, index + 1);
                        ifDelete = true;
                        return ifRemove;
                    }
                }
                if (lef != NULL) {
                    if (parent->getIsRoot() && parent->getChildNum() == 2) {
                        this->mergeParentLeft(parent, lef);
                        return ifRemove;
                    }
                    else {
                        this->mergeLeft(lef, parent->getKey(index));
                        parent->removeChild(index, index);
                        ifDelete = true;
                        return ifRemove;
                    }
                }
            }
        }
    }
    else {
        InnerNode *Ne = (InnerNode *) this->childrens[idx];
        ifRemove = Ne->remove(k, idx, this, te);
        if (te == true && parent != NULL) {
            if (this->nKeys < this->degree) {
                InnerNode *ri = NULL;
                InnerNode *lef = NULL;
                this->getBrother(index, parent, lef, ri);
                if (ri != NULL && ri->getKeyNum() > this->degree) {
                    this->redistributeRight(index, ri, parent);
                    ifDelete = NULL;
                    return ifRemove;
                }
                if (lef != NULL && lef->getKeyNum() > this->degree) {
                    this->redistributeLeft(index, lef, parent);
                    ifDelete = NULL;
                    return ifRemove;
                }
                if (ri != NULL) {
                    if (parent->getIsRoot() && parent->getChildNum() == 2) {
                        this->mergeParentRight(parent, ri);
                        return ifRemove;
                    }
                    else {
                        this->mergeRight(ri, parent->getKey(index + 1));
                        parent->removeChild(index, index + 1);
                        ifDelete = true;
                        return ifRemove;
                    }
                }
                if (lef != NULL) {
                    if (parent->getIsRoot() && parent->getChildNum() == 2) {
                        this->mergeParentLeft(parent, lef);
                        return ifRemove;
                    }
                    else {
                        this->mergeLeft(lef, parent->getKey(index));
                        parent->removeChild(index, index);
                        ifDelete = true;
                        return ifRemove;
                    }
                }
            }
        }
    }
    return ifRemove;
}

// If the leftBro and rightBro exist, the rightBro is prior to be used
void InnerNode::getBrother(const int& index, InnerNode* const& parent, InnerNode* &leftBro, InnerNode* &rightBro) {

    // TODO
    /*
    if (index > parent->nChild || index < 0)
        exit(1);
    if (index > 1) {
        leftBro = (InnerNode *)parent->getChild(index - 2);
        if (index <= parent->nChild - 1)
            rightBro = (InnerNode *)parent->getChild(index);
    }*/
    
    if (index >= parent->nChild || index < 0)
        exit(1);
    if (index >= 1) {
        leftBro = (InnerNode *)parent->getChild(index - 1);
    }
    if (index <= parent->getChildNum() - 2)
        rightBro = (InnerNode *)parent->getChild(index + 1);
    
}

// merge this node, its parent and left brother(parent is root)
void InnerNode::mergeParentLeft(InnerNode* const& parent, InnerNode* const& leftBro) {
    // TODO:
    Key k = parent->getKey(0);
    parent->removeChild(0, 1);
    parent->removeChild(0, 0);
    parent->insertNonFull(0, leftBro->getChild(0));
    for (int i = 0; i < leftBro->getKeyNum(); ++ i) {
        parent->insertNonFull(leftBro->getKey(i), leftBro->getChild(i + 1));
    }
    parent->insertNonFull(k, this->childrens[0]);
    for (int i = 0; i < this->nKeys; ++ i) {
        parent->insertNonFull(this->keys[i], this->childrens[i + 1]);
    }
}

// merge this node, its parent and right brother(parent is root)
void InnerNode::mergeParentRight(InnerNode* const& parent, InnerNode* const& rightBro) {
    // TODO:
    Key k = parent->getKey(0);
    parent->removeChild(0, 1);
    parent->removeChild(0, 0);
    parent->insertNonFull(0, this->childrens[0]);
    for (int i = 0; i < this->nKeys; ++ i) {
        parent->insertNonFull(this->keys[i], this->childrens[i + 1]);
    }
    parent->insertNonFull(k, rightBro->getChild(0));
    for (int i = 0; i < rightBro->getKeyNum(); ++ i) {
        parent->insertNonFull(rightBro->getKey(i), rightBro->getChild(i + 1));
    }
}

void InnerNode::updateChidren(const Key& k, const int& index, InnerNode* const& newChild) {
    this->keys[index] = k;
    this->childrens[index] = newChild;
}

// this node and its left brother redistribute
// the left has more entries
void InnerNode::redistributeLeft(const int& index, InnerNode* const& leftBro, InnerNode* const& parent) {
    // TODO:
    int num = this->nChild + leftBro->getChildNum();
    int right = num / 2;
    int left = num - right;
    InnerNode* te = new InnerNode (this->degree, this->getTree(), false);
    Key k = leftBro->getKey(left - 1);
    for (int i = 0; i < leftBro->getKeyNum() - left; ++ i) {
        te->insertNonFull(leftBro->getKey(left - 1 + i), leftBro->getChild(i + left));
    }
    for (int i = 0; i < leftBro->getKeyNum() - left; ++ i) {
        leftBro->removeChild(left - 1 + i, left + i);
    }
    te->insertNonFull(parent->getKey(index), this->childrens[0]);
    for (int i = 0; i < this->nKeys; ++ i) {
        te->insertNonFull(this->keys[i], this->childrens[i + 1]);
    }
    parent->updateChidren(k, index, te);
}

// this node and its right brother redistribute
// the right has more entries
void InnerNode::redistributeRight(const int& index, InnerNode* const& rightBro, InnerNode* const& parent) {
    // TODO:
    int num = this->nChild + rightBro->getChildNum();
    int left = num / 2;
    int right = num - left;
    int start = rightBro->getKeyNum() - right;
    Key k = rightBro->getKey(start);
    this->insertNonFull(parent->getKey(index), rightBro->getChild(0));
    for (int i = 0; i < start; ++ i) {
        this->insertNonFull(rightBro->getKey(i), rightBro->getChild(i + 1));
    }
    for (int i = 0; i < start + 1; ++ i) {
        rightBro->removeChild(i, i);
    }
    parent->updateChidren(k, index - 1, this);
}

// merge all entries to its left bro, delete this node after merging.
void InnerNode::mergeLeft(InnerNode* const& leftBro, const Key& k) {
    // TODO:
    leftBro->insertNonFull(k, this->childrens[0]);
    for (int i = 0; i < this->nKeys; ++ i) {
        leftBro->insertNonFull(this->keys[i], this->childrens[i + 1]);
    }
}

// merge all entries to its right bro, delete this node after merging.
void InnerNode::mergeRight(InnerNode* const& rightBro, const Key& k) {
    // TODO:
    this->insertNonFull(k, rightBro->childrens[0]);
    for (int i = 0; i < rightBro->getKeyNum(); ++ i) {
        this->insertNonFull(rightBro->getKey(i), rightBro->getChild(i + 1));
    }
}

// remove a children from the current node, used by remove func
void InnerNode::removeChild(const int& keyIdx, const int& childIdx) {
    // TODO:
    for (int i = keyIdx; i < this->nKeys - 1; ++ i) {
        this->keys[i] =  this->keys[i + 1];
    }
    for (int i = childIdx; i < this->nChild - 1; ++ i) {
        this->childrens[i] = this->childrens[i + 1];
    }
    -- this->nChild;
    -- this->nKeys;
}

// update the target entry, return true if the update succeed.
bool InnerNode::update(const Key& k, const Value& v) {
    // TODO:
    bool flag = false;
    int index = this->findIndex(k);
    if (index > this->nKeys)
        return flag;
    if (this->childrens[index]->ifLeaf()) {
        LeafNode *te = (LeafNode *) this->childrens[index];
        flag =  te->update(k, v);
    }
    else {
        InnerNode *te1 = (InnerNode *) this->childrens[index];
        flag = te1->update(k, v);
    }
    return flag;
}

// find the target value with the search key, return MAX_VALUE if it fails.
Value InnerNode::find(const Key& k) {
    // TODO:
    int index = this->findIndex(k);
    if (this->childrens[index]->ifLeaf()) {
        LeafNode * te1 = (LeafNode *)this->childrens[index];
        return te1->find(k);
    }
    else {
        InnerNode * te = (InnerNode *)this->childrens[index];
        return te->find(k);
    }
    return MAX_VALUE;
}

// get the children node of this InnerNode
Node* InnerNode::getChild(const int& idx) {

    // TODO
    if (idx < this->nChild)
        return this->childrens[idx];
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
    LeafGroup *tmp = (LeafGroup*)pmem_addr;
    this->pPointer = ppointer; 
    this->filePath = DATA_DIR + to_string(ppointer.fileId);//above three are gotten from getLeaf
    this->n = 0;
    this->bitmap = tmp->leaf[15].bitmap;
    this->fingerprints = tmp->leaf[15].fingerprints;
    this->pNext = NULL;
    this->kv = new KeyValue[2*LEAF_DEGREE];
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
    LeafGroup *tmp = (LeafGroup*)pmem_addr;
    this->pPointer = p; 
    this->filePath = DATA_DIR + to_string(p.fileId);//above three are gotten from getLeaf
    this->n = 0;
    uint64_t offset_num = (p.offset - LEAF_GROUP_HEAD) / calLeafSize();
    this->bitmap = tmp->leaf[offset_num].bitmap;
    this->fingerprints = tmp->leaf[offset_num].fingerprints;
    this->pNext = NULL;
    this->kv = new KeyValue[2*LEAF_DEGREE];
    this->prev = this->next = NULL;
    this->tree = t;
    this->isLeaf = true;
    this->degree = 56;
    Leaf *leaf;
    leaf = tmp->leaf;
    for (int i = 0; i < 14; i ++) {
        if ((leaf[offset_num].bitmap[i]) == 0) continue;
        for (int j = 0; j < 8; j ++) {
            if ((leaf[offset_num].bitmap[i]) & (1<<j)) {
                this->kv[this->n].k = leaf[offset_num].unit[i * 8 + j].key;
                this->kv[this->n].v = leaf[offset_num].unit[i * 8 + j].value;
                this->n ++;
            }
        }
        
    }
}

LeafNode::~LeafNode() {
    // TODO:
    this->persist();
    
    delete [] kv;
}

// insert an entry into the leaf, need to split it if it is full
KeyNode* LeafNode::insert(const Key& k, const Value& v) {
    KeyNode* newChild = NULL;
    // TODO:
    insertNonFull(k,v);
    if(this->n >= this->degree * 2){ // sort before split
        newChild = split();
    }
    this->persist();
    return newChild; 
}

// insert into the leaf node that is assumed not full
void LeafNode::insertNonFull(const Key& k, const Value& v) {
    // TODO:
    KeyValue kv;// no need to sort before split
    kv.k = k;
    kv.v = v;
    this->kv[this->n] = kv;
    int tmp = this->n / 8;
    this->bitmap[this->n / 8] |= 1 << (this->n % 8);
    ++this->n;
}


// split the leaf node
KeyNode* LeafNode::split() {
    KeyNode* newChild = new KeyNode();
    // TODO:
    //split
    newChild->key = this->findSplitKey();
    newChild->node = new LeafNode(this->tree);
    LeafNode* newNode = (LeafNode *)newChild->node;
    LeafNode* next = this->next;
    //modify the old leafNode
    for(int i = this->n / 2; i < this->n; ++i)
        this->bitmap[i / 8] &= (1<<((i + 1)%8));
    this->n /= 2;
    this->next = newNode;
    this->persist();
    //modify the new LeafNode
    newNode->n = this->n;
    for(int i = 0; i < newNode->n ; ++i){
        newNode->bitmap[i / 8] |= (1<<(i%8));
        newNode->kv[i]= this->kv[this->n + i];
    }
    newNode->prev = this;
    newNode->next = next;
    newNode->persist();
    newChild->node = newNode;
    return newChild;
}

// use to find a mediant key and delete entries less then middle
// called by the split func to generate new leaf-node
// qsort first then find
Key LeafNode::findSplitKey() {
    Key midKey = 0;
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
    midKey = this->kv[this->n/2].k;
    return midKey;
}

// get the targte bit in bitmap
// TIPS: bit operation
int LeafNode::getBit(const int& idx) {
    // TODO:
    if (idx < this->n)
        return this->bitmap[idx / 8] & (1 << idx % 8);
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
    PAllocator* p_allocator = PAllocator::getAllocator();
    for (int i = 0; i < this->n; ++ i) {
        if (this->kv[i].k == k) {
            for (int j = i; j < this->n - 1; ++ j) {
                this->kv[j] = this->kv[j + 1];
            }
            this->bitmap[(this->n - 1) / 8] &= ~(1 << ((this->n) % 8));
            ifRemove = true;
            -- this->n;
            if (this->n == 0) {
                ifDelete = true;
                p_allocator->freeLeaf(this->pPointer);
            }
            this->persist();
            break;
        }
    }
    return ifRemove;
}

// update the target entry
// return TRUE if the update succeed
bool LeafNode::update(const Key& k, const Value& v) {
    bool ifUpdate = false;
    // TODO:
    for (int i = 0; i < this->n; ++ i) {
        if (this->kv[i].k == k) {
            this->kv[i].v = v;
            ifUpdate = true;
            break;
        }
    }
    if (ifUpdate)
        this->persist();
    return ifUpdate;
}

// if the entry can not be found, return the max Value
Value LeafNode::find(const Key& k) {
    // TODO:
    for (int i = 0; i < this->n; ++ i) {
        if (this->kv[i].k == k)
            return this->kv[i].v;
    }
    return MAX_VALUE;
}

// find the first empty slot
int LeafNode::findFirstZero() {
    // TODO:
    for (int i = 0; i < 14; i ++) {
        for (int j = 0; j < 8; j ++) {
            if (this->bitmap[i]&(1<<j)) return i * 8 + j;
        }
    }
    return -1;
}

// persist the entire leaf
// use PMDK
void LeafNode::persist() {
    // TODO:
    LeafGroup *tmp_l;
    char * pmem_addr = this->pmem_addr;
    tmp_l = (LeafGroup*)pmem_addr;
    Leaf *leaf;
    leaf = tmp_l->leaf;
    PPointer tmp_p = this->pPointer;
    uint64_t offset_num = (tmp_p.offset - LEAF_GROUP_HEAD) / calLeafSize();
    leaf[offset_num].bitmap = this->bitmap;
    for (int i = 0; i < this->n; i ++) {
        leaf[offset_num].unit[i].key = this->kv[i].k;
        leaf[offset_num].unit[i].value = this->kv[i].v;
    }
    //pmem_msync(pmem_addr, sizeof(LeafGroup));
}

// called by the ~FPTree(), delete the whole tree
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
    PAllocator* p_allocator = PAllocator::getAllocator();
    uint64_t maxFileId = p_allocator->getMaxFileId(), index = 1; 
    bool flag = false;
    while(index < maxFileId){
        PPointer ppointer;
        ppointer.fileId = index;
        ppointer.offset = 0;
        char * pmem_addr = p_allocator->getLeafPmemAddr(ppointer);
        LeafGroup *leafgroup;
        leafgroup = (LeafGroup *)pmem_addr;
        uint num = 0;
        for(int n = 15; n >= 0; --n){
            if (num >= leafgroup->usedNum) break;
            if (!leafgroup->is_used[n]) continue;
            num ++;
            ppointer.offset = n * calLeafSize() + LEAF_GROUP_HEAD;
            LeafNode *l_node = new LeafNode(ppointer, this);
            KeyNode *k_node = new KeyNode;
            Key k = l_node->kv[0].k;
            k_node->key = k;
            k_node->node = (Node*)l_node;
            flag = true;
            this->root->insertLeaf(*k_node);
            // for (int i = 0; i < 14; i ++) {
            //     if ((leaf[n].bitmap[i]) == 0) continue;
            //     for (int j = 0; j < 8; j ++) {
            //         if ((leaf[n].bitmap[i]) & (1<<j)) {
            //             Key k = leaf[n].unit[i * 8 + j].key;
            //             Value v = leaf[n].unit[i * 8 + j].value;
            //             //this->insert(k, v);
                        
            //             KeyNode *k_node = new KeyNode;
            //             k_node->key = k;
            //             k_node->node = (Node*)l_node;
            //             this->root->insertLeaf(*k_node);
            //             flag = true; //there is something changed -> reload
            //         }
            //     }
            // }
        }
        ++index;
    }
    return flag;
}
