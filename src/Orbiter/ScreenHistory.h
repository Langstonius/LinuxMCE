#ifndef __SCREEN_HISTORY_H__
#define __SCREEN_HISTORY_H__
//-----------------------------------------------------------------------------------------------------
#include <list>
#include <string>
#include <map>
using namespace std;

#include "DesignObj_Orbiter.h"
using namespace DCE;

namespace DCE
{
	//-----------------------------------------------------------------------------------------------------
	/**
	* @brief Contains a snapshot of the screen that was viewed and the state so that it can be restored with GoBack
	*/
	//-----------------------------------------------------------------------------------------------------

	class Message;

	class ScreenHistory
	{
	private:

		Orbiter *m_pOrbiter;
		DesignObj_Orbiter *m_pObj; /** < The screen we're viewing */
		list<DesignObj_Orbiter *> m_listObjs; /** < The list of design objects for this screen */
		map<int, string> m_mapVariable; /** < Any variables we need to restore when returning to this screen */
		time_t m_tTime; /** < The moment of time when this screen was created */
		map<DesignObj_Orbiter *, bool> m_mapVisibilityContext; /** < We keep here the visibily status for each object */

		string m_sID; /** < The id for current design object */
		int m_nPK_Screen; /** < The PK_Screen of the screen */

		/** < If we get a go back, skip over this screen unless "Force" is set to true */
		bool m_bCantGoBack; 

	public:
		map<long, string> m_mapParameters; /** < Any parameters passed in when the screen was created */

		/** < We'll know if we need to add an object to history or not (maybe we are doing a go back) */
		static bool m_bAddToHistory; 

		/** < Constructor and destructor */
		ScreenHistory(int nPK_Screen, class ScreenHistory *pScreenHistory_Prior,Message *pMessage,Orbiter *pOrbiter);
		~ScreenHistory();

		/** < Getter and setter for current design object for this screen */
		DesignObj_Orbiter *GetObj();
		void SetObj(DesignObj_Orbiter *pObj);

		/** < It will add the current design object to history for this screen */
		void AddToHistory();

		/** < Returns true if list with design objects for this screen is empty */
		bool HistoryEmpty();

		/** < Purges the list with design object within history list for this screen */
		void PurgeHistory();

		/** < Returns true if we can go back in this screen (from a design obj to another) */
		bool GoBack();

		/** < Returns a dump with current state - used for debugging */
		string ToString();

		/** < can go back or not? */
		bool CantGoBack();
		void CantGoBack(bool bCantGoBack);

		/** < Setter and getter for current design obj's ID */
		string ScreenID();
		void ScreenID(string sID);

		/** < Setter and getter for screen's PK_Screen */
		int PK_Screen();
		void PK_Screen(int nPK_Screen);

		/** < Returns the moment of time when this screen was created */
		time_t TimeCreated() const;

		/** < Saves/restores to map with variables and the visibily status for each object */
		void SaveContext(const map<int, string>& mapVariable, 
			const map<DesignObj_Orbiter *, bool>& mapVisibilityContext);
		void RestoreContext(map<int, string>& mapVariable,
			map<DesignObj_Orbiter *, bool>& mapVisibilityContext);
	};
}
//-----------------------------------------------------------------------------------------------------
#endif
