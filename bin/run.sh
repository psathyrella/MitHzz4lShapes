# gg2zz central value is really this:
#pdfset=cteq6ll.LHpdf

#----------------------------------------------------------------------------------------
# make dummy flat ntuple with gen information needed to get pdf weights
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
gg2zz_Wgts() { # mcdb id: 1499, from lhe file: scale is 91.188
    exe=./bin/addpdfweights.exe
    label=ggZZ.115
    echo ../Limit/root.new/$label.root > ./data/$label.txt
    in=./data/$label.txt
    shapes=91p188
    for imem in {0..40}; do
	if [ $imem == 0 ]; then pdfset=cteq6m.LHpdf; else pdfset=cteq6.LHpdf; fi
	$exe --inputfile $in --output ./root/ntuples/$label-$pdfset-$imem.root --pdfset $pdfset --imem $imem --shapes $shapes &
    done
}

ZZ_powheg_Wgts() {
    exe=./bin/addpdfweights.exe
    label=qqZZ.115
    echo ../Limit/root.new/$label.root > ./data/$label.txt
    in=./data/$label.txt
    pdfset=CT10.LHgrid
    shapes=m4l
    for imem in {0..52}; do
	$exe --inputfile $in --output ./root/ntuples/$label-$pdfset-$imem.root --pdfset $pdfset --imem $imem --shapes $shapes &
    done
}

gghzz_Wgts() { # mcdb id 2749, and note pdf listed is *wrong*: mcdb says cteq6m, but lhe file says CT10
    exe=./bin/addpdfweights.exe
    label=ggH.115
    echo ../Limit/root.new/$label.root > ./data/$label.txt
    in=./data/$label.txt
    pdfset=CT10.LHgrid
    shapes=m4l
    for imem in {0..1}; do
	$exe --inputfile $in --output ./root/ntuples/$label-$pdfset-$imem.root --pdfset $pdfset --imem $imem --shapes $shapes&
    done
}
#gg2zz_Wgts
#ZZ_powheg_Wgts

#----------------------------------------------------------------------------------------
# run over all the ntuples generated with previous step, sum up the up and down errors
#----------------------------------------------------------------------------------------
gg2zz_UpDown() {
    label=ggZZ.115
    in=./data/pdfset-files.txt; rm -f $in
    # note: assumes central member is at top of file
    pdfset=cteq6m.LHpdf
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    pdfset=cteq6.LHpdf
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    exe=./bin/addpdfupdowns.exe
    $exe --inputfile $in --output ./root/ntuples/$label-out.root
}

ZZ_powheg_UpDown() {
    label=qqZZ.115
    pdfset=CT10.LHgrid
    in=./data/pdfset-files.txt; rm -f $in
    # note: assumes central member is at top of file
    for file in `ls ./root/ntuples/* | grep $label-$pdfset-[0-9][0-9]*.root`; do echo $file >> $in; done
    exe=./bin/addpdfupdowns.exe
    $exe --inputfile $in --output ./root/ntuples/$label-out.root
}
