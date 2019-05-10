#include"utility/p_allocator.h"
#include<iostream>
#include <fstream>
#include <string.h>
using namespace std;

// the file that store the information of allocator
const string P_ALLOCATOR_CATALOG_NAME = "p_allocator_catalog";
// a list storing the free leaves
const string P_ALLOCATOR_FREE_LIST    = "free_list";

PAllocator* PAllocator::pAllocator = new PAllocator();

PAllocator* PAllocator::getAllocator() {
    if (PAllocator::pAllocator == NULL) {
        PAllocator::pAllocator = new PAllocator();
    }
    return PAllocator::pAllocator;
}

/* data storing structure of allocator
   In the catalog file, the data structure is listed below
   | maxFileId(8 bytes) | freeNum = m | treeStartLeaf(the PPointer) |
   In freeList file:
   | freeList{(fId, offset)1,...(fId, offset)m} |
*/
struct catalog{
    uint64_t maxFileId;
    uint64_t freeNum;
    PPointer treeStartLeaf;
};
struct freeList_F{
    vector<PPointer> freelist;
};
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
        this->fingerprints = new Byte[112];
        this->unit = new Leaf_Unit[112];
    }
};
struct LeafGroup{
    uint64_t usedNum;
    bool is_used[16];
    Leaf leaf[16];
};
PAllocator::PAllocator() {
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
    ifstream freeListFile(freeListPath, ios::in|ios::binary);
    // judge if the catalog exists
    if (allocatorCatalog.is_open() && freeListFile.is_open()) {
        // exist
        // TODO:
        char* pmem_addr_allo, *pmem_addr_free;
        size_t maplen_allo, maplen_free;
        int is_pmem_allo, is_pmem_free;
        if((pmem_addr_allo = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(catalog), PMEM_FILE_CREATE, 
        0666, &maplen_allo, &is_pmem_allo)) == NULL) {
            perror("pmem_map_file");
            exit(1);
        }
        catalog *tmp;
        tmp = (catalog*)pmem_addr_allo;
        this->maxFileId =  tmp->maxFileId;
        this->freeNum = tmp->freeNum;
        this->startLeaf = tmp->treeStartLeaf;
        pmem_unmap(pmem_addr_allo, maplen_allo);

        if((pmem_addr_free = (char*)pmem_map_file(freeListPath.c_str(), this->freeNum*sizeof(PPointer), PMEM_FILE_CREATE, 
        0666, &maplen_free, &is_pmem_free)) == NULL) {
            perror("pmem_map_file");
            exit(1);
        }
        PPointer *freelist;
        freelist = (PPointer*)pmem_addr_free;
        for (uint i = 0; i < this->freeNum; i ++) {
            this->freeList.push_back(freelist[i]);
        }
        pmem_unmap(pmem_addr_free, maplen_free);
    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO:
        // allocatorCatalog.open(allocatorCatalogPath.c_str());
        // freeListFile.open(freeListPath.c_str());
        char* pmem_addr_allo, *pmem_addr_free;
        size_t maplen_allo, maplen_free;
        int is_pmem_allo, is_pmem_free;
        if((pmem_addr_allo = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(catalog), PMEM_FILE_CREATE, 
        0666, &maplen_allo, &is_pmem_allo)) == NULL) {
            perror("pmem_map_file");
            exit(1);
        }
        catalog *tmp;
        tmp = (catalog*)pmem_addr_allo;
        this->maxFileId =  tmp->maxFileId = 1;
        this->freeNum = tmp->freeNum = 0;
        this->startLeaf = tmp->treeStartLeaf;
        pmem_unmap(pmem_addr_allo, maplen_allo);

        if((pmem_addr_free = (char*)pmem_map_file(freeListPath.c_str(), sizeof(PPointer), PMEM_FILE_CREATE, 
        0666, &maplen_free, &is_pmem_free)) == NULL) {
            perror("pmem_map_file");
            exit(1);
        }
        PPointer free_list;
        // free_list = (PPointer*)pmem_addr_free;
        // free_list->fileId = 0;
        memcpy(pmem_addr_free, (char*)&free_list, maplen_free);
        pmem_unmap(pmem_addr_free, maplen_free);
    }
    this->initFilePmemAddr();
}

