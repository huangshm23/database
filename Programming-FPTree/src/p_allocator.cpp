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
            pmem_addr += 24;
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
    if (freeList.size() == 0) {
        if (!newLeafGroup()) {
            return false;
        }
    }
    p = freeList[0];
    pmem_addr = getLeafPmemAddr(p);
    vector<PPointer>::iterator it = freeList.begin();
    freeList.erase(it);
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
        if (offset < 16)
            return true;
    }
    return false;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    // TODO:
    if(!ifLeafExist(p))
        return false;
    uint64_t fid = p.fileId, offset = p.offset, result;
    ifstream fin(DATA_DIR + fid);
    string s[33], s, tmp;
    int index = 0;
    getline(fin,s);
    istringstream st(s);
    while(st >> tmp)
        s[index++] = tmp;
    s[offset] = "00000000"; //free a leaf
    ofstream fout(DATA_DIR + fid);
    for(int i = 0 ; i < index; ++i){
        fout << s[i] << " ";
    fout.close();
    freeList.push_back(p);
    ++freeNUm;
    return true;
}


bool PAllocator::persistCatalog(){ // writeback
    // TODO:
    char *pmemaddr_allo, *pmemaddr_free;
    if ((pmemaddr_allo = pmem_map_file(allocatorCatalogPath, PMEM_LEN, PMEM_FILE_CREATE,
                0666, &mapped_len_allo, &is_pmem_allo)) == NULL) 
        return false;
    if ((pmemaddr_free = pmem_map_file(freeListPath, PMEM_LEN, PMEM_FILE_CREATE,
                0666, &mapped_len_free, &is_pmem_free)) == NULL) 
        return false;
    string string_allo, string_free;
    string_allo = string_free = "";
    string_allo =  to_string(this->maxFileId) + " " + to_string(this->freeNum)  + " " 
                + to_string(this->startLeaf.fileId) + " " + to_string(this->startLeaf.offset);
    strcpy(pmemaddr_allo, string_allo);
    if (is_pmem_allo)
        pmem_persist(pmemaddr_allo, mapped_len_allo);
    else
        pmem_msync(pmemaddr_allo, mapped_len_allo);
    
    for(auto vtmp: this->freeList){
        pmemaddr_free += "" + to_string(vtmp.fileId) + " " + to_string(vtmp.offset) + " ";
    }
    strcpy(pmemaddr_free, string_free);
    if (is_pmem_free)
        pmem_persist(pmemaddr_free, mapped_len_free);
    else
        pmem_msync(pmemaddr_free, mapped_len_free);
    return true;
}

/*
  Leaf group structure: (uncompressed)
  | usedNum(8b) | bitmap(n * byte) | leaf1 |...| leafn |
*/
// create a new leafgroup, one file per leafgroup
bool PAllocator::newLeafGroup() { // wtf is this doing? where is structrue leafgroup?
    // TODO:
    char * pmem_addr;
    uint64_t curfid = maxFileId + 1;
    size_t mapped_len;
    int is_pmem;
    if( (pmem_addr = pmem_map_file(DATA_DIR + curfid, PMEM_LEN, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem)) != NULL ){
        ++maxFileId;
        freeNum += 16;
        for(uint64_t i = 0 ;i < 16; ++i){
            PPointer tmp;
            tmp.fileId = curfid;
            tmp.offset = i;
            freeList.push_back(tmp);
        }
        string string_group = "";
        uint64_t used_num = 0;
        string_group += to_string(used_num) + " ";
        string bitmap = "00000000";
        for(int i = 0 ; i < 32; ++i)
            string_group += bitmap +" ";
        strcpy(pmem_addr, string_group);
        if (is_pmem)
            pmem_persist(pmem_addr, mapped_len);
        else
            pmem_msync(pmem_addr, mapped_len);
        return true;
    }
    return false;
}
