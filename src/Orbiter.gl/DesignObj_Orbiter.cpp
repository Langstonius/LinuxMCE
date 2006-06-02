/*
 DesignObj_Orbiter
 
 Copyright (C) 2004 Pluto, Inc., a Florida Corporation
 
 www.plutohome.com		
 
 Phone: +1 (877) 758-8648
 
 This program is distributed according to the terms of the Pluto Public License, available at: 
 http://plutohome.com/index.php?section=public_license 
 
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.
 
 */
 
#include "PlutoUtils/CommonIncludes.h"	
#include "DataGrid.h"
#include "DesignObj_Orbiter.h"
#include "Orbiter.h"

#ifdef PRONTO
#include "CCF.h"
#endif
//=======================================================================================================
//Generic PlutoGraphic class methods
//-------------------------------------------------------------------------------------------------------
PlutoGraphic::PlutoGraphic()
{
	Initialize();

	Width = 0;
	Height = 0;
}
//-------------------------------------------------------------------------------------------------------
PlutoGraphic::PlutoGraphic(Orbiter *pOrbiter)
{
	Initialize();

	m_pOrbiter = pOrbiter;
	Width = 0;
	Height = 0;
};
//-------------------------------------------------------------------------------------------------------
PlutoGraphic::PlutoGraphic(string Filename, eGraphicManagement GraphicManagement, 
						   Orbiter *pOrbiter)
{
	Initialize();

	m_pOrbiter = pOrbiter;
	m_Filename = Filename;
	m_GraphicManagement = GraphicManagement;
	Width = 0;
	Height = 0;
};
//-------------------------------------------------------------------------------------------------------
/*virtual*/ PlutoGraphic::~PlutoGraphic()
{
}
//-------------------------------------------------------------------------------------------------------
/*virtual*/ void PlutoGraphic::Initialize() 
{ 
	m_GraphicManagement = GR_KEEPCOMPRESSED; 
	m_GraphicFormat = GR_UNKNOWN; 
	m_Filename = "";
	m_pOrbiter = NULL;
}
//=======================================================================================================
//Concrete class DesignObj_Orbiter
//-------------------------------------------------------------------------------------------------------
DesignObj_Orbiter::DesignObj_Orbiter(Orbiter *pCore)
{
	m_pDesignObj_Orbiter_Up=m_pDesignObj_Orbiter_Down=m_pDesignObj_Orbiter_Left=m_pDesignObj_Orbiter_Right=
		m_pDesignObj_Orbiter_TiedTo=NULL;
	m_pDataGridTable=NULL;
	m_pCore = pCore;

	m_MaxRow=m_MaxCol=0;
	m_vectGraphic.clear();
	m_vectSelectedGraphic.clear();
	m_vectHighlightedGraphic.clear();
	m_vectAltGraphics.clear();
	m_pvectCurrentGraphic = NULL;
	m_pGraphicToUndoSelect = NULL;
	m_pvectCurrentPlayingGraphic = NULL;
	m_pObj_Screen = NULL;
	m_bIsARemoteControl = false;

	m_iCurrentFrame = 0;

	m_pCriteria=NULL;
	m_GraphicToDisplay=GRAPHIC_NORMAL;
	m_GraphicToPlay=GRAPHIC_NORMAL;
	m_GridCurRow=-1;
	m_GridCurCol=-1;
	m_bOneTimeDontReset=false;
	m_pParentObject=NULL;
	m_pWebWindow=NULL;
	m_bHidden=false;
	m_pFloorplanObject=NULL;
#ifdef PRONTO
	m_pCCF=NULL;
#endif
	m_bDisableAspectLock=false;
	m_bOnScreen=false;
	m_bContainsDataGrid=false;
	m_bIsBoundToUser=m_bIsBoundToLocation=false;
};
//-------------------------------------------------------------------------------------------------------
DesignObj_Orbiter::~DesignObj_Orbiter()
{
/* todo 2.0, dynamic_cast isn't cross-platform, I don't think
	if (m_pGraphic) 
	{
		//only if this is an WinGraphic object!
		if (dynamic_cast<WinGraphic *>(m_pGraphic) && 
			(dynamic_cast<WinGraphic *>(m_pGraphic))->m_pUncompressedImage
		)
		{
			m_pCore->RemoveUncompressedImage((dynamic_cast<WinGraphic *>(m_pGraphic))->m_pUncompressedImage);
		}

		delete m_pGraphic;
		m_pGraphic = NULL;
	}
	
	if (m_pSelectedGraphic)
	{
		if (dynamic_cast<WinGraphic *>(m_pSelectedGraphic) && 
			(dynamic_cast<WinGraphic *>(m_pSelectedGraphic))->m_pUncompressedImage
		)
		{
			m_pCore->RemoveUncompressedImage((dynamic_cast<WinGraphic *>(m_pSelectedGraphic))->m_pUncompressedImage);
		}

		delete m_pSelectedGraphic;
		m_pSelectedGraphic = NULL;
	} 
	if (m_pHighlightedGraphic)
	{
		if (dynamic_cast<WinGraphic *>(m_pHighlightedGraphic) && 
			(dynamic_cast<WinGraphic *>(m_pHighlightedGraphic))->m_pUncompressedImage
		)
		{
			m_pCore->RemoveUncompressedImage((dynamic_cast<WinGraphic *>(m_pHighlightedGraphic))->m_pUncompressedImage);
		}

		delete m_pHighlightedGraphic;
		m_pHighlightedGraphic = NULL;
	} 

	int i;
*/

	for(list<class PlutoPopup*>::iterator it=m_listPopups.begin();it!=m_listPopups.end();++it)
		delete *it;
    m_listPopups.clear();

	m_pGraphicToUndoSelect = NULL;
	m_pvectCurrentGraphic = m_pvectCurrentPlayingGraphic = NULL;

	for(VectorPlutoGraphic::iterator itGraphic = m_vectGraphic.begin(); itGraphic != m_vectGraphic.end(); ++itGraphic)
		delete *itGraphic;
	m_vectGraphic.clear();

	for(VectorPlutoGraphic::iterator itSelectedGraphic = m_vectSelectedGraphic.begin(); itSelectedGraphic != m_vectSelectedGraphic.end(); ++itSelectedGraphic)
		delete *itSelectedGraphic;
	m_vectSelectedGraphic.clear();

	for(VectorPlutoGraphic::iterator itHighlightedGraphic = m_vectHighlightedGraphic.begin(); itHighlightedGraphic != m_vectHighlightedGraphic.end(); ++itHighlightedGraphic)
		delete *itHighlightedGraphic;
	m_vectHighlightedGraphic.clear();

	for(vector<VectorPlutoGraphic>::iterator itVectAltGraphic = m_vectAltGraphics.begin(); itVectAltGraphic != m_vectAltGraphics.end(); ++itVectAltGraphic)
	{
		for(VectorPlutoGraphic::iterator itAltGraphic = itVectAltGraphic->begin(); itAltGraphic != itVectAltGraphic->end(); ++itAltGraphic)
			delete *itAltGraphic;

		itVectAltGraphic->clear();
	}
	m_vectAltGraphics.clear();


	m_bvectLoop_Alt.clear();

	DesignObjZoneList::iterator iZone;
	for(iZone = m_ZoneList.begin(); iZone !=m_ZoneList.end(); ++iZone)
		delete *iZone; 

	DesignObjCommandList::iterator iA;
	for(iA = m_Action_StartupList.begin(); iA != m_Action_StartupList.end(); ++iA)
		delete *iA;
	for(iA = m_Action_TimeoutList.begin(); iA != m_Action_TimeoutList.end(); ++iA)
		delete *iA;
	for(iA = m_Action_LoadList.begin(); iA != m_Action_LoadList.end(); ++iA)
		delete *iA; 
	for(iA = m_Action_UnloadList.begin(); iA != m_Action_UnloadList.end(); ++iA)
		delete *iA; 
	for(iA = m_Action_HighlightList.begin(); iA != m_Action_HighlightList.end(); ++iA)
		delete *iA; 
	for(iA = m_Action_UnhighlightList.begin(); iA != m_Action_UnhighlightList.end(); ++iA)
		delete *iA; 
	VectorDesignObjText::iterator iT;
	for(iT=m_vectDesignObjText.begin(); iT != m_vectDesignObjText.end(); ++iT)
	{
		//DesignObjText *pDummy = *iT;
		delete *iT; 
	}

	//IMPORTANT!!!
	//because all DesignObj_Orbiter objects are stored on Orbiter::m_map_ObjAll,
	//we don't want to delete here children of the object 
	//their children will be deleted in Orbiter destructor using map_ObjAll
	m_ChildObjects.clear();

	delete m_pDataGridTable;
// todo 2.0	delete m_pWebWindow; 
#ifdef PRONTO
	delete m_pCCF;
#endif
};
//-------------------------------------------------------------------------------------------------------
string DesignObj_Orbiter::GetParameterValue(int ParameterID)
{
	map<int,string>::iterator ipParm = m_mapObjParms.find(ParameterID);
	if (ipParm==m_mapObjParms.end())
		return "";
	return m_pCore->SubstituteVariables((*ipParm).second,this,0,0);
}

