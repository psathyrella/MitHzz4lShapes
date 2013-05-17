#!/bin/bash

hresdir=/afs/cern.ch/work/d/dkralph/powheg/production/gg_H

for dset in `echo $hresdir/s12-h126zz4l-gf-resum??-hres-v2-dkr`; do
    root -b -l -q Shapes/macros/topToRoot.C+\(\"$dset/unmerged/HRes_mhXXX.top\"\)
done
