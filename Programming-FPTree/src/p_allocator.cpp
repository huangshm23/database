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
    Leaf_Unit unit[112];
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
        char* pmem_addr;
        size_t maplen;
        int is_pmem;
        pmem_addr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(catalog), PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        catalog *tmp;
        tmp = (catalog*)pmem_addr;
        this->maxFileId =  tmp->maxFileId;
        this->freeNum = tmp->freeNum;
        this->startLeaf = tmp->treeStartLeaf;

        pmem_addr = (char*)pmem_map_file(freeListPath.c_str(), 1024, PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        freeList_F *fLF;
        fLF = (freeList_F*)pmem_addr;
        for (uint i = 0; i < this->freeNum; i ++) {
            this->freeList.push_back(fLF->freelist[i]);
        }
    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO:
        allocatorCatalog.open(allocatorCatalogPath.c_str());
        freeListFile.open(freeListPath.c_str());
        char* pmem_addr;
        size_t maplen;
        int is_pmem;
        pmem_addr = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(catalog), PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        catalog *tmp;
        tmp = (catalog*)pmem_addr;
        this->maxFileId =  tmp->maxFileId = 1;
        this->freeNum = tmp->freeNum = 0;
        this->startLeaf = tmp->treeStartLeaf = PPointer();
        pmem_addr = (char*)pmem_map_file(freeListPath.c_str(), sizeof(freeList_F), PMEM_FILE_CREATE, 
        0666, &maplen, &is_pmem);
        freeList_F tmp_f;
        tmp_f.freelist.clear();
        pmem_addr = (char*)&tmp_f;
    }
    this->initFilePmemAddr();
}

PAllocator::~PAllocator() {
    // TODO:
    this->persistCatalog();
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    // TODO
    uint64_t fid = 1;
    char * pmem_addr;
    size_t mapped_len;
    int is_pmem;
    while (fid < this->maxFileId) {
        if ((pmem_addr = (char *)pmem_map_file( (DATA_DIR + to_string(fid)).c_str(), 1024, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem)) != NULL) {
            fId2PmAddr[fid] = pmem_addr;
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

    int is_pmem;
    size_t mapped_len;
    if (this->freeNum == 0) {
        if(!this->newLeafGroup())
            return false;
    }
    p = freeList.back();
    cout << freeList.size()<< '\n';

    if ((pmem_addr = (char *)pmem_map_file( (DATA_DIR + to_string(p.fileId)).c_str(), sizeof(LeafGroup), PMEM_FILE_CREATE,
    0666, &mapped_len, &is_pmem)) == NULL)
        return false;
    //pmem_addr = this->fId2PmAddr[p.fileId];
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
    uint64_t fid = p.fileId, offset = p.offset;
    char* pmemaddr;
    if (fId2PmAddr.find(fid)  != fId2PmAddr.end()) {
        if (offset >= 16)
            return false;
        pmemaddr = fId2PmAddr[fid];
        LeafGroup *tmp;
        tmp = (LeafGroup*)pmemaddr;
        tmp->is_used[offset] = 0;
        return true;
    }
    return false;
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
    if ((pmemaddr_free = (char*)pmem_map_file(allocatorCatalogPath.c_str(), sizeof(this->freeList), PMEM_FILE_CREATE,
                0666, &mapped_len_free, &is_pmem_free)) == NULL) 
        return false;
    // string string_allo, string_free;
    // string_allo = string_free = "";
    // string_allo = "" + to_string(this->maxFileId) + " " + to_string(this->freeNum)  + " " 
    // + to_string(this->startLeaf.fileId) + " " + to_string(this->startLeaf.offset);
    catalog cal_tmp;
    cal_tmp.freeNum = this->freeNum;
    cal_tmp.maxFileId = this->maxFileId;
    cal_tmp.treeStartLeaf = this->startLeaf;
    memcpy(pmemaddr_allo, (char*)&cal_tmp, mapped_len_allo);
    // strcpy(pmemaddr_allo, string_allo.c_str());
    // if (is_pmem_allo)
    //     pmem_persist(pmemaddr_allo, mapped_len_allo);
    // else
    //     pmem_msync(pmemaddr_allo, mapped_len_allo);
    pmem_unmap(pmemaddr_allo, mapped_len_allo);
    freeList_F tmp_free;
    for(auto vtmp: this->freeList){
        //string_free += "" + to_string(vtmp.fileId) + " " + to_string(vtmp.offset) + " ";
        tmp_free.freelist.push_back(vtmp);
    }
    // strcpy(pmemaddr_free, string_free.c_str());
    memcpy(pmemaddr_free, (char*)&tmp_free, mapped_len_free);
    // if (is_pmem_free)
    //     pmem_persist(pmemaddr_free, mapped_len_free);
    // else
    //     pmem_msync(pmemaddr_free, mapped_len_free);
    pmem_unmap(pmemaddr_allo, mapped_len_free);
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