PlutoRectangle DesignObj_Orbiter::GetHighlightRegion()
{
	PlutoRectangle r;

	r.X = max(0,m_rPosition.X-4);
	r.Y = max(0,m_rPosition.Y-4);
	r.Right( min(m_rPosition.Right()+4,m_pCore->m_Width-1) );
	r.Bottom( min(m_rPosition.Bottom()+4,m_pCore->m_Height-1) );
	return r;
}

//=======================================================================================================
//Concrete class DesignObj_DataGrid
//-------------------------------------------------------------------------------------------------------
DesignObj_DataGrid::~DesignObj_DataGrid() 
{
	for(int i=0;i<CACHE_SIZE;++i)
		delete m_pDataGridTableCache[i];		
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::CanGoUp()			
{ 
	return m_GridCurRow > 0;																
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::CanGoDown()		
{ 
	return m_pDataGridTable->GetRows() > m_GridCurRow + m_MaxRow - (HasMoreUp() ? 1 : 0); 
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::CanGoLeft()		
{ 
	return m_GridCurCol > 0;																
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::CanGoRight()		
{ 
	return m_pDataGridTable->GetCols() > m_GridCurCol + m_MaxCol;							
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::VerticalOnly()		
{ 
	return !CanGoLeft() && !CanGoRight();													
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::HasMoreUp()		
{ 
	return VerticalOnly() && CanGoUp();													
}
//-------------------------------------------------------------------------------------------------------
bool DesignObj_DataGrid::HasMoreDown()		
{ 
	return VerticalOnly() && CanGoDown();													
}
//-------------------------------------------------------------------------------------------------------
