#include<bits/stdc++.h>
#include<windows.h>
using namespace std;
const int N=(1<<25); 
int f3(int* a,int n) {
    if (n==1) return a[0];
    for (int i=0;i<n/2;i++) {
        a[i]=a[2*i]+a[2*i+1];
    }
    n=(n+1)/2;
    f3(a,n);
}
int main() {    
    ofstream outfile("f3_sum.txt"); 
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
            sum=f3(a,n); 
        }
        QueryPerformanceCounter(&l); 
        double time=static_cast<double>(l.QuadPart-f.QuadPart)/(arr.QuadPart*100)*1e6;
        outfile <<"n："<<n<<" time:"<<time<<" us, sum = "<<sum<<endl;
        delete[] a; 
    }
    return 0;
}