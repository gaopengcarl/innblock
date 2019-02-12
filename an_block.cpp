/*************************************************************************
    > File Name: an_block.cpp
    > Author: gaopeng QQ:22389860 all right reserved
    > Mail: gaopp_200217@163.com
    > Created Time: Thu 28 Sep 2017 04:34:30 PM CST
 ************************************************************************/

#include<iostream>
#include"an_block.h"
#include"mysqldefine.h"
#include"an_page.h"
using namespace std;

static bool physort(const recinfo& lVal,const recinfo& rVal)
    {
         return lVal.heap_no < rVal.heap_no;
    }

uint32_t get_owned(byte* frame,uint32_t offset)//通过真实偏移量获得n_owned行数
{
	return mach_read_from_1(frame+offset-REC_N_NEW_EXTRA_BYTES)&NOWN;	
}

int get_rec_info(recinfo* rec,byte* frame,int16_t offset)//通过真实偏移量获得rec信息
{
	int ret = 0;
	if(rec == NULL ||frame == NULL)
	{
		ERRPRINT("get_rec_info:NULL pointer!");
		ret = -1;	
		return ret;
	}
	ib_uint64_t info1; //del&&min&&owned
	ib_uint64_t info2; //heapno&&rectype
	info1 = mach_read_from_1(frame+offset-REC_N_NEW_EXTRA_BYTES);
	info2 = mach_read_from_2(frame+offset-REC_N_NEW_EXTRA_BYTES+1);
	rec->n_owned = info1&NOWN;
	rec->delflag = (info1&DELF)>>5;
	rec->minflag = (info1&MINF)>>4;

	rec->heap_no = (info2&HEAP)>>3;
	rec->record_type = (info2&RECT);

	return ret;
	
}


int an_block(/*in*/ byte* frame,size_t blocksize)
{
	ulint cnt = 0; //计数器
	ulint tsolt;   //总的slot数量
	int ret = 0;
	alllist blist; //链表类
	slotinfo tempslot; //临时slot结构体

	recinfo temprec;
	int16_t tempoffset = 0x0;
	int16_t oppo_offset = 0XFFFF;
	

#ifdef DEBUG
   cout <<(ulint)frame <<"\n";
#endif
	
	tsolt = mach_read_from_2(frame+FIL_PAGE_DATA);//获取总的slot数量
	frame = (byte*)ut_align_down(frame,blocksize);
#ifdef DEBUG
	 cout<<(ulint)frame <<"\n";
#endif
	for(cnt = 0;cnt<tsolt;cnt++)//循环获取slot信息
	{
		frame = (byte*)ut_align_down(frame,blocksize);
		tempslot.slotoffset = mach_read_from_2(frame+blocksize-PAGE_DIR-(cnt+1)*PAGE_DIR_SLOT_SIZE);
		frame = (byte*)ut_align_down(frame,blocksize);
		tempslot.n_owned = get_owned(frame,tempslot.slotoffset);
		blist.slot_add(tempslot);
	}
	
	for(cnt = 0;oppo_offset != 0x0000 ;cnt++)
	{
		frame = (byte*)ut_align_down(frame,blocksize);
		if(cnt ==  0)
		{
			/*infi*/
			get_rec_info(&temprec,frame,PAGE_NEW_INFIMUM);
			temprec.offset = PAGE_NEW_INFIMUM;
			oppo_offset = mach_read_from_2_16(frame+PAGE_NEW_INFIMUM-2);//相对偏移量读取
			tempoffset =  oppo_offset +PAGE_NEW_INFIMUM;//下一条记录的真实offset

		}
		else
		{
			 temprec.offset = tempoffset;
			 get_rec_info(&temprec,frame,tempoffset);
			 oppo_offset = mach_read_from_2_16(frame+tempoffset-2);
		     tempoffset =  oppo_offset + tempoffset;	

		}
#ifdef DEBUG
		cout<<"oppo:"<<oppo_offset <<"\n";
		cout<<tempoffset <<"\n";
#endif
		 blist.rec_add(temprec);

	}
        oppo_offset = 0XFFFF;//初始化
        tempoffset = 0x0;//初始化
        frame = (byte*)ut_align_down(frame,blocksize);
        oppo_offset = mach_read_from_2_16(frame+PAGE_FREE+PAGE_HEADER);//获取第一个删除记录偏移量这是绝对地址
        for(cnt = 0;oppo_offset != 0x0000 ;cnt++)
        {
                tempoffset =  oppo_offset+tempoffset;
                temprec.offset = tempoffset;
                get_rec_info(&temprec,frame,tempoffset);
                blist.del_add(temprec);
                frame = (byte*)ut_align_down(frame,blocksize);
                oppo_offset = mach_read_from_2_16(frame+tempoffset-2);//获取下一条记录的偏移地址
        }
        blist.copy();
        blist.phylist.sort(physort);
        printf("==== Block list info ====\n");
	blist.printrec();
        blist.printslot();
	return ret;
}



