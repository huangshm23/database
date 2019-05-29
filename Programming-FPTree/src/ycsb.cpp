#include "fptree/fptree.h"
#include <leveldb/db.h>
#include <string>

#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads/"; // TODO: the workload folder filepath

const string load = workload + "220w-rw-50-50-load.txt"; // TODO: the workload_load filename
const string run  = workload + "220w-rw-50-50-run.txt"; // TODO: the workload_run filename

const int READ_WRITE_NUM = 350000; // TODO: amount of operations

int main()
{        
    FPTree fptree(1028);
    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000];
    bool* ifInsert = new bool[2200000];
	FILE *ycsb, *ycsb_read;
	char *buf = NULL;
	size_t len = 0;
    struct timespec start, finish;
    double single_time;

    printf("===================FPtreeDB===================\n");
    printf("Load phase begins \n");

    // TODO: read the ycsb_load
    ycsb = fopen(load.c_str(), "r");
    if (ycsb == NULL) {
        printf("Failed to open the load file!\n");
        return -1;
    }
    buf = new char[9];
    //read data until read the end of file
    while(!feof(ycsb)) {
        if (fscanf(ycsb, "%s %lu", buf, &key[inserted]) == EOF) break;
        inserted ++;
    }
    fclose(ycsb);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in the fptree
    for (int i = 0; i < inserted; i ++) {
        fptree.insert(key[i], key[i]);
    }  

    clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);
    printf("Load phase finishes: %lu items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

	printf("Run phase begins\n");

	int operation_num = 0;
    inserted = 0;		
    // TODO: read the ycsb_run
    char tmp1[8];
    string tmp2;
    char tmp_f[READ_WRITE_NUM + 1];
    ycsb_read = fopen(run.c_str(), "r");
    if (ycsb_read == NULL) {
        printf("Failed to open the run file!\n");
        return -1;
    }
    buf = new char[9];
    while(!feof(ycsb_read)) {
        if (fscanf(ycsb_read, "%s %lu", tmp1, &key[operation_num]) == EOF) break;
        operation_num ++;
        if (tmp1[0] == 'I') {
            tmp_f[operation_num] = 'I';
        }
        else if (tmp1[0] == 'R') {
            tmp_f[operation_num] = 'R';
        }
        else if (tmp1[0] == 'U') {
            tmp_f[operation_num] = 'U';
        }
        else {
            printf("Load operation failed\n");
        }
        if(operation_num >= READ_WRITE_NUM) break;
    }
    fclose(ycsb_read);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the fptree
    printf("Operation start!\n");
    for (int i = 0; i < READ_WRITE_NUM; i ++) {
        if (tmp_f[i] == 'I') {
            inserted ++;
            fptree.insert(key[i], key[i]);
        }
        else if (tmp_f[i] == 'R') {
            queried ++;
            fptree.find(key[i]);
        }
        else if (tmp_f[i] == 'U') { //Update mean to insert or get
            fptree.update(key[i], key[i]);
        }
    }

	clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %lu/%lu items are inserted/searched\n", inserted, operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    
    // LevelDB
    printf("===================LevelDB====================\n");
    const string filePath = "/mnt/pmemdir/testdb"; // data storing folder(NVM)

    memset(key, 0, 2200000);
    memset(ifInsert, 0, 2200000);

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;
    // TODO: initial the levelDB
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, filePath, &db);
    assert(status.ok());
    inserted = 0;
    printf("Load phase begins \n");
    // TODO: read the ycsb_read
    ycsb = fopen(load.c_str(), "r");
    if (ycsb == NULL) {
        printf("Failed to open the load file!\n");
        return -1;
    }
    buf = new char[9];
    //read data until read the end of file
    while(!feof(ycsb)) {
        if (fscanf(ycsb, "%s %lu", buf, &key[inserted]) == EOF) break;
        inserted ++;
    }
    fclose(ycsb);

    clock_gettime(CLOCK_MONOTONIC, &start);
    // TODO: load the levelDB
    for (int i = 0; i < inserted; i ++) {
        snprintf(buf, (size_t)9, "%lu", key[i]);
        status = db->Put(leveldb::WriteOptions(), buf, buf);
    }
    delete[] buf;

    clock_gettime(CLOCK_MONOTONIC, &finish);
	single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 + (finish.tv_nsec - start.tv_nsec);
    printf("Load phase finishes: %lu items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n", single_time / inserted);

	printf("Run phase begin\n");
	operation_num = 0;
    inserted = 0;		
    // TODO: read the ycsb_run
    char tmp[READ_WRITE_NUM + 1];
    ycsb_read = fopen(run.c_str(), "r");
    if (ycsb_read == NULL) {
        printf("Failed to open the run file!\n");
        return -1;
    }
    buf = new char[9];
    while(!feof(ycsb_read)) {
        if (fscanf(ycsb_read, "%s %lu", tmp1, &key[operation_num]) == EOF) break;
        operation_num ++;
        if (tmp1[0] == 'I') {
            tmp[operation_num] = 'I';
        }
        else if (tmp1[0] == 'R') {
            tmp[operation_num] = 'R';
        }
        else if (tmp1[0] == 'U') {
            tmp[operation_num] = 'U';
        }
        else {
            printf("Load operation failed\n");
        }
        if(operation_num >= READ_WRITE_NUM) break;
    }
    fclose(ycsb_read);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: run the workload_run in levelDB
     printf("Operation start!\n");
    for (int i = 0; i < READ_WRITE_NUM; i ++) {
        if (tmp[i] == 'I') {
            inserted ++;
            snprintf(buf, (size_t)9, "%lu", key[i]);
            status = db->Put(leveldb::WriteOptions(), buf, buf);
        }
        else if (tmp[i] == 'R') {
            queried ++;
            snprintf(buf, (size_t)9, "%lu", key[i]);
            status = db->Get(leveldb::ReadOptions(), buf, &tmp2);
        }
        else if (tmp[i] == 'U') { //Update mean to insert or get
            snprintf(buf, (size_t)9, "%lu", key[i]);
            status = db->Get(leveldb::ReadOptions(), buf, &tmp2);
            if (!status.ok()) {
                status = db->Put(leveldb::WriteOptions(), buf, buf);
            }
        }
    }

	clock_gettime(CLOCK_MONOTONIC, &finish);
    fclose(ycsb_read);
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %lu/%lu items are inserted/searched\n", inserted, operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM/single_time);	
    return 0;
}
