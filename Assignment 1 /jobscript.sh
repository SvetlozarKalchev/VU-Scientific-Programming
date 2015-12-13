#!/bin/bash
#$ -cwd
#$ -N exercise1
#$ -o $JOB_ID.$JOB_NAME.out
#$ -j yes
#$ -pe orte 4
echo "Got $NSLOTS slots." > $JOB_ID.$JOB_NAME.out
echo "Date start: " `date` >> $JOB_ID.$JOB_NAME.out
/opt/global/MPI/OpenMPI/OpenMPI-1.6.5/bin/mpirun -np $NSLOTS ./exercise1
echo "Date end: " `date` >> $JOB_ID.$JOB_NAME.out
