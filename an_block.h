/*************************************************************************
  > File Name: an_block.h
  > Author: gaopeng QQ:22389860 all right reserved
  > Mail: gaopp_200217@163.com
  > Created Time: Thu 28 Sep 2017 04:36:46 PM CST
 ************************************************************************/

#ifndef _AN_BLK
#define _AN_BLK
#include<iostream>
#include<list>
#include"myfine.h"
#include"mysqldefine.h"
#include"tool.h"
using namespace std;
#endif

int an_block(byte*, size_t);

/***
 index struct support
**/
struct indbinfo
{
    ulint offset;//本块块号
    ulint next_offset; //下一个块的块号
    ulint prvs_offset; //上一个块的块号
    ulint level;//本块level
};


struct indexinfo
{
    ulint index_id;
    list<indbinfo> level0;
    list<indbinfo> level1;
    list<indbinfo> level2;
    list<indbinfo> level3;
    list<indbinfo> level4;
    list<indbinfo> level5;
    ~indexinfo()
    {
        level0.clear();
        level1.clear();
        level2.clear();
        level3.clear();
        level4.clear();
        level5.clear();
    }
};



class index_scan_abc
{
public:
    virtual int scan_head(FILE* fd,size_t bsize) = 0;
    virtual int printindv(int i) = 0;
    virtual int scan_sort() = 0;
    virtual int check_file(FILE* fd,size_t bsize) = 0;
private:
    virtual ulint get_size(FILE* fd) = 0;
    //virtual void level_add(indexinfo* index,indbinfo block) = 0;
    //virtual void index_add(indexinfo index) = 0;
};



class index_scan:public index_scan_abc
{
private:
    list<indexinfo> indexchain;
    list<indexinfo> sort_indexchain;
    ulint total_size;
public:
    ~index_scan()
    {
        indexchain.clear();
        sort_indexchain.clear();
    }
    int check_file(FILE *fd,size_t bsize)
    {
        int ret = 0;
        ulint file_space_id = 0;
        ulint fsp_space_id = 0;
        ulint h_chksum = 0;
        ulint f_chksum = 0;

        byte* frame;
        if(!(frame = (byte*)aligned_malloc(bsize,bsize))) //对其blocksize分配内存
        {

            ERRPRINT("aligned_malloc error:");
            ret = -1;
            return ret;
        }

        if( fread(frame, bsize, 1, fd) == 0)
        {
            perror("fread error:");
            ERRPRINT("fread error:");
            ret = -1;
            return ret;
        }

        frame = (byte*)ut_align_down(frame,bsize);
        file_space_id = mach_read_from_4(frame+FIL_PAGE_SPACE_ID);
        fsp_space_id = mach_read_from_4(frame+FSP_SPACE_ID+FIL_PAGE_DATA);
        h_chksum = mach_read_from_4(frame+FIL_PAGE_SPACE_OR_CHKSUM);
        f_chksum = mach_read_from_4(frame+bsize-FAIL_CHK);



#ifdef DEBUG
        cout<<"file_space_id:" <<file_space_id <<" fsp_space_id:"<<fsp_space_id <<endl;
        cout<<"h_chksum:" <<h_chksum <<" f_chksum:"<<f_chksum<<endl;
#endif
//|| h_chksum != f_chksum 5.6.25 BUG or h_chksum != f_chksum
        if(fsp_space_id != file_space_id  )
        {
            ERRPRINT("innodb file check filed fsp_space_id != file_space_id  \n");
            ret = -1;
            return ret;
        }
         fseek(fd,0,SEEK_SET);

         aligned_free(frame);
        return ret;
    }
    void index_add(indexinfo index) //将indexinfo对象将入到一级链表中
    {
        indexchain.push_back(index);

    }

