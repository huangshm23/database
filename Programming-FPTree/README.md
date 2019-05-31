
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

### 功能实现说明
1. Insert增（插入数据）<br>
插入操作实现主要是一层层递归查找到相应位置，直到最底层，进行真正的插入操作；主要需要实现六个函数：<br>
（1）中间结点的插入函数:该函数利用findIndex（）函数一层层递归向下寻找插入位置，直到最底层，调用叶子结点的插入函数进行真正的key-value插入，返回时，一层层调用中间结点的结点插入函数，进行索引和结点的相关插入；若是插入过程结点满了，则调用分裂函数。 <br>
（2）中间结点的结点插入函数： 该函数利用传入的key值查找到对应位置，进行结点插入函数。 <br>
（3）中间结点的分裂：一旦中间结点的孩子数等于2 * d + 2，则将该节点分裂，返回新节点的相关信息给上一层。 <br>
（4）叶子结点的插入：调用叶子的key-value插入函数，直接插入，如该节点满了，则分裂，最后持久化。 <br>
（5）叶子结点的分裂： 一旦该叶子结点插入的key-value对等于2*LEAF_DEGREE, 则进行分裂，返回新节点的相关信息给上一层。 <br>
（6）叶子结点的key-value插入函数： 直接插入key-value，对相关数据进行修改。 <br>
2. Remove删（删除数据）<br>
删除操作实现主要是一层层递归查找到相应位置，直到最底层，若是最底层未查找到对应key，则返回false；若是查找到，则删除该key-value，并更新相关数据，若是该节点在删除过后变空，则返回相关信息给上一层，上一层利用此进行删除结点的操作；而中间结点在删除某个孩子结点后，需要检查是否需要执行重分布或者合并操作（与左兄弟或者右兄弟重分布或者合并的优先级请查看相关说明）；<br>
主要需要实现九个函数：<br>
（1）中间结点的获得兄弟函数：利用传入的index和parent结点，直接查看当前节点的左右兄弟是否存在，若存在则获得对应指针。<br>
（2）中间结点的与左兄弟合并：将当前右兄弟的孩子都插入到当前节点中，返回上一层相关信息。<br>
（3）中间结点的与右兄弟合并：将当前节点的孩子都插入到左兄弟中，返回上一层相关信息。<br>
（4）中间结点的与左兄弟重分布：将俩者的孩子进行分配，左边有更多元素，将左兄弟的部分孩子插入到当前节点中，删除左兄弟的已经插入到当前节点的孩子。<br>
（5）中间结点的与右兄弟重分布：将俩者的孩子进行分配，左边有更多元素，将右兄弟的部分孩子插入到当前节点中，删除右兄弟的已经插入到当前节点的孩子。<br>
（6）中间结点的与父亲节点以及左兄弟合并：首先删除父亲节点的所有节点，然后按序将左孩子和当前节点的孩子插入到父亲当中。<br>
（7）中间结点的与父亲节点以及左兄弟合并：首先删除父亲节点的所有节点，然后按序将当前节点和右孩子的孩子插入到父亲当中。<br>
（8）叶子结点的删除函数：遍历查找是否存在要删除的key-value，若不存在，则返回false，若存在，则删除该key-value，并进行相关数据更新，且若是该节点为空，则返回上一层需要删除该节点的信息；最后进行持久化处理。<br>
（9）中间结点的删除函数：一层层递归查找到相应位置，直到最底层，若是最底层未查找到对应key，则返回false；若是查找到，则删除该key-value，并更新相关数据，若是该节点在删除过后变空，则返回相关信息给上一层，上一层利用此进行删除结点的操作；而中间结点在删除某个孩子结点后，需要检查是否需要执行重分布或者合并操作等。<br>
3. Update改（修改数据）<br>
修改操作实现是在查找的基础上实现的，一层层递归利用每一层的查找函数，直到最底层，如果查找到，便更改；<br>
主要实现俩个函数：<br>
（1）中间结点的修改：由于中间结点只是存储索引，所以修改并不直接在此进行，而是递归进入下一层，直到找到叶子结点。<br>
（2）叶子结点的修改：遍历所有key值，若查找到，则更改，持久化，并返回true，否则返回false。<br>
4. Find查（查找数据）<br>
查找操作实现主要是一层层递归利用中间结点或者叶子结点的查找函数，直到最后一层；<br>
主要需要实现中间结点和叶子结点的查找函数：<br>
（1）中间结点的查找：由于中间结点只是存储索引，所以遍历查找并在此进行，而是利用findIndex（）函数递归进入下一层，直到找到叶子结点。<br>
（2）叶子结点的查找：遍历所有key值，若查找到，则返回对应的value，否则返回MAX_VALUE。<br>
5. 系统恢复（BulkLoading方式）<br>
系统恢复操作实现主要是利用叶子分配器来加载keyNode，然后利用中间结点的叶子插入函数，一层层递归查找到最底层，进行插入；<br>
主要实现俩个函数：<br>
（1）FPTree的BulkLoading（）函数：利用PAllocator的函数生成记录的叶子，生成keyNode，调用中间结点的插入叶子函数，进行真正的插入。<br>
（2）中间结点的插入叶子函数：一层层递归向下调用中间结点的插入叶子函数，直到其孩子为叶子结点，插入叶子，返回是否需要分裂的相关信息。<br>

