## 一、前言
InnoDB中索引块的内部组织一直是大家比较感兴趣并且乐于研究的东西，我们从很多书籍和文章都不惜笔墨进行大量的描述比如<<MySQL运维内参>>中就能感受到作者用了大量篇幅描述什么是slot、什么是heap、记录的逻辑和物理顺序是怎么样的。

但是我们却很难直观的看到，因为数据文件是二进制文件。虽然我们可以通过例如LINUX的hexdump等类似命令进行查看，但是大量的16进制信息很难直观的提取出各种有用的信息，相信不少人和笔者一样都是通过肉眼进行查看，但是这显然是一种吃力又不讨好的方法。

在Oracle中我们可以通过dump block的方法查看block的信息，那么InnoDB是否也可以这样呢？

本着这种让大家更加直观的观察到底层索引块的信息的宗旨，笔者直接借用源码中的各种宏定义，使用C++和STL list容器实现了这样一个工具**innblock**。由于工作原因不能全身心投入代码编写，代码有些混乱。所以如果有bug还请大家见谅以及提出，笔者会尽快进行更新，感谢。

### 约定
>index page（索引页、索引块），InnoDB表是基于聚集索引的索引组织表，整个表其实不是聚集索引，就是普通索引。因此InnoDB表空间文件中，数据页其实也是索引页，所以下面我们统称为索引页，英文用page no表示；


## 二、innblock简介
本工具有2个功能。

第一个scan功能用于查找ibd文件中所有的索引页。

第二个analyze功能用于扫描数据块里的row data。

先看下 help 输出
 ```
------------------------------------------------------------------------
[Author]:gaopeng [Blog]:blog.itpub.net/7728585/abstract/1/ [QQ]:22389860
[Review]:yejinrong@zhishutang [Blog]:imysql.com [QQ]:4700963
-------USAGE:../innblock Datafile [scan/pageno] Blocksize
[Datafile]:innodb data file!
[scan]:physical scan data file to find index level and index block no
[pageno]:which block you will parse
[Blocksize](KB):block size of KB general is 16k only 4k/8k/16k/32k
------------------------------------------------------------------------
 ```

- scan功能
```
[root@test test]# ./innblock  testblock.ibd scan 16
```

- analyze功能
```
[root@test test]# ./innblock  testblock.ibd 3 16
```

可以执行 ```innblock help``` 获得更详细的使用帮助信息。


## 三、innblock的限制
1. 不支持REDUNDANT行格式的数据文件；
1. 只支持LINUX x64平台；
1. 本工具直接读取物理文件，部分dirty page可能延时刷盘而未能被读取到，可以让InnoDB及时刷盘再重新读取；
1. 最好在MySQL 5.6/5.7版本下测试；
1. 只能解析索引页，不支持inode page、undo log等类型的page；
1. scan功能会包含delete后的索引块和drop了的索引块.
1. 不能读取详细的row data；
1. 建议采用独立表空间模式，更便于观察；
1. 建议仅在测试环境下学习和研究使用。


## 四、innblock怎么用
首先，创建测试表，填充数据
 ```
mysql> create table testblock (
id1 int primary key,
name varchar(30),
id3 int,
key(name),
key(id3));

mysql> insert into testblock values(1,'gao',1),(2,'gao',2),(3,'gao',3),(4,'gao',4);
mysql> delete from testblock where id1=1;
 ```

### 1. 测试scan功能，扫描所有index page
 ```
[root@test]# innblock testblock.ibd scan 16
------------------------------------------------------------------------
Welcome to use this block analyze tool:
[Author]:gaopeng [Blog]:blog.itpub.net/7728585/abstract/1/ [QQ]:22389860
[Review]:yejinrong@zhishutang [Blog]:imysql.com [QQ]:4700963
------------------------------------------------------------------------
Datafile Total Size:131072
===INDEX_ID:248
level0 total block is (1)
block_no:         3,level:   0|*|
===INDEX_ID:249
level0 total block is (1)
block_no:         4,level:   0|*|
===INDEX_ID:250
level0 total block is (1)
block_no:         5,level:   0|*|
 ```

