
系统说明书
===========================

### 环境依赖
1. Ubuntu 18.04
2. 配置模拟NVM
3. 安装PMDK的libpmem库
4. [编译安装LevelDB](https://github.com/google/leveldb)
5. c++ 11标准编译

### 部署步骤（注：有链接教大家简单安装环境）
1. [配置模拟NVM](https://software.intel.com/zh-cn/articles/how-to-emulate-persistent-memory-on-an-intel-architecture-server)

2. 安装[PMDK的libpmem库](http://pmem.io/pmdk/libpmem/)

3. 编译安装[LevelDB](https://blog.csdn.net/Arcpii/article/details/85930702)

### 目录结构描述
```
|__data: 临时测试数据文件夹，生成的数据文件会放到这个位置。（注意运行测试文件时要使用./bin/XXX_test，保证相对路径成立）
|__gtest: 为Google Test项目目录，不用管  
|__include: 里包含所有用到的头文件  
   |__fptree: fptree的头文件所在文件夹  
      |__fptree.h: fptree地头文件  
   |__utility: fptree所用工具的头文件所在文件夹  
      |__utility.h: 指纹计算等工具函数所在头文件  
      |__clhash.h: 指纹计算所用哈希函数头文件  
      |__p_allocator.h: NVM内存分配器头文件  
|__src: 为项目源码所在地，完成里面所有的实现  
   |__bin: 可执行文件所在文件夹
      |__main: main.cpp的可执行文件
      |__lycsb: lycsb.cpp的可执行文件
      |__ycsb: ycsb.cpp的可执行文件
   |__fptree.cpp: fptree的源文件，项目核心文件(TODO)  
   |__clhash.c: 指纹计算的哈希函数源文件  
   |__p_allocator.cpp: NVM内存分配器源文件(TODO)  
   |__lycsb.cpp: LevelDB的YCSB测试代码(TODO)  
   |__ycsb.cpp: FPTreeDB和LevelDB的YCSB对比测试代码(TODO)  
   |__makefile: src下项目的编译文件  
|__workloads: 为YCSB测试负载文件，用于YCSB Benchmark测试  
   |__数据量-rw-读比例-写比例-load.txt: YCSB测试数据库装载文件  
   |__数据量-rw-读比例-写比例-run.txt: YCSB测试运行文件  
|__test: 为Google Test用户测试代码所在，请完成编译并通过所有测试  
   |__bin: 单元测试可执行文件所在文件夹
      |__fptree_test: fptree_test.cpp的可执行文件
      |__utility_test: utility_test.cpp的可执行文件
   |__fptree_test.cpp: fptree相关测试  
   |__utility_test.cpp: PAllocator等相关测试  
   |__makefile: gtest单元测试的编译文件   
```
### 系统的基本说明
#### 系统名称：FPTreeDB键值存储系统(单线程)
#### 系统功能（其对外可用的对数据的基本操作:增删改查）:
1. Insert增（插入数据）
2. Remove删（删除数据）
3. Update改（修改数据）
4. Find查（查找数据）
5. 系统恢复（BulkLoading方式）
#### 附加功能：
1. 键值数据库性能测试
2. Google Test单元测试（模块测试）
3. LevelDB的使用
#### 系统架构：
![FPTreeDB架构](../assert/FPTreeDB.png)

### 实现时间计划
1. 5/3晚前完成系统说明书，lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行,5/4进行检查优化并发布v1版本branch。
2. 5/10晚前完成fptree.cpp的部分功能实现（插入）和fptree_test.cpp部分的运行，5/11进行检查优化并发布v2版本branch
3. 5/17晚前完成fptree.cpp的部分功能实现（查找和更新）和fptree_test.cpp部分的运行，5/18进行检查优化并发布v3版本branch
4. 5/30晚前完成fptree.cpp的部分功能实现（删除）和fptree_test.cpp所有的运行，5/31进行检查优化并发布final版本branch，作为最后发布版本

### V1 版本内容更新
1. 增加系统说明书。
2. PAllocator实现并通过utility测试，p_allocator.cpp的实现和运行，utility_test.cpp的运行。注：即内存分配的实现。
3. LevelDB的使用以及测试，对应lycsb.cpp的实现和运行。注：测试levelDB的性能，方便与之后的系统比较。

### V1 版本使用说明
首先需进入到Programming-FPTree文件夹，测试步骤：
1. 进入src文件夹，执行make命令，生成相关可执行文件存放于bin文件夹，执行bin目录下的lycsb文件，测试levelDB的性能。
2. 退回到Programming-FPTree文件夹，进入test文件夹，执行make命令，测试内存分配的实现效果。注意这里执行test测试，需要在test/目录下运行可执行文件，即使用./bin/XXX_test，以保证相对路径成立。

### V2 版本内容说明
1. 完成fptree.cpp的部分内容，使其实现重载（bulkloading）和插入(Insert)操作，并且通过部分测试。
2. 在test/ 目录下，执行make指令可以编译并链接文件，生成bin目录下的可执行测试文件；在test/ 目录下执行./bin/fptree_test即可运行测试。（注：测试文件已经修改，将未用到的测试项目注释掉，剩下测试重载和插入操作的函数测试）
3. 使用make clean可以清除make生成的所有文件；执行make cleand可以清除data文件夹内的数据文件。

### 注意事项：
1. 需要挂载模拟NVM的文件夹如：/mnt/pmemdir/testdb文件夹，同时需要执行sudo chmod /mnt/pmemdir 0666命令修改pmemdir的权限。
2. 为了测试方便，这里使用的是Programming-FPTree主目录下的data文件夹作为数据文件夹，如果需要使用模拟NVM环境，则将文件夹路径修改为对应挂载的文件夹，如/mnt/pmemdir/testdb

### 作者列表
黄世明 何思远 黄善恒
