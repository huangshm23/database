#include "leveldb/db.h"
#include <stdint.h>
#include <string>


#define KEY_LEN 8
#define VALUE_LEN 8
using namespace std;

const string workload = "../workloads/";


//change the filePath to run different test
const string load = workload + "1w-rw-50-50-load.txt"; // TODO: (Finnished) the workload_load filename
const string run  = workload + "1w-rw-50-50-run.txt"; // TODO: (Finnished) the workload_run filename

const string filePath = "/mnt/pmemdir/testdb";

//decide how many operration will be read and run
const int READ_WRITE_NUM = 1000; // TODO: (Finished) how many operations

int main()
{        
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::WriteOptions write_options;

    // TODO: (Finished) open and initial the levelDB
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, filePath, &db);
    assert(status.ok());
    
    uint64_t inserted = 0, queried = 0, t = 0;
    uint64_t* key = new uint64_t[2200000]; // the key and value are same
    bool* ifInsert = new bool[2200000]; // the operation is insertion or not
	FILE *ycsb_load, *ycsb_run; // the files that store the ycsb operations
	char *buf = NULL;
	size_t len = 0;
    struct timespec start, finish; // use to caculate the time
    double single_time; // single operation time

    printf("Load phase begins \n");

    // TODO: (Finnished) read the ycsb_load and store
    ycsb_load = fopen(load.c_str(), "r");
    if (ycsb_load == NULL) {
        printf("Failed to open the load file!\n");
        return -1;
    }
    buf = new char[9];
    //read data until read the end of file
    while(!feof(ycsb_load)) {
        if (fscanf(ycsb_load, "%s %lu", buf, &key[inserted]) == EOF) break;
        inserted ++;
    }
    fclose(ycsb_load);


    clock_gettime(CLOCK_MONOTONIC, &start); //start to timing

    // TODO: (Finnished) load the workload in LevelDB
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

	int operation_num = 0;
    inserted = 0;		

    // TODO: (Finnished) read the ycsb_run and store
    char tmp1[8];
    string tmp2;
    char tmp[READ_WRITE_NUM + 1];
    ycsb_run = fopen(run.c_str(), "r");
    if (ycsb_run == NULL) {
        printf("Failed to open the run file!\n");
        return -1;
    }
    buf = new char[9];
    while(!feof(ycsb_run)) {
        if (fscanf(ycsb_run, "%s %lu", tmp1, &key[operation_num]) == EOF) break;
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
    fclose(ycsb_run);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: (Finnished) operate the levelDB
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
	single_time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %lu/%lu items are searched/inserted\n", operation_num - inserted, inserted);
    printf("Run phase throughput: %lf operations per second \n", READ_WRITE_NUM/single_time);

    delete db;
    return 0;
}
