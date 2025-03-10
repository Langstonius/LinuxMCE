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
#if !defined(AFX_ABOUTCTRL_H__B75B0B9E_367D_11D6_AB38_00D0B70C3D79__INCLUDED_)
#define AFX_ABOUTCTRL_H__B75B0B9E_367D_11D6_AB38_00D0B70C3D79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAboutCtrl : public CStatic
{
// Construction
public:
	CAboutCtrl();

// Attributes
protected:
	HDC m_hLogoDC; 
	HDC m_hBackgroundDC; 
	HDC m_hMemDC; 
	HDC m_hCreditsDC; 

	// logo dimensions
	int m_cxLogo, m_cyLogo;
	// logo angle
	int m_xAngle, m_yAngle;
	// logo 'fly' speed
	int m_nSpeed;
	// logo position
	int m_xPos, m_yPos; 

	int m_cxData;
	int m_cyData;

	int m_nCounter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetCredits(LPCTSTR lpszCredits);
	virtual ~CAboutCtrl();

	// Generated message map functions
protected:
	CString m_strCredits;
	void Initialize();
	void AnimateLogo();
	void LoadPicture(int nResourceID, HDC &hDestinationDC, int &nWidth, int &nHeight, HDC hDC);
	void LoadCredits(HDC &hDestination, int nWidth, int nHeight, HDC hDC);

	//{{AFX_MSG(CAboutCtrl)
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTCTRL_H__B75B0B9E_367D_11D6_AB38_00D0B70C3D79__INCLUDED_)
