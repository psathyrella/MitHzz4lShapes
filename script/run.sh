# gg2zz central value is really this:
#pdfset=cteq6ll.LHpdf

#----------------------------------------------------------------------------------------
# make dummy flat ntuple with information needed to get pdf weights
#----------------------------------------------------------------------------------------
read_bacon() {
    exe=./bin/readBacon.exe
    
    in=./data/datafiles-gg2zz.txt
    label=gg2zz
    $exe --inputfile $in  --output ./root/ntuples/$label.root &
    
    in=./data/datafiles-ZZ-powheg.txt
    label=ZZ-powheg
    $exe --inputfile $in  --output ./root/ntuples/$label.root &
}
#----------------------------------------------------------------------------------------
# add weight to ntuples for each pdf set
#----------------------------------------------------------------------------------------
gg2zz_Wgts() {
    exe=./bin/pdfreweighter.exe
    label=gg2zz
    echo ./root/ntuples/$label.root > ./data/$label.txt
    in=./data/$label.txt
    for imem in {0..40}; do
	if [ $imem == 0 ]; then pdfset=cteq6m.LHpdf; else pdfset=cteq6.LHpdf; fi
	$exe --inputfile $in --output ./root/ntuples/$label-$pdfset-$imem.root --pdfset $pdfset --imem $imem &
    done
}

ZZ_powheg_Wgts() {
    exe=./bin/pdfreweighter.exe
    label=ZZ-powheg
    echo ./root/ntuples/$label.root > ./data/$label.txt
    in=./data/$label.txt
    pdfset=CT10.LHgrid
    for imem in {0..52}; do
	$exe --inputfile $in --output ./root/ntuples/$label-$pdfset-$imem.root --pdfset $pdfset --imem $imem &
    done
}

#----------------------------------------------------------------------------------------
# run over all the ntuples generated with previous step, sum up the up and down errors
#----------------------------------------------------------------------------------------
gg2zz_UpDown() {
    label=gg2zz
    in=./data/pdfset-files.txt; rm -f $in
    # note: assumes central member is at top of file
    pdfset=cteq6m.LHpdf
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    pdfset=cteq6.LHpdf
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    exe=./bin/getUpDowns.exe
    $exe --inputfile $in --output ./root/ntuples/$label-out.root
}

ZZ_powheg_UpDown() {
    label=ZZ-powheg
    pdfset=CT10.LHgrid
    in=./data/pdfset-files.txt; rm -f $in
    # note: assumes central member is at top of file
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    exe=./bin/getUpDowns.exe
    $exe --inputfile $in --output ./root/ntuples/$label-out.root
}

gg2zz_UpDown