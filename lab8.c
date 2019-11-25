#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<inttypes.h>

struct pte
{
    //  int vpn:6;
     uint16_t pfn;
    //    int offset:10;
      int valid;
      int present;
      int dirty;
      int mode;
      int protect;
};

struct tlb
{
    int vpn;
    uint16_t pfn;
    int use;

};

long long binary(long long n)
{
    long long binaryNumber = 0;
    int remainder, i = 1, step = 1;
    while (n!=0) 
    {
        remainder = n%2;
        n /= 2;
        binaryNumber += remainder*i;
        i *= 10;
    }
    return binaryNumber;
}

void pb(uint16_t n)
{
    uint16_t num=0;
    int size=14;
    int a[14];
    int i=13;
    //int i=15;
    while (n) 
    {

        if (n & 1)
        {
            a[i]=1;
            i--;

        }
        else
        {
            a[i]=0;
            i--;

        }

        n >>= 1;
    }
    while(i>=0)
    {
        a[i]=0;
        i--;
    }
    for(i=0;i<size;i++)
    {
        printf("%d",a[i]);
    }
    //printf("\n");
}

int getvpn(int n)
{
    int vpn=n/pow(2,10);
    return vpn;
}

int getoffset(int n)
{

    int off,rem=pow(2,10);
    off=n%rem;
    return off;
}

int genvta(int *vta ,int n)
{
    for(int i=0;i<n;i++)
    {
        vta[i]=rand()%65536;
        for(int j=0;j<i;j++)
        {
            if(vta[j]==vta[i])
            {
                i--;
            }
        }

    }
}



int getpfn(int *vta, int n, struct pte *p1, struct tlb *t1)
{
    int vpn,off,pfn,ch=0,point,count=0;
    for (int i = 0; i < n; i++)
    {
        vpn=getvpn(vta[i]);
        off=getoffset(vta[i]);
        for(int j=0;j<10;j++)
        {
            if(t1[j].vpn==vpn)
            {
                pfn=t1[j].pfn;
                printf("virtual address is:  ");
                printf("%06lld%010lld \n",binary(vpn),binary(off));
                printf("physical address is:  ");
                pb(pfn);
                printf("%010lld \n",binary(off));
                t1[j].use=1;
                ch=1;
            }          

        }
        if(ch==0)
        {printf("not in tlb \n \n");
        for(int j=0;j<10;j++)
        {
            if(t1[j].use==0)
            point=j;
        }
        t1[point].vpn=vpn;
        t1[point].pfn=p1[vpn].pfn;
        i--;
        count++;
        /*pfn=p1[vpn].pfn;
        printf("virtual address is:  ");
        printf("%06lld%010lld \n",binary(vpn),binary(off));
        printf("physical address is:  ");
        pb(pfn);
        printf("%010lld \n",binary(off));}*/
        }
        ch=0;
        
        
    }
    return count;
    
}



//int printb()

int main()
{
    clock_t end;
    clock_t start;
    int i,n,vpn,off,miss;
    struct pte p1[64];
    struct tlb tlb1[10];
    int vta[100];
    for(i=0;i<64;i++)
    {
        //printf("%d \n",i);
        p1[i].pfn=rand()%16384;
        for(int j=0;j<i;j++)
        {
            if(p1[j].pfn==p1[i].pfn)
            i--;
        }
        //printf("%d \n",p1[i].pfn);
        p1[i].valid=rand()%2;
        p1[i].present=rand()%2;
        p1[i].dirty=rand()%2;
        p1[i].mode=rand()%2;
        p1[i].protect=rand()%8;

    }

    for (int i = 0; i < 10; i++)
    {
        tlb1[i].vpn=rand()%64;
        for (int j = 0; j < i; j++)
        {
            if(tlb1[i].vpn==tlb1[j].vpn)
            i--;
        }
        tlb1[i].pfn=p1[vpn].pfn;
        tlb1[i].use=0;
        
    }
    
    
    start=clock();
    genvta(vta,100);
    
    //for (size_t i = 0; i < 10; i++)
    miss=getpfn(vta,100,p1,tlb1);
    printf("%d",miss);
    //printf("%lld",binary(1023));
    end=clock()-start;
    double time=((double)(end))/CLOCKS_PER_SEC;
    printf("time taken is %f",time);
    //n=vta[5];
    //vpn=getvpn(n);
    //off=getoffset(n);
    //printf("%lld \n",binary(p1[vpn].pfn));
    //printf("%lld \n",binary(1024));
    //printf("%lld%010lld \n",binary(vpn),binary(off));
    //printf("%010lld \n",binary(off));
}