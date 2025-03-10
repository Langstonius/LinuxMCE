/*
     Copyright (C) 2004 Pluto, Inc., a Florida Corporation

     www.plutohome.com

     Phone: +1 (877) 758-8648
 

     This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.
     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
     of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

     See the GNU General Public License for more details.

*/

// If using the thread logger, these generated classes create lots of activity
#ifdef NO_SQL_THREAD_LOG
#undef THREAD_LOG
#endif

#ifdef WIN32
#include <WinSock2.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;
#include "PlutoUtils/StringUtils.h"
#include "Table_CommandGroup.h"
#include "Table_Array.h"
#include "Table_Installation.h"
#include "Table_Criteria.h"
#include "Table_DesignObj.h"
#include "Table_Template.h"
#include "Table_Icon.h"
#include "Table_Text.h"

#include "Table_CommandGroup_Command.h"
#include "Table_CommandGroup_EntertainArea.h"
#include "Table_CommandGroup_Room.h"
#include "Table_Device_CommandGroup.h"
#include "Table_EventHandler.h"


void Database_pluto_main::CreateTable_CommandGroup()
{
	tblCommandGroup = new Table_CommandGroup(this);
}

void Database_pluto_main::DeleteTable_CommandGroup()
{
	if( tblCommandGroup )
		delete tblCommandGroup;
}

bool Database_pluto_main::Commit_CommandGroup(bool bDeleteFailedModifiedRow,bool bDeleteFailedInsertRow)
{
	return tblCommandGroup->Commit(bDeleteFailedModifiedRow,bDeleteFailedInsertRow);
}

Table_CommandGroup::~Table_CommandGroup()
{
	map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator it;
	for(it=cachedRows.begin();it!=cachedRows.end();++it)
	{
		Row_CommandGroup *pRow = (Row_CommandGroup *) (*it).second;
		delete pRow;
	}

	for(it=deleted_cachedRows.begin();it!=deleted_cachedRows.end();++it)
	{
		Row_CommandGroup *pRow = (Row_CommandGroup *) (*it).second;
		delete pRow;
	}

	size_t i;
	for(i=0;i<addedRows.size();++i)
		delete addedRows[i];
	for(i=0;i<deleted_addedRows.size();++i)
		delete deleted_addedRows[i];
}


void Row_CommandGroup::Delete()
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
	Row_CommandGroup *pRow = this; // Needed so we will have only 1 version of get_primary_fields_assign_from_row
	
	if (!is_deleted)
	{
		if (is_added)	
		{	
			vector<TableRow*>::iterator i;	
			for (i = table->addedRows.begin(); (i!=table->addedRows.end()) && ( (Row_CommandGroup *) *i != this); i++);
			
			if (i!=	table->addedRows.end())
				table->addedRows.erase(i);
		
			table->deleted_addedRows.push_back(this);
			is_deleted = true;	
		}
		else
		{
			SingleLongKey key(pRow->m_PK_CommandGroup);
			map<SingleLongKey, TableRow*, SingleLongKey_Less>::iterator i = table->cachedRows.find(key);
			if (i!=table->cachedRows.end())
				table->cachedRows.erase(i);
						
			table->deleted_cachedRows[key] = this;
			is_deleted = true;	
		}	
	}
}

void Row_CommandGroup::Reload()
{
	Row_CommandGroup *pRow = this; // Needed so we will have only 1 version of get_primary_fields_assign_from_row

	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
	
	
	if (!is_added)
	{
		SingleLongKey key(pRow->m_PK_CommandGroup);
		Row_CommandGroup *pRow = table->FetchRow(key);
		
		if (pRow!=NULL)
		{
			*this = *pRow;	
			
			delete pRow;		
		}	
	}	
	
}

Row_CommandGroup::Row_CommandGroup(Table_CommandGroup *pTable):table(pTable)
{
	SetDefaultValues();
}

void Row_CommandGroup::SetDefaultValues()
{
	m_PK_CommandGroup = 0;
is_null[0] = false;
is_null[1] = true;
m_FK_Array = 0;
is_null[2] = true;
m_FK_Installation = 0;
m_Description = "";
is_null[3] = false;
is_null[4] = true;
m_CanTurnOff = 0;
is_null[5] = false;
m_AlwaysShow = 0;
is_null[6] = false;
m_CanBeHidden = 0;
is_null[7] = false;
is_null[8] = true;
m_FK_Criteria_Orbiter = 0;
is_null[9] = true;
m_FK_DesignObj = 0;
is_null[10] = true;
m_FK_Template = 0;
is_null[11] = true;
m_AltID = 0;
is_null[12] = true;
m_FK_Icon = 0;
is_null[13] = true;
m_Disabled = 0;
is_null[14] = false;
is_null[15] = true;
m_TemplateParm1 = 0;
is_null[16] = true;
m_TemplateParm2 = 0;
is_null[17] = true;
m_FK_Text = 0;
is_null[18] = true;
m_psc_id = 0;
is_null[19] = true;
m_psc_batch = 0;
is_null[20] = true;
m_psc_user = 0;
m_psc_frozen = 0;
is_null[21] = false;
is_null[22] = true;
is_null[23] = true;
m_psc_restrict = 0;


	is_added=false;
	is_deleted=false;
	is_modified=false;
}

