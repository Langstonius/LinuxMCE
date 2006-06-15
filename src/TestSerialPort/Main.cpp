#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"
#include "Generic_Serial_Device/IOUtils.h"
#include "DCEConfig.h"
#include "Logger.h"
#include "Serial/SerialPort.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <list>

#ifdef WIN32
#include <direct.h>
#include <conio.h>
#define chdir _chdir  // Why, Microsoft, why?
#define mkdir _mkdir  // Why, Microsoft, why?
#else

#endif

#define  VERSION "<=version=>"

using namespace std;
using namespace DCE;
DCEConfig dceConfig;

namespace DCE
{
	Logger *g_pPlutoLogger;
}

string ParseHex(string sInput);
string StripHex(const char *pBuffer,int Length);

bool GetBlockToSend(string &sBlock,string &sTransmitString,string::size_type &pos);

int main(int argc, char *argv[])
{

	g_pPlutoLogger = new FileLogger("/var/log/pluto/TestSerialPort.log");

	string sPort,sTransmitString,sSearchString,sMessage;
	bool bHardwareFlowControl=false;
	unsigned int iTimeout=30,iBaud=9600;
	eParityBitStop _eParityBitStop = epbsN81;

	bool bError=false;
	char c;

	for(int optnum=1;optnum<argc;++optnum)
	{
		c=argv[optnum][1];
		switch (c)
		{
		case 'p':
			sPort = argv[++optnum];
			break;
		case 'P':
			{
				string sPortSettings = StringUtils::ToUpper(argv[++optnum]);
				if( sPortSettings=="O81" )
					_eParityBitStop = epbsO81;
				else if( sPortSettings=="E81" )
					_eParityBitStop = epbsE81;
			}
			break;
		case 't':
			sTransmitString = argv[++optnum];
			break;
		case 's':
			sSearchString = ParseHex(argv[++optnum]);
			break;
		case 'm':
			sMessage = argv[++optnum];
			break;
		case 'i':
			iTimeout = atoi(argv[++optnum]);
			break;
		case 'b':
			iBaud = atoi(argv[++optnum]);
			break;
		case 'h':
			bHardwareFlowControl=true;
			break;
		default:
			cout << "Unknown: " << argv[optnum] << endl;
			bError=true;
			break;
		};
	}

	if ( bError || (sPort.size()==0 || sTransmitString.size()==0) )
	{
		cout << "TestSerialPort, v." << VERSION << endl
			<< "Usage: TestSerialPort [-p port] [-P N81|E81|O81] [-t transmit string]" << endl
			<< "[-s Search String] [-m message to log] [-i Timeout] [-b baud] [-h]" << endl
			<< "strings can include: \\xx (xx is a hex char), \\r and \\n, and to delay x ms, \\xm" << endl;

		exit(0);
	}

	char pBuffer[5000]="";
	size_t sReceived=0;

	g_pPlutoLogger->Write(LV_STATUS,"Starting: %s, port %s timeout %d",sMessage.c_str(),sPort.c_str(),iTimeout);
	try
	{
		CSerialPort serialPort(sPort,iBaud,_eParityBitStop,bHardwareFlowControl);
		string::size_type pos=0;
		while(true)
		{
			string sBlock;
			bool bResult = GetBlockToSend(sBlock,sTransmitString,pos);
			if( !bResult )
				break;
			if( sTransmitString=="\\BREAK\\" )
				int k=2;// Send Break;
			else
			{
				sBlock = ParseHex(sBlock);
				g_pPlutoLogger->Write(LV_STATUS,"send: %s",IOUtils::FormatHexAsciiBuffer(sBlock.c_str(),sBlock.size(),"31").c_str());
				serialPort.Write((char *) sBlock.c_str(),sBlock.size());
				serialPort.Flush();
			}
		}

		sReceived = serialPort.Read(pBuffer,5000,iTimeout*1000);

		if( sReceived==0 )
			g_pPlutoLogger->Write(LV_STATUS,"recv: timeout, nothing received");
		else
		{
			g_pPlutoLogger->Write(LV_STATUS,"recv: %s",IOUtils::FormatHexAsciiBuffer(pBuffer,sReceived,"33").c_str());
			cout << StripHex(pBuffer,sReceived) << endl;
		}	
	}
	catch(string sError)
	{
		g_pPlutoLogger->Write(LV_STATUS,"Exception: %s",sError.c_str());
	}

	if( sReceived==0 )
	{
		g_pPlutoLogger->Write(LV_STATUS,"Didn't receive anything");
		return 1;
	}

	if( sSearchString.size() )
	{
		int Length = sSearchString.size();
		if( Length>sReceived )
		{
			g_pPlutoLogger->Write(LV_STATUS,"Search string not found");
			return 1;
		}

		const char *pSearch = sSearchString.c_str();
		char *pEnd = pBuffer + sReceived - Length;
		for(char *p=pBuffer;p<=pEnd;++p)
			if( memcmp(p,pSearch,Length)==0 )
			{
				g_pPlutoLogger->Write(LV_STATUS,"Search string found");
				return 0;
			}

		g_pPlutoLogger->Write(LV_STATUS,"Search string not found");
		return 1;
	}
	
	return 0;
}

