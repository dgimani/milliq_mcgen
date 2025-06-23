import os

'''
tag = "test"

masses = [1.0, 80.0, 100.0]
nevts_per_job = 10
njobs = 10
'''

tag = "3Mar2025"
#masses = [1.0, 1.4, 1.6, 1.8, 2.0, 3.0, 3.5, 4.0, 4.5, 5.0, 7.0, 10.0, 14.0, 20.0, 28.0, 34.0, 
#          40.0, 44.0, 48.0, 52.0, 58.0, 68.0, 80.0, 100.0]
#masses = [0.1, 0.3, 0.35, 0.4, 0.5, 0.7, 0.9, 1.0, 1.3, 1.5, 1.7, 2.0, 2.3, 2.5, 2.7, 3.0, 3.5, 3.7, 4.0, 4.5, 4.7, 5.0, 5.5, 6.0, 6.5, 7.0, 8.0, 9.0, 10.0, 12.0, 15.0, 17.0, 20.0, 25.0, 27.0, 30.0, 33.0, 35.0, 40.0]
#masses = [1.7, 2.0, 2.3, 2.5, 2.7, 3.0, 3.5, 3.7, 4.7, 6.5, 7.0, 8.0, 9.0, 10.0, 12.0, 17.0, 20.0, 27.0, 33.0, 40.0]
masses = [4.0, 4.5, 5.0, 5.5, 10.0, 15.0, 25.0, 30.0, 35.0]
nevts_per_job = 1000000
njobs = 1000

#base_dir = "/ceph/cms/store/user/fsetti/milliQan_generation/madgraph_dy/"
#base_dir = "/ceph/cms/store/user/dimani/milliQan_generation/madgraph_DY/"
base_dir = "/home/users/dimani/CMSSW_13_3_0/src/MG_runs/out_mq5_13p6tev_v1_high_stats/"
os.system("mkdir -p %s"%(base_dir))
os.system("mkdir -p %s/%s"%(base_dir,tag))

for m in masses:
	sm = str(m).replace(".","p")
	outdir = os.path.join(base_dir,tag,"mass_%s"%(sm))
	os.system("mkdir -p %s"%(outdir))
	os.system("mkdir -p %s/logs"%(outdir))
	os.system("cp condor_exe.sh %s"%(outdir))
	os.system("cp input.tar.xz %s"%(outdir))
	fout = open("config.cmd",'w')
	fout.write("""
	    universe=Vanilla
	    when_to_transfer_output = ON_EXIT
	    transfer_input_files=input.tar.xz
	    
	    +DESIRED_Sites="T2_US_UCSD,T2_US_CALTECH,T2_US_WISCONSIN,T2_US_Purdue,T2_US_Vanderbilt,T2_US_Florida"
	    
	    +Owner = "dimani"
	    
	    log=/%s/logs/submit_$(Cluster)_$(Process).log
	    output=/%s/logs/1e.$(Cluster).$(Process).out
	    error =/%s/logs/1e.$(Cluster).$(Process).err
	    
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
	    
	"""%(outdir,outdir,outdir))

	run = False	
	for ijob in range(njobs):
		if not os.path.isfile("%s/output_%s.root"%(outdir,str(ijob+1))):
			fout.write("arguments={0} {1} {2} {3} {4}\nqueue\n\n".format(m,ijob+1, nevts_per_job, nevts_per_job*njobs, outdir.format(sm=sm)))
			#if not run:
			#	run = True
	fout.close()   
	os.system("cp config.cmd %s"%(outdir))
	print(run)
	#if(run):
	#	print("Now processing DY mass point: " + sm + "." )
	#	os.system("condor_submit config.cmd")


