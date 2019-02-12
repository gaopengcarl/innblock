/*************************************************************************
 *     > File Name: main.cpp
 *         > Author: gaopeng QQ:22389860 all right reserved
 *             > Mail: gaopp_200217@163.com
 *                 > Created Time: Thu 28 Sep 2017 03:19:13 PM CST
 *                  ************************************************************************/

#include<iostream>
#include"myfine.h"
#include"tool.h"
#include"an_block.h"
#include"usage_check.h"

using namespace std;
int main(int argc,char** argv)
{
    int ret = 0;
    FILE* fd = NULL;
    size_t blocksize = 0;
    size_t blocknum = 0;
    byte* frame = NULL;
    int only_scan = 0;
    int an_reslut = 0;

    block_info binfo;
    index_scan check;
    index_scan scan_s;
    
    an_reslut = par_analyze(argc,argv);
    if(an_reslut == 1)
    {
        only_scan = 1;
        sscanf(*(argv+3),"%ld",&blocksize);
        blocksize = blocksize << KB;
    }
    else if(an_reslut == 2)
    {
        sscanf(*(argv+2),"%ld",&blocknum);
        sscanf(*(argv+3),"%ld",&blocksize);
        blocksize = blocksize << KB;
    }
    else if(an_reslut == 3)
    {
        ret = 0;
        return ret;
    }
    else if(an_reslut == -1)
    {
        ret = -1;
        ERRPRINT("USAGE ERROR");
        return ret;
    }
    else
    {
        ;
    }
    wel();
#ifdef DEBUG
    cout<< blocksize << endl;
#endif
    if(!(frame = (byte*)aligned_malloc(blocksize,blocksize))) //对其blocksize分配内存
    {
        ERRPRINT("MEM MALLOC ERROR!");
        ret = -1;
        return ret;
    }
#ifdef DEBUG
    cout<<(ulint)frame<<endl;
#endif

    if((fd=fopen(argv[1],"rb"))==NULL)//二进制打开文件
    {
        ERRPRINT("FILEOPEN ERROR!");
        perror("FILEOPEN ERROR:\n");
        ret = -1;
        return ret;
    }


    if(check.check_file(fd,blocksize) != 0) //对数据文件合法性进行检测
    {
        ret = -1;
        return ret;
    }

    if(only_scan)
    {
        index_scan_abc* scan_f = &scan_s;
        if(scan_f->scan_head(fd,blocksize) == -1)
        {
            ret = -1;
            return ret;
        }
        scan_f->printindv(0);
    }
    else
    {
        fseek(fd,blocksize*(blocknum),SEEK_SET);//偏移掉用户输入的块数量
        if( fread(frame, blocksize, 1, fd) == 0)
        {

            ERRPRINT("fread file error");
            ret = -1;
            return ret;
        }

        /*获得block 基本信息*/
        {
            binfo.get_block_info(frame,blocksize);
            if(binfo.check_block() !=0)
            {
                ERRPRINT("block info check error!");
                ret = -1;
                return ret;
            }
            binfo.print();//打印block基本信息
        }
        /*获得block 链表信息 并且打印*/
        if(an_block(frame,blocksize) == -1)
        {
            ERRPRINT("BLOCK ANALYZE ERROR!");
            ret = -1;
            return ret;
        }
    }
    aligned_free(frame);//释放内存
    return ret;
}

