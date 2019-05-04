#include"utility/p_allocator.h"
#include<iostream>
#include<sstream>
#include<string>
#include<cstring>
#include<fstream>
using namespace std;

#define PMEM_LEN 4096
size_t mapped_len;
int is_pmem;

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
                    this->maxFileId = strtoull(tmp, NULL, 0);
                    break;
                case 1:
                    this->freeNum = strtoull(tmp, NULL, 0);
                    break;
                case 2:
                    PPointer pptmp;
                    pptmp.fileId = strtoull(tmp, NULL, 0);
                    sin_allocator >> tmp
                    pptmp.offset = strtoull(tmp, NULL, 0);
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
    persistCatalog();
}

// memory map all leaves to pmem address, storing them in the fId2PmAddr
void PAllocator::initFilePmemAddr() {
    // TODO:
    int account = 1, fd;
    uint64_t fid = 1;
    char * pmem_addr;
    while(fid < maxFileId){
        if ((fd = open(DATA_DIR + account, O_CREAT|O_RDWR, 0666)) < 0)
            break;
        if ((pmem_addr = pmem_map(fd)) == NULL) 
            break;
        else{
            fId2PmAddr[fid] = pmem_addr;
            ++fid;
        }
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) { // leaf in a leafgroup 
    // TODO:
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address))
        return fId2PmAddr[address];
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // TODO:
    uint64_t address = p.fileId;
    if(fId2PmAddr.find(address)){
        *pmem_addr = fId2PmAddr[address];
        return true;
    }
    return false;
}

bool PAllocator::ifLeafUsed(PPointer p) {
    // TODO:
    uint64_t fid = p.fileId, offset = p.offset, result;
    ifstream fin(DATA_DIR + fid); // open leafgroup file according to fileID not pmem_address
    string s,tmp;
    getline(fin, s);
    istringstream st(s);
    for(uint64_t i = 0; i < offset ; ++i)
        st >> tmp;
    st >> result;//bitmap corresponding to p.offset
    if(result)
        return true;
    return false;
}

bool PAllocator::ifLeafFree(PPointer p) {  //anything differ from ifLeafUsed?
    // TODO:
    if(ifLeafUsed(p))
        return false;
    return true;
}

// judge whether the leaf with specific PPointer exists. 
bool PAllocator::ifLeafExist(PPointer p) {
    // TODO:
    uint64_t fid = p.fileId, offset = p.offset, result;
    ifstream fin(DATA_DIR + fid); 
    string s,tmp;
    getline(fin, s);
    uint64_t account = 0;
    istringstream st(s);
    while(st >> tmp)
        ++account;
    if(!account)
        return false;
    else if((account - 1) / 2 >= offset) 
        return true;
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
    ofstream allocatorCatalog(allocatorCatalogPath);
    ofstream freeListFile(freeListPath);
    if(allocatorCatalog.is_open() && freeListFile.is_open()){
        allocatorCatalog  << this->maxFileId << " " << this->freeNum  << " "
            << this->startLeaf.fileId << " " << this->startLeaf.offset  ;
        freeListFile << " | freeList{";
        int account = 1;
        for(auto vtmp: this->freeList){
            cout << "(" << vtmp.fileId << "," << vtmp.offset << ")" << account;
            ++account;
            if(account <= this->freeList.size())
                cout << ",";
        }
        freeListFile << "}";
        allocatorCatalog.close();
        freeListFile.close();
    }
    return false;
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
