    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #define TRUE 1
    #define FALSE 0
    #define INVALID -1    
    #define total_instruction 320
    #define total_vp 32
    #define clear_period 50
    
    typedef struct  
    {
        int pn, pfn, counter, time;
    }pl_type;
    
    pl_type pl[32];
    
    typedef struct pfc_struct
    {
        int pn, pfn;
        struct pfc_struct *next;
    }pfc_type;
    
    pfc_type pfc[32], *freepf_head, *busypf_head, *busypf_tail; int diseffect, a[total_instruction];
    int page[total_instruction], offset[total_instruction];void initialize(int);
    void FIFO(int);
    void LRU(int);
    void OPT(int);
    void LFU(int);
    void NUR(int);
    
    int main()
    {
        int s, i, j;
        srand(10*getpid());
        
        s=(float)319*rand()/32767/32767+1; // 前面两位是页号，后面是偏移地址
        
        for (i=0; i<total_instruction; i+=4)
        {
            if (s<0||s>319)
            {
                printf("When i==%d, Error, s==%d\n",i,s);
                exit(0);
            }
            a[i] = s;
            a[i+1] = a[i]+1;
            a[i+2] = (float)a[i]*rand()/32767/32767/2;
            a[i+3] = a[i+2]+1;
            s=(float)(318-a[i+2])*rand()/32767/32767/2+a[i+2]+2;
            if ((a[i+2]>318)||(s>319))
                printf("a[%d+2], a number which is: %d and s==%d\n",
                     i, a[i+2], s);
        }
      for (i=0; i<total_instruction; i++)
        {
            page[i] = a[i]/10;
            offset[i] = a[i]%10;
        }
        for (i=4; i<=32; i++)
        {
            printf("%2d page frames", i);
            FIFO(i); 
            LRU(i);
            OPT(i);
            LFU(i);
            NUR(i);
            printf("\n");
        }
    }
    
    void initialize(int total_pf) // 通过total_pf控制页框大小
    {
        int i;
        diseffect=0;
    
        for (i=0; i<total_vp; i++) // 初始虚拟页面
        {
            pl[i].pn=i;
            pl[i].pfn=INVALID;
            pl[i].counter=0;
            pl[i].time=-1;
        }      for(i=0; i<total_pf-1; i++) // 初始页表
        {
            pfc[i].next=&pfc[i+1];
            pfc[i].pfn=i;
        }
        pfc[total_pf-1].next = NULL;
        pfc[total_pf-1].pfn = total_pf-1;
        freepf_head = &pfc[0];  //空闲页面指针
    }
    
    void FIFO(int total_pf)
    {
        int i,j;
        pfc_type *p;
        initialize(total_pf);
        busypf_head = busypf_tail = NULL;
        for (i=0; i<total_instruction; i++)
        {
            if (pl[page[i]].pfn==INVALID)
            {
                diseffect+=1;
                if (freepf_head==NULL) // 页面置换
                {
                    p=busypf_head->next;
                    pl[busypf_head->pn].pfn=INVALID; // 清空页框
                    freepf_head = busypf_head;
                    freepf_head->next = NULL;
                    busypf_head = p; //指向下一页
                }
                p=freepf_head->next;              freepf_head->next = NULL;
                freepf_head->pn = page[i]; //填页号
                pl[page[i]].pfn = freepf_head->pfn;
    
                // 尾插入
                if (busypf_tail==NULL)  // 第一次
                    busypf_head = busypf_tail = freepf_head;
                else{
                    busypf_tail->next = freepf_head;
                    busypf_tail = freepf_head;
                }
                freepf_head = p;
            }
        }
        printf("FIFO:%6.4f", 1-(float)diseffect/320);
    }
    
    void LRU(int total_pf)
    {
        int min, minj, i, j, present_time; //时间
        initialize(total_pf);
        present_time=0;
    
        for (i=0;i<total_instruction;i++)
        {
            if (pl[page[i]].pfn==INVALID)
            {
                diseffect++;
                if (freepf_head==NULL)
                {                  min=32767;
                    for (j=0; j<total_vp; j++)  //从页框中找present_time最小的做置换
                    {
                        if (min>pl[j].time&&pl[j].pfn!=INVALID)
                        {
                            min=pl[j].time;
                            minj=j;
                        }
                    }
                    freepf_head=&pfc[pl[minj].pfn];
                    pl[minj].pfn = INVALID;
                    pl[minj].time = -1;
                    freepf_head->next = NULL;
                }
                pl[page[i]].pfn = freepf_head->pfn;
                pl[page[i]].time = present_time;
                freepf_head = freepf_head->next;
            }else
                pl[page[i]].time = present_time; // 在内存中，更新时间
            present_time++; 
        }
        printf("LRU:%6.4f", 1-(float)diseffect/320);
    }
    
    void NUR(int total_pf)
    {
        int i, j, dp, cont_flag, old_dp;
        pfc_type *t;       initialize(total_pf);
        dp=0;
        for (i=0; i<total_instruction; i++)
        {
            if (pl[page[i]].pfn==INVALID)
            {
                diseffect++;
                if (freepf_head==NULL)
                {
                    cont_flag = TRUE;
                    old_dp = dp;
                    while (cont_flag)
                    {
                        if (pl[dp].counter==0&&pl[dp].pfn!=INVALID)
                        {
                            cont_flag = FALSE;
                        }
                        else{
                            dp++;
                            if (dp==total_vp)
                            {
                                dp=0;
                            }
                            if (dp==old_dp)
                            {
                                for (j=0; j<total_vp; j++)
                                {
                                    pl[j].counter=0;
                                }
                            }                    }
                    }
                    freepf_head = &pfc[pl[dp].pfn];
                    pl[dp].pfn = INVALID;
                    freepf_head->next = NULL;
                }   
                pl[page[i]].pfn = freepf_head->pfn;
                freepf_head =freepf_head->next; 
            }else{
                pl[page[j]].counter = 1;
            }
            if (i%clear_period==0)
            {
                for (j=0;j<total_vp;j++)
                {
                    pl[j].counter = 0;
                }
            }
        }
        printf("NRU:%6.4f", 1-(float)diseffect/320);
    }
    
       void OPT(int total_pf)
    {
        int i, j, max, maxpage, d, dist[total_vp];
        pfc_type *t;
        initialize(total_pf);
        for (i=0; i<total_instruction; i++)
        {          if (pl[page[i]].pfn==INVALID)
            {
                diseffect++;
                if (freepf_head==NULL)
                {
                    for (j=0; j<total_vp;j++)
                    {
                        if (pl[j].pfn!=INVALID)
                        {
                            dist[j]=32767;
                        }else
                            dist[j]=0;
                    }
                    d=1;
                    for (j=i+1; j<total_instruction;j++)
                    {
                        if (pl[page[j]].pfn!=INVALID)
                        {
                            dist[page[j]]=d;
                        }
                        d++;
                    }
                
                    max=-1;
                    for(j=0;j<total_vp;j++)
                    {
                        if (max<dist[j])
                        {
                            max=dist[j];
                            maxpage=j;
                        }                  }
                    freepf_head = &pfc[pl[maxpage].pfn];
                    freepf_head->next = NULL;
                    pl[maxpage].pfn = INVALID;
                }
                pl[page[i]].pfn = freepf_head->pfn;
                freepf_head = freepf_head->next;
            }
        }
        printf("OPT:%6.4f", 1-(float)diseffect/320);
    }   void LFU(int total_pf)
    {
        int i, j, min , minpage;
        pfc_type* t;
    
        initialize(total_pf);
        for (i=0; i<total_instruction; i++)
        {
            if (pl[page[i]].pfn==INVALID)
            {
                diseffect++;
                if (freepf_head==NULL)
                {
                    min = 32767;
                    for(j=0; j<total_vp; j++)
                    {                       if (min>pl[j].counter&&pl[j].pfn!=INVALID)
                        {
                            min=pl[j].counter;
                            minpage=j;
                        }
                        pl[j].counter==0;
                    }
                    freepf_head = &pfc[pl[minpage].pfn];
                    pl[minpage].pfn=INVALID;
                    freepf_head->next = NULL;
                }
                pl[page[i]].pfn = freepf_head->pfn;
                freepf_head = freepf_head->next;
                pl[page[i]].counter++;
            }else
                pl[page[i]].counter++;
        }
        printf("LFU:%6.4f", 1-(float)diseffect/320);
    }
