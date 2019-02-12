#ifndef _AN_BLK
#define _AN_BLK
#include<iostream>
#include<list>
#include"myfine.h"
#include"mysqldefine.h"
#include"tool.h"
#endif
#include <algorithm>
#include <functional>
using namespace std;


struct recinfo
{
    ulint delflag;
    ulint minflag;
    ulint n_owned;
    ulint heap_no;
    ulint record_type;
    int16_t offset;
};


struct slotinfo
{
    uint32_t slotoffset;
    uint32_t n_owned;
};

class alllist
{
private:
    list<slotinfo> slotlist;
    list<recinfo>  dellist;
public:
    list<recinfo>  reclist;
    list<recinfo>  phylist;
    alllist(){}
    ~alllist()
    {
        slotlist.clear();
        reclist.clear();
        dellist.clear();
    }

    void slot_add(slotinfo slot)
    {
        slotlist.push_front(slot);
    }

    void rec_add(recinfo rec)
    {
        reclist.push_back(rec);
    }

    void del_add(recinfo rec)
    {
        dellist.push_back(rec);
    }

    
    void printslot()
    {
        int cou = 0;
        list<slotinfo>::iterator it;
        cout<<"-----Total slot:"<< slotlist.size()<<" slot list:\n";
        for(it = slotlist.begin(); it != slotlist.end(); ++it)
        {
            cou++;

            if(it->slotoffset == 0X63)
            {
                printf("(%d) INFIMUM slot offset:%u n_owned:%u\n",cou,it->slotoffset,it->n_owned);
            }
            else if(it->slotoffset == 0x70)
            {
                printf("(%d) SUPREMUM slot offset:%u n_owned:%u\n",cou,it->slotoffset,it->n_owned);
            }
            else
            {
                printf("(%d) normal record offset:%u n_owned:%u\n",cou,it->slotoffset,it->n_owned);
            }
        }
    }
    void copy()
   {
    phylist = reclist;
   }


    byte is_del(ulint delflag)
    {
        if(delflag == 1)
        {
            return 'Y';
        }
        else
        {
            return 'N';
        }
    }

    void printrec()
    {
        int cou = 0;
        list<recinfo>::iterator itlogic;
        list<recinfo>::iterator itphy;
        list<recinfo>::iterator it;
        cout<<"-----Total used rows:"<< reclist.size()<<" used rows list(logic):\n";
        for(itlogic = reclist.begin() ; itlogic != reclist.end(); ++itlogic)
        {
            cou++;
            if(itlogic->record_type == REC_STATUS_INFIMUM)
            {
                printf("(%d) INFIMUM record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itlogic->offset,itlogic->heap_no,(ulint)(itlogic->n_owned),is_del(itlogic->delflag),(ulint)(itlogic->minflag),(ulint)(itlogic->record_type));
            }
            else if(itlogic->record_type == REC_STATUS_SUPREMUM)
            {
                printf("(%d) SUPREMUM record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itlogic->offset,itlogic->heap_no,(ulint)(itlogic->n_owned),is_del(itlogic->delflag),(ulint)(itlogic->minflag),(ulint)(itlogic->record_type));
            }
            else
            {
                printf("(%d) normal record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itlogic->offset,itlogic->heap_no,(ulint)(itlogic->n_owned),is_del(itlogic->delflag),(ulint)(itlogic->minflag),(ulint)(itlogic->record_type));
            }
        }
        cou = 0;
        cout<<"-----Total used rows:"<< phylist.size()<<" used rows list(phy):\n";
        for(itphy = phylist.begin() ; itphy != phylist.end(); ++itphy)
                {
                    cou++;
                    if(itphy->record_type == REC_STATUS_INFIMUM)
                    {
                        printf("(%d) INFIMUM record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itphy->offset,itphy->heap_no,(ulint)(itphy->n_owned),is_del(itphy->delflag),(ulint)(itphy->minflag),(ulint)(itphy->record_type));
                    }
                    else if(itphy->record_type == REC_STATUS_SUPREMUM)
                    {
                        printf("(%d) SUPREMUM record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itphy->offset,itphy->heap_no,(ulint)(itphy->n_owned),is_del(itphy->delflag),(ulint)(itphy->minflag),(ulint)(itphy->record_type));
                    }
                    else
                    {
                        printf("(%d) normal record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu rectype:%lu\n",cou,itphy->offset,itphy->heap_no,(ulint)(itphy->n_owned),is_del(itphy->delflag),(ulint)(itphy->minflag),(ulint)(itphy->record_type));
                    }
                }
        cou = 0;


        cout<<"-----Total del rows:"<< dellist.size()<<" del rows list(logic):\n";
        for(it = dellist.begin(); it != dellist.end(); ++it)
        {
            cou++;
            printf("(%d) normal record offset:%d heapno:%lu n_owned %lu,delflag:%c minflag:%lu  rectype:%lu\n",cou,it->offset,it->heap_no,(ulint)(it->n_owned),is_del(it->delflag),(ulint)(it->minflag),(ulint)(it->record_type));
        }
    }
};
