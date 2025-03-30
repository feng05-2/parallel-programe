#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
using namespace std;
using namespace std::chrono;
void s1(int** mat,int* vec,int* res,int n) {
    for(int i=0;i<n;i++) {
        res[i]=0;
    }
    for(int j=0;j<n;j++) {
        for(int i=0;i<n;i++) {
            res[j]=mat[i][j]*vec[i]+res[j];
        }
    }
}
int main() {
    ofstream resultFile("s1data.txt");
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);
    const int maxSize=5000;
    int* dataPool = new int[maxSize*maxSize];
    int* vec=new int[maxSize];
    int* res=new int[maxSize];
    for(int n=10;n<=maxSize;n+=(n<=300?10:100)) {
        int** mat=new int*[n];
        for(int i=0;i<n;i++) {
            mat[i]=&dataPool[i*maxSize]; 
        }
        for(int i = 0; i < n; i++) {
            vec[i] = dist(gen);
            for(int j = 0; j < n; j++) {
                mat[i][j] = dist(gen);
            }
        }
        int reps=(n<=100)?1000:
                  (n<=300)?500:
                  (n<=1000)?100:
                  (n<=2000)?50:10;
        s1(mat,vec,res,n); 
        auto start=high_resolution_clock::now();
        for(int cnt=0;cnt<reps;cnt++) {
            s1(mat,vec,res,n);
        }
        auto end=high_resolution_clock::now();
        auto duration=duration_cast<microseconds>(end - start);
        resultFile <<n<< " " <<duration.count() / double(reps)<< "\n";
        delete[] mat;
    }
    delete[] dataPool;
    delete[] vec;
    delete[] res;
    resultFile.close();
    return 0;
}