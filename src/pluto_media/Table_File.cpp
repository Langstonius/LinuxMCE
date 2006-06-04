// If using the thread logger, these generated classes create lots of activity
#ifdef NO_SQL_THREAD_LOG
#undef THREAD_LOG
#endif

#ifdef WIN32
#include <winsock.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <mysql.h>

using namespace std;
#include "PlutoUtils/StringUtils.h"
#include "Table_File.h"
#include "Table_MediaSubType.h"
#include "Table_FileFormat.h"

#include "Table_Bookmark.h"
#include "Table_File_Attribute.h"
#include "Table_Picture_File.h"
#include "Table_PlaylistEntry.h"


void Database_pluto_media::CreateTable_File()
{
	tblFile = new Table_File(this);
}

void Database_pluto_media::DeleteTable_File()
{
	if( tblFile )
		delete tblFile;
}

bool Database_pluto_media::Commit_File(bool bDeleteFailedModifiedRow,bool bDeleteFailedInsertRow)
{
	return tblFile->Commit(bDeleteFailedModifiedRow,bDeleteFailedInsertRow);
}

Table_File::~Table_File()
{
	map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator it;
	for(it=cachedRows.begin();it!=cachedRows.end();++it)
	{
		Row_File *pRow = (Row_File *) (*it).second;
		delete pRow;
	}

	for(it=deleted_cachedRows.begin();it!=deleted_cachedRows.end();++it)
	{
		Row_File *pRow = (Row_File *) (*it).second;
		delete pRow;
	}

	size_t i;
	for(i=0;i<addedRows.size();++i)
		delete addedRows[i];
	for(i=0;i<deleted_addedRows.size();++i)
		delete deleted_addedRows[i];
}


void Row_File::Delete()
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
	Row_File *pRow = this; // Needed so we will have only 1 version of get_primary_fields_assign_from_row
	
	if (!is_deleted)
		if (is_added)	
		{	
			vector<TableRow*>::iterator i;	
			for (i = table->addedRows.begin(); (i!=table->addedRows.end()) && ( (Row_File *) *i != this); i++);
			
			if (i!=	table->addedRows.end())
				table->addedRows.erase(i);
		
			table->deleted_addedRows.push_back(this);
			is_deleted = true;	
		}
		else
		{
			SingleLongKey key(pRow->m_PK_File);
			map<SingleLongKey, TableRow*, SingleLongKey_Less>::iterator i = table->cachedRows.find(key);
			if (i!=table->cachedRows.end())
				table->cachedRows.erase(i);
						
			table->deleted_cachedRows[key] = this;
			is_deleted = true;	
		}	
}

void Row_File::Reload()
{
	Row_File *pRow = this; // Needed so we will have only 1 version of get_primary_fields_assign_from_row

	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
	
	
	if (!is_added)
	{
		SingleLongKey key(pRow->m_PK_File);
		Row_File *pRow = table->FetchRow(key);
		
		if (pRow!=NULL)
		{
			*this = *pRow;	
			
			delete pRow;		
		}	
	}	
	
}

Row_File::Row_File(Table_File *pTable):table(pTable)
{
	SetDefaultValues();
}

void Row_File::SetDefaultValues()
{
	m_PK_File = 0;
is_null[0] = false;
is_null[1] = true;
m_EK_MediaType = 0;
is_null[2] = true;
m_FK_MediaSubType = 0;
is_null[3] = true;
m_FK_FileFormat = 0;
is_null[4] = true;
m_Path = "";
is_null[5] = false;
m_Filename = "0";
is_null[6] = false;
m_Missing = 0;
is_null[7] = false;
m_IsDirectory = 0;
is_null[8] = false;
is_null[9] = true;
m_psc_id = 0;
is_null[10] = true;
m_psc_batch = 0;
is_null[11] = true;
m_psc_user = 0;
m_psc_frozen = 0;
is_null[12] = false;
m_psc_mod = "00000000000000";
is_null[13] = false;
is_null[14] = true;
m_psc_restrict = 0;


	is_added=false;
	is_deleted=false;
	is_modified=false;
}