long int Row_CommandGroup::PK_CommandGroup_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_PK_CommandGroup;}
long int Row_CommandGroup::FK_Array_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Array;}
long int Row_CommandGroup::FK_Installation_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Installation;}
string Row_CommandGroup::Description_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_Description;}
string Row_CommandGroup::Hint_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_Hint;}
short int Row_CommandGroup::CanTurnOff_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_CanTurnOff;}
short int Row_CommandGroup::AlwaysShow_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_AlwaysShow;}
short int Row_CommandGroup::CanBeHidden_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_CanBeHidden;}
long int Row_CommandGroup::FK_Criteria_Orbiter_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Criteria_Orbiter;}
long int Row_CommandGroup::FK_DesignObj_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_DesignObj;}
long int Row_CommandGroup::FK_Template_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Template;}
long int Row_CommandGroup::AltID_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_AltID;}
long int Row_CommandGroup::FK_Icon_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Icon;}
string Row_CommandGroup::AutoGeneratedDate_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_AutoGeneratedDate;}
short int Row_CommandGroup::Disabled_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_Disabled;}
long int Row_CommandGroup::TemplateParm1_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_TemplateParm1;}
long int Row_CommandGroup::TemplateParm2_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_TemplateParm2;}
long int Row_CommandGroup::FK_Text_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_FK_Text;}
long int Row_CommandGroup::psc_id_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_id;}
long int Row_CommandGroup::psc_batch_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_batch;}
long int Row_CommandGroup::psc_user_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_user;}
short int Row_CommandGroup::psc_frozen_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_frozen;}
string Row_CommandGroup::psc_mod_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_mod;}
long int Row_CommandGroup::psc_restrict_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return m_psc_restrict;}

		
void Row_CommandGroup::PK_CommandGroup_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_PK_CommandGroup = val; is_modified=true; is_null[0]=false;}
void Row_CommandGroup::FK_Array_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Array = val; is_modified=true; is_null[1]=false;}
void Row_CommandGroup::FK_Installation_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Installation = val; is_modified=true; is_null[2]=false;}
void Row_CommandGroup::Description_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_Description = val; is_modified=true; is_null[3]=false;}
void Row_CommandGroup::Hint_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_Hint = val; is_modified=true; is_null[4]=false;}
void Row_CommandGroup::CanTurnOff_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_CanTurnOff = val; is_modified=true; is_null[5]=false;}
void Row_CommandGroup::AlwaysShow_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_AlwaysShow = val; is_modified=true; is_null[6]=false;}
void Row_CommandGroup::CanBeHidden_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_CanBeHidden = val; is_modified=true; is_null[7]=false;}
void Row_CommandGroup::FK_Criteria_Orbiter_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Criteria_Orbiter = val; is_modified=true; is_null[8]=false;}
void Row_CommandGroup::FK_DesignObj_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_DesignObj = val; is_modified=true; is_null[9]=false;}
void Row_CommandGroup::FK_Template_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Template = val; is_modified=true; is_null[10]=false;}
void Row_CommandGroup::AltID_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_AltID = val; is_modified=true; is_null[11]=false;}
void Row_CommandGroup::FK_Icon_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Icon = val; is_modified=true; is_null[12]=false;}
void Row_CommandGroup::AutoGeneratedDate_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_AutoGeneratedDate = val; is_modified=true; is_null[13]=false;}
void Row_CommandGroup::Disabled_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_Disabled = val; is_modified=true; is_null[14]=false;}
void Row_CommandGroup::TemplateParm1_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_TemplateParm1 = val; is_modified=true; is_null[15]=false;}
void Row_CommandGroup::TemplateParm2_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_TemplateParm2 = val; is_modified=true; is_null[16]=false;}
void Row_CommandGroup::FK_Text_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_FK_Text = val; is_modified=true; is_null[17]=false;}
void Row_CommandGroup::psc_id_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_id = val; is_modified=true; is_null[18]=false;}
void Row_CommandGroup::psc_batch_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_batch = val; is_modified=true; is_null[19]=false;}
void Row_CommandGroup::psc_user_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_user = val; is_modified=true; is_null[20]=false;}
void Row_CommandGroup::psc_frozen_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_frozen = val; is_modified=true; is_null[21]=false;}
void Row_CommandGroup::psc_mod_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_mod = val; is_modified=true; is_null[22]=false;}
void Row_CommandGroup::psc_restrict_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

m_psc_restrict = val; is_modified=true; is_null[23]=false;}

		
bool Row_CommandGroup::FK_Array_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[1];}
bool Row_CommandGroup::FK_Installation_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[2];}
bool Row_CommandGroup::Hint_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[4];}
bool Row_CommandGroup::FK_Criteria_Orbiter_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[8];}
bool Row_CommandGroup::FK_DesignObj_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[9];}
bool Row_CommandGroup::FK_Template_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[10];}
bool Row_CommandGroup::AltID_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[11];}
bool Row_CommandGroup::FK_Icon_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[12];}
bool Row_CommandGroup::AutoGeneratedDate_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[13];}
bool Row_CommandGroup::TemplateParm1_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[15];}
bool Row_CommandGroup::TemplateParm2_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[16];}
bool Row_CommandGroup::FK_Text_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[17];}
bool Row_CommandGroup::psc_id_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[18];}
bool Row_CommandGroup::psc_batch_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[19];}
bool Row_CommandGroup::psc_user_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[20];}
bool Row_CommandGroup::psc_frozen_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[21];}
bool Row_CommandGroup::psc_mod_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[22];}
bool Row_CommandGroup::psc_restrict_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