我们发现有3个索引，索引ID（INDEX_ID）分别是 248、249、250，查看数据字典确认
 ```
mysql> SELECT A.SPACE AS TBL_SPACEID, A.TABLE_ID, A.NAME AS TABLE_NAME, FILE_FORMAT, ROW_FORMAT, SPACE_TYPE,  B.INDEX_ID , B.NAME AS INDEX_NAME, PAGE_NO, B.TYPE AS INDEX_TYPE FROM INNODB_SYS_TABLES A LEFT JOIN INNODB_SYS_INDEXES B ON A.TABLE_ID =B.TABLE_ID WHERE A.NAME = 'test/testblock’;
+-------------+----------+----------------+-------------+------------+------------+----------+------------+---------+------------+
| TBL_SPACEID | TABLE_ID | TABLE_NAME     | FILE_FORMAT | ROW_FORMAT | SPACE_TYPE | INDEX_ID | INDEX_NAME | PAGE_NO | INDEX_TYPE |
+-------------+----------+----------------+-------------+------------+------------+----------+------------+---------+------------+
|         242 |      168 | test/testblock | Barracuda   | Dynamic    | Single     |      248 | PRIMARY    |       3 |          3 |
|         242 |      168 | test/testblock | Barracuda   | Dynamic    | Single     |      249 | name       |       4 |          0 |
|         242 |      168 | test/testblock | Barracuda   | Dynamic    | Single     |      250 | id3        |       5 |          0 |
+-------------+----------+----------------+-------------+------------+------------+----------+------------+---------+------------+
 ```

### 2. analyze功能展示
我们选取 pageno=3 那个索引页进行扫描，可见下面信息
 ```
[root@test test]# innblock testblock.ibd 3 16
------------------------------------------------------------------------
Welcome to use this block analyze tool:
[Author]:gaopeng [Blog]:blog.itpub.net/7728585/abstract/1/ [QQ]:22389860
[Review]:yejinrong@zhishutang [Blog]:imysql.com [QQ]:4700963
------------------------------------------------------------------------
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:6          n_rows:3
heap_top:244        del_bytes:31          last_ins_offset:0
page_dir:2          page_n_dir:3
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510679871
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:5 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(5) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
-----Total used rows:5 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:1 del rows list(logic):
(1) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0  rectype:0
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:4
(2) INFIMUM slot offset:99 n_owned:1
 ```


## 五、输出信息详解
我在工具的help文档中也有详细的解释，这里单独对analyze功能解析数据块的输出详解一番，并且我也会给出这些值来自源码的哪个宏定义。这部分知识点在<<MySQL运维内参>>中也有详细说明。


### 1、基本信息(Block base info)
- [block_no]:page offset no inside space,begin is 0(取自 FIL_PAGE_OFFSET)
索引页码(index page no)，该页相对于表空间的偏移量，从0开始计数。如果page no = 3，则实际上是第4个index page。

- [space_id]:this contains the space id of the page(FIL_PAGE_SPACE_ID)
本索引页所属的表空间ID，可以在 INNODB_SYS_TABLES、INNODB_SYS_TABLESPACES、INNODB_SYS_DATAFILES 等系统视图中查看。

- [index_id]:index id where the page belongs.This field should not be written to after page creation. (PAGE_INDEX_ID)
本索引页所属的索引ID，可以在 INNODB_SYS_INDEXES 系统视图中查看。

- [slot_nums]:number of slots in page directory(PAGE_N_DIR_SLOTS)
本索引页中所包含的slot(槽)的数量。

- [heaps_rows]:number of records in the heap include delete rows after purge and INFIMUM/SUPREMUM(取自PAGE_N_HEAP)
本索引页中的全部记录数量，这其中包含了已经deleted且已被purged的记录（这种记录会被放到索引页的garbage队列中），以及两个伪记录INFIMUM/SUPREMUM。

- [n_rows]:number of records not include delete rows after pruge and INFIMUM/SUPREMUM(PAGE_N_RECS)
本索引页中的记录数，不含deleted且已被purged的记录，以及两个伪记录INFIMUM、SUPREMUM。

- [heap_top]:pointer offset to record heap top (PAGE_HEAP_TOP)
指向本索引页已分配的最大物理存储空间的偏移量。

- [del_bytes]:number of bytes in deleted records after purge(PAGE_GARBAGE)
本索引页中所有deleted了的且已被purged的记录的总大小。

- [last_ins_offset]:pointer to the last inserted record, or NULL if this info has been reset by a delete(PAGE_LAST_INSERT)
指向本索引页最后插入记录的位置偏移量，如果最后操作是delete，则这个偏移量为空。通过判断索引页内数据最后插入的方向，用于索引分裂判断。

