
	    universe=Vanilla
	    when_to_transfer_output = ON_EXIT
	    should_transfer_files = YES
	    transfer_input_files=input.tar.xz
	    
	    +DESIRED_Sites="T2_US_UCSD"
	    
	    +Owner = "dimani"
	    run_as_owner = True
	    
	    log=//ceph/cms/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0/logs/submit_$(Cluster)_$(Process).log
	    output=//ceph/cms/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0/logs/1e.$(Cluster).$(Process).out
	    error=//ceph/cms/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0/logs/1e.$(Cluster).$(Process).err
	    
	    +project_Name = "cmssurfandturf"
	    use_x509userproxy = True
	    Requirements = (HAS_SINGULARITY=?=True) && (TARGET.K8SNamespace =!= "abc")
	    +SingularityImage="/cvmfs/singularity.opensciencegrid.org/cmssw/cms:rhel8"
	    +RequestK8SNamespace="cms-ucsd-t2"
	    
	    x509userproxy=/tmp/x509up_u81936
			notification=Never
	    
			executable=condor_exe.sh
	    transfer_executable=True
	    
	    
	    RequestMemory = 2048
	    RequestDisk = 5000
	    RequestCpus = 1
	    
arguments=5.0 1 10 10 /ceph/cms/store/user/dimani/milliQan_generation/madgraph_DY/22Feb2025/mass_5p0
queue