return is_null[23];}

			
void Row_CommandGroup::FK_Array_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[1]=val;
is_modified=true;
}
void Row_CommandGroup::FK_Installation_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[2]=val;
is_modified=true;
}
void Row_CommandGroup::Hint_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[4]=val;
is_modified=true;
}
void Row_CommandGroup::FK_Criteria_Orbiter_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[8]=val;
is_modified=true;
}
void Row_CommandGroup::FK_DesignObj_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[9]=val;
is_modified=true;
}
void Row_CommandGroup::FK_Template_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[10]=val;
is_modified=true;
}
void Row_CommandGroup::AltID_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[11]=val;
is_modified=true;
}
void Row_CommandGroup::FK_Icon_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[12]=val;
is_modified=true;
}
void Row_CommandGroup::AutoGeneratedDate_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[13]=val;
is_modified=true;
}
void Row_CommandGroup::TemplateParm1_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[15]=val;
is_modified=true;
}
void Row_CommandGroup::TemplateParm2_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[16]=val;
is_modified=true;
}
void Row_CommandGroup::FK_Text_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[17]=val;
is_modified=true;
}
void Row_CommandGroup::psc_id_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[18]=val;
is_modified=true;
}
void Row_CommandGroup::psc_batch_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[19]=val;
is_modified=true;
}
void Row_CommandGroup::psc_user_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[20]=val;
is_modified=true;
}
void Row_CommandGroup::psc_frozen_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[21]=val;
is_modified=true;
}
void Row_CommandGroup::psc_mod_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[22]=val;
is_modified=true;
}
void Row_CommandGroup::psc_restrict_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);
is_null[23]=val;
is_modified=true;
}
	

string Row_CommandGroup::PK_CommandGroup_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[0])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_PK_CommandGroup);

return buf;
}

string Row_CommandGroup::FK_Array_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[1])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Array);

return buf;
}

string Row_CommandGroup::FK_Installation_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[2])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Installation);

return buf;
}

string Row_CommandGroup::Description_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[3])
return "NULL";

char *buf = new char[101];
db_wrapper_real_escape_string(table->database->m_pDB, buf, m_Description.c_str(), (unsigned long) min((size_t)50,m_Description.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_CommandGroup::Hint_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[4])
return "NULL";

char *buf = new char[61];
db_wrapper_real_escape_string(table->database->m_pDB, buf, m_Hint.c_str(), (unsigned long) min((size_t)30,m_Hint.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_CommandGroup::CanTurnOff_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[5])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_CanTurnOff);

return buf;
}

string Row_CommandGroup::AlwaysShow_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[6])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_AlwaysShow);

return buf;
}

string Row_CommandGroup::CanBeHidden_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[7])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_CanBeHidden);

return buf;
}

string Row_CommandGroup::FK_Criteria_Orbiter_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[8])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Criteria_Orbiter);

return buf;
}

string Row_CommandGroup::FK_DesignObj_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[9])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_DesignObj);

return buf;
}

string Row_CommandGroup::FK_Template_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[10])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Template);

return buf;
}

string Row_CommandGroup::AltID_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[11])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_AltID);

return buf;
}

string Row_CommandGroup::FK_Icon_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[12])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Icon);

return buf;
}

string Row_CommandGroup::AutoGeneratedDate_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[13])
return "NULL";

char *buf = new char[39];
db_wrapper_real_escape_string(table->database->m_pDB, buf, m_AutoGeneratedDate.c_str(), (unsigned long) min((size_t)19,m_AutoGeneratedDate.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_CommandGroup::Disabled_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[14])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_Disabled);

return buf;
}

string Row_CommandGroup::TemplateParm1_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[15])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_TemplateParm1);

return buf;
}

string Row_CommandGroup::TemplateParm2_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[16])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_TemplateParm2);

return buf;
}

string Row_CommandGroup::FK_Text_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[17])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_Text);

return buf;
}

string Row_CommandGroup::psc_id_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[18])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_id);

return buf;
}

string Row_CommandGroup::psc_batch_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[19])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_batch);

return buf;
}

string Row_CommandGroup::psc_user_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[20])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_user);

return buf;
}

string Row_CommandGroup::psc_frozen_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[21])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_psc_frozen);

return buf;
}

string Row_CommandGroup::psc_mod_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[22])
return "NULL";

char *buf = new char[39];
db_wrapper_real_escape_string(table->database->m_pDB, buf, m_psc_mod.c_str(), (unsigned long) min((size_t)19,m_psc_mod.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_CommandGroup::psc_restrict_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

if (is_null[23])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_restrict);

return buf;
}




Table_CommandGroup::Key::Key(long int in_PK_CommandGroup)
{
			pk_PK_CommandGroup = in_PK_CommandGroup;
	
}

Table_CommandGroup::Key::Key(Row_CommandGroup *pRow)
{
			PLUTO_SAFETY_LOCK_ERRORSONLY(sl,pRow->table->database->m_DBMutex);

			pk_PK_CommandGroup = pRow->m_PK_CommandGroup;
	
}		

bool Table_CommandGroup::Key_Less::operator()(const Table_CommandGroup::Key &key1, const Table_CommandGroup::Key &key2) const
{
			if (key1.pk_PK_CommandGroup!=key2.pk_PK_CommandGroup)
return key1.pk_PK_CommandGroup<key2.pk_PK_CommandGroup;
else
return false;	
}	

bool Table_CommandGroup::Commit(bool bDeleteFailedModifiedRow,bool bDeleteFailedInsertRow)
{
	bool bSuccessful=true;
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_DBMutex);

