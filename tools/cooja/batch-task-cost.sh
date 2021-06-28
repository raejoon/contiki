#!/bin/bash

for SOLO_INTVAL in 1 10 60
do
    for TASK_INTVAL in 1 10 60
    do
        for NUM_TASK in 1 2 4 8
        do
            ant run_nogui -Dargs=/home/raejoon/Documents/task-cost/csc/8-$NUM_TASK-$TASK_INTVAL-$SOLO_INTVAL.csc
            mv build/COOJA.testlog /home/raejoon/Documents/task-cost/testlog/8-$NUM_TASK-$TASK_INTVAL-$SOLO_INTVAL.log
        done
    done
done
