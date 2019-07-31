#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

#include "../../decayMCP/MCPTree/MCPTree.h"

typedef LorentzPtEtaPhiMf LorentzVector;

float MCP_ETAMIN = 0.16 - 0.08;
float MCP_ETAMAX = 0.16 + 0.08;
float MCP_PHIMIN = -0.03;
float MCP_PHIMAX = 1.0;

// check if an mCP 4-vector is within pre-defined eta/phi bounds
// note that the phi selection gets inverted based on mCP charge sign,
// since they curve opposite directions in magnetic field
bool WithinBounds(LorentzVector p4, int q){
    // q is +1 or -1 to indicate sign of charge
    if(q > 0)
        return 
            p4.eta() >= MCP_ETAMIN &&
            p4.eta() <= MCP_ETAMAX &&
            p4.phi() >= MCP_PHIMIN &&
            p4.phi() <= MCP_PHIMAX;
    else
        return 
            p4.eta() >= MCP_ETAMIN &&
            p4.eta() <= MCP_ETAMAX &&
            p4.phi() >= -MCP_PHIMAX &&
            p4.phi() <= -MCP_PHIMIN;        
}

int main(int argc, char **argv){

    char opt;
    bool help = false;
    float m_mCP = 0.01;
    int evt_offset = 0;
    uint n_events_total = 0;
    string output_name = "", input_name = "";
    while((opt = getopt(argc, argv, ":hi:o:N:e:")) != -1) {
        switch(opt){
        case 'h':
            help = true;
            break;
        case 'i':
            input_name = string(optarg);
            break;
        case 'o':
            output_name = string(optarg);
            break;
        case 'N':
            n_events_total = atoi(optarg);
            break;
        case 'e':
            evt_offset = atoi(optarg);
            break;
        case '?':
            std::cout << "\nWARNING: unrecognized option " << argv[optind-1] << std::endl;
            break;
        case ':':
            std::cout << "\nERROR: option " << argv[optind-1] << " requires a value\n";
            help = true;
            break;
        }
    }

    if(help || output_name=="" || n_events_total <= 0 || evt_offset < 0){
        std::cout << "\nusage:\n";
        std::cout << "    " << argv[0] << " -i infile -o outfile -N n_events_total [-e evtnum_offset=0]\n\n";
        return 1;
    }    

    MCP_PHIMIN = -0.03;
    MCP_PHIMAX = max(0.4, 0.35 - 0.85*log10(m_mCP));
    float deta = m_mCP >= 0.999 ? 0.06 : 0.12;
    MCP_ETAMIN = 0.16 - deta;
    MCP_ETAMAX = 0.16 + deta;
    
    MCPTree outtree;

    TFile *fout = new TFile(output_name.c_str(), "RECREATE");
    outtree.Init();

    outtree.n_events_total = n_events_total;
    outtree.decay_mode = 0;
    outtree.BR_q1 = 1.0;
    outtree.filter_eff = 1.0;
    outtree.weight = 1.0;
    outtree.weight_up = 1.0;
    outtree.weight_dn = 1.0;
    outtree.parent_pdgId = 22;
    outtree.mCP_etamin = MCP_ETAMIN;
    outtree.mCP_etamax = MCP_ETAMAX;
    outtree.mCP_phimin = MCP_PHIMIN;
    outtree.mCP_phimax = MCP_PHIMAX;    

    outtree.tree()->SetBranchStatus("filter_eff", 0); // turn off and fill later once we're done

    ifstream fin;
    fin.open(input_name);
    if( !fin ){
        std::cout << "Couldn't open input file!\n";
        return 1;
    }
    float px1, py1, pz1, E1, px2, py2, pz2, E2;
    if( !(fin >> outtree.xsec) ){
        std::cout << "Couldn't read xsec from file!\n";
        return 1;
    }
    int npass=0, ntotal=0;
    while(fin >> px1 >> py1 >> pz1 >> E1 >> px2 >> py2 >> pz2 >> E2){
        outtree.event = npass + evt_offset;
        outtree.p4_p->SetPxPyPzE(px1, py1, pz1, E1);
        outtree.p4_m->SetPxPyPzE(px2, py2, pz2, E2);
        outtree.parent_p4->SetPxPyPzE(px1+px2, py1+py2, pz1+pz2, E1+E2);        
        ntotal++;
        if(WithinBounds(*outtree.p4_p, 1) || WithinBounds(*outtree.p4_m, -1)){
            outtree.Fill();
            npass++;
        }
    }

    // fill the filter_eff branch
    outtree.filter_eff = (float)npass / ntotal;
    std::cout << "    Total attempted events: " << ntotal << std::endl;
    std::cout << "Computed filter efficiency: " << outtree.filter_eff << std::endl;
    outtree.filter_eff = 1.0; // since n_events_total is set to the PRE-filter nevents, it is already accounted for
    outtree.tree()->SetBranchStatus("filter_eff", 1);
    TBranch *beff = outtree.tree()->GetBranch("filter_eff");
    for(uint i=0; i<npass; i++){
        outtree.tree()->GetEntry(i);
        beff->Fill();
    }
    std::cout << "\n";

    outtree.Write(fout);

    fout->Close();
    return 0;
}
