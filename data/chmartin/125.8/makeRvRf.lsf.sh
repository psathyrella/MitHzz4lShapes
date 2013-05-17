#! /bin/bash

cd ${LS_SUBCWD}

eval `scram runtime -sh`

combine -M MultiDimFit RvRfWorkspace.root -n RvRfGrid_Obs -m 125.8 --algo=grid --points=10000