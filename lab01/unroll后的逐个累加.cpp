#include<bits/stdc++.h>
#include<windows.h>
#include<stdlib.h>
using namespace std;
const int N (1<<25); 
int f11(int* a, int n) {
    int sum=0;
    int i;
    for(i = 0; i < n/4; i++) {
        sum= sum + a[4*i];
        sum= sum + a[4*i+1];
        sum= sum + a[4*i+2];
        sum= sum + a[4*i+3];
    }
    for(int j=4*i;j<n;j++){
    	sum=sum+a[j];
	}
	return sum;
}
int main() {    
    ofstream outfile("f11_sum.txt"); 
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
            sum=f11(a,n); 
        }
        QueryPerformanceCounter(&l); 
        double time=static_cast<double>(l.QuadPart-f.QuadPart)/(arr.QuadPart*100)*1e6;
        outfile <<"n："<<n<<" time:"<<time<<" us, sum = "<<sum<<endl;
        delete[] a; 
    }
    return 0;
}
