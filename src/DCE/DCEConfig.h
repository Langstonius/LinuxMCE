#ifndef DCEConfig_H
#define DCEConfig_H
//------------------------------------------------------------------------------------------------------
#include "RA/RA_Config.h"
#include "PlutoUtils/StringUtils.h"
//------------------------------------------------------------------------------------------------------
#include <string>
#include <map>

using namespace std;

//------------------------------------------------------------------------------------------------------
class DCEConfig : public RA_Config
{
public:
	// serialized values
	string m_sDBHost,m_sDBUser,m_sDBPassword,m_sDBName,m_sDCERouter;
	string m_sMenuPath,m_sPicturePath;  // Used by the server to find the menus and pictures.  Picture Path may be used by the Establishment also
	string m_sSisFilesPath;
	string m_sConfigFile;

	int m_iPK_Device_Computer,m_iDBPort,m_iDCERouterPort,m_iPK_Installation;
	int m_iPK_Distro;
	map<string,string> m_mapParameters;
	string m_mapParameters_Find(string Token) {	map<string,string>::iterator it = m_mapParameters.find( Token ); return it == m_mapParameters.end() ? string() : (*it).second; }
	bool m_mapParameters_Exists(string Token) {	map<string,string>::iterator it = m_mapParameters.find( Token ); return it != m_mapParameters.end(); }

	inline int ReadInteger(string sToken, int iDefaultValue = 0);
	inline string ReadString(string sToken, string sDefaultValue = "");

	bool WriteSettings();

	void AddString(string sToken, string sValue) { m_mapParameters[sToken]=sValue; }
	void AddInteger(string sToken, int iValue) { m_mapParameters[sToken]=StringUtils::itos(iValue); }

	DCEConfig(string sFilename="/etc/pluto.conf");
	virtual ~DCEConfig();
};
//------------------------------------------------------------------------------------------------------
#endif 