long int Row_File::PK_File_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_PK_File;}
long int Row_File::EK_MediaType_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_EK_MediaType;}
long int Row_File::FK_MediaSubType_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_FK_MediaSubType;}
long int Row_File::FK_FileFormat_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_FK_FileFormat;}
string Row_File::DateAdded_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_DateAdded;}
string Row_File::Path_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_Path;}
string Row_File::Filename_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_Filename;}
long int Row_File::Missing_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_Missing;}
long int Row_File::IsDirectory_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_IsDirectory;}
long int Row_File::psc_id_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_id;}
long int Row_File::psc_batch_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_batch;}
long int Row_File::psc_user_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_user;}
short int Row_File::psc_frozen_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_frozen;}
string Row_File::psc_mod_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_mod;}
long int Row_File::psc_restrict_get(){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return m_psc_restrict;}

		
void Row_File::PK_File_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_PK_File = val; is_modified=true; is_null[0]=false;}
void Row_File::EK_MediaType_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_EK_MediaType = val; is_modified=true; is_null[1]=false;}
void Row_File::FK_MediaSubType_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_FK_MediaSubType = val; is_modified=true; is_null[2]=false;}
void Row_File::FK_FileFormat_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_FK_FileFormat = val; is_modified=true; is_null[3]=false;}
void Row_File::DateAdded_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_DateAdded = val; is_modified=true; is_null[4]=false;}
void Row_File::Path_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_Path = val; is_modified=true; is_null[5]=false;}
void Row_File::Filename_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_Filename = val; is_modified=true; is_null[6]=false;}
void Row_File::Missing_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_Missing = val; is_modified=true; is_null[7]=false;}
void Row_File::IsDirectory_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_IsDirectory = val; is_modified=true; is_null[8]=false;}
void Row_File::psc_id_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_id = val; is_modified=true; is_null[9]=false;}
void Row_File::psc_batch_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_batch = val; is_modified=true; is_null[10]=false;}
void Row_File::psc_user_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_user = val; is_modified=true; is_null[11]=false;}
void Row_File::psc_frozen_set(short int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_frozen = val; is_modified=true; is_null[12]=false;}
void Row_File::psc_mod_set(string val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_mod = val; is_modified=true; is_null[13]=false;}
void Row_File::psc_restrict_set(long int val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

m_psc_restrict = val; is_modified=true; is_null[14]=false;}

		
bool Row_File::EK_MediaType_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[1];}
bool Row_File::FK_MediaSubType_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[2];}
bool Row_File::FK_FileFormat_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[3];}
bool Row_File::DateAdded_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[4];}
bool Row_File::psc_id_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[9];}
bool Row_File::psc_batch_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[10];}
bool Row_File::psc_user_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[11];}
bool Row_File::psc_frozen_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[12];}
bool Row_File::psc_restrict_isNull() {PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

return is_null[14];}

			
void Row_File::EK_MediaType_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[1]=val;
is_modified=true;
}
void Row_File::FK_MediaSubType_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[2]=val;
is_modified=true;
}
void Row_File::FK_FileFormat_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[3]=val;
is_modified=true;
}
void Row_File::DateAdded_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[4]=val;
is_modified=true;
}
void Row_File::psc_id_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[9]=val;
is_modified=true;
}
void Row_File::psc_batch_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[10]=val;
is_modified=true;
}
void Row_File::psc_user_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[11]=val;
is_modified=true;
}
void Row_File::psc_frozen_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[12]=val;
is_modified=true;
}
void Row_File::psc_restrict_setNull(bool val){PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);
is_null[14]=val;
is_modified=true;
}
	

string Row_File::PK_File_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[0])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_PK_File);

return buf;
}

string Row_File::EK_MediaType_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[1])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_EK_MediaType);

return buf;
}

string Row_File::FK_MediaSubType_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[2])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_MediaSubType);

return buf;
}

string Row_File::FK_FileFormat_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[3])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_FK_FileFormat);

return buf;
}

