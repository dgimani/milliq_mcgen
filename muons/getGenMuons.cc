#include "TFile.h"
#include "tdrstyle.C"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

/* 

To RUN:

   root -l -q -b 'getGenMuons.cc("WJets")'
   root -l -q -b 'getGenMuons.cc("DY")'

*/

// WJetsToLNu_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Winter22NanoAOD-122X_mcRun3_2021_realistic_v9-v1/NANOAODSIM
TString fn_WJets_nanoAOD = "./data/run3/1f48109d-c7d9-416f-8f6b-71906ce61d45.root";
// DYJetsToLL_M-50_TuneCP5_13p6TeV-madgraphMLM-pythia8/Run3Winter22NanoAOD-122X_mcRun3_2021_realistic_v9_ext2-v1/NANOAODSIM
TString fn_DY_nanoAOD = "./data/run3/fb32c113-0866-4653-b462-a7c99263ef13.root";
// Run 2 WJets pT
TString fn_muon_pt_WJets_run2 = "./data/run2/WJets_CutPtSpect_v2.root";
// Run 2 DY pT
TString fn_muon_pt_DY_run2 = "./data/run2/DY_CutPtSpect_v2.root";

std::chrono::time_point<std::chrono::system_clock> t_old;
std::vector<double> deq;

// copied from "../MCPTree/MCPTree.h"
void progress( int curr, int tot, int period, unsigned int smoothing) {
    if(curr%period == 0) {
        auto now = std::chrono::system_clock::now();
        double dt = ((std::chrono::duration<double>)(now - t_old)).count();
        t_old = now;
        // if (deq.size() >= smoothing) deq.pop_front();                                                                                                                                                            
        if (deq.size() >= smoothing) deq.erase(deq.begin());
        deq.push_back(dt);
        double avgdt = std::accumulate(deq.begin(),deq.end(),0.)/deq.size();
        float prate = (float)period/avgdt;
        float peta = (tot-curr)/prate;
        if (isatty(1)) {
            float pct = (float)curr/(tot*0.01);
            if( ( tot - curr ) <= period ) pct = 100.0;
            printf("\015\033[32m ---> \033[1m\033[31m%4.1f%% \033[34m [%.3f kHz, ETA: %.0f s] \033[0m\033[32m  <---\033[0m\015 ", pct, prate/1000.0, peta);
            if( ( tot - curr ) > period ) fflush(stdout);
            else cout << endl;
        }
    }
}

bool Has_Boson_Ancestor(int ipart, int motherID,
			const TTreeReaderArray<int>& genpart_pdgId,
			const TTreeReaderArray<int>& genpart_motherIdx) {

    bool has_boson_ancestor = false;
    int parent_idx = genpart_motherIdx[ipart];

    while (parent_idx != -1) {
	if (abs(genpart_pdgId[parent_idx]) == motherID) {
	    has_boson_ancestor = true;
	    break;
	}
	if (abs(genpart_pdgId[parent_idx]) != 13) break; // ignore muon from pion etc, sometimes in pythia muon->muon
	parent_idx = genpart_motherIdx[parent_idx];
    }

    return has_boson_ancestor;
}