    void level_add(indexinfo& index,indbinfo block)//将合适的level 加入到二级 链表中 引用是必须的 第二个参数只能为变量
    {

        if(block.level == 0)
        {
            index.level0.push_back(block);
        }
        if(block.level == 1)
        {
            index.level1.push_back(block);
        }
        if(block.level == 2)
        {
            index.level2.push_back(block);
        }
        if(block.level == 3)
        {
            index.level3.push_back(block);
        }
        if(block.level == 4)
        {
            index.level4.push_back(block);
        }
        if(block.level == 5)
        {
            index.level5.push_back(block);
        }

    }
    ulint get_size(FILE* fd)
    {
        fseek(fd,0,SEEK_END);
        ulint total_size = ftell(fd);
        fseek(fd,0,SEEK_SET);
        return total_size;
    }

    /*-1 no offset=next offset error */
    int sort_index(list<indbinfo>& sour_level,list<indbinfo>& dest_level)
    {
       list<indbinfo>::iterator indx_level;
       indbinfo  blockinfo;
       ulint temp_offset = 0;
       while(sour_level.size() !=0)
       {
           indx_level =sour_level.begin(); //每次循环都必须将指针移动到头部
           for(;indx_level!=sour_level.end();++indx_level )//进行循环，每次循环都会去掉sour中的一个元素
           {
              if(indx_level->next_offset == FIL_NULL)
               {
    #ifdef DEBUG
                  printf("find last block\n");
    #endif
                 temp_offset = indx_level->prvs_offset; //获取最后的下一个块地址 5.6单链表?
                 blockinfo = *indx_level; //完成复制到blockinfo
                 dest_level.push_front(blockinfo);//在tempinfo的leveln链表中加入这个元素 末尾扫描头插
                 sour_level.erase(indx_level);//在原有leveln中删除这个元素
                 break;
              }
              else
                   {
                     if(indx_level->offset == temp_offset) //如果不是最后块，但是offset等于上一次取得的next_offset
                     {
                        temp_offset = indx_level->prvs_offset;//获取元素的下一个块地址
                        blockinfo = *indx_level;//完成复制到blockinfo
                        dest_level.push_front(blockinfo);//在tempinfo的leveln链表中加入这个元素 末尾扫描头插
                        sour_level.erase(indx_level);//在原有leveln中删除这个元素
                        break;
                     }

                     else
                     {
                        ERRPRINT("sort_index err no offset find: %lu\n",temp_offset);
                        return -1;
                     }
                   }
             }
       }
     return 0;	
    }

public:
    /* -1 error 进程排序并且在indexchain->indexinfo 中删除相应的level信息*/
    int scan_sort()
    {
          list<indexinfo>::iterator indx;
          for(indx = indexchain.begin(); indx != indexchain.end(); ++indx)//顺序方位各个索引indexinfo
          {
              indexinfo tempinfo; //建立临时indexinfo

              tempinfo.index_id = indx->index_id; //获取index_id

              //插入一级索引id链表 下面开始依次处理本索引各个level block
              //level 5:
              if(sort_index(indx->level5,tempinfo.level5) == -1)
              {
                  return -1;
              }

              //level 4:
              if(sort_index(indx->level4,tempinfo.level4) == -1)
              {
                  return -1;
              }

              //level 3:
              if(sort_index(indx->level3,tempinfo.level3) == -1)
              {
                  return -1;
              }

                //level 2:
              if(sort_index(indx->level2,tempinfo.level2) == -1)
              {
                  return -1;
              }
                 //level 1:
              if(sort_index(indx->level1,tempinfo.level1) == -1)
              {
                  return -1;
              }
                  //level 0:

              if(sort_index(indx->level0,tempinfo.level0) == -1)
              {
                  return -1;
              }
            sort_indexchain.push_back(tempinfo);
          }
	return 0;

    }
 /* -1 error 扫描一个真个文件将索引信息分类如果需要排序按照offset需要调用 scan_sort()*/
    int scan_head(FILE* fd,size_t bsize)
    {

        list<indexinfo>::iterator it;
        ulint total_size = 0;
        ulint cur_size = bsize;
        ulint cnt = 1;
        int ret = 0;
        int is_new = 0;
        byte* frame = NULL;

        total_size = get_size(fd);

        setbuf(stdout,NULL);
        if(!(frame = (byte*)aligned_malloc(bsize,bsize))) //对其blocksize分配内存
        {

            ERRPRINT("aligned_malloc error:");
            ret = -1;
            return ret;
        }
        printf("Datafile Total Size:%lu\n",total_size);
        while(cur_size <= total_size)//第一次循环整个数据文件
        {

            is_new=0;
#ifdef DEBUG
            printf("cnt:%lu bzize:%lu totalsize:%lu cursize:%lu\n",cnt,bsize,total_size,cur_size);
#endif
            memset(frame,0,bsize);
            if( fread(frame, bsize, 1, fd) == 0)
            {
                perror("fread error:");
                ERRPRINT("fread error:");
                ret = -1;
                return ret;
            }

            if(mach_read_from_2( frame+FIL_PAGE_TYPE) == FIL_PAGE_INDEX )//判断是否是index_page
            {
                frame = (byte*)ut_align_down(frame,bsize);
                //if(mach_read_from_4(frame+FIL_PAGE_PREV) == FIL_NULL)//判断是否为某个level的第一个块
                //不再进行判断直接全部进入链表，在内存中进行重组避免文件的多次访问
                {
                    indexinfo tempinfo;
                    indbinfo  blockinfo;
                    frame = (byte*)ut_align_down(frame,bsize);
                    blockinfo.offset = mach_read_from_4(frame+FIL_PAGE_OFFSET);
                    frame = (byte*)ut_align_down(frame,bsize);
                    blockinfo.next_offset = mach_read_from_4(frame+FIL_PAGE_NEXT);
                    frame = (byte*)ut_align_down(frame,bsize);
                    blockinfo.prvs_offset = mach_read_from_4(frame+FIL_PAGE_PREV);
                    frame = (byte*)ut_align_down(frame,bsize);
                    blockinfo.level = mach_read_from_2(frame+PAGE_HEADER+PAGE_LEVEL);
                    frame = (byte*)ut_align_down(frame,bsize);
                    tempinfo.index_id = mach_read_from_8(frame+PAGE_HEADER+PAGE_INDEX_ID);


                    for(it = indexchain.begin(); it != indexchain.end(); ++it)
                    {

                        if(tempinfo.index_id == it->index_id ) //如果找到相同的index_id已经挂载直接
                        {
                            is_new = 1;

#ifdef DEBUG
                            cout<<"hint1"<<endl;
                            printf("index_id:%lu level:%lu next_offset:%lu offset:%lu\n",tempinfo.index_id,blockinfo.level,blockinfo.next_offset,blockinfo.offset);
#endif
                            level_add(*it,blockinfo);
                        }
                    }
                    if(is_new == 0) //如果没有找到将tempinfo加入到一级链表
                    {

#ifdef DEBUG
                        cout<<"hint2"<<endl;
                        printf("index_id:%lu level:%lu next_offset:%lu offset:%lu\n",tempinfo.index_id,blockinfo.level,blockinfo.next_offset,blockinfo.offset);
#endif
                        index_add(tempinfo);
                        indexinfo& tmp = indexchain.back();
                        level_add(tmp,blockinfo);           //将头块加入二级链表
                    }
                }
            }
            cnt++;
            cur_size = bsize*cnt;
        }
        fseek(fd,0,SEEK_SET);
        aligned_free(frame);
     return 0;
    }