string Row_File::DateAdded_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[4])
return "NULL";

char *buf = new char[39];
mysql_real_escape_string(table->database->m_pMySQL, buf, m_DateAdded.c_str(), (unsigned long) min((size_t)19,m_DateAdded.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_File::Path_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[5])
return "NULL";

char *buf = new char[301];
mysql_real_escape_string(table->database->m_pMySQL, buf, m_Path.c_str(), (unsigned long) min((size_t)150,m_Path.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_File::Filename_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[6])
return "NULL";

char *buf = new char[201];
mysql_real_escape_string(table->database->m_pMySQL, buf, m_Filename.c_str(), (unsigned long) min((size_t)100,m_Filename.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_File::Missing_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[7])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_Missing);

return buf;
}

string Row_File::IsDirectory_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[8])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_IsDirectory);

return buf;
}

string Row_File::psc_id_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[9])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_id);

return buf;
}

string Row_File::psc_batch_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[10])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_batch);

return buf;
}

string Row_File::psc_user_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[11])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_user);

return buf;
}

string Row_File::psc_frozen_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[12])
return "NULL";

char buf[32];
sprintf(buf, "%hi", m_psc_frozen);

return buf;
}

string Row_File::psc_mod_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[13])
return "NULL";

char *buf = new char[29];
mysql_real_escape_string(table->database->m_pMySQL, buf, m_psc_mod.c_str(), (unsigned long) min((size_t)14,m_psc_mod.size()));
string s=string()+"\""+buf+"\"";
delete[] buf;
return s;
}

string Row_File::psc_restrict_asSQL()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

if (is_null[14])
return "NULL";

char buf[32];
sprintf(buf, "%li", m_psc_restrict);

return buf;
}




Table_File::Key::Key(long int in_PK_File)
{
			pk_PK_File = in_PK_File;
	
}

Table_File::Key::Key(Row_File *pRow)
{
			PLUTO_SAFETY_LOCK_ERRORSONLY(sl,pRow->table->database->m_MySqlMutex);

			pk_PK_File = pRow->m_PK_File;
	
}		

bool Table_File::Key_Less::operator()(const Table_File::Key &key1, const Table_File::Key &key2) const
{
			if (key1.pk_PK_File!=key2.pk_PK_File)
return key1.pk_PK_File<key2.pk_PK_File;
else
return false;	
}	

bool Table_File::Commit(bool bDeleteFailedModifiedRow,bool bDeleteFailedInsertRow)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_MySqlMutex);

//insert added
	while (!addedRows.empty())
	{
		vector<TableRow*>::iterator i = addedRows.begin();
	
		Row_File *pRow = (Row_File *)*i;
	
		
string values_list_comma_separated;
values_list_comma_separated = values_list_comma_separated + pRow->PK_File_asSQL()+", "+pRow->EK_MediaType_asSQL()+", "+pRow->FK_MediaSubType_asSQL()+", "+pRow->FK_FileFormat_asSQL()+", "+pRow->DateAdded_asSQL()+", "+pRow->Path_asSQL()+", "+pRow->Filename_asSQL()+", "+pRow->Missing_asSQL()+", "+pRow->IsDirectory_asSQL()+", "+pRow->psc_id_asSQL()+", "+pRow->psc_batch_asSQL()+", "+pRow->psc_user_asSQL()+", "+pRow->psc_frozen_asSQL()+", "+pRow->psc_restrict_asSQL();

	
		string query = "insert into File (`PK_File`, `EK_MediaType`, `FK_MediaSubType`, `FK_FileFormat`, `DateAdded`, `Path`, `Filename`, `Missing`, `IsDirectory`, `psc_id`, `psc_batch`, `psc_user`, `psc_frozen`, `psc_restrict`) values ("+
			values_list_comma_separated+")";
			
		if (mysql_query(database->m_pMySQL, query.c_str()))
		{	
			database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastMySqlError << endl;
			if( bDeleteFailedInsertRow )
			{
				addedRows.erase(i);
				delete pRow;
			}
			return false;
		}
	
		if (mysql_affected_rows(database->m_pMySQL)!=0)
		{
			
			
			long int id = (long int) mysql_insert_id(database->m_pMySQL);
		
			if (id!=0)
pRow->m_PK_File=id;
	
			
			addedRows.erase(i);
			SingleLongKey key(pRow->m_PK_File);	
			cachedRows[key] = pRow;
					
			
			pRow->is_added = false;	
			pRow->is_modified = false;	
		}	
				
	}	


