/* 
	DataGrid
	
	Copyright (C) 2004 Pluto, Inc., a Florida Corporation
	
	www.plutohome.com		
	
	Phone: +1 (877) 758-8648
	
	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
	of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
	
	See the GNU General Public License for more details.
*/

#include "PlutoUtils/minilzo.h"
#include "PlutoUtils/CommonIncludes.h"	
#include "DataGrid.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"
#include "Logger.h"

#ifdef WINCE
#include "../Orbiter/CENet/WinCE.h"
#endif

// Message parameter list.

#define DCELL_TEXT 1
#define DCELL_GRAPHIC_DATA 2
#define DCELL_GRAPHIC_FORMAT 3
#define DCELL_ONTOUCH_MESSAGE 4
#define DCELL_VALUE	5
#define DCELL_ALT_COLOR 6
#define DCELL_STYLE 7
#define DCELL_SELECTABLE 8

#define HEAP_ALLOC(var,size) \
        lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

using namespace DCE;

void DataGridCell::Initialize()
{
	m_bSelectable = true;
	m_bDontFireGridEvent = false;
	m_PK_StyleDetail = 0;
	m_AltColor=0;
	m_Colspan=1;
	m_Rowspan=1;
	m_pMessage=NULL;	
	m_pGraphicData=NULL;
	m_Text = NULL;
	m_Value = NULL;
	m_ImagePath = NULL;
	m_GraphicLength = m_MessageLength = m_TextLength = m_ValueLength = m_ImagePathLength = m_NumAttributes = m_AttributesLength = 0;
	m_GraphicFormat = GR_UNKNOWN;
	m_pGraphic = NULL;
}

DataGridCell::~DataGridCell()
{
	if (m_Text) 
		delete[] m_Text;
	if (m_Value)
		delete[] m_Value;
	if (m_ImagePath)
		delete[] m_ImagePath;

	if(NULL != m_pMessage)
	{
		delete m_pMessage;
		m_pMessage = NULL;
	}

	if(NULL != m_pGraphicData)
	{
		delete m_pGraphicData;
		m_pGraphicData = NULL;
	}
	delete m_pGraphic;
}

DataGridCell::DataGridCell(string Text, string Value)
{
	Initialize();

	m_Text = new char[Text.length()+1];
	strcpy(m_Text,Text.c_str());
	m_TextLength = (int)Text.length();

	if( Value.size()==0 )
	{
		m_Value=NULL;
		m_ValueLength=0;
	}
	else
	{
		m_Value = new char[Value.length()+1]; 
		strcpy(m_Value,Value.c_str());
		m_ValueLength = (int)Value.length();
	}
}

void DataGridCell::SetValue(string Value)
{
	delete m_Value;

	m_Value = new char[Value.length()+1]; 
	strcpy(m_Value,Value.c_str());
	m_ValueLength = (int)Value.length();
}

DataGridCell::DataGridCell(int Size, char *Data)
{
	Initialize();

	// The first chunk of the data is the serializable part

	memcpy((DataGridCellSerializableData *)this, Data, sizeof(DataGridCellSerializableData));
	Data+=sizeof(DataGridCellSerializableData);
	if (m_TextLength)
	{
		m_Text = new char[m_TextLength+1];
		memcpy(m_Text, Data, m_TextLength);
		m_Text[m_TextLength]=0;
		Data+=m_TextLength;
	}
	if (m_ValueLength)
	{
		m_Value = new char[m_ValueLength+1];
		memcpy(m_Value, Data, m_ValueLength);
		m_Value[m_ValueLength]=0;
		Data+=m_ValueLength;
	}
	if (m_GraphicLength)
	{
		m_pGraphicData = new char[m_GraphicLength];
		memcpy(m_pGraphicData, Data, m_GraphicLength);
		Data+=m_GraphicLength;
	}
	if (m_MessageLength)
	{
		m_pMessage = new Message(m_MessageLength, Data);
		Data+=m_MessageLength;
	}
	if (m_ImagePathLength)
	{
		m_ImagePath = new char[m_ImagePathLength+1];
		memcpy(m_ImagePath, Data, m_ImagePathLength);
		m_ImagePath[m_ImagePathLength]=0;
	}
	if (m_NumAttributes)
	{
		for(int i=0;i<m_NumAttributes;++i)
		{
			char *pAttribute = Data;
			Data += strlen(pAttribute) +1;
			char *pValue = Data;
			Data += strlen(pValue) +1;
			m_mapAttributes[pAttribute]=pValue;
		}
	}
}

