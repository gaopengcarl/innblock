#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"myfine.h"

int help(void)
{
        printf("--------------------------result man page ----------------------------------------------------------\n");
        printf("[notice!]this tool is only support index block dump of every level.\n");
        printf("--------------------------feature 1 for scan--------------------------------------------------------\n");
        printf("This feature is help to find every index include how many blocks to support analyze feature,because\n");
        printf("block no is must for feature 2.\n");
        printf("Use this feature like:\n");
        printf("[./innblock Datafile scan Blocksize]\n");
        printf("Outfile detail:\n");
        printf("[Datafile Total Size]:datafile total size.\n");
        printf("[INDEX_ID]:index id where the page belongs.This field should not be written to after page creation.\n");
        printf("[leveln total block is]:below this index how many leveln blocks include.\n");
        printf("[leveln block detail]:every block of this level.\n");
        printf("[notice!]this feature only scan total file not consider B+ struct,so if this block is delete or total\n");
        printf("         B+ indexd drop will include in scan result.\n");
        printf("--------------------------feature 2 for analyze------------------------------------------------------\n");
        printf("This feature dump all fixed infomation in index block.\n");
        printf("Use this feature like:\n");
        printf("[./innblock Datafile Blockno Blocksize]\n");
        printf("Outfile detail:\n");
        printf("[Block base info]:this part is base info from 0-74 bytes.\n");
        printf("[block_no]:page offset no inside space,begin is 0.\n");
        printf("[space_id]:this contains the space id of the page.\n");
        printf("[index_id]:index id where the page belongs.This field should not be written to after page creation.\n");
        printf("[slot_nums]:number of slots in page directory.\n");
        printf("[heaps_rows]:number of records in the heap include delete rows after purge and INFIMUM/SUPREMUM.\n");
        printf("[n_rows]:number of records not include delete rows after purge and INFIMUM/SUPREMUM.\n");
        printf("[heap_top]:pointer offset to record heap top \n");
        printf("[del_bytes]:number of bytes in deleted records after purge.\n");
        printf("[last_ins_offset]:pointer to the last inserted record, or NULL if this info has been reset by a delete.\n");
        printf("[page_dir]:last insert direction: PAGE_LEFT, ...\n");
        printf("[page_n_dir]:number of consecutive inserts to the same direction.\n");
        printf("[leaf_inode_space leaf_inode_pag_no leaf_inode_offset]:leaf segment postion and in inode block offset,\n");
        printf("            only root block.\n");
        printf("[no_leaf_inode_space no_leaf_inode_pag_no no_leaf_inode_offset]:no_leaf segment postion and in inode \n");
        printf("           block offset,only root block.\n");
        printf("[last_modify_lsn]:lsn of the end of the newest modification log record to the page.\n");
        printf("[page_type]:for this tool only B+_TREE.\n");
        printf("[level]:level of the node in an index tree; the leaf level is the level 0.\n");
        printf("[Block list info]:this part is list information in block include:\n");
        printf("       1.not delete purge rows and not delete logic sequence list(next offset list).\n");
        printf("       2.not delete purge rows and not delete physics sequence list(sort by heap no).\n");
        printf("       3.purge delete logic sequence list(next offset list).\n");
        printf("       4.slot physics sequence list.\n");
        printf("[record offset]:real offset in block of this record.\n");
        printf("[heapno]:physics heapno of this record.\n");
        printf("[n_owned]:if this record is slot record n_owned is how many this slot include,other is 0.\n");
        printf("[delflag]:this record is delete will Y,if not purge in list 1,if purge in list 3.\n");
        printf("[rectype]:[REC_STATUS_ORDINARY=0(B+ leaf record)][REC_STATUS_NODE_PTR=1(not B+ leaf record)]\n");
        printf("          [REC_STATUS_INFIMUM=2][REC_STATUS_SUPREMUM=3].\n");
        printf("[slot offset]:where(offset) this slot point,this is a record offset.no purge delete record.\n");
        printf("[n_owned]:how many this slot include recorods.no purge delete record.\n");
        printf("--------------------------result man end -----------------------------------------------------------\n");
        return 0;
}


int usage(void)
{

        printf("----------------------------------------------------------------------------------------------------\n");
        printf("[Author]:gaopeng [Blog]:blog.itpub.net/7728585/abstract/1/ [QQ]:22389860\n");
        printf("[Review]:yejinrong@zhishutang [Blog]:imysql.com [QQ]:4700963\n");
        printf("-------USAGE:../innblock Datafile [scan/pageno] Blocksize \n");
        printf("[Datafile]:innodb data file!\n");
        printf("[scan]:physical scan data file to find index level and index block no\n");
        printf("[pageno]:which block you will parse\n");
        printf("[Blocksize](KB):block size of KB general is 16k only 4k/8k/16k/32k \n");
        printf("----------------------------------------------------------------------------------------------------\n");
        return 0;
}

int wel(void)
{
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("Welcome to use this block analyze tool:\n");
    printf("[Author]:gaopeng [Blog]:blog.itpub.net/7728585/abstract/1/ [QQ]:22389860\n");
    printf("[Review]:yejinrong@zhishutang [Blog]:imysql.com [QQ]:4700963\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    return 0;
}


int check_block(const char* bsize )//检查 4K 8K 16K 32K
{
    if(!strcmp(bsize,"4") || !strcmp(bsize,"8") || !strcmp(bsize,"16") || !strcmp(bsize,"32"))
    {
        return 0;
    }
    else
    {
        ERRPRINT("Only 4k/8k/16k/32k block");
        usage();
        return -1;
    }
}


int check_asii_num(const char* num) //检查ASCII 数字0-9
{
    int i;
    int ret = 0;

    for(i=0;i<(int)strlen(num);i++)
    {
        if(*(num+i)<0X30 || *(num+i)>0x39 )
        {
            ret = -1;
            break;
        }
    }
    if(ret == -1 )
    {
        ERRPRINT("Blocknum must number type");
        return ret;
    }

    return ret;
}


/* 1 scan 2 anlyze 3 help*/
int par_analyze(int cargc,char** cargv)
{
    int ret = 0;

    if(cargc == 2)
    {
        if( !strcmp(cargv[1],"help"))
        {
            usage();
            help();
            ret = 3;
            return ret;
        }
    }

    if(cargc != 4  )
    {
            usage();
            ret = -1;
            return ret;
    }

    if( !strcmp(cargv[2],"scan"))
    {
        ret = 1;
        if(check_block(cargv[3]) != 0)
        {
            ret = -1;
            return ret;
        }
    }
    else if(check_asii_num(cargv[2]) == 0)
    {
        ret = 2;
        if(check_block(cargv[3]) != 0)
        {
            ret = -1;
            return ret;
        }
    }
    else
    {
        usage();
        ret = -1;
        return ret;
    }

    return ret;

}
