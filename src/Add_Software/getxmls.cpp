/*
getxmls
Copyright (C) 2004 Pluto, Inc., a Florida Corporation

www.plutohome.com		

Phone: +1 (877) 758-8648

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

See the GNU General Public License for more details.
*/

#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>

#include "../PlutoUtils/uuencode.h"
#include "../DCE/Logger.h"
#include "../PlutoUtils/StringUtils.h"

#include "PackageXMLParser.h"
#include "DatabaseHelper.h"
#include "XMLFinder.h"

#define MaxBuf 17
#define UserAgent "--user-agent=\"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.0.4) Gecko/20060406 Firefox/1.5.0.4 (Debian-1.5.dfsg+1.5.0.4-1)\""
namespace DCE
{
	Logger *g_pPlutoLogger;
}
using namespace DCE;
using namespace std;

const string templBegin[2]={"<a class=l href=\"","<em class=yschurl>"};
const string templEnd[2]={"\">","</em>"};

void ProcessXML(string sFileName, vector<string> &virusFree)
{
	//g_pPlutoLogger->Write(LV_WARNING, "Processing XML %s", sFileName.c_str());
	
	string sXmlData;
	FileUtils::ReadTextFile(sFileName, sXmlData);
	sXmlData = StringUtils::Replace(sXmlData, "&", "&amp;");

	PackageXMLParser parser;
	parser.Parse(sXmlData);
	const list<PackageInfo>& listPackages = parser.GetParsedData();

	DatabaseHelper dbhelper;
	dbhelper.ProcessPackages(listPackages, virusFree);
}

bool FetchURL(string sUrl, string sOutputFile, string sUserAgent)
{
	string sCmd = string("wget --timeout 60 --tries=1 -q ")+UserAgent+" -O " + sOutputFile + " \""+sUrl+"\"";
//	cout << "DEBUG[FetchURL] " << sCmd.c_str() << endl;
	return !system(sCmd.c_str());
}

int main(int argc, char *argv[])
{
	g_pPlutoLogger = new FileLogger(stdout);
	int res;
	bool bError=false;
	string host="dcerouter",user="root",passwd="",db="pluto_main";
	vector<string> url;
	url.push_back("http://www.google.com/search?q=10faostb");
	for(int i=1;i<argc;i++){
		if(argv[i][0]!='-'){
			cerr<<"Unknown option"<<endl;
			bError=true;
			break;
		}
		char c=argv[i][1];
		switch(c){
			case 'h':host=argv[++i];break;
			case 'u':user=argv[++i];break;
			case 'p':passwd=argv[++i];break;
			case 'd':db=argv[++i];break;
			case 'U':url.push_back(argv[++i]);break;
			default:cerr<<"Unknown option"<<endl;
				bError=true;
		}
	}
	if(bError){
		cout << "Usage: "<< argv[0]
		<< " [-h MySql hostname] [-u MySql username] " << endl
			<< " [-p MySql password] [-d MySql database] " << endl
			<< " [-U URL of google search]" << endl;
		return false;
	}
	
	// getting list of virus free packages
	cout << "Getting virus-free packages list" << endl;
	res = FetchURL("http://linuxmce.com/virus_free.php", "/tmp/virusfree.list", UserAgent);
	vector<string> virusFree;
	// loading virus-free list
	if (res)
	{
		ifstream fin("/tmp/virusfree.list");
		while (fin)
		{
			char buf[1024];
			fin.getline(buf, 1024);
			virusFree.push_back(buf);
		}
	}
	
	for(size_t noEngine=0;noEngine<url.size();noEngine++)
	{
		cout << "Processing search engine: " << url[noEngine] << endl;
		res = FetchURL(url[noEngine], "/tmp/search.html", UserAgent);
		if (res)
		{
			string link,packageName,version;
			vector<string> vectUrls;
			XMLFinder::FindURLs("/tmp/search.html", vectUrls);
			for(vector<string>::iterator it = vectUrls.begin(); it != vectUrls.end(); ++it)
			{
				string sUrl = *it;
				vector<string> vectXMLs;
				
				cout << "Analyzing URL: " << sUrl << endl;
				
				if (StringUtils::EndsWith(sUrl, ".xml", true))
				{
					cout << "URL is XML, adding to processing queue" << endl;
					vectXMLs.push_back(sUrl);
				}
				else
				{
					cout << "URL is non-XML, scanning for links to XML" << endl;
					sUrl = StringUtils::Replace(sUrl, "\"", "\\\"");
					res = FetchURL(sUrl, "/tmp/out.html", UserAgent);
					if (res)
					{
					XMLFinder::FindXMLs("/tmp/out.html", vectXMLs);
					}
				}
				
				for(vector<string>::iterator itx = vectXMLs.begin(); itx != vectXMLs.end(); ++itx)
				{
					string sXmlUrl = *itx;
					cout << "XML to download : " << sXmlUrl << endl;

					sXmlUrl = StringUtils::Replace(sXmlUrl, "\"", "\\\"");
					bool res = FetchURL(sXmlUrl, "/tmp/out.xml", UserAgent);
					if(res)
						ProcessXML("/tmp/out.xml", virusFree);
				}
			}
		}
	}

	return true;
}
