#!/bin/bash

#SBATCH -J Alpha_Gamma
#SBATCH -o ./%x%j.%N.out
#SBATCH -D .
#SBATCH --clusters=serial
#SBATCH --partition=serial_std

value_array_gamma='0'
value_array_alpha='0'

for a in ${value_array_alpha}
do
    (
    for g in ${value_array_gamma}
    do
        (
            export "ALPHA=${a}"
            export "GAMMA=${g}"
            strategy="ReinforcementLearning"
            simulation="fallingDrop"
            yaml_file="./build/examples/md-flexible/${simulation}ReinforcementLearningNoProgress.yaml"
            txt_file="alpha_${a}_gamma_${g}.txt"
            ./build/examples/md-flexible/md-flexible "--yaml-filename" ${yaml_file} | tee ${txt_file}
            unset "ALPHA"
            unset "GAMMA"
        )
    done
    )
done