//insert added
	while (!addedRows.empty())
	{
		vector<TableRow*>::iterator i = addedRows.begin();
	
		Row_CommandGroup *pRow = (Row_CommandGroup *)*i;
	
		
string values_list_comma_separated;
values_list_comma_separated = values_list_comma_separated + pRow->PK_CommandGroup_asSQL()+", "+pRow->FK_Array_asSQL()+", "+pRow->FK_Installation_asSQL()+", "+pRow->Description_asSQL()+", "+pRow->Hint_asSQL()+", "+pRow->CanTurnOff_asSQL()+", "+pRow->AlwaysShow_asSQL()+", "+pRow->CanBeHidden_asSQL()+", "+pRow->FK_Criteria_Orbiter_asSQL()+", "+pRow->FK_DesignObj_asSQL()+", "+pRow->FK_Template_asSQL()+", "+pRow->AltID_asSQL()+", "+pRow->FK_Icon_asSQL()+", "+pRow->AutoGeneratedDate_asSQL()+", "+pRow->Disabled_asSQL()+", "+pRow->TemplateParm1_asSQL()+", "+pRow->TemplateParm2_asSQL()+", "+pRow->FK_Text_asSQL()+", "+pRow->psc_id_asSQL()+", "+pRow->psc_batch_asSQL()+", "+pRow->psc_user_asSQL()+", "+pRow->psc_frozen_asSQL()+", "+pRow->psc_restrict_asSQL();

	
		string query = "insert into CommandGroup (`PK_CommandGroup`, `FK_Array`, `FK_Installation`, `Description`, `Hint`, `CanTurnOff`, `AlwaysShow`, `CanBeHidden`, `FK_Criteria_Orbiter`, `FK_DesignObj`, `FK_Template`, `AltID`, `FK_Icon`, `AutoGeneratedDate`, `Disabled`, `TemplateParm1`, `TemplateParm2`, `FK_Text`, `psc_id`, `psc_batch`, `psc_user`, `psc_frozen`, `psc_restrict`) values ("+
			values_list_comma_separated+")";
			
		if (db_wrapper_query(database->m_pDB, query.c_str()))
		{	
			database->m_sLastDBError = db_wrapper_error(database->m_pDB);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastDBError << endl;
			bool bResult=database->DBConnect(true);
			int iResult2=-1;
			if( bResult )
				iResult2 = db_wrapper_query(database->m_pDB, query.c_str());
			
			LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::Commit Cannot perform query [%s] %s reconnect: %d result2: %d",query.c_str(),database->m_sLastDBError.c_str(),(int) bResult, iResult2);
			if( iResult2!=0 )  // We can keep going if the time it worked
			{
				if( bDeleteFailedInsertRow )
				{
					addedRows.erase(i);
					delete pRow;
				}
				break;   // Go ahead and continue to do the updates
			}
		}
	
		if (db_wrapper_affected_rows(database->m_pDB)!=0)
		{
			
			
			long int id = (long int) db_wrapper_insert_id(database->m_pDB);
		
			if (id!=0)
		pRow->m_PK_CommandGroup=id;
else 
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"PK_CommandGroup is auto increment but has no value %s",database->m_sLastDBError.c_str());	
			
			addedRows.erase(i);
			SingleLongKey key(pRow->m_PK_CommandGroup);	
			cachedRows[key] = pRow;
					
			
			pRow->is_added = false;	
			pRow->is_modified = false;	
		}	
				
	}	


//update modified
	

	for (map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = cachedRows.begin(); i!= cachedRows.end(); i++)
		if	(((*i).second)->is_modified_get())
	{
		Row_CommandGroup* pRow = (Row_CommandGroup*) (*i).second;	
		SingleLongKey key(pRow->m_PK_CommandGroup);

		char tmp_PK_CommandGroup[32];
sprintf(tmp_PK_CommandGroup, "%li", key.pk);


string condition;
condition = condition + "`PK_CommandGroup`=" + tmp_PK_CommandGroup;
	
			
		
string update_values_list;
update_values_list = update_values_list + "`PK_CommandGroup`="+pRow->PK_CommandGroup_asSQL()+", `FK_Array`="+pRow->FK_Array_asSQL()+", `FK_Installation`="+pRow->FK_Installation_asSQL()+", `Description`="+pRow->Description_asSQL()+", `Hint`="+pRow->Hint_asSQL()+", `CanTurnOff`="+pRow->CanTurnOff_asSQL()+", `AlwaysShow`="+pRow->AlwaysShow_asSQL()+", `CanBeHidden`="+pRow->CanBeHidden_asSQL()+", `FK_Criteria_Orbiter`="+pRow->FK_Criteria_Orbiter_asSQL()+", `FK_DesignObj`="+pRow->FK_DesignObj_asSQL()+", `FK_Template`="+pRow->FK_Template_asSQL()+", `AltID`="+pRow->AltID_asSQL()+", `FK_Icon`="+pRow->FK_Icon_asSQL()+", `AutoGeneratedDate`="+pRow->AutoGeneratedDate_asSQL()+", `Disabled`="+pRow->Disabled_asSQL()+", `TemplateParm1`="+pRow->TemplateParm1_asSQL()+", `TemplateParm2`="+pRow->TemplateParm2_asSQL()+", `FK_Text`="+pRow->FK_Text_asSQL()+", `psc_id`="+pRow->psc_id_asSQL()+", `psc_batch`="+pRow->psc_batch_asSQL()+", `psc_user`="+pRow->psc_user_asSQL()+", `psc_frozen`="+pRow->psc_frozen_asSQL()+", `psc_restrict`="+pRow->psc_restrict_asSQL();

	
		string query = "update CommandGroup set " + update_values_list + " where " + condition;
			
		if (db_wrapper_query(database->m_pDB, query.c_str()))
		{	
			database->m_sLastDBError = db_wrapper_error(database->m_pDB);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastDBError << endl;
			bool bResult=database->DBConnect(true);
			int iResult2=-1;
			if( bResult )
				iResult2 = db_wrapper_query(database->m_pDB, query.c_str());

			LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::Commit Cannot perform update query [%s] %s reconnect: %d result2: %d",query.c_str(),database->m_sLastDBError.c_str(),(int) bResult, iResult2);
			if( iResult2!=0 )  // We can keep going if the time it worked
			{
				if( bDeleteFailedModifiedRow )
				{
					cachedRows.erase(i);
					delete pRow;
				}
				break;  // Go ahead and do the deletes
			}
		}
	
		pRow->is_modified = false;	
	}	
	