string ParseHex(string sInput)
{
    string::size_type s=0;
    while( ( s=sInput.find( '\\', s ) ) != string::npos ) // search for the first apperence of the back slash
	{
		if( sInput[s+1]=='\\' )  // It's an escaped back slash.  Convert to a single one
	        sInput.replace( s, 2, "\\" );
		else if( sInput[s+1]!='s' ) // \s precedes a special command.  We don't handle that here
		{
			if( sInput[s+1]=='r' )
				sInput.replace( s, 2, "\r" );
			else if( sInput[s+1]=='n' )
				sInput.replace( s, 2, "\n" );
			else
			{
				int iValue;
				string sValue = sInput.substr(s+1,2);
				sscanf(sValue.c_str(),"%x",&iValue);
				string sR = " ";
				sR[0]=(char) iValue;
				sInput.replace( s, 3, sR );
			}
		}
		s++;
    }

    return sInput;
}

bool GetBlockToSend(string &sBlock,string &sTransmitString,string::size_type &pos)
{
	if(pos>=sTransmitString.size())
		return false;

	if( sTransmitString.size()-pos>3 && sTransmitString[pos]=='\\' && sTransmitString[pos+1]=='s' )
	{
		int Delay = atoi(sTransmitString.substr(pos+2).c_str());
		g_pPlutoLogger->Write(LV_STATUS,"Sleep %dms",Delay);
		Sleep(Delay);
		pos = sTransmitString.find('m',pos);
		if( pos==string::npos || pos>=sTransmitString.size()-1 )
			return false;
		pos++;
	}

	string::size_type pos_delay=0;
	if( (pos_delay=sTransmitString.find("\\s",pos))==string::npos )
	{
		sBlock=sTransmitString.substr(pos);  // Todo -- look for \s1000m to delay x milliseconds
		pos=sTransmitString.size();
		return true;
	}

	sBlock = sTransmitString.substr(pos,pos_delay-pos);
	pos = pos_delay;
	return true;
}

string StripHex(const char *pBuffer,int Length)
{
	string Result;

	for(int i=0;i<Length;++i)
	{
		char c=pBuffer[i];
		if( c>=' ' && c<='~' )
			Result += c;
		else if( c=='\r' )
			Result += "\\r";
		else if( c=='\n' )
			Result += "\\n";
		else
		{
			char hxbuff[5];
			sprintf(hxbuff, "%0hhx", (int) c);
			Result+="\\";
			if( c<16 )
				Result+="0"; // pad to 2 places
			Result += hxbuff;
		}
	}
	return Result;
}
