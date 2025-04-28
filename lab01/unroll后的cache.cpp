#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
using namespace std;
using namespace std::chrono;
void s1(int* A,int* a,int* s,int n) {
    for (int i=0;i<n;i++) {
        int sum0=0,sum1=0,sum2=0,sum3=0;
        int j=0;
        for(;j<=n-4;j+=4){
            sum0+=A[i*n+j]*a[j];
            sum1+=A[i*n+j+1]*a[j+1];
            sum2+=A[i*n+j+2]*a[j+2];
            sum3+=A[i*n+j+3]*a[j+3];
        }
        int sum=sum0 + sum1 + sum2 + sum3;
        for (;j<n;j++) {
            sum=A[i*n+j]*a[j]+sum;
        }
        s[i]=sum;
    }
}
int main() {
    ofstream resultFile("s11data.txt");
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);
    const int maxSize = 5000;
    int* A = new int[maxSize * maxSize];
    int* a = new int[maxSize];
    int* s = new int[maxSize];
    for (int n = 10; n <= maxSize; n += (n <= 300 ? 10 : 100)) {   
        for(int i = 0; i < n; i++) {
            a[i] = dist(gen);
            for(int j = 0; j < n; j++) {
                A[i*n + j] = dist(gen);
            }
        }
        int reps = (n <= 100) ? 1000 :
                  (n <= 300) ? 500 :
                  (n <= 1000) ? 100 :
                  (n <= 2000) ? 50 : 10;
        s1(A, a, s, n);
        auto start = high_resolution_clock::now();
        for(int cnt = 0; cnt < reps; cnt++) {
            s1(A, a, s, n);
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        resultFile << n << " " << duration.count() / double(reps) << "\n";
    }
    delete[] A;
    delete[] a;
    delete[] s;
    resultFile.close();
    return 0;
}
