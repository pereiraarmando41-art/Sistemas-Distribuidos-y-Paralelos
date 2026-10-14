#!/bin/bash

hilos=(8 4 2)
reinas=(14 15 16 17 18)

for h in "${hilos[@]}"; do
    for r in "${reinas[@]}"; do 
        sbatch script.sh $r $h
    done
done