//delete deleted added
	while (!deleted_addedRows.empty())
	{	
		vector<TableRow*>::iterator i = deleted_addedRows.begin();
		Row_CommandGroup* pRow = (Row_CommandGroup*) (*i);
		delete pRow;
		deleted_addedRows.erase(i);
	}	


//delete deleted cached
	
	while (!deleted_cachedRows.empty())
	{	
		map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = deleted_cachedRows.begin();
	
		SingleLongKey key = (*i).first;
		Row_CommandGroup* pRow = (Row_CommandGroup*) (*i).second;	

		char tmp_PK_CommandGroup[32];
sprintf(tmp_PK_CommandGroup, "%li", key.pk);


string condition;
condition = condition + "`PK_CommandGroup`=" + tmp_PK_CommandGroup;

	
		string query = "delete from CommandGroup where " + condition;
		
		if (db_wrapper_query(database->m_pDB, query.c_str()))
		{	
			database->m_sLastDBError = db_wrapper_error(database->m_pDB);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastDBError << endl;
			bool bResult=database->DBConnect(true);
			int iResult2=-1;
			if( bResult )
				iResult2 = db_wrapper_query(database->m_pDB, query.c_str());

			LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::Commit Cannot perform delete query [%s] %s reconnect: %d result2: %d",query.c_str(),database->m_sLastDBError.c_str(),(int) bResult, iResult2);
			if( iResult2!=0 )  // We can keep going if the time it worked
				return false;
		}	
		
		pRow = (Row_CommandGroup*) (*i).second;;
		delete pRow;
		deleted_cachedRows.erase(key);
	}
	
	return bSuccessful;
}