    void formatprint(list<indbinfo>& level)
    {
         ulint cnt=0;
         list<indbinfo>::iterator indx_level;
         for(indx_level =level.begin();indx_level!=level.end();++indx_level )
         {
              printf("block_no:%10lu,level:%4lu|*|",indx_level->offset,indx_level->level);
              cnt++;
              if(cnt%3 == 0)
              {
                  printf("\n");
              }

         }
         printf("\n");
    }
 /* 0 :打印排序前
    1 :打印排序后，排序后排序前的list为空
  */
    int printindv(int i)
    {
        if(i==0)
        {
            printind(indexchain);
        }
        else if(i==1)
        {
            printind(sort_indexchain);
        }
        else
        {
            ERRPRINT("UNKWON FLAG\n");
            return -1;
        }
     return 0;
    }
    void printind(list<indexinfo>& indexchain)
    {
        list<indexinfo>::iterator indx;

        for(indx = indexchain.begin(); indx != indexchain.end(); ++indx)
        {
            printf("===INDEX_ID:%lu\n",indx->index_id );
            {
                if(indx->level5.size() != 0 )
                {
                    printf("level5 total block is (%lu)\n",(ulint)(indx->level5.size()));
                    formatprint(indx->level5);
                }
                if(indx->level4.size() != 0 )
                {
                    printf("level4 total block is (%lu)\n",(ulint)(indx->level4.size()));
                    formatprint(indx->level4);
                }
                if(indx->level3.size() != 0 )
                {
                    printf("level3 total block is (%lu)\n",(ulint)(indx->level3.size()));
                    formatprint(indx->level3);
                }
                if(indx->level2.size() != 0 )
                {
                    printf("level2 total block is (%lu)\n",(ulint)(indx->level2.size()));
                    formatprint(indx->level2);
                }
                if(indx->level1.size() != 0 )
                {
                    printf("level1 total block is (%lu)\n",(ulint)(indx->level1.size()));
                    formatprint(indx->level1);
                }
                if(indx->level0.size() != 0 )
                {
                    printf("level0 total block is (%lu)\n",(ulint)(indx->level0.size()));
                    formatprint(indx->level0);
                }
            }


        }

    }

};



