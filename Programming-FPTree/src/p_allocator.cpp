#include"utility/p_allocator.h"
#include<iostream>
#include<sstream>
#include<fstream>
#include<ifstream>
#include<ofstraem>
usin_allocatorg namespace std;

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

int findComma(string s, int sindex){
    for(int i = sindex; i < s.size(); ++i)
        if(s[i] == ',')
            return i;
    return -1;
}

int findLeftBracket(string s, int sindex){
    for(int i = sindex; i < s.size(); ++i)
        if(s[i] == '(')
            return i;
    return -1;
}

int findRightBracket(string s, int sindex){
    for(int i = sindex; i < s.size(); ++i)
        if(s[i] == ')')
            return i;
    return -1;
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
        getline(allocatorCatalog, s))
        istringstream sin_allocator(s);
        while(sin_allocator >> tmp){
            if(tmp == "|" || tmp == "freeNum" || tmp == "=")
                continue;
            switch(account){
                case 0:
                    this->maxFileId = (uint64_t)tmp;
                    break;
                case 1:
                    this->freeNum = (uint64_t)tmp;
                    break;
                case 2:
                    PPointer pptmp;
                    pptmp.fileId = (uint64_t)tmp;
                    if(sin_allocator >> tmp && tmp != "|")
                        pptmp.offset = (uint64_t)tmp;
                    else
                        pptmp.offset = 0;
                default:
                    break;
            }
            ++account;
        }
        int index , left , right, comma;
        index = left = right = comma = 0;
        string fid, offset;
        while(getline(freeListFile, s)){
            istringstream sin_freeList(s);
            while(s >> tmp){
                if(tmp == "|")
                    continue;
                while(left != -1 && right != -1 && comma != -1){
                    left = findLeftBracket(tmp, index);
                    comma = findComma(tmp, left + 1);
                    right = findRightBracket(tmp, comma+1);
                    index = right + 1;
                    fid = tmp.substr(left, comma);
                    offset = tmp.substr(comma + 1, right);
                    PPointer pptmp;
                    pptmp.fileId = (uint64_t)fid;
                    pptmp.offset = (uint64_t)offset;
                    this->freeList.push_back(pptmp);
                } 
            }
        }
    } else {
        // not exist, create catalog and free_list file, then open.
        // TODO:
        fstream fn;
        fn.open(allocatorCatalogPath.c_str(), ofstream::out);
        fn.open(freeListFile.c_str(), ofsream::out);
        allocatorCatalog.close();
        allocatorCatalog.clear();
        allocatorCatalog.open(allocatorCatalogPath.c_str())
        freeListFile.close();
        freeListFile.clear();
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
    for(auto vtmp : freeList){
        uint64_t address = vtmp.fileId + vtmp.offset;
        fId2PmAddr[address] = address; // key is also the value
    }
}

// get the pmem address of the target PPointer from the map fId2PmAddr
char* PAllocator::getLeafPmemAddr(PPointer p) {
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address))
        return fId2PmAddr[address];
    return NULL;
}

// get and use a leaf for the fptree leaf allocation
// return 
bool PAllocator::getLeaf(PPointer &p, char* &pmem_addr) {
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address)){
        *pmem_addr = fId2PmAddr[address];
        return true;
    }
    return false;
}

bool PAllocator::ifLeafUsed(PPointer p) {
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address))
        if(fId2PmAddr[address] != NULL)
            return true;
    return false;
}

bool PAllocator::ifLeafFree(PPointer p) {  //anything differ from ifLeafUsed?
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address))
        if(fId2PmAddr[address] != NULL)
            return false;
    return true;
}

// judge whether the leaf with specific PPointer exists. 
bool PAllocator::ifLeafExist(PPointer p) {
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address))
        return true;
    return false;
}

// free and reuse a leaf
bool PAllocator::freeLeaf(PPointer p) {
    // TODO:
    uint64_t address = p.fileId + p.offset;
    if(fId2PmAddr.find(address)){
        if(fId2PmAddr[address] != NULL)
            fId2PmAddr[address] = NULL;
        return true;
    }
    return false;
}

bool PAllocator::persistCatalog(){ // writeback
    // TODO:
    ofstream allocatorCatalog(allocatorCatalogPath);
    ofstream freeListFile(freeListPath);
    if(allocatorCatalog.is_open() && freeListFile.is_open()){
        allocatorCatalog << "| " << this->maxFileId << " | freeNum = " << this->freeNum 
            << " | " << this->startLeaf.fileId << " " << this->startLeaf.offset << " |" ;
        freeListFile << " | freeList{";
        int account = 1;
        for(auto vtmp: this->freeList){
            cout << "(" << vtmp.fileId << "," << vtmp.offset << ")" << account;
            ++account;
            if(account <= this->freeList.size())
                cout << ",";
        }
        freeListFile << "} |";
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
    return false;
}