//update modified
	

	for (map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = cachedRows.begin(); i!= cachedRows.end(); i++)
		if	(((*i).second)->is_modified_get())
	{
		Row_File* pRow = (Row_File*) (*i).second;	
		SingleLongKey key(pRow->m_PK_File);

		char tmp_PK_File[32];
sprintf(tmp_PK_File, "%li", key.pk);


string condition;
condition = condition + "`PK_File`=" + tmp_PK_File;
	
			
		
string update_values_list;
update_values_list = update_values_list + "`PK_File`="+pRow->PK_File_asSQL()+", `EK_MediaType`="+pRow->EK_MediaType_asSQL()+", `FK_MediaSubType`="+pRow->FK_MediaSubType_asSQL()+", `FK_FileFormat`="+pRow->FK_FileFormat_asSQL()+", `DateAdded`="+pRow->DateAdded_asSQL()+", `Path`="+pRow->Path_asSQL()+", `Filename`="+pRow->Filename_asSQL()+", `Missing`="+pRow->Missing_asSQL()+", `IsDirectory`="+pRow->IsDirectory_asSQL()+", `psc_id`="+pRow->psc_id_asSQL()+", `psc_batch`="+pRow->psc_batch_asSQL()+", `psc_user`="+pRow->psc_user_asSQL()+", `psc_frozen`="+pRow->psc_frozen_asSQL()+", `psc_restrict`="+pRow->psc_restrict_asSQL();

	
		string query = "update File set " + update_values_list + " where " + condition;
			
		if (mysql_query(database->m_pMySQL, query.c_str()))
		{	
			database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastMySqlError << endl;
			if( bDeleteFailedModifiedRow )
			{
				cachedRows.erase(i);
				delete pRow;
			}
			return false;
		}
	
		pRow->is_modified = false;	
	}	
	

//delete deleted added
	while (!deleted_addedRows.empty())
	{	
		vector<TableRow*>::iterator i = deleted_addedRows.begin();
		Row_File* pRow = (Row_File*) (*i);
		delete pRow;
		deleted_addedRows.erase(i);
	}	


//delete deleted cached
	
	while (!deleted_cachedRows.empty())
	{	
		map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = deleted_cachedRows.begin();
	
		SingleLongKey key = (*i).first;
		Row_File* pRow = (Row_File*) (*i).second;	

		char tmp_PK_File[32];
sprintf(tmp_PK_File, "%li", key.pk);


string condition;
condition = condition + "`PK_File`=" + tmp_PK_File;

	
		string query = "delete from File where " + condition;
		
		if (mysql_query(database->m_pMySQL, query.c_str()))
		{	
			database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
			cerr << "Cannot perform query: [" << query << "] " << database->m_sLastMySqlError << endl;
			return false;
		}	
		
		pRow = (Row_File*) (*i).second;;
		delete pRow;
		deleted_cachedRows.erase(key);
	}
	
	return true;
}