/* support block  all info*/
struct block_info
{
    /*38*/
    ulint h_chksum; //FIL_PAGE_SPACE_OR_CHKSUM 4
    ulint block_no; //FIL_PAGE_OFFSET 4
    ulint b_lsn;//FIL_PAGE_LSN 8
    ulint space_id;//FIL_PAGE_SPACE_ID 4
    ulint page_type;//FIL_PAGE_TYPE 2

    /*56*/

    ulint slot_n;//PAGE_N_DIR_SLOTS 2
    ulint heap_top;//PAGE_HEAP_TOP 2
    ulint n_heap;//PAGE_N_HEAP 2
    ulint del_bytes;//PAGE_GARBAGE 2
    ulint last_ins_offset;//PAGE_LAST_INSERT 2
    ulint page_dir;//PAGE_DIRECTION 2
    ulint page_n_dir;//PAGE_N_DIRECTION 2
    ulint n_rows;//PAGE_N_RECS 2
    ulint max_trx_id;//PAGE_MAX_TRX_ID 8
    ulint page_level;//PAGE_LEVEL 2
    ulint index_id;//PAGE_INDEX_ID 8

    /*no leaf inode list*/
    ulint no_leaf_inode_space;//FSEG_HDR_SPACE 4
    ulint no_leaf_inode_pageno;//FSEG_HDR_PAGE_NO 4
    ulint no_leaf_inode_offset;//FSEG_HDR_OFFSET 2

    /*leaf inode list*/

    ulint leaf_inode_space;//FSEG_HDR_SPACE 4
    ulint leaf_inode_pageno;//FSEG_HDR_PAGE_NO 4
    ulint leaf_inode_offset;//FSEG_HDR_OFFSET 2

    /**/
     ulint f_chksum;//FAIL_CHK 4
     ulint f_b_lsn;//FAIL_LSN 4