bool Table_CommandGroup::GetRows(string where_statement,vector<class Row_CommandGroup*> *rows)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_DBMutex);

	string query;
	if( StringUtils::StartsWith(where_statement,"where ",true) || 
		StringUtils::StartsWith(where_statement,"join ",true) ||
		StringUtils::StartsWith(where_statement,"left ",true) ||
		StringUtils::StartsWith(where_statement,"right ",true) ||
		StringUtils::StartsWith(where_statement,"full ",true) ||
		StringUtils::StartsWith(where_statement,"outer ",true) )
		query = "select `CommandGroup`.* from CommandGroup " + where_statement;
	else if( StringUtils::StartsWith(where_statement,"select ",true) )
		query = where_statement;
	else if( where_statement.size() )
		query = "select `CommandGroup`.* from CommandGroup where " + where_statement;
	else
		query = "select `CommandGroup`.* from CommandGroup";
		
	if (db_wrapper_query(database->m_pDB, query.c_str()))
	{	
		database->m_sLastDBError = db_wrapper_error(database->m_pDB);
		cerr << "Cannot perform query: [" << query << "] " << database->m_sLastDBError << endl;
		bool bResult=database->DBConnect(true);
		int iResult2=-1;
		if( bResult )
			iResult2 = db_wrapper_query(database->m_pDB, query.c_str());

		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::GetRows Cannot perform query [%s] %s reconnect: %d result2: %d",query.c_str(),database->m_sLastDBError.c_str(),(int) bResult, iResult2);
		if( iResult2!=0 )  // We can keep going if the time it worked
			return false;
	}	

	DB_RES *res = db_wrapper_store_result(database->m_pDB);
	
	if (!res)
	{
		cerr << "db_wrapper_store_result returned NULL handler" << endl;
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::GetRows db_wrapper_store_result returned NULL handler");
		database->m_sLastDBError = db_wrapper_error(database->m_pDB);
		return false;
	}	
	
	DB_ROW row;
						
		
	while ((row = db_wrapper_fetch_row(res)) != NULL)
	{	
		unsigned long *lengths = db_wrapper_fetch_lengths(res);

		Row_CommandGroup *pRow = new Row_CommandGroup(this);
		
		if (row[0] == NULL)
{
pRow->is_null[0]=true;
pRow->m_PK_CommandGroup = 0;
}
else
{
pRow->is_null[0]=false;
sscanf(row[0], "%li", &(pRow->m_PK_CommandGroup));
}

if (row[1] == NULL)
{
pRow->is_null[1]=true;
pRow->m_FK_Array = 0;
}
else
{
pRow->is_null[1]=false;
sscanf(row[1], "%li", &(pRow->m_FK_Array));
}

if (row[2] == NULL)
{
pRow->is_null[2]=true;
pRow->m_FK_Installation = 0;
}
else
{
pRow->is_null[2]=false;
sscanf(row[2], "%li", &(pRow->m_FK_Installation));
}

if (row[3] == NULL)
{
pRow->is_null[3]=true;
pRow->m_Description = "";
}
else
{
pRow->is_null[3]=false;
pRow->m_Description = string(row[3],lengths[3]);
}

if (row[4] == NULL)
{
pRow->is_null[4]=true;
pRow->m_Hint = "";
}
else
{
pRow->is_null[4]=false;
pRow->m_Hint = string(row[4],lengths[4]);
}

if (row[5] == NULL)
{
pRow->is_null[5]=true;
pRow->m_CanTurnOff = 0;
}
else
{
pRow->is_null[5]=false;
sscanf(row[5], "%hi", &(pRow->m_CanTurnOff));
}

if (row[6] == NULL)
{
pRow->is_null[6]=true;
pRow->m_AlwaysShow = 0;
}
else
{
pRow->is_null[6]=false;
sscanf(row[6], "%hi", &(pRow->m_AlwaysShow));
}

if (row[7] == NULL)
{
pRow->is_null[7]=true;
pRow->m_CanBeHidden = 0;
}
else
{
pRow->is_null[7]=false;
sscanf(row[7], "%hi", &(pRow->m_CanBeHidden));
}

if (row[8] == NULL)
{
pRow->is_null[8]=true;
pRow->m_FK_Criteria_Orbiter = 0;
}
else
{
pRow->is_null[8]=false;
sscanf(row[8], "%li", &(pRow->m_FK_Criteria_Orbiter));
}

if (row[9] == NULL)
{
pRow->is_null[9]=true;
pRow->m_FK_DesignObj = 0;
}
else
{
pRow->is_null[9]=false;
sscanf(row[9], "%li", &(pRow->m_FK_DesignObj));
}

if (row[10] == NULL)
{
pRow->is_null[10]=true;
pRow->m_FK_Template = 0;
}
else
{
pRow->is_null[10]=false;
sscanf(row[10], "%li", &(pRow->m_FK_Template));
}

if (row[11] == NULL)
{
pRow->is_null[11]=true;
pRow->m_AltID = 0;
}
else
{
pRow->is_null[11]=false;
sscanf(row[11], "%li", &(pRow->m_AltID));
}

if (row[12] == NULL)
{
pRow->is_null[12]=true;
pRow->m_FK_Icon = 0;
}
else
{
pRow->is_null[12]=false;
sscanf(row[12], "%li", &(pRow->m_FK_Icon));
}

if (row[13] == NULL)
{
pRow->is_null[13]=true;
pRow->m_AutoGeneratedDate = "";
}
else
{
pRow->is_null[13]=false;
pRow->m_AutoGeneratedDate = string(row[13],lengths[13]);
}

if (row[14] == NULL)
{
pRow->is_null[14]=true;
pRow->m_Disabled = 0;
}
else
{
pRow->is_null[14]=false;
sscanf(row[14], "%hi", &(pRow->m_Disabled));
}

if (row[15] == NULL)
{
pRow->is_null[15]=true;
pRow->m_TemplateParm1 = 0;
}
else
{
pRow->is_null[15]=false;
sscanf(row[15], "%li", &(pRow->m_TemplateParm1));
}

if (row[16] == NULL)
{
pRow->is_null[16]=true;
pRow->m_TemplateParm2 = 0;
}
else
{
pRow->is_null[16]=false;
sscanf(row[16], "%li", &(pRow->m_TemplateParm2));
}

if (row[17] == NULL)
{
pRow->is_null[17]=true;
pRow->m_FK_Text = 0;
}
else
{
pRow->is_null[17]=false;
sscanf(row[17], "%li", &(pRow->m_FK_Text));
}

if (row[18] == NULL)
{
pRow->is_null[18]=true;
pRow->m_psc_id = 0;
}
else
{
pRow->is_null[18]=false;
sscanf(row[18], "%li", &(pRow->m_psc_id));
}

if (row[19] == NULL)
{
pRow->is_null[19]=true;
pRow->m_psc_batch = 0;
}
else
{
pRow->is_null[19]=false;
sscanf(row[19], "%li", &(pRow->m_psc_batch));
}

if (row[20] == NULL)
{
pRow->is_null[20]=true;
pRow->m_psc_user = 0;
}
else
{
pRow->is_null[20]=false;
sscanf(row[20], "%li", &(pRow->m_psc_user));
}

if (row[21] == NULL)
{
pRow->is_null[21]=true;
pRow->m_psc_frozen = 0;
}
else
{
pRow->is_null[21]=false;
sscanf(row[21], "%hi", &(pRow->m_psc_frozen));
}

if (row[22] == NULL)
{
pRow->is_null[22]=true;
pRow->m_psc_mod = "";
}
else
{
pRow->is_null[22]=false;
pRow->m_psc_mod = string(row[22],lengths[22]);
}

if (row[23] == NULL)
{
pRow->is_null[23]=true;
pRow->m_psc_restrict = 0;
}
else
{
pRow->is_null[23]=false;
sscanf(row[23], "%li", &(pRow->m_psc_restrict));
}



		//checking for duplicates

		SingleLongKey key(pRow->m_PK_CommandGroup);
		
		map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = cachedRows.find(key);
			
		if (i!=cachedRows.end())
		{
			delete pRow;
			pRow = (Row_CommandGroup *)(*i).second;
		}

		rows->push_back(pRow);
		
		cachedRows[key] = pRow;
	}

	db_wrapper_free_result(res);			
		
	return true;					
}

Row_CommandGroup* Table_CommandGroup::AddRow()
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_DBMutex);

	Row_CommandGroup *pRow = new Row_CommandGroup(this);
	pRow->is_added=true;
	addedRows.push_back(pRow);
	return pRow;		
}



