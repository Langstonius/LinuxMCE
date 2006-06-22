#ifndef __ID3INFO_H_
#define __ID3INFO_H_

#include <map>
#include <string>
using namespace std;

void GetId3Info(string sFilename, map<int,string>& mapAttributes);
void SetId3Info(string sFilename, const map<int,string>& mapAttributes);
void RemoveId3Tag(string sFilename, int nTagType, string sValue);

void GetUserDefinedInformation(string sFilename, char *&pData, size_t& Size);
void SetUserDefinedInformation(string sFilename, char *pData, size_t& Size);

#endif