     int get_block_info(byte* frame,ulint bsize)
     {
         h_chksum = mach_read_from_4(frame+FIL_PAGE_SPACE_OR_CHKSUM);
         block_no = mach_read_from_4(frame+FIL_PAGE_OFFSET);
         b_lsn = mach_read_from_8(frame+FIL_PAGE_LSN);
         space_id = mach_read_from_4(frame+FIL_PAGE_SPACE_ID);
         page_type = mach_read_from_2(frame+FIL_PAGE_TYPE);

         slot_n = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_N_DIR_SLOTS);
         heap_top = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_HEAP_TOP);
         n_heap = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_N_HEAP);
         del_bytes = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_GARBAGE);
         last_ins_offset = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_LAST_INSERT);
         page_dir = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_DIRECTION);
         page_n_dir = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_N_DIRECTION);
         n_rows = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_N_RECS);
         max_trx_id = mach_read_from_8(frame+FIL_PAGE_DATA+PAGE_MAX_TRX_ID);
         page_level = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_LEVEL);
         index_id = mach_read_from_8(frame+FIL_PAGE_DATA+PAGE_INDEX_ID);

         f_chksum = mach_read_from_4(frame+bsize-FAIL_CHK);
         f_b_lsn = mach_read_from_4(frame+bsize-FAIL_LSN);

         leaf_inode_space =  mach_read_from_4(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_LEAF);
         leaf_inode_pageno = mach_read_from_4(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_LEAF+FSEG_HDR_PAGE_NO);
         leaf_inode_offset = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_LEAF+FSEG_HDR_OFFSET);

         no_leaf_inode_space = mach_read_from_4(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_TOP);
         no_leaf_inode_pageno = mach_read_from_4(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_TOP+FSEG_HDR_PAGE_NO);
         no_leaf_inode_offset = mach_read_from_2(frame+FIL_PAGE_DATA+PAGE_BTR_SEG_TOP+FSEG_HDR_OFFSET);

	return 0;
     }

     void print()
     {
         printf("==== Block base info ====\n");
         printf("block_no:%-10lu space_id:%-12lu index_id:%-12lu\n",block_no,space_id,index_id);
         printf("slot_nums:%-9lu heaps_rows:%-10lu n_rows:%-10lu\n",slot_n,(n_heap&HEAP_N),n_rows);
         printf("heap_top:%-10lu del_bytes:%-11lu last_ins_offset:%-11lu\n",heap_top,del_bytes,last_ins_offset);
         printf("page_dir:%-10lu page_n_dir:%-11lu\n",page_dir,page_n_dir);
         printf("leaf_inode_space:%-10lu leaf_inode_pag_no:%-10lu\n",leaf_inode_space,leaf_inode_pageno);
         printf("leaf_inode_offset:%-10lu\n",leaf_inode_offset);
         printf("no_leaf_inode_space:%-7lu no_leaf_inode_pag_no:%-10lu\n",no_leaf_inode_space,no_leaf_inode_pageno);
         printf("no_leaf_inode_offset:%-10lu\n",no_leaf_inode_offset);
         printf("last_modify_lsn:%lu\n",b_lsn);
         printf("page_type:B+_TREE level:%-10lu\n",page_level);
     }

     int check_block()
     {
         int ret = 0;
         /*
         if(h_chksum != f_chksum)
         {
             ret = -1;
             ERRPRINT("bloch check error CHKSUM!");
             return ret;
         }
         */
#ifdef DEBUG
      printf("b_lsn & LSN_HIGH: %lu f_b_lsn %lu\n",(b_lsn & LSN_HIGH),f_b_lsn);
#endif
         if((b_lsn & LSN_HIGH) != f_b_lsn)
         {
             ret = -1;
             ERRPRINT("bloch check error LSN!");
             return ret;
         }
         if(page_type != FIL_PAGE_INDEX)
         {
             ret = -1;
             ERRPRINT("bloch check error not b+ block!");
             return ret;
         }

       return ret;
     }


};