bool Table_File::GetRows(string where_statement,vector<class Row_File*> *rows)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_MySqlMutex);

	string query;
	if( StringUtils::StartsWith(where_statement,"where ",true) || 
		StringUtils::StartsWith(where_statement,"join ",true) ||
		StringUtils::StartsWith(where_statement,"left ",true) ||
		StringUtils::StartsWith(where_statement,"right ",true) ||
		StringUtils::StartsWith(where_statement,"full ",true) ||
		StringUtils::StartsWith(where_statement,"outer ",true) )
		query = "select `File`.* from File " + where_statement;
	else if( StringUtils::StartsWith(where_statement,"select ",true) )
		query = where_statement;
	else if( where_statement.size() )
		query = "select `File`.* from File where " + where_statement;
	else
		query = "select `File`.* from File";
		
	if (mysql_query(database->m_pMySQL, query.c_str()))
	{	
		database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
		cerr << "Cannot perform query: [" << query << "] " << database->m_sLastMySqlError << endl;
		return false;
	}	

	MYSQL_RES *res = mysql_store_result(database->m_pMySQL);
	
	if (!res)
	{
		cerr << "mysql_store_result returned NULL handler" << endl;
		database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
		return false;
	}	
	
	MYSQL_ROW row;
						
		
	while ((row = mysql_fetch_row(res)) != NULL)
	{	
		unsigned long *lengths = mysql_fetch_lengths(res);

		Row_File *pRow = new Row_File(this);
		
		if (row[0] == NULL)
{
pRow->is_null[0]=true;
pRow->m_PK_File = 0;
}
else
{
pRow->is_null[0]=false;
sscanf(row[0], "%li", &(pRow->m_PK_File));
}

if (row[1] == NULL)
{
pRow->is_null[1]=true;
pRow->m_EK_MediaType = 0;
}
else
{
pRow->is_null[1]=false;
sscanf(row[1], "%li", &(pRow->m_EK_MediaType));
}

if (row[2] == NULL)
{
pRow->is_null[2]=true;
pRow->m_FK_MediaSubType = 0;
}
else
{
pRow->is_null[2]=false;
sscanf(row[2], "%li", &(pRow->m_FK_MediaSubType));
}

if (row[3] == NULL)
{
pRow->is_null[3]=true;
pRow->m_FK_FileFormat = 0;
}
else
{
pRow->is_null[3]=false;
sscanf(row[3], "%li", &(pRow->m_FK_FileFormat));
}

if (row[4] == NULL)
{
pRow->is_null[4]=true;
pRow->m_DateAdded = "";
}
else
{
pRow->is_null[4]=false;
pRow->m_DateAdded = string(row[4],lengths[4]);
}

if (row[5] == NULL)
{
pRow->is_null[5]=true;
pRow->m_Path = "";
}
else
{
pRow->is_null[5]=false;
pRow->m_Path = string(row[5],lengths[5]);
}

if (row[6] == NULL)
{
pRow->is_null[6]=true;
pRow->m_Filename = "";
}
else
{
pRow->is_null[6]=false;
pRow->m_Filename = string(row[6],lengths[6]);
}

if (row[7] == NULL)
{
pRow->is_null[7]=true;
pRow->m_Missing = 0;
}
else
{
pRow->is_null[7]=false;
sscanf(row[7], "%li", &(pRow->m_Missing));
}

if (row[8] == NULL)
{
pRow->is_null[8]=true;
pRow->m_IsDirectory = 0;
}
else
{
pRow->is_null[8]=false;
sscanf(row[8], "%li", &(pRow->m_IsDirectory));
}

if (row[9] == NULL)
{
pRow->is_null[9]=true;
pRow->m_psc_id = 0;
}
else
{
pRow->is_null[9]=false;
sscanf(row[9], "%li", &(pRow->m_psc_id));
}

if (row[10] == NULL)
{
pRow->is_null[10]=true;
pRow->m_psc_batch = 0;
}
else
{
pRow->is_null[10]=false;
sscanf(row[10], "%li", &(pRow->m_psc_batch));
}

if (row[11] == NULL)
{
pRow->is_null[11]=true;
pRow->m_psc_user = 0;
}
else
{
pRow->is_null[11]=false;
sscanf(row[11], "%li", &(pRow->m_psc_user));
}

if (row[12] == NULL)
{
pRow->is_null[12]=true;
pRow->m_psc_frozen = 0;
}
else
{
pRow->is_null[12]=false;
sscanf(row[12], "%hi", &(pRow->m_psc_frozen));
}

if (row[13] == NULL)
{
pRow->is_null[13]=true;
pRow->m_psc_mod = "";
}
else
{
pRow->is_null[13]=false;
pRow->m_psc_mod = string(row[13],lengths[13]);
}

if (row[14] == NULL)
{
pRow->is_null[14]=true;
pRow->m_psc_restrict = 0;
}
else
{
pRow->is_null[14]=false;
sscanf(row[14], "%li", &(pRow->m_psc_restrict));
}



		//checking for duplicates

		SingleLongKey key(pRow->m_PK_File);
		
		map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i = cachedRows.find(key);
			
		if (i!=cachedRows.end())
		{
			delete pRow;
			pRow = (Row_File *)(*i).second;
		}

		rows->push_back(pRow);
		
		cachedRows[key] = pRow;
	}

	mysql_free_result(res);			
		
	return true;					
}

