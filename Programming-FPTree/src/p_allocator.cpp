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
        //initilize allocatorCatalog
        string s, tmp;
        int account = 0;
        getline(allocatorCatalog, s));
        istringstream sin_allocator(s);
        while(sin_allocator >> tmp){
            switch(account){
                case 0:
                    this->maxFileId = strtoull(tmp.c_str(), NULL, 0);
                    break;
                case 1:
                    this->freeNum = strtoull(tmp.c_str(), NULL, 0);
                    break;
                case 2:
                    PPointer pptmp;
                    pptmp.fileId = strtoull(tmp.c_str(), NULL, 0);
                    sin_allocator >> tmp
                    pptmp.offset = strtoull(tmp.c_str(), NULL, 0);
                    break;
                default:
                    break;
            }
            ++account;
        }
        string fid, offset;
        int index = 0;
        while(getline(freeListFile, s)){
            istringstream sin_freeList(s);
            while(s >> tmp){
                PPointer pptmp;
                fid = tmp;
                s >> tmp;
                offset = tmp;
                pptmp.fileId = strtoull(fid, NULL, 0);
                pptmp.offset = strtoull(offset, NULL, 0);
                freeList.push_back(pptmp);
            }
        }
    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO:
        pmem_map_file(allocatorCatalogPath.c_str(), PMEM_LEN, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem);
        pmem_map_file(freeListFile.c_str(), PMEM_LEN, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem);
        allocatorCatalog.open(allocatorCatalogPath.c_str());
        freeListFile.open(freeListFile.c_str());
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
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address) != fId2PmAddr.end()){
        pmem_addr = fId2PmAddr[address];
        return true;
    }
    return false;
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
    if ((pmemaddr_allo = pmem_map_file(PATH, PMEM_LEN, PMEM_FILE_CREATE,
                0666, &mapped_len_allo, &is_pmem_allo)) == NULL) 
        return false;
    if ((pmemaddr_free = pmem_map_file(PATH, PMEM_LEN, PMEM_FILE_CREATE,
                0666, &mapped_len_free, &is_pmem_free)) == NULL) 
        return false;
    string string_allo, string_free;
    string_allo = string_free = "";
    string_allo = "" + to_string(this->maxFileId) + " " + to_string(this->freeNum)  + " " 
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
    if( (pmem_addr = pmem_map_file(DATA_DIR + curfid, PMEM_LEN, PMEM_FILE_CREATE,
                    0666, &mapped_len, &is_pmem)) != NULL ){
        ++maxFileId;
        return true;
    }
    return false;
}
