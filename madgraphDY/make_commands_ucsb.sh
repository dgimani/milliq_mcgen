#!/usr/bin/env sh

# Hardcoded base directory
basedir="runs/out_mq5_13p6tev_v1"
logdir="$basedir/logs/"

# Check if the base directory exists
if ! ls -1d $basedir >& /dev/null ; then
    echo "Need *valid* base directory: $basedir"
    exit
fi

# Get the executable path
exe=$(ls -1 MG5*/bin/mg5_aMC | tail -n 1)

# Create the log directory if it doesn't exist
mkdir -p $logdir

# Define the output .sh file
output_file="submit_jobs.sh"
echo "#!/bin/bash" > $output_file  # Start with a bash header

# Loop through all the .dat cards in the base directory and generate the bsub command list
for card in `ls ${basedir}/proc*.dat`; do
    # Get the log file name from the card file name
    logname=${card##*/}
    logname=${logname%.*}.log

    # Check if the log already indicates completion
    if [ -e $logdir/$logname ] ; then
        if grep --quiet "INFO: Done" $logdir/$logname ; then
            continue
        fi
    fi

    # Write the command with 'bsub python' at the front to the .sh file
    echo "bsub python ./${exe} $card >& $logdir/$logname" >> $output_file
done

# Make the output script executable
chmod +x $output_file

echo "Commands have been written to $output_file and are ready to submit."