Row_File* Table_File::AddRow()
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_MySqlMutex);

	Row_File *pRow = new Row_File(this);
	pRow->is_added=true;
	addedRows.push_back(pRow);
	return pRow;		
}



Row_File* Table_File::GetRow(long int in_PK_File)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_MySqlMutex);

	SingleLongKey row_key(in_PK_File);

	map<SingleLongKey, class TableRow*, SingleLongKey_Less>::iterator i;
	i = deleted_cachedRows.find(row_key);	
		
	//row was deleted	
	if (i!=deleted_cachedRows.end())
		return NULL;
	
	i = cachedRows.find(row_key);
	
	//row is cached
	if (i!=cachedRows.end())
		return (Row_File*) (*i).second;
	//we have to fetch row
	Row_File* pRow = FetchRow(row_key);

	if (pRow!=NULL)
		cachedRows[row_key] = pRow;
	return pRow;	
}



Row_File* Table_File::FetchRow(SingleLongKey &key)
{
	PLUTO_SAFETY_LOCK_ERRORSONLY(sl,database->m_MySqlMutex);

	//defines the string query for the value of key
	char tmp_PK_File[32];
sprintf(tmp_PK_File, "%li", key.pk);


string condition;
condition = condition + "`PK_File`=" + tmp_PK_File;


	string query = "select * from File where " + condition;		

	if (mysql_query(database->m_pMySQL, query.c_str()))
	{	
		database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
		cerr << "Cannot perform query: [" << query << "] " << database->m_sLastMySqlError << endl;
		return NULL;
	}	

	MYSQL_RES *res = mysql_store_result(database->m_pMySQL);
	
	if (!res)
	{
		cerr << "mysql_store_result returned NULL handler" << endl;
		database->m_sLastMySqlError = mysql_error(database->m_pMySQL);
		return NULL;
	}	
	
	MYSQL_ROW row = mysql_fetch_row(res);
	
	if (!row)
	{
		//dataset is empty
		mysql_free_result(res);			
		return NULL;		
	}	
						
	unsigned long *lengths = mysql_fetch_lengths(res);

	Row_File *pRow = new Row_File(this);
		
	if (row[0] == NULL)
{
pRow->is_null[0]=true;
pRow->m_PK_File = 0;
}
else
{
pRow->is_null[0]=false;
sscanf(row[0], "%li", &(pRow->m_PK_File));
}

if (row[1] == NULL)
{
pRow->is_null[1]=true;
pRow->m_EK_MediaType = 0;
}
else
{
pRow->is_null[1]=false;
sscanf(row[1], "%li", &(pRow->m_EK_MediaType));
}

if (row[2] == NULL)
{
pRow->is_null[2]=true;
pRow->m_FK_MediaSubType = 0;
}
else
{
pRow->is_null[2]=false;
sscanf(row[2], "%li", &(pRow->m_FK_MediaSubType));
}

if (row[3] == NULL)
{
pRow->is_null[3]=true;
pRow->m_FK_FileFormat = 0;
}
else
{
pRow->is_null[3]=false;
sscanf(row[3], "%li", &(pRow->m_FK_FileFormat));
}

if (row[4] == NULL)
{
pRow->is_null[4]=true;
pRow->m_DateAdded = "";
}
else
{
pRow->is_null[4]=false;
pRow->m_DateAdded = string(row[4],lengths[4]);
}

if (row[5] == NULL)
{
pRow->is_null[5]=true;
pRow->m_Path = "";
}
else
{
pRow->is_null[5]=false;
pRow->m_Path = string(row[5],lengths[5]);
}

if (row[6] == NULL)
{
pRow->is_null[6]=true;
pRow->m_Filename = "";
}
else
{
pRow->is_null[6]=false;
pRow->m_Filename = string(row[6],lengths[6]);
}

if (row[7] == NULL)
{
pRow->is_null[7]=true;
pRow->m_Missing = 0;
}
else
{
pRow->is_null[7]=false;
sscanf(row[7], "%li", &(pRow->m_Missing));
}

if (row[8] == NULL)
{
pRow->is_null[8]=true;
pRow->m_IsDirectory = 0;
}
else
{
pRow->is_null[8]=false;
sscanf(row[8], "%li", &(pRow->m_IsDirectory));
}

if (row[9] == NULL)
{
pRow->is_null[9]=true;
pRow->m_psc_id = 0;
}
else
{
pRow->is_null[9]=false;
sscanf(row[9], "%li", &(pRow->m_psc_id));
}

if (row[10] == NULL)
{
pRow->is_null[10]=true;
pRow->m_psc_batch = 0;
}
else
{
pRow->is_null[10]=false;
sscanf(row[10], "%li", &(pRow->m_psc_batch));
}

if (row[11] == NULL)
{
pRow->is_null[11]=true;
pRow->m_psc_user = 0;
}
else
{
pRow->is_null[11]=false;
sscanf(row[11], "%li", &(pRow->m_psc_user));
}

if (row[12] == NULL)
{
pRow->is_null[12]=true;
pRow->m_psc_frozen = 0;
}
else
{
pRow->is_null[12]=false;
sscanf(row[12], "%hi", &(pRow->m_psc_frozen));
}

if (row[13] == NULL)
{
pRow->is_null[13]=true;
pRow->m_psc_mod = "";
}
else
{
pRow->is_null[13]=false;
pRow->m_psc_mod = string(row[13],lengths[13]);
}

if (row[14] == NULL)
{
pRow->is_null[14]=true;
pRow->m_psc_restrict = 0;
}
else
{
pRow->is_null[14]=false;
sscanf(row[14], "%li", &(pRow->m_psc_restrict));
}



	mysql_free_result(res);			
	
	return pRow;						
}