void DataGridCell::SetImage(char *Data, int Length, enum eGraphicFormat Format)
{
	m_pGraphicData = Data;
	m_GraphicLength = Length;
	m_GraphicFormat = Format;
}

void DataGridCell::SetImagePath(const char *ImagePath)
{
	delete m_ImagePath;

	m_ImagePath = new char[strlen(ImagePath)+1]; 
	strcpy(m_ImagePath,ImagePath);
	m_ImagePathLength = (int)strlen(ImagePath);
}

const char *DataGridCell::GetText()
{
	return (m_Text) ? (const char *)m_Text : "";
}

const char *DataGridCell::GetValue()
{
	return (m_Value) ? (const char *)m_Value : "";
}

void DataGridCell::SetMessage(Message* pMessage)
{	
	m_pMessage = pMessage;
}

void DataGridCell::ToData(unsigned long &Size, char* &Data)
{
	char *MessageData = NULL;
	if (m_pMessage)
		m_pMessage->ToData(m_MessageLength, MessageData);

	m_NumAttributes=(int) m_mapAttributes.size();
	m_AttributesLength=0;
	for(map<string,string>::iterator it=m_mapAttributes.begin();it!=m_mapAttributes.end();++it)
		m_AttributesLength += (int) it->first.size() + (int) it->second.size() + 2;

	Size = sizeof(DataGridCellSerializableData)+m_ImagePathLength+m_TextLength+m_ValueLength+m_GraphicLength+m_MessageLength+m_AttributesLength;
	Data = new char[Size];
	char *Datap = Data;
	memcpy(Datap, (DataGridCellSerializableData *)this, sizeof(DataGridCellSerializableData));
	Datap+=sizeof(DataGridCellSerializableData);
	if (m_TextLength)
	{
		memcpy(Datap, m_Text, m_TextLength);
		Datap+=m_TextLength;
	}
	if (m_ValueLength)
	{
		memcpy(Datap, m_Value, m_ValueLength);
		Datap+=m_ValueLength;
	}
	if (m_GraphicLength)
	{
		memcpy(Datap, m_pGraphicData, m_GraphicLength);
		Datap+=m_GraphicLength;
	}
	if (m_MessageLength)
	{
		memcpy(Datap, MessageData, m_MessageLength);
		Datap+=m_MessageLength;
	}
	if (m_ImagePathLength)
	{
		memcpy(Datap, m_ImagePath, m_ImagePathLength);
		Datap+=m_ImagePathLength;
	}
	if (m_NumAttributes)
	{
		for(map<string,string>::iterator it=m_mapAttributes.begin();it!=m_mapAttributes.end();++it)
		{
			strcpy(Datap, it->first.c_str());
			Datap+=it->first.size()+1;
			strcpy(Datap, it->second.c_str());
			Datap+=it->second.size()+1;
		}
	}

	if(NULL != MessageData)
	{
		delete[] MessageData;
		MessageData = NULL;
	}
}


DataGridTable::DataGridTable()
{
	m_iPK_Datagrid = 0;
	m_bRePopulateEachTimeRequested = false;
	Message::Message();
	ClearData();
}

