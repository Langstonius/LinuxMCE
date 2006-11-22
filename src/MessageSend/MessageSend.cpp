// OCDeviceTemplate.cpp : Defines the entry point for the console application.
//
#include "PlutoUtils/CommonIncludes.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"
#include "PlutoUtils/uuencode.h"
#include "DCE/Logger.h"
#include "DCE/Message.h"
#include "DCE/DeviceData_Impl.h"

#include <iostream>

namespace DCE
{
    Logger *g_pPlutoLogger;
}

using namespace DCE;
using namespace std;

int uuencode(const char *FileIn,const char *FileOut)
{
	size_t Size;
	char *pData = FileUtils::ReadFileIntoBuffer(FileIn,Size);
	if( !pData )
	{
		cerr << "Cannot open file: " << FileIn << endl;
		return 1;
	}

	size_t SizeEncoded = MaxEncodedSize(Size);
	char *pDataEncoded = new char[SizeEncoded];
	int Bytes=Ns_HtuuEncode((unsigned char *) pData, Size, (unsigned char *) pDataEncoded);
	pDataEncoded[Bytes]=0;
	if( FileOut )
	{
		FILE *file = fopen(FileOut,"wb");
		if( !file || fwrite(pDataEncoded,1,Bytes,file)!=Bytes )
		{
			cerr << "Cannot open or write to file: " << FileOut << endl;
			if( file )
				fclose(file);
			return 1;
		}
		fclose(file);
	}
	else
		cout << pDataEncoded;

	delete[] pData;
	delete[] pDataEncoded;
	return 0;
}

int uudecode(const char *File,const char *Data)
{
	char *pDataEncoded=(char *) Data;
	size_t Size = strlen(Data);
	FILE *file_in = fopen(Data,"rb"); // See if Data is a filename
	if( file_in )
	{
		cout << "Read contents of file: " << Data << endl;
		pDataEncoded = FileUtils::ReadFileIntoBuffer(Data,Size);
		fclose(file_in);
	}
	char *pDataDecoded = new char[Size];
	int Bytes = Ns_HtuuDecode((unsigned char *) pDataEncoded, (unsigned char *) pDataDecoded, Size);
	FILE *file = fopen(File,"wb");
	if( !file || fwrite(pDataDecoded,1,Bytes,file)!=Bytes )
	{
		cerr << "Cannot open or write to file: " << File << endl;
		if( file )
			fclose(file);
		return 1;
	}
	fclose(file);
	if( pDataEncoded!=Data )
		delete[] pDataEncoded;
	delete[] pDataDecoded;
	return 0;
}

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
    int err;
    WSADATA wsaData;
    err = WSAStartup(MAKEWORD( 1, 1 ),(LPWSADATA)  &wsaData);