class Row_MediaSubType* Row_File::FK_MediaSubType_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_MediaSubType *pTable = table->database->MediaSubType_get();
return pTable->GetRow(m_FK_MediaSubType);
}
class Row_FileFormat* Row_File::FK_FileFormat_getrow()
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_FileFormat *pTable = table->database->FileFormat_get();
return pTable->GetRow(m_FK_FileFormat);
}


void Row_File::Bookmark_FK_File_getrows(vector <class Row_Bookmark*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_Bookmark *pTable = table->database->Bookmark_get();
pTable->GetRows("`FK_File`=" + StringUtils::itos(m_PK_File),rows);
}
void Row_File::File_Attribute_FK_File_getrows(vector <class Row_File_Attribute*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_File_Attribute *pTable = table->database->File_Attribute_get();
pTable->GetRows("`FK_File`=" + StringUtils::itos(m_PK_File),rows);
}
void Row_File::Picture_File_FK_File_getrows(vector <class Row_Picture_File*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_Picture_File *pTable = table->database->Picture_File_get();
pTable->GetRows("`FK_File`=" + StringUtils::itos(m_PK_File),rows);
}
void Row_File::PlaylistEntry_FK_File_getrows(vector <class Row_PlaylistEntry*> *rows)
{
PLUTO_SAFETY_LOCK_ERRORSONLY(sl,table->database->m_MySqlMutex);

class Table_PlaylistEntry *pTable = table->database->PlaylistEntry_get();
pTable->GetRows("`FK_File`=" + StringUtils::itos(m_PK_File),rows);
}