- [page_dir]:last insert direction: PAGE_LEFT, ...(PAGE_DIRECTION)
本索引页中数据最后插入的方向，同样用于索引分裂判断。

- [page_n_dir]:number of consecutive inserts to the same direction(PAGE_N_DIRECTION)
向同一个方向插入数据的行数，同样用于索引分裂中进行判断

- [leaf_inode_space leaf_inode_pag_no leaf_inode_offset]:leaf segment postion and in inode block offset,only root block(PAGE_BTR_SEG_LEAF开始 10字节)
- [no_leaf_inode_space no_leaf_inode_pag_no no_leaf_inode_offset]:no_leaf segment postion and in inode block offset,only root block(取自PAGE_BTR_SEG_TOP 开始 10字节)
这6个值只在root节点会有信息，分别表示了叶子段和非叶子段的inode的位置和在inode块中的偏移量，其他块都为0。

- [last_modify_lsn]:lsn of the end of the newest modification log record to the page(FIL_PAGE_LSN)
本块最后一次修改的LSN。

- [page_type]:for this tool only B+_TREE(FIL_PAGE_TYPE)
对于本工具而言始终为B+ TREE，因为不支持其它page type。

- [level]:level of the node in an index tree; the leaf level is the level 0(PAGE_LEVEL)
本索引页所处的B+ TREE的层级。注意，叶子结点的PAGE LEVEL为0。


### 2、四个相关链表(Block list info)
- Total used rows:5 used rows list(logic):
not delete purge rows and not delete logic sequence list(next offset list).
这个链表是逻辑有序链表，也是我们平时所说的块内数据有序的展示。它的顺序当然按照主键或者ROWID进行排列，因为是通过物理偏移量链表实现的，实际上就是逻辑上有序。我在实现的时候实际上是取了INFIMUM的偏移量开始进行扫描直到最后，但是注意被deleted且已经被purged的记录不在其中。

- Total used rows:5 used rows list(phy):
not delete purge rows and not delete physics sequence list(sort by heap no).
这个链表是物理上的顺序，实际上就是heap no的顺序，我在实现的时候实际上就是将上面的逻辑链表按照heap no进行排序完成的，所以块内部是逻辑有序物理无序的，同样注意被deleted且已被purged的记录不在其中。

- Total del rows:1 del rows list(logic):
purge delete logic sequence list(next offset list).
这个链表是逻辑上的，也就是被deleted且被purged后的记录都存在于这个链表中，通过读取块的PAGE_FREE获取链表信息。

- Total slot:2 slot list:
slot physics sequence list.
这是slot(槽的)信息，通过扫描块尾部8字节以前信息进行分析得到，我们可以发现在slot中存储的是记录的偏移量。

