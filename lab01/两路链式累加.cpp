#include <bits/stdc++.h>
#include <windows.h>
#include <stdlib.h>
using namespace std;
const int N=(1<<25);
int f2(int* a, int n) {
    int sum1=0,sum2=0;
    for(int i=0;i<n/2;i++){
    sum1=sum1+a[2*i];
    sum2=sum2+a[2*i+1];
    }
   return sum1+sum2;
}
int main() {    
    ofstream outfile("f2_sum.txt"); 
    for(int t=0;t<25;t++) {
        int n=1<<t;
        int* a=new int[N+5];
        for(int i=0;i<n;i++) {
            a[i]=3*i+71;
        }
        int sum = 0;
        LARGE_INTEGER arr,f,l;        
        QueryPerformanceFrequency(&arr); 
        QueryPerformanceCounter(&f);
        for(int i=0;i<100;i++) {
            sum=f2(a,n); 
        }
        QueryPerformanceCounter(&l); 
        double time=static_cast<double>(l.QuadPart-f.QuadPart)/(arr.QuadPart*100)*1e6;
        outfile <<"n："<<n<<" time:"<<time<<" us, sum = "<<sum<<endl;
        delete[] a; 
    }
    return 0;
}