PAllocator::~PAllocator() {
    // TODO:
    this->persistCatalog();
    PAllocator::pAllocator = NULL;
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    // TODO
    uint64_t fid = 1;
    size_t mapped_len;
    int is_pmem;
    while (fid < this->maxFileId) {
        if ((fId2PmAddr[fid] = (char *)pmem_map_file( (DATA_DIR + to_string(fid)).c_str(), sizeof(LeafGroup), 
        PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL) {
            perror("pmem_map_file");
            exit(1);
        }        
        ++ fid;
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address) != fId2PmAddr.end())
        return fId2PmAddr[address];
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // TODO

    if (this->freeNum == 0) {
        if(!this->newLeafGroup())
            return false;
    }
    p = freeList.back();

    if (this->fId2PmAddr.find(p.fileId) == this->fId2PmAddr.end()) return false;
    pmem_addr = this->fId2PmAddr[p.fileId];
    LeafGroup *tmp;
    tmp = (LeafGroup*)pmem_addr; 
    uint64_t off_num = (p.offset - LEAF_GROUP_HEAD) / calLeafSize();
    tmp->usedNum += 1;
    tmp->is_used[off_num] = 1;

    freeList.pop_back();
    this->freeNum = freeList.size();
    return true;
}

bool PAllocator::ifLeafUsed(PPointer p) {
    // TODO
     // open leafgroup file according to fileID not pmem_address
    if(!ifLeafExist(p))
        return false;
    for (uint i = 0; i < freeList.size(); ++ i) {
        if (freeList[i] == p) {
            return false;
        }
    }
    return true;
}

bool PAllocator::ifLeafFree(PPointer p) {
    // TODO
    if(!ifLeafExist(p))
        return false;
    if(ifLeafUsed(p))
        return false;
    return true;
}

// judge whether the leaf with specific PPointer exists. 
bool PAllocator::ifLeafExist(PPointer p) {
    // TODO
    uint64_t fid = p.fileId, offset = p.offset;
    if (fId2PmAddr.find(fid)  != fId2PmAddr.end()) {
        if (offset <= LEAF_GROUP_HEAD + (LEAF_GROUP_AMOUNT - 1) * calLeafSize())
            return true;
    }
    return false;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {

    // TODO
    char* pmem_addr;
    if (p.fileId >= this->maxFileId || 
    p.offset > LEAF_GROUP_HEAD + calLeafSize() * (LEAF_GROUP_AMOUNT - 1)) {
        return false;
    }
     if (this->fId2PmAddr.find(p.fileId) == this->fId2PmAddr.end()) return false;
    pmem_addr = this->fId2PmAddr[p.fileId];

    uint64_t off_num = (p.offset - LEAF_GROUP_HEAD) / calLeafSize(); 
    LeafGroup *tmp;
    tmp = (LeafGroup*)pmem_addr;
    tmp->is_used[off_num] = 0;
    this->freeList.push_back(p);
    this->freeNum ++;
    return true;
}


bool PAllocator::persistCatalog(){ // writeback
    // TODO:
    char *pmemaddr_allo, *pmemaddr_free;
    int is_pmem_allo, is_pmem_free;
    size_t mapped_len_allo, mapped_len_free;
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    if ((pmemaddr_allo = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(catalog), PMEM_FILE_CREATE,
                0666, &mapped_len_allo, &is_pmem_allo)) == NULL) 
        return false;
    if ((pmemaddr_free = (char*)pmem_map_file(freeListPath.c_str(), this->freeNum * sizeof(PPointer), PMEM_FILE_CREATE,
                0666, &mapped_len_free, &is_pmem_free)) == NULL) 
        return false;

    catalog cal_tmp;
    cal_tmp.freeNum = this->freeNum;
    cal_tmp.maxFileId = this->maxFileId;
    cal_tmp.treeStartLeaf = this->startLeaf;
    memcpy(pmemaddr_allo, (char*)&cal_tmp, mapped_len_allo);

    PPointer free_list[this->freeNum];
    for(uint i = 0; i < this->freeList.size(); i ++){
        free_list[i] = this->freeList[i];
    }
    memcpy(pmemaddr_free, (char*)free_list, mapped_len_free);
    return true;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup

bool PAllocator::newLeafGroup() {
    // TODO
    uint64_t fid = this->maxFileId;
    size_t mapped_len;
    int is_pmem;
    char * pmem_addr;
    if ((pmem_addr = (char *)pmem_map_file( (DATA_DIR + to_string(fid)).c_str(), sizeof(LeafGroup), PMEM_FILE_CREATE,
    0666, &mapped_len, &is_pmem)) == NULL)
        return false;
    LeafGroup tmp1;
    tmp1.usedNum = 0;
    memset(tmp1.is_used, 0, sizeof(tmp1.is_used));
    memcpy(pmem_addr, (char*)&tmp1, mapped_len);
    this->fId2PmAddr[fid] = pmem_addr;
    this->maxFileId ++;
    this->freeNum += 16;
    PPointer tmp;
    tmp.fileId = fid;
    for (int i = 0; i < 16; i++) {
        tmp.offset = i*calLeafSize() + LEAF_GROUP_HEAD;
        this->freeList.push_back(tmp);
    }
    return true;
}