在这里链表中包含一些信息，这里就用help中的解析给出了。
- [record offset]:real offset in block of this record.
- [heapno]:physics heapno of this record.
- [n_owned]:if this record is slot record n_owned is how many this slot include,other is 0.
- [delflag]:this record is delete will Y,if not purge in list 1,if purge in list 3.
- [rectype]:
   [REC_STATUS_ORDINARY=0(B+ leaf record)
   [REC_STATUS_NODE_PTR=1(not B+ leaf record)]
   [REC_STATUS_INFIMUM=2]
   [REC_STATUS_SUPREMUM=3]
- [slot offset]:where(offset) this slot point,this is a record offset.no purge delete record.
- [n_owned]:how many this slot include recorods.no purge delete record.


## 六、几个测试案列
本节全部使用测试表如下:
 ```
mysql> create table testblock (
id1 int primary key,
name varchar(30),
id3 int,
key(name),
key(id3)
);
```

初始化测试数据：
```
mysql> insert into testblock values(1,'gao',1),(2,'gao',2),(3,'gao',3),(4,'gao',4);
 ```


### 1、执行delete后还未commit的记录只打 delete 标记
发起事务，先执行delete，暂不commit
 ```
mysql> begin; delete from testblock where id1=1;
 ```

分析结果：
 ```
[root@test]# innblock testblock.ibd  3 16
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:6          n_rows:4
heap_top:244        del_bytes:0           last_ins_offset:220
page_dir:2          page_n_dir:3
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510695376
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:6 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0 rectype:0
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(6) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
-----Total used rows:6 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
(3) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0 rectype:0
(4) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(6) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:0 del rows list(logic):
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:5
(2) INFIMUM slot offset:99 n_owned:1
 ```

我们看到其中有一条记录是
 ```
(2) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0 rectype:0
 ```

其 **delflag = Y**，**offset = 127**，这条记录只是delete，但还没 commit，也还没被 purged，因此不会出现在 del rows list链表中。

同时注意到几个信息：
- del_bytes:0
- n_rows:4
- heaps_rows:6

三个信息结合起来看，表示还没有真正被清除的数据。


### 2、执行delete后commit的记录，被purged后真正被清除，进入删除链表

接着上面的事务，继续执行commit
 ```
mysql> commit;
Query OK, 0 rows affected (0.00 sec)
 ```

分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:6          n_rows:3
heap_top:244        del_bytes:31          last_ins_offset:0
page_dir:2          page_n_dir:3
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510695802
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:5 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(5) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
-----Total used rows:5 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:1 del rows list(logic):
(1) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0  rectype:0
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:4
(2) INFIMUM slot offset:99 n_owned:1
 ```

我们看到，执行commit，这条偏移量为127的记录被purged后入了del rows list链表
 ```
(1) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0  rectype:0
 ```

其**delflag = Y**，同时我们观察到
- del_bytes:31  //上一次看到的值是 0
- n_rows:3  //上一次看到的值是 4
- heaps_rows:6 //和上一次的值一样，因为这里计算的是物理记录数

可见，commit且被purged的数据才是真正的删除（清除）。


### 3、先删除后insert更大新记录，旧的heap no不会重用
上面删除的记录的heapno为2，接着插入新记录
 ```
insert into testblock values(5,'gaopeng',1);
 ```

显然它的长度大于删除记录的长度。

分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:7          n_rows:4
heap_top:279        del_bytes:31          last_ins_offset:251
page_dir:5          page_n_dir:0
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510695994
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:6 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:251 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
(6) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
-----Total used rows:6 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(6) normal record offset:251 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:1 del rows list(logic):
(1) normal record offset:127 heapno:2 n_owned 0,delflag:Y minflag:0  rectype:0
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:5
(2) INFIMUM slot offset:99 n_owned:1
 ```
我们看到有一条新记录
 ```
(5) normal record offset:251 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
 ```

这条记录的**heapno = 6**，而删除的旧记录 **heapno=2**，这表明它没有重用del rows list中的空间，因为删除记录的空间根本放不下这条新记录，所以只能重新分配。同时我们注意到 **heap_top = 279 ** ，这里也发生了变化，体现了实际为这行数据分配了新的heapno。


### 4、delete后，再insert更小或者相同大小记录，heap no会重用
在上面的基础上，我们插入新记录
 ```
insert into testblock values(6,'gao',1);
 ```

分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:7          n_rows:5
heap_top:279        del_bytes:0           last_ins_offset:127
page_dir:2          page_n_dir:1
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510700272
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:7 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:251 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
(6) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(7) SUPREMUM record offset:112 heapno:1 n_owned 6,delflag:N minflag:0 rectype:3
-----Total used rows:7 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 6,delflag:N minflag:0 rectype:3
(3) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(6) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(7) normal record offset:251 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:0 del rows list(logic):
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:6
(2) INFIMUM slot offset:99 n_owned:1
 ```

我们这次新写入的数据长度和删除的数据长度一致，我们发现heapno重用了del rows list中的记录没有了，而在数据逻辑顺序中多了一条
 ```
(6) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
 ```

我们发现**heapno=2**的记录 **delflag** 不再是 **Y**了，同时 **heap_top = 279 ** 也没有变化，**del_bytes:31** 变成了 **del_bytes:0**，都充分说明了这块空间得到重用。


### 5、测试del list中的空间重用只会检测第一个条删除的记录
清空数据表后执行测试
 ```
mysql> insert into testblock values(1,'gao',1),(2,'gao',2),(3,'gao',3),(4,'gaopeng',4);
mysql> delete from testblock where id1=4;
mysql> delete from testblock where id1=3;
mysql> insert into testblock values(5,'gaopeng',5);
 ```

在这里，我们先删除 **[id1=4]** 记录，后删除 **[id1=3]** 记录。
由于**del list是头插法**，所以后删除的 [id1=3] 的记录会放在del list链表的最头部，也就是 **[del list header] => [id1=3] => [id1=4]**。虽然 [id=4] 的记录空间足以容下新记录 (5,'gaopeng’,5)，但并没被重用。**因为InnoDB只检测第一个 del list 中的第一个空位 [id1=3]**，显然这个记录空间不足以容下新记录 (5,’gaopeng',5)，所以还是新开辟了heap。

分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:7          n_rows:3
heap_top:283        del_bytes:66          last_ins_offset:255
page_dir:5          page_n_dir:0
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510728551
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:5 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:255 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
(5) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
-----Total used rows:5 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
(3) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:255 heapno:6 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:2 del rows list(logic):
(1) normal record offset:189 heapno:4 n_owned 0,delflag:Y minflag:0  rectype:0
(2) normal record offset:220 heapno:5 n_owned 0,delflag:Y minflag:0  rectype:0
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:4
(2) INFIMUM slot offset:99 n_owned:1
 ```

我们看到 del list 中共有2条记录（没被重用），却新增加了 **heapno = 6** 的记录。


### 6、del_bytes(PAGE_GARBAGE)是否包含碎片空间
从重组函数 **btr_page_reorganize_low** 来看，PAGE_GARBAGE确实包含了碎片空间。

清空数据表后执行测试
 ```
mysql> insert into testblock values(1,'gao',1),(2,'gao',2),(3,'gao',3),(4,'gaopeng',4);
mysql> delete from testblock where id1=4;
 ```

分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:6          n_rows:3
heap_top:248        del_bytes:35          last_ins_offset:0
page_dir:2          page_n_dir:3
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510748484
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:5 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
-----Total used rows:5 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 4,delflag:N minflag:0 rectype:3
(3) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:1 del rows list(logic):
(1) normal record offset:220 heapno:5 n_owned 0,delflag:Y minflag:0  rectype:0
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:4
(2) INFIMUM slot offset:99 n_owned:1
 ```

注意这里 **del_bytes:35** 就是删除这条记录的空间的使用量。接下来执行SQL

 ```
mysql> insert into testblock values(5,'gao',5);
 ```

再次分析结果：
 ```
==== Block base info ====
block_no:3          space_id:242          index_id:248
slot_nums:2         heaps_rows:6          n_rows:4
heap_top:248        del_bytes:4           last_ins_offset:220
page_dir:5          page_n_dir:0
leaf_inode_space:242        leaf_inode_pag_no:2
leaf_inode_offset:242
no_leaf_inode_space:242     no_leaf_inode_pag_no:2
no_leaf_inode_offset:50
last_modify_lsn:510748643
page_type:B+_TREE level:0
==== Block list info ====
-----Total used rows:6 used rows list(logic):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(3) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
(6) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
-----Total used rows:6 used rows list(phy):
(1) INFIMUM record offset:99 heapno:0 n_owned 1,delflag:N minflag:0 rectype:2
(2) SUPREMUM record offset:112 heapno:1 n_owned 5,delflag:N minflag:0 rectype:3
(3) normal record offset:127 heapno:2 n_owned 0,delflag:N minflag:0 rectype:0
(4) normal record offset:158 heapno:3 n_owned 0,delflag:N minflag:0 rectype:0
(5) normal record offset:189 heapno:4 n_owned 0,delflag:N minflag:0 rectype:0
(6) normal record offset:220 heapno:5 n_owned 0,delflag:N minflag:0 rectype:0
-----Total del rows:0 del rows list(logic):
-----Total slot:2 slot list:
(1) SUPREMUM slot offset:112 n_owned:5
(2) INFIMUM slot offset:99 n_owned:1
 ```

注意到 **del_bytes:4**，这个刚好就是 'gaopeng'  7字节减去 'gao'  3字节剩下的4字节，我们也看到了 **[heapno=5]** 这个记录被重用了（del list为空，heaono=5的记录 delflag 不为 Y）。

总之本工具可以按照你的想法进行各种测试和观察。


## 七、内存泄露检测
实际上本工具我并没有显示的分配内存，内存分配基本使用了STL LIST容器检测结果如下：
 ```
==11984== LEAK SUMMARY:
==11984==    definitely lost: 0 bytes in 0 blocks
==11984==    indirectly lost: 0 bytes in 0 blocks
==11984==      possibly lost: 0 bytes in 0 blocks
==11984==    still reachable: 568 bytes in 1 blocks
==11984==         suppressed: 0 bytes in 0 blocks
==11984== Reachable blocks (those to which a pointer was found) are not shown.
==11984== To see them, rerun with: --leak-check=full --show-reachable=yes
 ```
