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
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <regex>
#include <boost/variant.hpp>
#include <boost/any.hpp>

// defines
#define GetCurrentDir getcwd

// external includes
// Using JSON library: https://github.com/nlohmann/json
#include "clibs/json.hpp"
// include subprocess lib
#include "clibs/subprocess.hpp"
// include converter
#include "convert.c"



// define namespaces
using namespace std;
using namespace subprocess;


string getCwd( void ) {
	/*
	Get current work dir
	*/
	char buff[FILENAME_MAX];
	GetCurrentDir( buff, FILENAME_MAX );
	std::string current_working_dir(buff);
	return current_working_dir;
}


string sPhoenixPath = getCwd();


class httpServer{
	/*
	Class to start ROOT THttpServer instance for Phoenix Event Display 
	More info: https://github.com/root-project/jsroot/blob/master/docs/HttpServer.md
	*/
	public:
		httpServer(string webPath, string dataPath){
			/*
			Http Server constructor
			*/
			// server instance preparation
			prepareConnectionString();
			// init server
			this->s = new THttpServer(this->conString.c_str());
			// link dirs
			this->s->AddLocation(this->webPathLink.c_str(), webPath.c_str());
			this->s->AddLocation(this->datPathLink.c_str(), dataPath.c_str());
		}

		void stopServer(){
			// TODO: fix stop server to clear memory
			//delete this->s;
		}

		string getUrl(string geomFilePath, string objectFilePath){
			/*
			Get Phoenix event url
			*/
			// get base name of target files
			string fNameG = geomFilePath.substr(geomFilePath.find_last_of("/") + 1);
			string fNameO = objectFilePath.substr(objectFilePath.find_last_of("/") + 1);
			// set file paths in phoenix link folder
			string rPhoenixPathG = this->datPathLink.substr(1) + fNameG;
			string rPhoenixPathO = this->datPathLink.substr(1) + fNameO;
			return "http://127.0.0.1:" + to_string(this->port) + "/w/phoenix.html?geometry=" + rPhoenixPathG + "&objects=" + rPhoenixPathO;
		}

		void _setRandomPort(){
			/*
			Set random port for server
			*/
			srand (time (NULL));
			this->port = rand() % 60000 + 10000;
		}

		void prepareConnectionString(){
			/*
			Prepare connection string
			*/
			_setRandomPort();
			this->conString = regex_replace(this->conString, regex("<port>"), to_string(port));
		}

		// server instance
		THttpServer* s;
		// server init param string 
		string conString = "http:<port>;ro";
		int port;
		// link dir names
		string webPathLink = "/w/";
		string datPathLink = "/data/";
};


string getFilePath(const string& path) {
	/*
	Get folder path of target file
	*/
	auto pathEnd = path.find_last_of("/");
	auto pathName = pathEnd == string::npos ? path : path.substr(0, pathEnd);
	return pathName;
}


void phd(TString inFile = "~/bmnroot/macro/run/bmnsim.root", bool removeConvertedFiles = true){
	// Author: Ilia Zavodevkin <zedcode.05@gmail.com> 2022-03-28
	//
	////////////////////////////////////////////////////////////////////////////////
	//                                                                            //
	// phd.C (PHoenix-event-Display)                                              //
	//                                                                            //
	// An example how to run bm@n .root converter and load data to Phoenix-event  //
	//                                                                            //
	////////////////////////////////////////////////////////////////////////////////
	
	// run converter and get result export paths
	//  [0] - idx of geometry file path
	//  [1] - idx of objects file path
	string* exportPaths = convert(inFile);
	// out paths
	cout << "[*] Input files:" << endl;
	cout << ">>  " << exportPaths[0] << endl;
	cout << ">>  " << exportPaths[1] << endl;

	// set server data path
	string sDataPath = getFilePath(exportPaths[0]);

	// Phoenix web server
	httpServer hs = httpServer(sPhoenixPath, sDataPath);

	// set browser cmd string with event url
	string runBrCmd = "xdg-open " + hs.getUrl(exportPaths[0], exportPaths[1]);
	cout << "[*] Open browser with Phoenix url: " << runBrCmd << endl;
	// start browser in subprocess
	subprocess::popen cmd("xdg-open", { hs.getUrl(exportPaths[0], exportPaths[1]).c_str() });

	// log: pause
	do { cout << "[!] Program is paused!\n" << "[!] Press Enter to continue..\n"; } while (cin.get() != '\n');

	// stop server
	hs.stopServer();
	// close subprocess
	cmd.close();
	cout << "[*] Web server: stopped" << endl;

	// remove converted files if removeConvertedFiles == true
	if (removeConvertedFiles == true){
		for (Int_t idxFile = 0; idxFile < 2; idxFile++){
			string rmCmd = "rm " + exportPaths[idxFile];
			system( rmCmd.c_str() );
		}
		cout << "[+] Delete converted files: success!" << endl;
	}

    return 0;
}
