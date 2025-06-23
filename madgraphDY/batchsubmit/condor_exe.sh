#!/bin/bash

#
# args
#

MASS=$1
FILEID=$2
NEVENTS=$3
NEVENTSTOTAL=$4
COPYDIR=$5

echo "[wrapper] MASS      = " ${MASS}
echo "[wrapper] FILEID    = " ${FILEID}
echo "[wrapper] NEVENTS   = " ${NEVENTS}
echo "[wrapper] NEVENTSTOTAL = " ${NEVENTSTOTAL}
echo "[wrapper] COPYDIR   = " ${COPYDIR}

echo
echo "[wrapper] uname -a:" `uname -a`

source /cvmfs/cms.cern.ch/cmsset_default.sh  > /dev/null 2>&1

#export SCRAM_ARCH=slc7_amd64_gcc700
#export CMSSW_VERSION=CMSSW_10_2_15
export SCRAM_ARCH=el8_amd64_gcc12
export CMSSW_VERSION=CMSSW_13_3_0

echo "[wrapper] SCRAM_ARCH" $SCRAM_ARCH
echo "[wrapper] CMSSW_VERSION" $CMSSW_VERSION

###version using cvmfs install of CMSSW
echo "[wrapper] setting env"
# source /cvmfs/cms.cern.ch/cmsset_default.sh
OLDDIR=`pwd`
cd /cvmfs/cms.cern.ch/$SCRAM_ARCH/cms/cmssw/$CMSSW_VERSION/src
#cmsenv
eval `scramv1 runtime -sh`

cd $OLDDIR

echo

echo "[wrapper] hostname  = " `hostname`
echo "[wrapper] date      = " `date`
echo "[wrapper] linux timestamp = " `date +%s`

#
# untar input sandbox
#

#TMPDIR=tmp
#mkdir $TMPDIR
#mv input.tar.xz $TMPDIR
#cd $TMPDIR
pwd
echo "[wrapper] extracting input sandbox"
tar -xJf input.tar.xz

echo "[wrapper] input contents are"
ls -a

#
# run it
#
python3 --version
which python3

#module load python/3.7.0
# Check Python availability
echo "[wrapper] Using Python: $PYTHON_BIN"
echo "[wrapper] running: python write_cards_metis.py ${MASS} ${NEVENTS} ${FILEID}"
python3 write_cards_metis.py ${MASS} ${NEVENTS} ${FILEID}
pwd
ls ./cards


echo $LD_LIBRARY_PATH

echo $LD_LIBRARY_PATH
echo "[wrapper] Python version after loading module:"
python3 --version
which python3
#export PYTHONPATH=$(pwd)/python-packages:$PYTHONPATH
#export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=/cvmfs/cms-ib.cern.ch/sw/x86_64/nweek-02877/el9_amd64_gcc11/external/tbb/v2021.8.0-f1baa67db24d91165f6abed6217b6443/lib:/cvmfs/cms-ib.cern.ch/sw/x86_64/nweek-02877/el9_amd64_gcc11/external/zstd/1.5.2-4341c217fdb8259db5ea57a7c59a8008/lib:$LD_LIBRARY_PATH
python3 -c "import six; print('[wrapper] six module imported successfully')"

echo "[wrapper] running: ./MG5_aMC_v2_9_20/bin/mg5_aMC cards//proc.dat"
./MG5_aMC_v2_9_20/bin/mg5_aMC cards//proc.dat

echo "[wrapper] running: python extract_kinematics_metis.py"
python3 extract_kinematics_metis.py

echo "[wrapper] running: ./ntupler/run -i ./cards/mgoutput/Events/run_01/mcp_p4s.txt -o output.root -N ${NEVENTSTOTAL} -e $((($FILEID-1)*$NEVENTS))"
./ntupler/run -i ./cards/mgoutput/Events/run_01/mcp_p4s.txt -o output.root -N ${NEVENTSTOTAL} -e $((($FILEID-1)*$NEVENTS))

#
# do something with output
#

echo "[wrapper] output is"
ls -lrth

#
# clean up
#

OUTFILE="output.root"
mv output.root output_${FILEID}.root
pwd
ls -lh
#echo "$transfer_output_files"

ls /home/users/dimani
#DEST_DIR="/home/users/dimani/CMSSW_13_3_0/src/MG_runs/out_mq5_13p6tev_v1_high_stats"
#ls $DEST_DIR
#DEST_FILE="${DEST_DIR}/output_${FILEID}.root"
#mkdir -p $DEST_DIR  # Ensure the directory exists

whoami
id

# Move the output file
#echo "[wrapper] Moving output file to $DEST_FILE"
#mv $OUTFILE $DEST_FILE
#cp $OUTFILE $DEST_FILE

# Verify the file was moved successfully
#if [ -e "$DEST_FILE" ]; then
#    echo "[wrapper] File successfully moved to $DEST_FILE"
#else
#    echo "[ERROR] File move failed!"
#    exit 1
#fi

#if [ ! -d "${COPYDIR}" ]; then
#    echo "creating output directory " ${COPYDIR}
#    mkdir ${COPYDIR}
#fi

#substr="/ceph/cms"
#COPYDIR_CEPH=${COPYDIR#$substr}



#gfal-ls davs://redirector.t2.ucsd.edu:1095/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0/
#touch testfile.txt
#gfal-copy file://`pwd`/testfile.txt davs://redirector.t2.ucsd.edu:1095/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0/testfile.txt


#env -i X509_USER_PROXY=${X509_USER_PROXY} gfal-copy -p -f -t 4200 --verbose  --checksum ADLER32 file://`pwd`/$OUTFILE davs://redirector.t2.ucsd.edu:1095/${COPYDIR_CEPH}/output_${FILEID}.root

echo "[wrapper] cleaning up"
#cd ../..
#rm -r $TMPDIR
#echo "[wrapper] cleaned up"
#ls
