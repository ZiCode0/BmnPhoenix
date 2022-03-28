// root includes
#include <TGeoManager.h>
#include <TROOT.h>
#include <TFile.h>
#include <TChain.h>

// fair includes
#include <FairGeoParSet.h>

// bmn includes
//#include "/home/zed/bmnroot/bmndata/CbmMCTrack.h"

// c++ includes
#include <iostream>
#include <fstream>
#include <string.h>
#include <regex>
#include <boost/variant.hpp>
#include <boost/any.hpp>

// external includes
// Using JSON library: https://github.com/nlohmann/json
#include "clibs/json.hpp"

// define namespaces
using namespace std;
using json = nlohmann::ordered_json;

// home dir path
const char* home = getenv("HOME");


double_t* _getPointCoordinates(const char* branchNameOfPoint, TClonesArray* currentBranch, Int_t idx){
	// SiliconPoint
	if (strcmp(branchNameOfPoint, "SiliconPoint") == 0){
		BmnSiliconPoint* point = (BmnSiliconPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// FDPoint
	else if (strcmp(branchNameOfPoint, "FDPoint") == 0){
		BmnFDPoint* point = (BmnFDPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// MWPCPoint
	else if (strcmp(branchNameOfPoint, "MWPCPoint") == 0){
		BmnMwpcPoint* point = (BmnMwpcPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// BdfPoint
	else if (strcmp(branchNameOfPoint, "BdPoint") == 0){
		BmnBdPoint* point = (BmnBdPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// StsPoint
	// else if (strcmp(branchNameOfPoint, "StsPoint") == 0){
	// 	CbmStsPoint* point = (CbmStsPoint*) currentBranch->UncheckedAt(idx);
	// 	return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	// }
	// CSCPoint
	else if (strcmp(branchNameOfPoint, "CSCPoint") == 0){
		BmnCSCPoint* point = (BmnCSCPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// TOF400Point
	else if (strcmp(branchNameOfPoint, "TOF400Point") == 0){
		BmnTOF1Point* point = (BmnTOF1Point*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// TOF700Point
	else if (strcmp(branchNameOfPoint, "TOF700Point") == 0){
		BmnTOFPoint* point = (BmnTOFPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// DCHPoint
	else if (strcmp(branchNameOfPoint, "DCHPoint") == 0){
		BmnDchPoint* point = (BmnDchPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// EcalPoint
	else if (strcmp(branchNameOfPoint, "EcalPoint") == 0){
		BmnEcalPoint* point = (BmnEcalPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// ZdcPoint
	else if (strcmp(branchNameOfPoint, "ZdcPoint") == 0){
		BmnZdcPoint* point = (BmnZdcPoint*) currentBranch->UncheckedAt(idx);
		return new double_t[3]{point->GetX(), point->GetY(), point->GetZ()};
	}
	// empty coords
	return new double_t[3]();
}


string _replaceHomeDirInPath(string targetPath){
	targetPath = regex_replace(targetPath, regex("~"), home);
	return targetPath;
}


json _getEventHits(TChain* inpChain, string* branchTargetNames, Int_t idxEvent){
	json rHits;

	// branch loop
    for (Int_t iBr = 0; iBr < sizeof(branchTargetNames); iBr++) {
		// get current branch name
		string currentBranchName = branchTargetNames[iBr];
		cout << "   >> Parsing branch: " << currentBranchName << ".." << endl;

		// link branch
		TClonesArray* currentBranch = nullptr;
		inpChain->SetBranchAddress(currentBranchName.c_str(), &currentBranch);

		// select event entry
		inpChain->GetEntry(idxEvent);
		// init event hits array []
		json jEventHits = json::array();

		// points loop
		for (Int_t iTrack = 0; iTrack < currentBranch->GetEntriesFast(); iTrack++) {
			// get point object
			double_t* t = _getPointCoordinates(currentBranchName.c_str(), currentBranch, iTrack);
			// add buffer
			jEventHits.push_back({t[0], t[1], t[2]});
			delete t;
		}
		rHits["Hits"][currentBranchName] = jEventHits;
	}
	return rHits;
}


json _makeEventsObjectsAsJson(TChain* inpChain, string* branchTargetNames){
	json rEvents;

	// parse events
	for (Int_t idxEvent = 0; idxEvent < inpChain->GetEntries(); idxEvent++) {
		//
		cout << "[*] Parsing event: " << idxEvent << ".." << endl;
		// set event metadata
		string eventName = "Event #" + to_string(idxEvent);
		int runNumber = 0;
		// fill event header data
		rEvents[eventName]["event number"] = idxEvent;
		rEvents[eventName]["run number"] = runNumber;
		// get event hits
		json rEventHits = _getEventHits(inpChain, branchTargetNames, idxEvent);
		rEvents[eventName]["Hits"] = rEventHits["Hits"];
	}	
	return rEvents;
}


string ExportGeometry(TString inFile){
	// init vars
	FairGeoParSet* f_sim_fgps;
	TGeoManager* f_sim_gm;
	// make string out path
	string outFileS( string(inFile) + "-geometry.root" );
	// replace "~"" with "$HOME" path
	outFileS = _replaceHomeDirInPath(outFileS);
	// make char out path
	const char* outFile = outFileS.c_str();
	// // make conversion
	TFile f_sim(inFile);
	// get FairGeoParSet object
	f_sim_fgps = (FairGeoParSet*)f_sim.Get("FairGeoParSet");
	// get TGeoManager class object to extrract geometry
	f_sim_gm = (TGeoManager*)f_sim_fgps->GetGeometry();
	// make export to .root file
	f_sim_gm->Export(outFile);
	// log: success
	cout << "[+] Export geometry success!" << endl;
	return outFileS;
}


string ExportObjects(TString inFile){
	// set target branches to get Hits objects
	// hit branches:
	//		SiliconPoint, FDPoint, MWPCPoint, BdfPoint, CSCPoint,
	//		TOF400Point, TOF700Point, DCHPoint, EcalPoint, ZdcPoint
	string hitBranchTargetNames[] = {"SiliconPoint", "FDPoint", "MWPCPoint", "BdPoint", "CSCPoint",
									 "TOF400Point", "TOF700Point", "DCHPoint", "EcalPoint", "ZdcPoint"};
	// create an empty result json 
	json resultJson;

	// make string out path
	string outFileObjS( string(inFile) + "-objects.json" );
	// replace "~"" with "$HOME" path
	outFileObjS = _replaceHomeDirInPath(outFileObjS);
	// make char out path
	const char* outFileObjC = outFileObjS.c_str();

	// init out result file
	ofstream outFileObj(outFileObjC);

	// init TChain var
	TChain* out = new TChain("bmndata");
	// read TChain data
	out->Add(inFile.Data());
	
	// get Events qith Hits in JSON format
	resultJson = _makeEventsObjectsAsJson(out, hitBranchTargetNames);
	
	// output result json 	
	//cout << resultJson.dump(/* 4 */) << endl;
	// // write result json
	outFileObj << resultJson.dump(4) << endl;
	outFileObj.close();
	// log: success
	cout << "[+] Export object success!" << endl;
	return outFileObjS;
}


string* convert(TString inFile = "~/bmnroot/macro/run/bmnsim.root"){
	// Author: Ilia Zavodevkin <zedcode.05@gmail.com> 2022-03-25

	////////////////////////////////////////////////////////////////////////////////
	//                                                                            //
	// convert.C                                                                  //
	//                                                                            //
	// An example how to convert bm@n .root data to Phoenix-event                 //
	//                                                                            //
	////////////////////////////////////////////////////////////////////////////////
	string exportGeometryFilePath = ExportGeometry(inFile);
	string exportObjectsFilePath = ExportObjects(inFile);
	// return result as string path array
	string* resultPaths = new string[2] {exportGeometryFilePath, exportObjectsFilePath};
	return resultPaths;
}
