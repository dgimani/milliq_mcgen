import ROOT as r
import array

fout = r.TFile("fakeProtonInput.root","RECREATE")
tout = r.TTree("Events", "Events")

nevents = 10
event = array.array('i', [0])
p4_p = r.TLorentzVector()
p4_m = r.TLorentzVector()
tout.Branch("p4_p", p4_p)
tout.Branch("p4_m", p4_m)
tout.Branch("event", event, "event/I")

for i in range(nevents):
    event[0] = i 
    E = 6800
    px = 1
    py = 0
    pz = 6799.999861739
    p4_p.SetPxPyPzE(px, py, pz, E)
    p4_m.SetPxPyPzE(px, py, pz, E)
    print(event)
    tout.Fill()

tout.Write()
fout.Close()

