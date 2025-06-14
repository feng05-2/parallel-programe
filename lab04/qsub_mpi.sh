#!/bin/sh
#PBS -N qsub_mpi
#PBS -e test.e
#PBS -o test.o
#PBS -l nodes=1:ppn=8

NODES=$(cat $PBS_NODEFILE | sort | uniq)
# 注意把所有的ntt换成你的选题

for node in $NODES; do
    scp master_ubss1:/home/${USER}/guess/main ${node}:/home/${USER} 1>&2
    scp -r master_ubss1:/home/${USER}/guess/files ${node}:/home/${USER}/ 1>&2
done

/usr/local/bin/mpiexec -np 6 -machinefile $PBS_NODEFILE /home/${USER}/main

scp -r /home/${USER}/files/ master_ubss1:/home/${USER}/guess/ 2>&1
