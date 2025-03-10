/*
 Main

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com
 

 Phone: +1 (877) 758-8648


 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */
#if !defined(AFX_LISTCTRLHIDDENSB_H__16168A9D_2FBD_4689_84BD_2EB4509C255E__INCLUDED_)
#define AFX_LISTCTRLHIDDENSB_H__16168A9D_2FBD_4689_84BD_2EB4509C255E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlHiddenSB.h : header file
//

#define LCSB_CLIENTDATA 1
#define LCSB_NCOVERRIDE 2

//Definision of which is used as default.
//#define SB_HORZ             0
//#define SB_VERT             1
//#define SB_BOTH             3

/////////////////////////////////////////////////////////////////////////////
// CListCtrlHiddenSB window

class CListCtrlHiddenSB : public CListCtrl
{
// Construction
public:
	CListCtrlHiddenSB();

// Attributes
public:

// Operations
public:
	void HideScrollBars(int Type, int Which=SB_BOTH);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlHiddenSB)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListCtrlHiddenSB();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlHiddenSB)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	BOOL NCOverride;
	int Who;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLHIDDENSB_H__16168A9D_2FBD_4689_84BD_2EB4509C255E__INCLUDED_)
