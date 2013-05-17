in=./data/datafiles.txt
mc="--mc"
exe=./bin/readBacon.exe
label=test
$exe --inputfile $in  --output ./root/ntuples/$label.root $mc