DataGridTable::DataGridTable(int Size, char *Data) 
{
	m_iPK_Datagrid = 0;
	m_bRePopulateEachTimeRequested = false;
#ifndef DISABLE_LZO_DATAGRID
	int UncompressedLength = *((int *)Data);
	char *UncompressedData = new char[UncompressedLength];
	lzo_uint new_len;

#ifdef UNDER_CE
	__try
	{
		lzo1x_decompress((lzo_byte *)Data+4,Size - 4,(lzo_byte *)UncompressedData,&new_len,NULL);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//g_pPlutoLogger->Write(LV_CRITICAL,"lzo decompress data failed %p %d %p",
		//	Data,Size,UncompressedData);
	}
#else
	lzo1x_decompress((lzo_byte *)Data+4,Size - 4,(lzo_byte *)UncompressedData,&new_len,NULL);
#endif

#else
	int UncompressedLength = Size;
	char *UncompressedData = Data;
#endif

	char *Datap = UncompressedData;
	memcpy((DataGridTableSerializableData *)this, Datap, sizeof(DataGridTableSerializableData));
	Datap+=sizeof(DataGridTableSerializableData);
	int IndexSize = sizeof(DataGridTableCellIndex) * m_CellCount;
	DataGridTableCellIndex *ntIndex = new DataGridTableCellIndex[m_CellCount];
	memcpy(ntIndex, Datap, IndexSize);
	Datap+=IndexSize;
    for(int i=0;i<m_CellCount;++i)
	{
		int CellSize = ntIndex[i].m_Size;
		DataGridCell *pNewCell = new DataGridCell(CellSize, Datap);
		Datap+=CellSize;
		m_MemoryDataTable[ntIndex[i].m_ColRow]=pNewCell;
	}

	delete[] ntIndex;
#ifndef DISABLE_LZO_DATAGRID
	delete[] UncompressedData;
#endif
}

DataGridTable::~DataGridTable()
{
	ClearData();
}

int DataGridTable::GetRows()
{
	return m_TotalRows;
}

int DataGridTable::GetCols()
{
	return m_TotalColumns;
}

DataGridCell *DataGridTable::GetData(int Column, int Row)
{
	MemoryDataTable::iterator iDR = m_MemoryDataTable.find(MAKECOLROW(Column, Row));
	if (iDR!=m_MemoryDataTable.end())
		return (*iDR).second;
	else
		return NULL;
}

void DataGridTable::SetData(int Column, int Row, DataGridCell *Value)
{
	if (Column < 0 || Row <0)
		return;

	if (Row+1 > m_TotalRows)
		m_TotalRows = Row+1;

	if (Column >= m_ColumnCount+1+m_StartingColumn)
		m_ColumnCount = Column+1-m_StartingColumn;

	if (Column+1 > m_TotalColumns)
		m_TotalColumns = Column+1;

	m_MemoryDataTable[MAKECOLROW(Column, Row)]=Value;
}

void DataGridTable::ClearData()
{
	for(MemoryDataTable::iterator iDR = m_MemoryDataTable.begin();iDR != m_MemoryDataTable.end(); ++iDR)
	{
        delete (*iDR).second;
	}
	m_ColumnCount = 0;
	m_TotalColumns = 0;
	m_StartingColumn = 0;
	m_TotalRows = 0;
	m_RowCount = 0;
	m_StartingRow = 0;
	m_MemoryDataTable.clear();
}   

void DataGridTable::ToData(string GridID,int &Size, char* &Data)
{
	// Default: Serialize the whole table.
	ToData(GridID, Size, Data, 0, 0, GetCols(), GetRows());
}

