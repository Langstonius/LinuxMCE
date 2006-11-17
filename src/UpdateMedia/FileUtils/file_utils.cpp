#include "file_utils.h"

#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>

using namespace std;

namespace UpdateMediaFileUtils
{
	bool IsSymlink(string sFilePath)
	{
#ifdef WIN32
		return false;
#else
		const int cnDummyBufferSize = 4;
		char *pDummyBuffer = new char[cnDummyBufferSize];
		memset(pDummyBuffer, 0, cnDummyBufferSize);

		ssize_t link_length = readlink (sFilePath.c_str(), pDummyBuffer, cnDummyBufferSize); 	

		delete [] pDummyBuffer;

		return link_length > 0;
#endif
	}

	string SymlinkSource(string sFilePath)
	{
#ifdef WIN32
		return "";
#else
		const int cnBufferSize = 1024;
		char *pBuffer = new char[cnBufferSize];
		memset(pBuffer, 0, cnBufferSize);

		ssize_t link_length = readlink (sFilePath.c_str(), pBuffer, cnBufferSize); 	

		string sResult(link_length > 0 ? pBuffer : "");
		delete [] pBuffer;

		return sResult;
#endif
	}

	int MountedDeviceForFile(string sFilePath)
	{
#ifdef WIN32
		struct __stat64 buf;
#else
		struct stat64 buf;
#endif

#ifdef WIN32
		if(!_stat64(sFilePath.c_str(), &buf))
#else
		if(!stat64(sFilePath.c_str(), &buf))
#endif
			return buf.st_dev;

		cout << "ERROR: " << errno << " - desc: " << strerror(errno) << endl;

		return 0;
	}

	int PlutoDeviceForFile(string sFilePath, const map<int, int>& mapMountedDevices)
	{
		map<int, int>::const_iterator it = mapMountedDevices.find(MountedDeviceForFile(sFilePath));
		if(it != mapMountedDevices.end())
			return it->second;

		return 0;
	}

	void MapMountedDevicesToPlutoDevices(const list<string>& listFiles, map<int, int>& mapMountedDevices)
	{
		const string csMntDevicePath("/mnt/device");
		for(list<string>::const_iterator it = listFiles.begin(), end = listFiles.end(); it != end; ++it)
		{
			string sFilePath = *it;

			int nMountedDevice = MountedDeviceForFile(sFilePath);
			if(nMountedDevice != 0 && sFilePath.find(csMntDevicePath) == 0)
			{
                string sDeviceId = sFilePath.substr(csMntDevicePath.length() + 1);
				int nPlutoDevice = atoi(sDeviceId.c_str());
				if(nPlutoDevice != 0)
				{		
					mapMountedDevices[nMountedDevice] = nPlutoDevice;

					cout << "Mapped file " << sFilePath << " : " << nMountedDevice << " -> " << nPlutoDevice << endl; 
				}
			}
		}
	}
}