void getGenMuons(TString processName /* WJets or DY */) {

    // Open the CMS nanoAOD file and access the Events tree
    TString fn_nanoAOD = "";
    if (processName == "WJets") fn_nanoAOD = fn_WJets_nanoAOD;
    if (processName == "DY") fn_nanoAOD = fn_DY_nanoAOD;
    
    TFile *file = TFile::Open(fn_nanoAOD);
    TTreeReader reader("Events", file);
    Long64_t nEntries = reader.GetEntries(true);

    // Define the TTreeReaderValue pointers to the Muon_pt, Muon_eta, and Muon_phi branches
    TTreeReaderArray<float> genpart_pt(reader, "GenPart_pt");
    TTreeReaderArray<float> genpart_eta(reader, "GenPart_eta");
    TTreeReaderArray<float> genpart_phi(reader, "GenPart_phi");
    TTreeReaderArray<int> genpart_status(reader, "GenPart_status");
    TTreeReaderArray<int> genpart_pdgId(reader, "GenPart_pdgId");
    TTreeReaderArray<int> genpart_motherIdx(reader, "GenPart_genPartIdxMother");

    // Create histograms to plot the eta and pT distribution of the selected muon
    TH1F* h_eta = new TH1F("h_eta", "Muon Eta Distribution", 50, -3.0, 3.0);
    TH1F* h_phi = new TH1F("h_phi", "Muon Phi Distribution", 50, -4.0, 4.0);
    TH1F* h_pt = new TH1F("h_pt", "Muon PT Distribution", 40, 0, 100);

    // Loop over the events in the tree
    int counter = 0;
    while (reader.Next()) {

      //If (counter > 100000) break;
      //if (counter % 10000) cout << "Processed " << counter << " events" << endl;

	progress(counter, nEntries, 1000, 300);

	// Loop over the generator level particles in the event
	for (int ipart = 0; ipart < genpart_pdgId.GetSize(); ++ipart){

	    int motherID = -999;	  
	    if (processName == "WJets") motherID = 24;
	    if (processName == "DY") motherID = 23;

	    // Select only the status 1 muons that decay from the W or Z boson
	    if ( !(abs(genpart_pdgId[ipart]) == 13 && genpart_status[ipart] == 1) ) continue;

	    bool has_boson_ancestor = Has_Boson_Ancestor(ipart, motherID, genpart_pdgId, genpart_motherIdx);

	    float pt = genpart_pt[ipart];
	    float eta = genpart_eta[ipart];
	    float phi = genpart_phi[ipart];

	    // If the particle has a W/Z boson ancestor
	    if (has_boson_ancestor) {

		//printf("Generator level muon pT: %f, eta: %f, phi: %f\n", genpart_pt[ipart], genpart_eta[ipart], genpart_phi[ipart]);

		// Get the momentum of the selected muon
		TLorentzVector muon;
		muon.SetPtEtaPhiM(pt, eta, phi, 0.10566);

		// Fill the histograms with the eta and pT of the selected muon
		h_eta->Fill(muon.Eta());
		h_phi->Fill(muon.Phi());
		h_pt->Fill(muon.Pt());
	    }

	}
	counter++;
    }

    // Get pT distribution used for Run 2
    TString fn_pt_run2 = "";
    if (processName == "WJets") fn_pt_run2 = fn_muon_pt_WJets_run2;
    if (processName == "DY") fn_pt_run2 = fn_muon_pt_DY_run2;

    TFile *file_run2 = TFile::Open(fn_pt_run2);
    TH1F* h_pt_run2 = (TH1F*)file_run2->Get("pt");

    setTDRStyle();
   
    // Make a TCanvas, split it in 3 (pT, eta, phi)
    TCanvas* c = new TCanvas("c","",800,600);
    TPad* pad1 = new TPad("pad1", "Pad 1", 0.0, 0.5, 1.0, 1.0);
    TPad* pad2 = new TPad("pad2", "Pad 2", 0.0, 0.0, 0.5, 0.5);
    TPad* pad3 = new TPad("pad3", "Pad 3", 0.5, 0.0, 1.0, 0.5);
    pad1->Draw();
    pad2->Draw();
    pad3->Draw();

    pad1->cd();
    h_pt->Draw();
    h_pt->GetXaxis()->SetTitle("pT (GeV)");

    pad2->cd();
    h_eta->Draw();
    h_eta->GetXaxis()->SetTitle("#eta");

    pad3->cd();
    h_phi->Draw();
    h_phi->GetXaxis()->SetTitle("#phi");

    TString fn_plot = "./plots/GenMuon_pt_eta_phi_" + processName + ".pdf";
    c->SaveAs(fn_plot);

    //h_pt->Draw("pe");
    //h_pt->GetXaxis()->SetTitle("pT (GeV)");
    //h_pt->Scale(h_pt_run2->Integral()/h_pt->Integral());
    //h_pt_run2->Draw("pe same");
    //h_pt_run2->SetLineColor(2);

    // Create a TLegend and add entries to it
    /*
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9); // x1,y1,x2,y2 (in normalized coordinates)
    legend->AddEntry(h_pt, "Run 3 sample", "l");
    legend->AddEntry(h_pt_run2, "Run 2 sample", "l");
    legend->Draw("same");
    */
}

