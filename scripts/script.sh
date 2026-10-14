#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=1
#SBATCH -o /nethome/sdyp5/output_%j.txt
#SBATCH -e /nethome/sdyp5/errors_%j.txt
#SBATCH --time=00:10:00  #Tiempo límite (HH:MM:SS) 
mpirun --bind-to none nreinas $1 $2