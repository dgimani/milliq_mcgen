import os
import time
import json
import subprocess
import numpy as np
import ROOT as r
import ctypes

# outdir = "/hadoop/cms/store/user/bemarsh/milliqan/milliq_mcgen/ntuples_mapp_theta5_v1"
outdir = "/net/cms26/cms26r0/schmitz/milliq_mcgen/gentuples"

# masses = [0.010, 0.020, 0.030, 0.050, 0.100, 0.200, 0.300, 0.350, 0.400, 0.500, 0.700, 1.000, 1.400, 1.600, 1.800, 2.000, 3.000, 3.500, 4.000, 4.500, 5.000]
# masses = [0.010, 0.020, 0.030, 0.050, 0.100, 0.200, 0.300, 0.350, 0.400, 0.500]
# masses = [2.000, 3.000, 3.500, 4.000, 4.500, 5.000]
masses = [0.010, 0.10, 1.0, 5.0]

# we target this amount of events per mass point, in practice it will be a bit higher since we
# put a floor on the number of events in each mode
#N_target_events = 5e7 #usual size
N_target_events = 5e5

# the number of events in each job (i.e. each output file).
# Since every mode must have at least 1 job, this is effectively the minimum number of events per mode
# nevts_per_job = 500000  #usual size
nevts_per_job = 50000


xsec_file = r.TFile("../../scripts/plot-xsecs/xsecs.root")

def get_xsec(decay_mode, m):
    if decay_mode == 0:
        decay_mode = "total"

    g = xsec_file.Get("xsecs_"+str(decay_mode))
    x = ctypes.c_double(0)
    y = ctypes.c_double(0)
    n = -1
    while n < g.GetN() - 1 and x.value < m:
        n += 1
        g.GetPoint(n, x, y)

    if n == 0:
        return y.value

    if x.value < m:
        return 0.0

    xprev = ctypes.c_double()
    yprev = ctypes.c_double()
    g.GetPoint(n - 1, xprev, yprev)
    return yprev.value + (y.value - yprev.value) / (x.value - xprev.value) * (m - xprev.value)

samp_names = {
    1:  "b_jpsi",
    2:  "b_psiprime",
    3:  "rho",
    4:  "omega",
    5:  "phi",
    6:  "pi0",
    7:  "eta",
    8:  "etaprime_photon",
    9:  "omega_pi0",
    10: "etaprime_omega",
    11: "jpsi",
    12: "psiprime",
    13: "ups1S",
    14: "ups2S",
    15: "ups3S",
}

points = []
for m in masses:
    xs = [get_xsec(i, m) for i in range(16)]

    total_xsec = xs[0]
    sorted_xs = sorted(zip(range(1,16),xs[1:]), key=lambda x:x[1], reverse=True)
    
    cum_xs = np.cumsum([x[1] for x in sorted_xs]) / total_xsec

    max_idx = max(0, np.argmax(cum_xs > 0.9999))

    print("\nmass =", m)
    for i, (dm, xs) in enumerate(sorted_xs[:max_idx+1]):
        Nevt = xs / total_xsec * N_target_events
        Nevt = max(Nevt, nevts_per_job)
        Nevt = int(round(Nevt / nevts_per_job) * nevts_per_job)
        subdir = os.path.join(outdir, "m_{0}".format(str(m).replace(".","p")), samp_names[dm])
        os.system("mkdir -p "+subdir)
        print("  {0:2d} {1:.3e} {2:.4f} {3:8d}".format(dm, xs, cum_xs[i], Nevt))
        points.append({"decay_mode":dm, "mass":m, "n_events":Nevt, "outdir":subdir})
        with open("blah.json", 'w') as fid:
            json.dump(points[-1], fid, indent=4, ensure_ascii=True)
        os.system("cp blah.json "+os.path.join(subdir,"metadata.json"))
        os.system("rm blah.json")
        

fout = open("../bsub_jobs.sh",'w')
fout.write("#!/bin/bash\n\n")

print("# SAMPLES:", len(points))
cmds = []
for p in points:
    njobs = int(p["n_events"] / nevts_per_job)
    for j in range(njobs):
        localname = "output_{0}_{1}_{2}.root".format(p["decay_mode"],p["mass"],j+1)
        final_name = os.path.join(p["outdir"], "output_{0}.root".format(j+1))
        cmd = "./runDecays -d {0} -o {1} -m {2} -n {3} -N {4} -e {5}".format(
            p["decay_mode"],
            localname,
            p["mass"],
            nevts_per_job,
            p["n_events"],
            nevts_per_job * j
        )
        fout.write("bsub " + cmd + "\n")
fout.close()

print("# JOBS:", len(cmds))