void DataGridTable::ToData(string GridID,int &Size, char* &Data, int *ColStart, int *RowStart, int ColCount, int RowCount)
{
#ifdef DEBUG
g_pPlutoLogger->Write( LV_DATAGRID, "inside todata" );
#endif

	m_CellCount = 0;

	if( !(*RowStart) && !RowCount )
		RowCount = GetRows();

	int MaxCell = ColCount * RowCount;
	DataGridTableCellIndex *ntIndex = new DataGridTableCellIndex[MaxCell];
	DataGridCell **ntCells = new DataGridCell*[MaxCell];
	char **pCellData = new char*[MaxCell];
	unsigned long *pCellSize = new unsigned long[MaxCell];

	int ntTotalCellSize = 0;

	// Set the subset-related parameters. 
	m_StartingRow = (*RowStart);
	m_StartingColumn = (*ColStart);
	int GridRowsLeft = GetRows() - (*RowStart);
	m_RowCount = RowCount < GridRowsLeft ? RowCount : GridRowsLeft;
	m_ColumnCount = ColCount;

	int ActualFirstColumn = m_bKeepColumnHeader ? 1 : 0;
	register int i;
#ifdef DEBUG
g_pPlutoLogger->Write( LV_DATAGRID, "rows: %d cols: %d",m_RowCount,m_ColumnCount );
#endif

	for(i=0;i<m_RowCount;++i)
	{
		for(register int j=0;j<m_ColumnCount;++j)
		{
			DataGridCell *Cell = GetData((j==0 && m_bKeepColumnHeader) ? 0 : j+m_StartingColumn, (i==0 && m_bKeepRowHeader) ? 0 : i+m_StartingRow);
			if (!Cell && j==ActualFirstColumn)
			{
				// Hmm, no data on the first cell.  Maybe this is the middle of a columnspan?  If so
				// find the column and make the adjustment.
				for(int cssearch = m_StartingColumn+ActualFirstColumn-1; cssearch>=ActualFirstColumn; cssearch--)
				{
					Cell = GetData(cssearch, (i==0 && m_bKeepRowHeader) ? 0 : i+m_StartingRow);
                    if (Cell)
					{
						if (Cell->m_Colspan >= (1 + (m_StartingColumn-cssearch)-ActualFirstColumn))
						{
							Cell->m_Colspan -= (m_StartingColumn-cssearch+1);
							// This will work, break out of the loop and use it.
							break;
						}
						else
						{
							// There's a cell, but its column span doesn't reach. Discard and cancel.
							Cell = NULL;
							break;
						}
					}
				}

			}
			if (Cell)
			{
				if (Cell->m_Colspan + j > m_ColumnCount)
				{
					Cell->m_Colspan = m_ColumnCount - j;
				}
				ntIndex[m_CellCount].m_ColRow = MAKECOLROW((j==0 && m_bKeepColumnHeader) ? 0 : j+m_StartingColumn, (i==0 && m_bKeepRowHeader) ? 0 : i+m_StartingRow);

				char *pData=NULL;
				unsigned long CurrentCellSize;
				Cell->ToData(CurrentCellSize,pData);

				ntIndex[m_CellCount].m_Size = CurrentCellSize;
				ntCells[m_CellCount] = Cell;
				pCellData[m_CellCount] = pData;
				pCellSize[m_CellCount] = CurrentCellSize;
				m_CellCount++;
				ntTotalCellSize += CurrentCellSize;
			}
		}
	}
#ifdef DEBUG
g_pPlutoLogger->Write( LV_DATAGRID, "after loop" );
#endif

	int IndexSize = sizeof(DataGridTableCellIndex) * m_CellCount;

	int UncompressedSize = 	(sizeof(DataGridTableSerializableData)) + // The serializable data
							(IndexSize) + // The index
							(ntTotalCellSize);  // The cell data itself

	char *UncompressedData = new char[UncompressedSize];
	char *Datap = UncompressedData;


	memcpy(Datap, (DataGridTableSerializableData *)this, sizeof(DataGridTableSerializableData));
	Datap+=sizeof(DataGridTableSerializableData);
	memcpy(Datap, ntIndex, IndexSize);
	Datap+=IndexSize;
#ifdef DEBUG
g_pPlutoLogger->Write( LV_DATAGRID, "cell count %d",m_CellCount );
#endif
    for(i=0;i<m_CellCount;++i)
	{
		unsigned long CellSize=pCellSize[i];
		char *CellData=pCellData[i];
		ntCells[i]->ToData(CellSize, CellData);

		if (CellSize != ntIndex[i].m_Size)
		{
			printf("ERROR: Cell is of unexpected size!");
			exit(0);
		}
		memcpy(Datap, CellData, CellSize);
		Datap+=CellSize;
		delete[] CellData;
	}

	delete[] ntIndex;
	delete[] ntCells;
	delete[] pCellData;
	delete[] pCellSize;
#ifndef DISABLE_LZO_DATAGRID
	Data = new char[UncompressedSize + UncompressedSize / 64 + 16 + 3 + 4];
    lzo_uint out_len;
	*((int *)Data)=UncompressedSize;
    lzo1x_1_compress((lzo_byte *)UncompressedData,UncompressedSize,(lzo_byte *)Data+4,&out_len,wrkmem);
	Size = (int)out_len+4;
#ifdef DEBUG
g_pPlutoLogger->Write( LV_DATAGRID, "before delete[]" );
#endif

	// Shouldn't this have been here?
	delete[] UncompressedData;

#else
	Data = UncompressedData;
	Size = UncompressedSize;
#endif

}
	
