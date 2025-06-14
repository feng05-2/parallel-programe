#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include<mpi.h>
#include<cstring>
using namespace std;
using namespace chrono;

// 编译指令如下
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O2

//int main(int argc,char* argv[])

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    double time_hash=0; // 用于MD5哈希的时间
    double time_guess=0;// 哈希和猜测的总时长
    double time_train=0;// 模型训练的总时长
    PriorityQueue q;
    //q.value_count=stoi(argv[1]);
    auto start_train=system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train=system_clock::now();
    if(world_rank==0){
      auto duration_train=duration_cast<microseconds>(end_train - start_train);
      time_train=double(duration_train.count()) * microseconds::period::num / microseconds::period::den;
      q.init();
      cout << "here" << endl;
    }    
    int curr_num=0;
    auto start=system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history=0;
    // std::ofstream a("./files/results.txt");
    bool st=false;
    while (1)
    {
        MPI_Bcast(&st,1,MPI_C_BOOL,0,MPI_COMM_WORLD);
        if(st){
            break;
        }
        q.PopNext();
        if(world_rank==0){
        q.total_guesses=q.guesses.size();
        if (q.total_guesses - curr_num >= 100000)
        {
            cout << "Guesses generated: " <<history + q.total_guesses << endl;
            curr_num=q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n=10000000;
            if (history + q.total_guesses>10000000)
            {
                auto end=system_clock::now();
                auto duration=duration_cast<microseconds>(end - start);
                time_guess=double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds"<< endl;
                cout << "Hash time:" << time_hash << "seconds"<<endl;
                cout << "Train time:" << time_train <<"seconds"<<endl;
                st=1;
                break;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num>1000000)
{
    auto start_hash=system_clock::now();
    //simd算法
    //bit32 states[4][4];
    //string inputs[4];
    //int num=0;
    //for (string pw : q.guesses)
   //{
        //inputs[num]=pw;
        //num++;
      //  if (num==4)
      // {
      //      MD5Hash_simd(inputs,states);

             //处理结果
      //     for (int i=0;i<4;++i)
      //      {
                // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
                // a << inputs[i] << "\t";
                // for (int i1=0;i1<4;i1 += 1)
                // {
                //     a << std::setw(8) << std::setfill('0') << hex << states[i][i1];
                // }
                // a << endl;
        //    }

      //  num=0;
     //  }
  // }
   // if (num>0)
// {
 //      for (;num<4;num++)
     //  {
      //      inputs[num]="";// 补充空字符串
     //   }
      //  MD5Hash_simd(inputs,states);

        // 处理结果
      // for (int i=0;i<num;i++)
      //  {
            // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
            // a << inputs[i] << "\t";
            // for (int i1=0;i1<4;i1 += 1)
            // {
            //     a << std::setw(8) << std::setfill('0') << hex << states[i][i1];
            // }
            // a << endl;
       // }
    //}
    //simd8算法
   // bit32 states[8][4];
   // string inputs[8];
   // int num=0;
   // for (string pw : q.guesses)
 //  {
 //       inputs[num]=pw;
 //       num++;
 //       if (num==8)
 //       {
 //           MD5Hash_simd8(inputs,states);

             //处理结果
   //        for (int i=0;i<8;++i)
   //         {
                // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
                // a << inputs[i] << "\t";
                // for (int i1=0;i1<4;i1 += 1)
                // {
                //     a << std::setw(8) << std::setfill('0') << hex << states[i][i1];
                // }
                // a << endl;
  //          }

  //      num=0;
  //    }
  // }
   // if (num>0)
   // {
   //    for (;num<8;num++)
   //    {
   //         inputs[num]="";// 补充空字符串
   //     }
   //     MD5Hash_simd8(inputs,states);

        // 处理结果
    //   for (int i=0;i<num;i++)
   //     {
            // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
            // a << inputs[i] << "\t";
            // for (int i1=0;i1<4;i1 += 1)
            // {
            //     a << std::setw(8) << std::setfill('0') << hex << states[i][i1];
            // }
            // a << endl;
   //     }
   // }
    //串行算法
    bit32 state[4];
   for (string pw : q.guesses)
    {
        // TODO：对于SIMD实验，将这里替换成你的SIMD MD5函数
      MD5Hash(pw, state);

        // 以下注释部分用于输出猜测和哈希，但是由于自动测试系统不太能写文件，所以这里你可以改成cout
        // a<<pw<<"\t";
        // for (int i1 = 0; i1 < 4; i1 += 1)
        // {
        //     a << std::setw(8) << std::setfill('0') << hex << state[i1];
        // }
        // a << endl;
   }
    // 在这里对哈希所需的总时长进行计算
    auto end_hash=system_clock::now();
    auto duration=duration_cast<microseconds>(end_hash - start_hash);
    time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;

    // 记录已经生成的口令总数
    history+=curr_num;
    curr_num=0;
    q.guesses.clear();
}
        }
    }
    MPI_Finalize();
}