#### 系统架构：
![FPTreeDB架构](../assert/FPTreeDB.png)

### 实现时间计划
1. 5/3晚前完成系统说明书，lycsb.cpp，p_allocator.cpp的实现和运行，utility_test.cpp的运行,5/4进行检查优化并发布v1版本branch
2. 5/10晚前完成fptree.cpp的部分功能实现（插入）和fptree_test.cpp部分的运行，5/11进行检查优化并发布v2版本branch
3. 5/17晚前完成fptree.cpp的部分功能实现（查找和更新）和fptree_test.cpp部分的运行，5/18进行检查优化并发布v3版本branch
4. 5/30晚前完成fptree.cpp的部分功能实现（删除）和fptree_test.cpp所有的运行，5/31进行检查优化并发布final版本branch，作为最后发布版本

### final 版本内容更新
1. 完成fptree.cpp的删除功能实现和通过fptree_test.cpp所有的运行
2. 完成ycsb.cpp的实现和测试
3. 完成main.cpp的实现和测试

### final 版本使用说明
首先需进入到Programming-FPTree文件夹，测试步骤：
1. 进入src文件夹，执行make命令，生成相关可执行文件存放于bin文件夹，执行bin目录下的ycsb文件，对FPTree和levelDB的性能进行比较。
2. 执行bin目录下的main文件，测试FPTree的性能。
2. 退回到Programming-FPTree文件夹，进入test文件夹，执行make命令，执行bin目录下的fptree_test文件，测试fptree_test.cpp所有的运行。注意这里执行test测试，需要在test/目录下运行可执行文件，即使用./bin/XXX_test，以保证相对路径成立。

### final 版本内容说明
1. 完成fptree.cpp的所有内容，使其实现五种操作，并且通过全部测试。
2. 在test/ 目录下，执行make指令可以编译并链接文件，生成bin目录下的可执行测试文件；在test/ 目录下执行./bin/fptree_test即可运行测试。
3. 每次进行ycsb和main的测试前需要清除所有的数据文件，防止出现错误。
4. 使用make clean可以清除make生成的所有文件；执行make cleand可以清除数据文件夹内的数据文件。

### 注意事项：
1. 需要挂载模拟NVM的文件夹有：/mnt/pmemdir/fptree文件夹，同时需要执行sudo chmod 777 /mnt/pmemdir/fptree 命令修改访问fptree文件夹的权限。
2. 若是出现段错误，可能需要执行make cleand清除错误数据文件，然后重新进行测试。
3. 若是出现权限问题，可以执行执行sudo chmod 777 路径 命令修改对应文件夹权限。
4. 若是性能测试时，执行速度过慢，可能是未挂载对应文件夹。
### 作者列表
黄世明 何思远 黄善恒
