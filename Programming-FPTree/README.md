
DEMO
===========================

### 环境依赖
Ubuntu 18.04
配置模拟NVM
安装PMDK的libpmem库
编译安装LevelDB
c++ 11标准编译

### 部署步骤
1. [配置模拟NVM](https://software.intel.com/zh-cn/articles/how-to-emulate-persistent-memory-on-an-intel-architecture-server)

2. 安装[PMDK的libpmem库](http://pmem.io/pmdk/libpmem/)

3. 编译安装[LevelDB](https://github.com/google/leveldb)

### 目录结构描述
```
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

### 实现时间计划
1. 5/3晚前完成系统说明书，lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行,5/4进行检查优化并发布v1版本branch。
2. 5/10晚前完成fptree.cpp的部分实现和fptree_test.cpp部分的运行，5/11进行检查优化并发布v2版本branch
3. 5/17晚前完成fptree.cpp的部分实现和fptree_test.cpp部分的运行，5/18进行检查优化并发布v3版本branch
4. 5/30晚前完成fptree.cpp的部分实现和fptree_test.cpp所有的运行，5/31进行检查优化并发布final版本branch，作为最后发布版本

### V1 版本内容更新
1. 增加系统说明书。
2. PAllocator实现并通过utility测试，p_allocator.cpp的实现和运行，utility_test.cpp的运行。
3. LevelDB的使用以及测试，对应lycsb.cpp的实现和运行。

### 作者列表
张家桥 黄世明 何思远 黄善恒