Row_CommandGroup* Table_CommandGroup::GetRow(long int in_PK_CommandGroup)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_DBMutex);

	SingleLongKey row_key(in_PK_CommandGroup);

	map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i;
	i = deleted_cachedRows.find(row_key);	
		
	//row was deleted	
	if (i!=deleted_cachedRows.end())
		return NULL;
	
	i = cachedRows.find(row_key);
	
	//row is cached
	if (i!=cachedRows.end())
		return (Row_CommandGroup*) (*i).second;
	//we have to fetch row
	Row_CommandGroup* pRow = FetchRow(row_key);

	if (pRow!=NULL)
		cachedRows[row_key] = pRow;
	return pRow;	
}



Row_CommandGroup* Table_CommandGroup::FetchRow(SingleLongKey &key)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_DBMutex);

	//defines the string query for the value of key
	char tmp_PK_CommandGroup[32];
sprintf(tmp_PK_CommandGroup, "%li", key.pk);


string condition;
condition = condition + "`PK_CommandGroup`=" + tmp_PK_CommandGroup;


	string query = "select * from CommandGroup where " + condition;		

	if (db_wrapper_query(database->m_pDB, query.c_str()))
	{	
		database->m_sLastDBError = db_wrapper_error(database->m_pDB);
		cerr << "Cannot perform query: [" << query << "] " << database->m_sLastDBError << endl;
		bool bResult=database->DBConnect(true);
		int iResult2=-1;
		if( bResult )
			iResult2 = db_wrapper_query(database->m_pDB, query.c_str());

		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::FetchRow Cannot perform query [%s] %s reconnect: %d result2: %d",query.c_str(),database->m_sLastDBError.c_str(),(int) bResult, iResult2);
		if( iResult2!=0 )  // We can keep going if the time it worked
			return NULL;
	}	

	DB_RES *res = db_wrapper_store_result(database->m_pDB);
	
	if (!res)
	{
		cerr << "db_wrapper_store_result returned NULL handler" << endl;
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Table_CommandGroup::FetchRow db_wrapper_store_result returned NULL handler");
		database->m_sLastDBError = db_wrapper_error(database->m_pDB);
		return NULL;
	}	
	
	DB_ROW row = db_wrapper_fetch_row(res);
	
	if (!row)
	{
		//dataset is empty
		db_wrapper_free_result(res);			
		return NULL;		
	}	
						
	unsigned long *lengths = db_wrapper_fetch_lengths(res);

	Row_CommandGroup *pRow = new Row_CommandGroup(this);
		
	if (row[0] == NULL)
{
pRow->is_null[0]=true;
pRow->m_PK_CommandGroup = 0;
}
else
{
pRow->is_null[0]=false;
sscanf(row[0], "%li", &(pRow->m_PK_CommandGroup));
}

if (row[1] == NULL)
{
pRow->is_null[1]=true;
pRow->m_FK_Array = 0;
}
else
{
pRow->is_null[1]=false;
sscanf(row[1], "%li", &(pRow->m_FK_Array));
}

if (row[2] == NULL)
{
pRow->is_null[2]=true;
pRow->m_FK_Installation = 0;
}
else
{
pRow->is_null[2]=false;
sscanf(row[2], "%li", &(pRow->m_FK_Installation));
}

if (row[3] == NULL)
{
pRow->is_null[3]=true;
pRow->m_Description = "";
}
else
{
pRow->is_null[3]=false;
pRow->m_Description = string(row[3],lengths[3]);
}

if (row[4] == NULL)
{
pRow->is_null[4]=true;
pRow->m_Hint = "";
}
else
{
pRow->is_null[4]=false;
pRow->m_Hint = string(row[4],lengths[4]);
}

if (row[5] == NULL)
{
pRow->is_null[5]=true;
pRow->m_CanTurnOff = 0;
}
else
{
pRow->is_null[5]=false;
sscanf(row[5], "%hi", &(pRow->m_CanTurnOff));
}

if (row[6] == NULL)
{
pRow->is_null[6]=true;
pRow->m_AlwaysShow = 0;
}
else
{
pRow->is_null[6]=false;
sscanf(row[6], "%hi", &(pRow->m_AlwaysShow));
}

if (row[7] == NULL)
{
pRow->is_null[7]=true;
pRow->m_CanBeHidden = 0;
}
else
{
pRow->is_null[7]=false;
sscanf(row[7], "%hi", &(pRow->m_CanBeHidden));
}

if (row[8] == NULL)
{
pRow->is_null[8]=true;
pRow->m_FK_Criteria_Orbiter = 0;
}
else
{
pRow->is_null[8]=false;
sscanf(row[8], "%li", &(pRow->m_FK_Criteria_Orbiter));
}

if (row[9] == NULL)
{
pRow->is_null[9]=true;
pRow->m_FK_DesignObj = 0;
}
else
{
pRow->is_null[9]=false;
sscanf(row[9], "%li", &(pRow->m_FK_DesignObj));
}

if (row[10] == NULL)
{
pRow->is_null[10]=true;
pRow->m_FK_Template = 0;
}
else
{
pRow->is_null[10]=false;
sscanf(row[10], "%li", &(pRow->m_FK_Template));
}

if (row[11] == NULL)
{
pRow->is_null[11]=true;
pRow->m_AltID = 0;
}
else
{
pRow->is_null[11]=false;
sscanf(row[11], "%li", &(pRow->m_AltID));
}

if (row[12] == NULL)
{
pRow->is_null[12]=true;
pRow->m_FK_Icon = 0;
}
else
{
pRow->is_null[12]=false;
sscanf(row[12], "%li", &(pRow->m_FK_Icon));
}

