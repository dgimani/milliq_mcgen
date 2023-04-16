#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

void getGenMuons() {
    // Open the CMS nanoAOD file and access the Events tree
    // WJets
    TFile *file = TFile::Open("/Users/meihualin/Projects/milliqan/milliq_mcgen/muons/data/run3/1f48109d-c7d9-416f-8f6b-71906ce61d45.root");
    TTreeReader reader("Events", file);

    // Define the TTreeReaderValue pointers to the Muon_pt, Muon_eta, and Muon_phi branches
    TTreeReaderArray<float> genpart_pt(reader, "GenPart_pt");
    TTreeReaderArray<float> genpart_eta(reader, "GenPart_eta");
    TTreeReaderArray<float> genpart_phi(reader, "GenPart_phi");
    TTreeReaderArray<int> genpart_status(reader, "GenPart_status");
    TTreeReaderArray<int> genpart_pdgId(reader, "GenPart_pdgId");
    TTreeReaderArray<int> genpart_motherIdx(reader, "GenPart_genPartIdxMother");

    // Create histograms to plot the eta and pT distribution of the selected muon
    TH1F* h_eta = new TH1F("h_eta", "Muon Eta Distribution", 50, -3.0, 3.0);
    TH1F* h_pt = new TH1F("h_pt", "Muon PT Distribution", 40, 0, 100);

    // Loop over the events in the tree
    int counter = 0;
    while (reader.Next()) {

      //If (counter > 100000) break;
      if (counter % 1000) cout << "Processed " << counter << " events" << endl;

	// Loop over the generator level particles in the event
	for (int ipart = 0; ipart < genpart_pdgId.GetSize(); ++ipart){


	    // Select only the status 1 muons that decay from the W boson
	    
	    if (abs(genpart_pdgId[ipart]) == 13 && genpart_status[ipart] == 1){

                bool has_w_ancestor = false;
                int parent_idx = genpart_motherIdx[ipart];

                while (parent_idx != -1) {
                    if (abs(genpart_pdgId[parent_idx]) == 24) {
                        has_w_ancestor = true;
                        break;
                    }
		    if (abs(genpart_pdgId[parent_idx]) != 13) break;
                    parent_idx = genpart_motherIdx[parent_idx];
                }
		
		//if (abs(genpart_pdgId[parent_idx]) == 24) has_w_ancestor = true;

		float pt = genpart_pt[ipart];
		float eta = genpart_eta[ipart];
		float phi = genpart_phi[ipart];
                // If the particle has a W boson ancestor, print out its pT, eta, and phi
                if (has_w_ancestor && pt > 10 && abs(eta) > 0.01 && abs(eta) < 0.21) {

		  //printf("Generator level muon pT: %f, eta: %f, phi: %f\n", genpart_pt[ipart], genpart_eta[ipart], genpart_phi[ipart]);

		    // Get the momentum of the selected muon
		    TLorentzVector muon;
		    muon.SetPtEtaPhiM(pt, eta, phi, 0.10566);

		    // Fill the histograms with the eta and pT of the selected muon
		    h_eta->Fill(muon.Eta());
		    h_pt->Fill(muon.Pt());
                }

	    }
	    
	}

	counter++;
    }

    // Close the file
    //file->Close();


    TFile *file_run2 = TFile::Open("/Users/meihualin/Projects/milliqan/milliq_mcgen/muons/data/run2/WJets_CutPtSpect_v2.root");
    TH1F* h_pt_run2 = (TH1F*)file_run2->Get("pt");
    
    TCanvas* c = new TCanvas("c","",800,800);
    h_pt->Draw("pe");
    h_pt->GetXaxis()->SetTitle("pT (GeV)");
    h_pt->Scale(h_pt_run2->Integral()/h_pt->Integral());
    h_pt_run2->Draw("pe same");
    h_pt_run2->SetLineColor(2);
    h_pt->SetStats(0);
    h_pt_run2->SetStats(0);

    // Create a TLegend and add entries to it
    TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9); // x1,y1,x2,y2 (in normalized coordinates)
    legend->AddEntry(h_pt, "Run 3 sample", "l");
    legend->AddEntry(h_pt_run2, "Run 2 sample", "l");
    legend->Draw("same");

    c->SaveAs("test.png");
    c->SaveAs("test.pdf");
  
}

