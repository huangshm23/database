#include"utility/p_allocator.h"
#include<iostream>
#include <fstream>
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
PAllocator::PAllocator() {
    string allocatorCatalogPath = DATA_DIR + P_ALLOCATOR_CATALOG_NAME;
    string freeListPath         = DATA_DIR + P_ALLOCATOR_FREE_LIST;
    ifstream allocatorCatalog(allocatorCatalogPath, ios::in|ios::binary);
    ifstream freeListFile(freeListPath, ios::in|ios::binary);
    // judge if the catalog exists
    if (allocatorCatalog.is_open() && freeListFile.is_open()) {
        // exist
        // TODO:
        char* pmem_addr;
        size_t maplen;
        int is_pmem;
        pmem_addr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), 1024, PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        this->maxFileId =  *(uint64_t*)pmem_addr;
        this->freeNum = *(uint64_t*)(pmem_addr + 8);
        this->startLeaf = *(PPointer*)(pmem_addr + 16);
        pmem_unmap(pmem_addr, maplen);
        pmem_addr = (char*)pmem_map_file(freeListPath.c_str(), 1024, PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        PPointer tmp;
        for (int i = 0; i < this->freeNum; i ++) {
            tmp = *(PPointer*)(pmem_addr + i * sizeof(PPointer));
            this->freeList.push_back(tmp);
        }

    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO:
        allocatorCatalog.open(allocatorCatalogPath.c_str());
        freeListFile.open(freeListPath.c_str());
        char* pmem_addr;
        size_t maplen;
        int is_pmem;
        pmem_addr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), 1024, PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        this->maxFileId =  *(uint64_t*)pmem_addr = 1;
        this->freeNum = *(uint64_t*)(pmem_addr + 8) = 0;
        this->startLeaf = *(PPointer*)(pmem_addr + 16) = PPointer();
        pmem_unmap(pmem_addr, maplen);
    }
    this->initFilePmemAddr();
}

PAllocator::~PAllocator() {
    // TODO:
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    // TODO
    int account = 1, fd;
    uint64_t fid = 1;
    char * pmem_addr;
    size_t mapped_len;
    int is_pmem;
    while (fid < this->maxFileId) {
        if (pmem_addr = pmem_map_file( DATA_DIR + fid, PMEM_LEN, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem) != NULL) {
            pmem_addr += 24;
            fId2PmAddr[fid] = pmem_addr;
        }        
        ++ fid;
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address))
        return fId2PmAddr[address];
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // TODO
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address)){
        *pmem_addr = fId2PmAddr[address];
        return true;
    }
    return false;
}

bool PAllocator::ifLeafUsed(PPointer p) {
    // TODO
     // open leafgroup file according to fileID not pmem_address
    if(!ifLeafExist(p))
        return false;
    for (int i = 0; i < freeList.size(); ++ i) {
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
    char * pmem_addr;
    if (fId2PmAddr.find(fid)) {
        if (offset < 16)
            return true;
    }
    return false;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    // TODO
    
    return false;
}

bool PAllocator::persistCatalog() {
    // TODO
    return false;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup() {
    // TODO
    return false;
}