#else
int main(int argc, char *argv[])
{
#endif

	if( (argc==3 || argc==4) && strcmp(argv[1],"uuencode")==0 )
		return uuencode(argv[2],argc==4 ? argv[3] : NULL);
	else if( argc==4 && strcmp(argv[1],"uudecode")==0 )
		return uudecode(argv[2],argv[3]);
    else if (argc<6)
    {
		cout << "Usage: MessageSend server [-targetType [device|category|template]] [-r | -o] [-p path] [-bl BroadcastLevel] DeviceFrom DeviceTo MsgType(1=Command, 2=Event) MsgID [parm1id param1value] [parm2id parm2value] ..." << endl
			<< "\tthe server is the name/ip of the router, such as localhost" << endl
			<< "\tthe default target type is the device." << endl
            << "\t-p specifies the path for output params" << endl
            << "\t-r means the message will be sent with a response request" << endl
			<< "\t-o means the message will expect out parameters" << endl
			<< "\t\twhich will be echoed to the screen as resp: [response]" << endl
			<< "\t-bl means the broadcast level if the message is sent to category or template;" << endl
			<< "\t\tvalues: none, direct_siblings, same_computer, same_room, same_house*, all_houses" << endl
			<< "\t\tneeds valid device ID for this to be useful" << endl;
			<< "\tthe parm ID can be prefixed with a letter:" << endl
			<< "\t\tD send as a data paramter, rather than text" << endl
			<< "\t\tU send as a data paramter, rather than text, parameter is UU-encoded" << endl
			<< "\t\tB the value is a filename, the contents sent as a data parameter" << endl
			<< "\t\tT the value is a filename, the contents sent as a text parameter" << endl
			<< endl
			<< "Alternatively: MessageSend uuencode file_in [file_out] (writes to stdout no file_out)" << endl
			<< "or MessageSend uudecode file_out [uuencodeddata | file_in]" << endl; 

        return 1;
    }

    g_pPlutoLogger = new FileLogger(stdout);
    if (g_pPlutoLogger == NULL)
        fprintf(stderr,"Problem creating logger.  Check params.\n");


    string ServerAddress=argv[1];
	Event_Impl *pEvent = new Event_Impl(DEVICEID_MESSAGESEND, 0, ServerAddress);
	for(int i=0;true;++i) // Wait up to 30 seconds
	{
		pEvent->m_pClientSocket->SendString("READY");
		string sResponse;
		if( !pEvent->m_pClientSocket->ReceiveString(sResponse,5) )
		{
			g_pPlutoLogger->Write(LV_CRITICAL,"Cannot communicate with router");
			exit(1);
		}
		if( sResponse=="YES" )
			break;
		else if( sResponse=="NO" )
		{
			if( i>5 )
			{
				g_pPlutoLogger->Write(LV_CRITICAL,"Router not ready after 30 seconds.  Aborting....");
				exit(1);
			}
			g_pPlutoLogger->Write(LV_STATUS,"DCERouter still loading.  Waiting 5 seconds");
			Sleep(5000);
		}
		else
		{
			g_pPlutoLogger->Write(LV_CRITICAL,"Router gave unknown response to ready request %s",sResponse.c_str());
			exit(1);
		}
	}

    Message *pMsg=new Message(argc-2,&argv[2]);

    string sOutputParametersPath = pMsg->sOutputParametersPath;
    if(sOutputParametersPath != "")
        sOutputParametersPath += "/";

	eExpectedResponse ExpectedResponse = pMsg->m_eExpectedResponse;
	// We need a response.  It will be a string if there are no out parameters
	if( ExpectedResponse != ER_ReplyMessage ) // No out parameters
	{
		string sResponse; // We'll use this only if a response wasn't passed in
		bool bResult;

		if( ExpectedResponse == ER_DeliveryConfirmation )
			bResult = pEvent->SendMessage( pMsg, sResponse );
		else
			bResult = pEvent->SendMessage( pMsg );

		if( ExpectedResponse != ER_DeliveryConfirmation )
		{
			if( !bResult )
				cerr << "Failed to send" << endl;
		    Sleep(50);  // Wait a moment so we don't close the socket before this is sent
			return bResult ? 0 : -1;
		}
		cout << "RESP: " << sResponse << endl;
		return bResult ? 0 : -1;
	}

	// There are out parameters, we need to get a message back in return
	Message *pResponse = pEvent->SendReceiveMessage( pMsg );
	if( !pResponse || pResponse->m_dwID != 0 )
	{
		if(pResponse)
			delete pResponse;

		cerr << "Failed to send message" << endl;
		return -1;
	}

	for( map<long, string>::iterator it=pResponse->m_mapParameters.begin();it!=pResponse->m_mapParameters.end();++it)
	{
		cout << (*it).first << ":" << (*it).second << endl;
	}

	for( map<long, char *>::iterator it=pResponse->m_mapData_Parameters.begin();it!=pResponse->m_mapData_Parameters.end();++it)
	{
		cout << "D" << (*it).first << ":" << (*it).second << ":" << (*it).first << ".out" << endl;
        
        string sOutputFile = sOutputParametersPath + StringUtils::itos((*it).first) + ".out";
        cout << "Writing parameter to: " << sOutputFile << endl;
		FILE *file = fopen( sOutputFile.c_str(),"wb");
		if( file )
		{
			fwrite( (*it).second,pResponse->m_mapData_Lengths[(*it).first],1,file );
			fclose(file);
		}
	}


    Sleep(50);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

