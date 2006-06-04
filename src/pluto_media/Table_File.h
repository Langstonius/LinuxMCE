#ifndef __Table_File_H__
#define __Table_File_H__

#include "TableRow.h"
#include "Database_pluto_media.h"
#include "PlutoUtils/MultiThreadIncludes.h"
#include "Define_File.h"
#include "SerializeClass/SerializeClass.h"

// If we declare the maps locally, the compiler will create multiple copies of them
// making the output files enormous.  The solution seems to be to create some predefined
// maps for the standard types of primary keys (single long, double long, etc.) and
// put them in a common base class, which is optionally included as tablebase below

class DECLSPECIFIER TableRow;
class DECLSPECIFIER SerializeClass;

class DECLSPECIFIER Table_File : public TableBase , SingleLongKeyBase
{
private:
	Database_pluto_media *database;
	struct Key;	//forward declaration
	
public:
	Table_File(Database_pluto_media *pDatabase):database(pDatabase)
	{
	};
	~Table_File();

private:		
	friend class Row_File;
	struct Key
	{
		friend class Row_File;
		long int pk_PK_File;

		
		Key(long int in_PK_File);
	
		Key(class Row_File *pRow);
	};
	struct Key_Less
	{			
		bool operator()(const Table_File::Key &key1, const Table_File::Key &key2) const;
	};	

	
	

public:				
	// Normally the framework never deletes any Row_X objects, since the application will
	// likely have pointers to them.  This means that if a Commit fails because a row
	// cannot be committed, all subsequent calls to Commit will also fail unless you fix
	// the rows since they will be re-attempted.  If you set either flag to true, the failed
	// row can be deleted.  Use with caution since your pointers become invalid!
	bool Commit(bool bDeleteFailedModifiedRow=false,bool bDeleteFailedInsertRow=false);
	bool GetRows(string where_statement,vector<class Row_File*> *rows);
	class Row_File* AddRow();
	Database_pluto_media *Database_pluto_media_get() { return database; }
	
		
	class Row_File* GetRow(long int in_PK_File);
	

private:	
	
		
	class Row_File* FetchRow(SingleLongKey &key);
		
			
};

class DECLSPECIFIER Row_File : public TableRow, public SerializeClass
	{
		friend struct Table_File::Key;
		friend class Table_File;
	private:
		Table_File *table;
		
		long int m_PK_File;
long int m_EK_MediaType;
long int m_FK_MediaSubType;
long int m_FK_FileFormat;
string m_DateAdded;
string m_Path;
string m_Filename;
long int m_Missing;
long int m_IsDirectory;
long int m_psc_id;
long int m_psc_batch;
long int m_psc_user;
short int m_psc_frozen;
string m_psc_mod;
long int m_psc_restrict;

		bool is_null[15];
	
	public:
		long int PK_File_get();
long int EK_MediaType_get();
long int FK_MediaSubType_get();
long int FK_FileFormat_get();
string DateAdded_get();
string Path_get();
string Filename_get();
long int Missing_get();
long int IsDirectory_get();
long int psc_id_get();
long int psc_batch_get();
long int psc_user_get();
short int psc_frozen_get();
string psc_mod_get();
long int psc_restrict_get();

		
		void PK_File_set(long int val);
void EK_MediaType_set(long int val);
void FK_MediaSubType_set(long int val);
void FK_FileFormat_set(long int val);
void DateAdded_set(string val);
void Path_set(string val);
void Filename_set(string val);
void Missing_set(long int val);
void IsDirectory_set(long int val);
void psc_id_set(long int val);
void psc_batch_set(long int val);
void psc_user_set(long int val);
void psc_frozen_set(short int val);
void psc_mod_set(string val);
void psc_restrict_set(long int val);

		
		bool EK_MediaType_isNull();
bool FK_MediaSubType_isNull();
bool FK_FileFormat_isNull();
bool DateAdded_isNull();
bool psc_id_isNull();
bool psc_batch_isNull();
bool psc_user_isNull();
bool psc_frozen_isNull();
bool psc_restrict_isNull();

			
		void EK_MediaType_setNull(bool val);
void FK_MediaSubType_setNull(bool val);
void FK_FileFormat_setNull(bool val);
void DateAdded_setNull(bool val);
void psc_id_setNull(bool val);
void psc_batch_setNull(bool val);
void psc_user_setNull(bool val);
void psc_frozen_setNull(bool val);
void psc_restrict_setNull(bool val);
	
	
		void Delete();
		void Reload();		
	
		Row_File(Table_File *pTable);
	
		bool IsDeleted(){return is_deleted;};
		bool IsModified(){return is_modified;};			
		class Table_File *Table_File_get() { return table; };

		// Return the rows for foreign keys 
		class Row_MediaSubType* FK_MediaSubType_getrow();
class Row_FileFormat* FK_FileFormat_getrow();


		// Return the rows in other tables with foreign keys pointing here
		void Bookmark_FK_File_getrows(vector <class Row_Bookmark*> *rows);
void File_Attribute_FK_File_getrows(vector <class Row_File_Attribute*> *rows);
void File_Users_FK_File_getrows(vector <class Row_File_Users*> *rows);
void Picture_File_FK_File_getrows(vector <class Row_Picture_File*> *rows);
void PlaylistEntry_FK_File_getrows(vector <class Row_PlaylistEntry*> *rows);


		// Setup binary serialization
		void SetupSerialization(int iSC_Version) {
			StartSerializeList() + m_PK_File+ m_EK_MediaType+ m_FK_MediaSubType+ m_FK_FileFormat+ m_DateAdded+ m_Path+ m_Filename+ m_Missing+ m_IsDirectory+ m_psc_id+ m_psc_batch+ m_psc_user+ m_psc_frozen+ m_psc_mod+ m_psc_restrict;
		}
	private:
		void SetDefaultValues();
		
		string PK_File_asSQL();
string EK_MediaType_asSQL();
string FK_MediaSubType_asSQL();
string FK_FileFormat_asSQL();
string DateAdded_asSQL();
string Path_asSQL();
string Filename_asSQL();
string Missing_asSQL();
string IsDirectory_asSQL();
string psc_id_asSQL();
string psc_batch_asSQL();
string psc_user_asSQL();
string psc_frozen_asSQL();
string psc_mod_asSQL();
string psc_restrict_asSQL();

	};

#endif