if (row[13] == NULL)
{
pRow->is_null[13]=true;
pRow->m_AutoGeneratedDate = "";
}
else
{
pRow->is_null[13]=false;
pRow->m_AutoGeneratedDate = string(row[13],lengths[13]);
}

if (row[14] == NULL)
{
pRow->is_null[14]=true;
pRow->m_Disabled = 0;
}
else
{
pRow->is_null[14]=false;
sscanf(row[14], "%hi", &(pRow->m_Disabled));
}

if (row[15] == NULL)
{
pRow->is_null[15]=true;
pRow->m_TemplateParm1 = 0;
}
else
{
pRow->is_null[15]=false;
sscanf(row[15], "%li", &(pRow->m_TemplateParm1));
}

if (row[16] == NULL)
{
pRow->is_null[16]=true;
pRow->m_TemplateParm2 = 0;
}
else
{
pRow->is_null[16]=false;
sscanf(row[16], "%li", &(pRow->m_TemplateParm2));
}

if (row[17] == NULL)
{
pRow->is_null[17]=true;
pRow->m_FK_Text = 0;
}
else
{
pRow->is_null[17]=false;
sscanf(row[17], "%li", &(pRow->m_FK_Text));
}

if (row[18] == NULL)
{
pRow->is_null[18]=true;
pRow->m_psc_id = 0;
}
else
{
pRow->is_null[18]=false;
sscanf(row[18], "%li", &(pRow->m_psc_id));
}

if (row[19] == NULL)
{
pRow->is_null[19]=true;
pRow->m_psc_batch = 0;
}
else
{
pRow->is_null[19]=false;
sscanf(row[19], "%li", &(pRow->m_psc_batch));
}

if (row[20] == NULL)
{
pRow->is_null[20]=true;
pRow->m_psc_user = 0;
}
else
{
pRow->is_null[20]=false;
sscanf(row[20], "%li", &(pRow->m_psc_user));
}

if (row[21] == NULL)
{
pRow->is_null[21]=true;
pRow->m_psc_frozen = 0;
}
else
{
pRow->is_null[21]=false;
sscanf(row[21], "%hi", &(pRow->m_psc_frozen));
}

if (row[22] == NULL)
{
pRow->is_null[22]=true;
pRow->m_psc_mod = "";
}
else
{
pRow->is_null[22]=false;
pRow->m_psc_mod = string(row[22],lengths[22]);
}

if (row[23] == NULL)
{
pRow->is_null[23]=true;
pRow->m_psc_restrict = 0;
}
else
{
pRow->is_null[23]=false;
sscanf(row[23], "%li", &(pRow->m_psc_restrict));
}



	db_wrapper_free_result(res);			
	
	return pRow;						
}


class Row_Array* Row_CommandGroup::FK_Array_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Array *pTable = table->database->Array_get();
return pTable->GetRow(m_FK_Array);
}
class Row_Installation* Row_CommandGroup::FK_Installation_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Installation *pTable = table->database->Installation_get();
return pTable->GetRow(m_FK_Installation);
}
class Row_Criteria* Row_CommandGroup::FK_Criteria_Orbiter_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Criteria *pTable = table->database->Criteria_get();
return pTable->GetRow(m_FK_Criteria_Orbiter);
}
class Row_DesignObj* Row_CommandGroup::FK_DesignObj_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_DesignObj *pTable = table->database->DesignObj_get();
return pTable->GetRow(m_FK_DesignObj);
}
class Row_Template* Row_CommandGroup::FK_Template_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Template *pTable = table->database->Template_get();
return pTable->GetRow(m_FK_Template);
}
class Row_Icon* Row_CommandGroup::FK_Icon_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Icon *pTable = table->database->Icon_get();
return pTable->GetRow(m_FK_Icon);
}
class Row_Text* Row_CommandGroup::FK_Text_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Text *pTable = table->database->Text_get();
return pTable->GetRow(m_FK_Text);
}


void Row_CommandGroup::CommandGroup_Command_FK_CommandGroup_getrows(vector <class Row_CommandGroup_Command*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_CommandGroup_Command *pTable = table->database->CommandGroup_Command_get();
pTable->GetRows("`FK_CommandGroup`=" + StringUtils::itos(m_PK_CommandGroup),rows);
}
void Row_CommandGroup::CommandGroup_EntertainArea_FK_CommandGroup_getrows(vector <class Row_CommandGroup_EntertainArea*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_CommandGroup_EntertainArea *pTable = table->database->CommandGroup_EntertainArea_get();
pTable->GetRows("`FK_CommandGroup`=" + StringUtils::itos(m_PK_CommandGroup),rows);
}
void Row_CommandGroup::CommandGroup_Room_FK_CommandGroup_getrows(vector <class Row_CommandGroup_Room*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_CommandGroup_Room *pTable = table->database->CommandGroup_Room_get();
pTable->GetRows("`FK_CommandGroup`=" + StringUtils::itos(m_PK_CommandGroup),rows);
}
void Row_CommandGroup::Device_CommandGroup_FK_CommandGroup_getrows(vector <class Row_Device_CommandGroup*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_Device_CommandGroup *pTable = table->database->Device_CommandGroup_get();
pTable->GetRows("`FK_CommandGroup`=" + StringUtils::itos(m_PK_CommandGroup),rows);
}
void Row_CommandGroup::EventHandler_FK_CommandGroup_getrows(vector <class Row_EventHandler*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_DBMutex);

class Table_EventHandler *pTable = table->database->EventHandler_get();
pTable->GetRows("`FK_CommandGroup`=" + StringUtils::itos(m_PK_CommandGroup),rows);
}



