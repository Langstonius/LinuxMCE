#ifndef DesignObj_Generator_h
#define DesignObj_Generator_h

#include "OGText.h"
#include "Orbiter/DesignObj_Data.h"

typedef vector<class DesignObj_Generator *> listDesignObj_Generator;

class DesignObj_Generator : public DesignObj_Data
{
public:
	class Database_pluto_main * m_mds;
	class Row_DesignObj * m_pRow_DesignObj;
	DesignObj_Generator * m_ocoParent,*m_pDesignObj_TopMost;
	class OrbiterGenerator *m_pOrbiterGenerator;
	class PlutoRectangle m_rBitmapOffset;
	map<int,string> m_VariableMap;
	int m_PK_DesignObj_Goto;

	// These are only used on topmost screens
	string m_sFloorPlanData;
	int m_iNumFloorplanItems;

	// These are the input values, the 'real' location on the server that Orbiter Gen uses.  The regular variables without Orig
	// are the rendered versions.
	string m_sOrigBackgroundFile, m_sOrigSelectedFile, m_sOrigHighlightGraphicFilename;
	vector<string> m_vectOrigAltGraphicFilename;

	vector<string> m_vectRegenMonitor;  // A list of strings indicating dependencies for regenerating

	class Row_DesignObjVariation * m_pRow_DesignObjVariation,*m_pRow_DesignObjVariation_Standard;
	vector<class Row_DesignObjVariation *> m_alDesignObjVariations;
//	vector<class CGZone *> m_alZones;
//	vector<class CGCommand *> m_alLoadCommands,m_alUnloadCommands,m_alTimeoutCommands,m_alStartupCommands;

	vector<class DesignObj_Generator *> m_alChildDesignObjs;
	vector<class CGArray *> m_alNonMPArrays,m_alMPArray;
	vector<class CGVarAss *> m_alVariableAssignments;
	DesignObj_Generator *m_DesignObj_GeneratorGoto;
	string m_sDesignObjGoto;
	bool m_bContainsArrays;
	bool m_bDontShare,m_bUsingCache,m_bRendered,m_bPreserveAspectRatio,m_bPreserveTransparencies;
	bool m_bCanBeHidden,m_bChildrenBehind,m_bDontMergeBackground;
	bool m_bValuesScaled,m_bContainsFloorplans;
	bool m_bUseOCG,m_bIsPopup,m_bLocationSpecific;
	PlutoPoint m_pFloorplanFillPoint;
	int m_iScale,m_iScaleFromParent;  // If the user put a Scale this object command, this object and all it's children will be scaled.  m_iScaleFromParent is the scale of the parent object
	int m_iFloorplanPage,m_iFloorplanDevice; // Only used for floorplan objects
	int m_iPK_CommandGroup_Touch_Extra;  // An extra action group to execute, used when this object is a button in an action group array.  
	class Row_Device * m_pRow_Device_Goto;

	DesignObj_Generator() { m_bRendered=false; } // when we're serializing from disk
	DesignObj_Generator(class OrbiterGenerator *pOrbiterGenerator,class Row_DesignObj * drDesignObj,class PlutoRectangle rPosition,class DesignObj_Generator *ocoParent,bool bAddToGenerated,bool bDontShare);
	~DesignObj_Generator();

	int LookForGoto(DesignObjCommandList *alCommands);
	void HandleGoto(int PK_DesignObj_Goto);
	static void PickVariation(OrbiterGenerator *pGenerator,class Row_DesignObj *drDesignObj,class Row_DesignObjVariation **drDesignObjVariation,class Row_DesignObjVariation **drStandardVariation, vector<class Row_DesignObjVariation *> *alDesignObjVariations);
	static void PickStyleVariation(class Row_Style * drStyle,OrbiterGenerator *pGenerator,MapTextStyle &mapTextStyle);
	static TextStyle *PickStyleVariation(vector<Row_StyleVariation *> &vectrsv,OrbiterGenerator *pGenerator,int Version);
	void HandleRotation(int iRotate);

	vector<class ArrayValue *> *GetArrayValues(class Row_DesignObjVariation_DesignObj * drOVO);
	void ScaleAllValues(int FactorX,int FactorY,class DesignObj_Generator *pTopmostObject);
	class PlutoPoint *ScaleValue(class PlutoPoint *pt,int FactorX,int FactorY);
	string GetParm(int PK_Parameter);
	string GetParm(int PK_Parameter,bool bReplaceNulls);
	string GetParm(int PK_Parameter,class Row_DesignObjVariation * drDesignObjVariation);
	string GetParm(int PK_Parameter,class Row_DesignObjVariation * drDesignObjVariation,bool bReplaceNulls);

	string SubstituteVariables(string Text,bool *bContainsRunTimeVariables);

	DesignObj_Generator *GetTopMostObject() {return m_ocoParent ? m_ocoParent->GetTopMostObject() : this;}
	bool CachedVersionOK();
	void PurgeForRegeneration(); // If we just read from a cache, but won't use the cache, purge everything so we can regen
	void WriteRegenVersion(string sFilename);
	bool ReadRegenVersion(string sFilename);
	bool ReadFloorplanInfo(string sFilename);
	void WriteFloorplanInfo(string sFilename);
	string GetText(int PK_Text); // Returns the text string in the current language
};

#endif
