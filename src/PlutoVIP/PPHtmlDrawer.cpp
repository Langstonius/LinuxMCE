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
/////////////////////////////////////////////////////////////////////
// PPHtmlDrawer.cpp : implementation file
//-----------------------------------------
// Author:			Eugene Pustovoyt
// Created:			8 May 2003
// Last Modified:	05 May 2004
// Current Version: 1.1
//
//--- History ------------------------------ 
// 2004/03/01 *** Releases version 1.0 ***
//------------------------------------------
// 2004/04/13 [+] Added a "speed" parameter to a <ilst> tag for animation
// 2004/04/20 [+]
//			  [+] For non-MFC program added STL class CStdString
// 2004/05/05 [+] Added an EnableOutput method
//			  [-] Fixed an error in SetImageList method (thanks to topus)
/////////////////////////////////////////////////////////////////////
//
// "GotoURL" function by Stuart Patterson
// As seen in the August, 1997 Windows Developer's Journal.
// Copyright 1997 by Miller Freeman, Inc. All rights reserved.
// Modified by Chris Maunder to use TCHARs instead of chars.
//
// "Default hand cursor" from Paul DiLascia's Jan 1998 MSJ article.
// Modified by Zorglab to use standard hand cursor on WinMe,2k,XP
//
/////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PPHtmlDrawer.h"
#include "atlconv.h"    // for Unicode conversion - requires #include <afxdisp.h> // MFC OLE automation classes

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PPHTMLDRAWER_NO_HOVERLINK	-2	//A hot area is not exist under the cursor

/*
#define m_szOffsetShadow.cx		4 //
#define m_szOffsetShadow.cy		4 //
#define m_szDepthShadow.cx		7 //
#define m_szDepthShadow.cy		7 //
#define PPHTMLDRAWER_SHADOW_COLOR		RGB (64, 64, 64) //A gradient shadow's color
*/


/////////////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer

CPPHtmlDrawer::CPPHtmlDrawer()
{
	m_hInstDll = NULL;
	m_bFreeInstDll = FALSE;
	m_hDC = NULL;
	
	m_csCallbackRepaint.hWnd = NULL;
	m_csCallbackRepaint.nMessage = 0;
	m_csCallbackRepaint.lParam = 0;
	m_csCallbackRepaint.wParam = 0;
	
	m_csCallbackLink.hWnd = NULL;
	m_csCallbackLink.nMessage = 0;
	m_csCallbackLink.lParam = 0;
	m_csCallbackLink.wParam = 0;

//	m_clrShadow = PPHTMLDRAWER_SHADOW_COLOR;

	m_hLinkCursor = NULL; // No cursor as yet
	m_nHoverIndexLink = PPHTMLDRAWER_NO_HOVERLINK;

	SetListOfTags();
    SetTableOfColors();
	SetDefaultCursor();
	EnableEscapeSequences();
	SetImageShadow(4, 4);
	SetDefaultCssStyles();
	EnableOutput();
	SetDisabledColor(::GetSysColor(COLOR_BTNSHADOW));
}

CPPHtmlDrawer::~CPPHtmlDrawer()
{
	SetResourceDll(NULL);

	if (NULL != m_hLinkCursor)
	{
		::DestroyCursor(m_hLinkCursor);
		m_hLinkCursor = NULL;
	}
	
	if (m_ImageList.m_hImageList != NULL)
		m_ImageList.DeleteImageList();
}

void CPPHtmlDrawer::EnableOutput(BOOL bEnable /* = TRUE */)
{
	m_bIsEnable = bEnable;
} //End of EnableOutput

void CPPHtmlDrawer::SetDisabledColor(COLORREF color)
{
	m_crDisabled = color;
}

HICON CPPHtmlDrawer::GetIconFromResources(DWORD dwID, int nWidth /* = 0 */, int nHeight /* = 0 */) const
{
	if (0 == dwID) return NULL;

	// Find correct resource handle
	HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(dwID), RT_GROUP_ICON);
	// Set icon when the mouse is IN the button
	HICON hIcon = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(dwID), IMAGE_ICON, nWidth, nHeight, LR_DEFAULTCOLOR);
	
	return hIcon;
}

HICON CPPHtmlDrawer::GetIconFromFile(LPCTSTR lpszPath, int nWidth /* = 0 */, int nHeight /* = 0 */) const
{
	HICON hIcon = (HICON)::LoadImage(NULL, lpszPath, IMAGE_ICON, nWidth, nHeight, LR_LOADFROMFILE | LR_DEFAULTCOLOR);
	
	return hIcon;
}

HICON CPPHtmlDrawer::GetIconFromDll(DWORD dwID, int nWidth /* = 0 */, int nHeight /* = 0 */, LPCTSTR lpszPathDll /* = NULL */) const
{
	if (0 == dwID) return NULL;

	HICON hIcon = NULL;

	HINSTANCE hInstDll = NULL;
	BOOL bNewDll = FALSE;

	if (NULL == lpszPathDll)
	{
		if (NULL != m_hInstDll)
			hInstDll = m_hInstDll;
	}
	else
	{
		//Load New Library
		hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
		if (NULL != hInstDll)
			bNewDll = TRUE;	
	}

	if (NULL != hInstDll)
	{
		hIcon = (HICON)::LoadImage(hInstDll, MAKEINTRESOURCE(dwID), IMAGE_ICON, nWidth, nHeight, LR_DEFAULTCOLOR);

		if (bNewDll)
			::FreeLibrary(hInstDll);
	}

	return hIcon;
}

HBITMAP CPPHtmlDrawer::GetBitmapFromResources(DWORD dwID) const
{
	if (0 == dwID) return NULL;

	// Find correct resource handle
	HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(dwID), RT_BITMAP);
	// Load bitmap
	HBITMAP hBitmap = (HBITMAP)::LoadImage(hInstResource, MAKEINTRESOURCE(dwID), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	
	return hBitmap;
}

HBITMAP CPPHtmlDrawer::GetBitmapFromFile(LPCTSTR lpszPath) const
{
	HBITMAP hBitmap = (HBITMAP)::LoadImage(NULL, lpszPath, IMAGE_BITMAP,
		0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

	return hBitmap;
}

HBITMAP CPPHtmlDrawer::GetBitmapFromDll(DWORD dwID, LPCTSTR lpszPathDll /* = NULL */) const
{
	if (0 == dwID) return NULL;

	HBITMAP hBitmap = NULL;

	HINSTANCE hInstDll = NULL;
	BOOL bNewDll = FALSE;

	if (NULL == lpszPathDll)
	{
		if (NULL != m_hInstDll)
			hInstDll = m_hInstDll;
	}
	else
	{
		//Load New Library
		hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
		if (NULL != hInstDll)
			bNewDll = TRUE;	
	}

	if (NULL != hInstDll)
	{
		hBitmap = (HBITMAP)::LoadImage(hInstDll, MAKEINTRESOURCE(dwID), IMAGE_BITMAP,
			0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

		if (bNewDll)
			::FreeLibrary(hInstDll);
	}

	return hBitmap;
}

CPPString CPPHtmlDrawer::GetStringFromResource(DWORD dwID) const
{
	if (0 == dwID) return _T("");

	CPPString str;
	str.LoadString(dwID);

	return str;
}

CPPString CPPHtmlDrawer::GetStringFromDll(DWORD dwID, LPCTSTR lpszPathDll /* = NULL */) const
{
	if (0 == dwID) return _T("");

	CPPString str = _T("");

	HINSTANCE hInstDll = NULL;
	BOOL bNewDll = FALSE;

	if (NULL == lpszPathDll)
	{
		if (NULL != m_hInstDll)
			hInstDll = m_hInstDll;
	}
	else
	{
		//Load New Library
		hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
		if (NULL != hInstDll)
			bNewDll = TRUE;	
	}

	if (NULL != hInstDll)
	{
#ifdef _UNICODE
#define CHAR_FUDGE 1    // one TCHAR unused is good enough
#else
#define CHAR_FUDGE 2    // two BYTES unused for case of DBC last char
#endif
		// try fixed buffer first (to avoid wasting space in the heap)
		TCHAR szTemp[256];
		
		DWORD dwLen = ::LoadString(hInstDll, dwID, szTemp, (sizeof(szTemp) * sizeof(TCHAR)));
		// If resource not found (or ::LoadString failure)
		if (0 != dwLen) 
		{
			if ((sizeof(szTemp) * sizeof(TCHAR)) - dwLen > CHAR_FUDGE)
			{
				str = szTemp;
			} // if
			else
			{
				// try buffer size of 512, then larger size until entire string is retrieved
				int nSize = 256;
				do
				{
					nSize += 256;
					dwLen = ::LoadString(hInstDll, dwID, str.GetBuffer(nSize-1), nSize);
				} while (nSize - dwLen <= CHAR_FUDGE);
				str.ReleaseBuffer();
			}
#undef CHAR_FUDGE
		}

		if (bNewDll)
			::FreeLibrary(hInstDll);
	}
	return str;
}

///////////////////////////////////////////////////////////
// Get tooltip string for menu and toolbar items from the 
// resources of the application.
// 
// Parameters:
//		nID - Resource ID of the string
//		nNumParam - Which parameter will gets:
//					 0=long,
//					 1=short,
//					 2=disable
//
//
// Format prompt string:  long prompt \n short prompt \n disable prompt
////////////////////////////////////////////////////////////
CPPString CPPHtmlDrawer::GetResCommandPrompt(UINT nID, UINT nNumParam /* = 0 */)
{
	CPPString str = GetStringFromResource(nID);
	if (!str.IsEmpty())
	{
		int nFirst = 0;
		int nLast = nFirst;
		UINT nCount = 0;
		while ((nCount <= nNumParam) && (nFirst < str.GetLength()))
		{
			nLast = str.Find(_T('\n'), nFirst);
			if (nLast < 0)
			{
				//Char wasn't found
				if (nCount == nNumParam)
					str = str.Mid(nFirst, str.GetLength() - nFirst);
				else
					str.Empty();
				
				return str;
			}
			else
			{
				//Char was found
				if (nCount == nNumParam)
				{
					str = str.Mid(nFirst, nLast - nFirst);
					return str;
				}
				else
				{
					nFirst = nLast + 1;
				} //if
			} //if
			nCount ++;
		} //while
	} //if

	return _T("");
} //End of GetResCommandPrompt

/////////////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer message handlers

void CPPHtmlDrawer::SetListOfTags()
{
	AddTagToList(_T("b"), TAG_BOLD, _T("bold"));
	AddTagToList(_T("i"), TAG_ITALIC, _T("italic"));
	AddTagToList(_T("em"), TAG_ITALIC, _T("italic"));
	AddTagToList(_T("u"), TAG_UNDERLINE, _T("underline"));
	AddTagToList(_T("s"), TAG_STRIKEOUT, _T("strikeout"));
	AddTagToList(_T("strike"), TAG_STRIKEOUT, _T("strikeout"));
	AddTagToList(_T("font"), TAG_FONT, _T("font"));
	AddTagToList(_T("hr"), TAG_HLINE, _T(""));
	AddTagToList(_T("br"), TAG_NEWLINE, _T(""));
	AddTagToList(_T("\n"), TAG_NEWLINE, _T(""));
	AddTagToList(_T("t"), TAG_TABULATION, _T(""));
	AddTagToList(_T("\t"), TAG_TABULATION, _T(""));
	AddTagToList(_T("left"), TAG_LEFT, _T("left"));
	AddTagToList(_T("center"), TAG_CENTER, _T("center"));
	AddTagToList(_T("right"), TAG_RIGHT, _T("right"));
	AddTagToList(_T("baseline"), TAG_BASELINE, _T("baseline"));
	AddTagToList(_T("top"), TAG_TOP, _T("top"));
	AddTagToList(_T("vcenter"), TAG_VCENTER, _T("vcenter"));
	AddTagToList(_T("middle"), TAG_VCENTER, _T("vcenter"));
	AddTagToList(_T("bottom"), TAG_BOTTOM, _T("vcenter"));
	AddTagToList(_T("bmp"), TAG_BITMAP, _T(""));
	AddTagToList(_T("icon"), TAG_ICON, _T(""));
	AddTagToList(_T("ilst"), TAG_IMAGELIST, _T(""));
	AddTagToList(_T("string"), TAG_STRING, _T(""));
	AddTagToList(_T("body"), TAG_NEWSTYLE, _T("body"));
	AddTagToList(_T("h1"), TAG_NEWSTYLE, _T("h1"));
	AddTagToList(_T("h2"), TAG_NEWSTYLE, _T("h2"));
	AddTagToList(_T("h3"), TAG_NEWSTYLE, _T("h3"));
	AddTagToList(_T("h4"), TAG_NEWSTYLE, _T("h4"));
	AddTagToList(_T("h5"), TAG_NEWSTYLE, _T("h5"));
	AddTagToList(_T("h6"), TAG_NEWSTYLE, _T("h6"));
	AddTagToList(_T("code"), TAG_NEWSTYLE, _T("code"));
	AddTagToList(_T("pre"), TAG_NEWSTYLE, _T("pre"));
	AddTagToList(_T("big"), TAG_NEWSTYLE, _T("big"));
	AddTagToList(_T("small"), TAG_NEWSTYLE, _T("small"));
	AddTagToList(_T("sub"), TAG_NEWSTYLE, _T("sub"));
	AddTagToList(_T("sup"), TAG_NEWSTYLE, _T("sup"));
	AddTagToList(_T("span"), TAG_SPAN, _T("span"));
	AddTagToList(_T("a"), TAG_HYPERLINK, _T("link"));
} //End of SetListOfTags

void CPPHtmlDrawer::AddTagToList(LPCTSTR lpszName, DWORD dwTagIndex, LPCTSTR lpszFullName)
{
	STRUCT_TAGPROP tp;
	tp.dwTagIndex = dwTagIndex;
	tp.strTagName = lpszFullName;

	iterMapTags iterMap = m_mapTags.find(lpszName);
	
	if (iterMap != m_mapTags.end())
	{
		iterMap->second = tp; //Modifies
	}
	else
		m_mapTags.insert(std::make_pair(lpszName, tp)); //Add new
} //End of AddTagToList

DWORD CPPHtmlDrawer::GetTagFromList(LPCTSTR lpszName, CPPString & strFullName, BOOL & bCloseTag)
{
	CPPString str = lpszName;
	str.MakeLower();
	bCloseTag = (str.GetAt(0) == _T('/')) ? TRUE : FALSE;
	if (bCloseTag)
		str = str.Mid(1);

	iterMapTags iterMap = m_mapTags.find(str);
	
	if (iterMap != m_mapTags.end())
	{
		STRUCT_TAGPROP tp = iterMap->second;
		strFullName = tp.strTagName;
		
		return tp.dwTagIndex;
	} //if

	strFullName.Empty();
	return TAG_NONE;
} //End of GetTagFromList

///////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////
void CPPHtmlDrawer::SetTableOfColors()
{
	//Frequency used
	SetColorName(_T("aqua"), RGB(0x00, 0xFF, 0xFF)); 
	SetColorName(_T("black"), RGB(0x00, 0x00, 0x00)); 
	SetColorName(_T("blue"), RGB(0x00, 0x00, 0xFF)); 
	SetColorName(_T("brown"), RGB(0xA5, 0x2A, 0x2A)); 
	SetColorName(_T("cyan"), RGB(0x00, 0xFF, 0xFF));
	SetColorName(_T("gold"), RGB(0xFF, 0xD7, 0x00)); 
	SetColorName(_T("gray"), RGB(0x80, 0x80, 0x80)); 
	SetColorName(_T("green"), RGB(0x00, 0x80, 0x00)); 
	SetColorName(_T("magenta"), RGB(0xFF, 0x00, 0xFF)); 
	SetColorName(_T("maroon"), RGB(0x80, 0x00, 0x00)); 
	SetColorName(_T("navy"), RGB(0x00, 0x00, 0x80)); 
	SetColorName(_T("olive"), RGB(0x80, 0x80, 0x00)); 
	SetColorName(_T("orange"), RGB(0xFF, 0xA5, 0x00)); 
	SetColorName(_T("pink"), RGB(0xFF, 0xC0, 0xCB)); 
	SetColorName(_T("purple"), RGB(0x80, 0x00, 0x80)); 
	SetColorName(_T("red"), RGB(0xFF, 0x00, 0x00)); 
	SetColorName(_T("silver"), RGB(0xC0, 0xC0, 0xC0)); 
	SetColorName(_T("snow"), RGB(0xFF, 0xFA, 0xFA)); 
	SetColorName(_T("violet"), RGB(0xEE, 0x82, 0xEE)); 
	SetColorName(_T("white"), RGB(0xFF, 0xFF, 0xFF)); 
	SetColorName(_T("yellow"), RGB(0xFF, 0xFF, 0x00)); 

	//Common Used
	SetColorName(_T("aliceblue"), RGB(0xF0, 0xF8, 0xFF)); 
	SetColorName(_T("antiquewhite"), RGB(0xFA, 0xEB, 0xD7)); 
	SetColorName(_T("aquamarine"), RGB(0x7F, 0xFF, 0xD4)); 
	SetColorName(_T("azure"), RGB(0xF0, 0xFF, 0xFF)); 
	SetColorName(_T("beige"), RGB(0xF5, 0xF5, 0xDC)); 
	SetColorName(_T("bisque"), RGB(0xFF, 0xE4, 0xC4));
	SetColorName(_T("blanchedalmond"), RGB(0xFF, 0xEB, 0xCD)); 
	SetColorName(_T("blueviolet"), RGB(0x8A, 0x2B, 0xE2)); 
	SetColorName(_T("burlywood"), RGB(0xDE, 0xB8, 0x87)); 
	SetColorName(_T("cadetblue"), RGB(0x5F, 0x9E, 0xA0)); 
	SetColorName(_T("chartreuse"), RGB(0x7F, 0xFF, 0x00)); 
	SetColorName(_T("chocolate"), RGB(0xD2, 0x69, 0x1E)); 
	SetColorName(_T("coral"), RGB(0xFF, 0x7F, 0x50)); 
	SetColorName(_T("cornflowerblue"), RGB(0x64, 0x95, 0xED)); 
	SetColorName(_T("cornsilk"), RGB(0xFF, 0xF8, 0xDC)); 
	SetColorName(_T("crimson"), RGB(0xDC, 0x14, 0x3C)); 
	SetColorName(_T("darkblue"), RGB(0x00, 0x00, 0x8B)); 
	SetColorName(_T("darkcyan"), RGB(0x00, 0x8B, 0x8B)); 
	SetColorName(_T("darkgoldenrod"), RGB(0xB8, 0x86, 0x0B)); 
	SetColorName(_T("darkgray"), RGB(0xA9, 0xA9, 0xA9)); 
	SetColorName(_T("darkgreen"), RGB(0x00, 0x64, 0x00)); 
	SetColorName(_T("darkkhaki"), RGB(0xBD, 0xB7, 0x6B)); 
	SetColorName(_T("darkmagenta"), RGB(0x8B, 0x00, 0x8B)); 
	SetColorName(_T("darkolivegreen"), RGB(0x55, 0x6B, 0x2F)); 
	SetColorName(_T("darkorange"), RGB(0xFF, 0x8C, 0x00)); 
	SetColorName(_T("darkorchid"), RGB(0x99, 0x32, 0xCC)); 
	SetColorName(_T("darkred"), RGB(0x8B, 0x00, 0x00)); 
	SetColorName(_T("darksalmon"), RGB(0xE9, 0x96, 0x7A)); 
	SetColorName(_T("darkseagreen"), RGB(0x8F, 0xBC, 0x8B)); 
	SetColorName(_T("darkslateblue"), RGB(0x48, 0x3D, 0x8B)); 
	SetColorName(_T("darkslategray"), RGB(0x2F, 0x4F, 0x4F)); 
	SetColorName(_T("darkturquoise"), RGB(0x00, 0xCE, 0xD1)); 
	SetColorName(_T("darkviolet"), RGB(0x94, 0x00, 0xD3)); 
	SetColorName(_T("deeppink"), RGB(0xFF, 0x14, 0x93)); 
	SetColorName(_T("deepskyblue"), RGB(0x00, 0xBF, 0xFF)); 
	SetColorName(_T("dimgray"), RGB(0x69, 0x69, 0x69)); 
	SetColorName(_T("dodgerblue"), RGB(0x1E, 0x90, 0xFF)); 
	SetColorName(_T("firebrick"), RGB(0xB2, 0x22, 0x22)); 
	SetColorName(_T("floralwhite"), RGB(0xFF, 0xFA, 0xF0)); 
	SetColorName(_T("forestgreen"), RGB(0x22, 0x8B, 0x22)); 
	SetColorName(_T("fuchsia"), RGB(0xFF, 0x00, 0xFF)); 
	SetColorName(_T("gainsboro"), RGB(0xDC, 0xDC, 0xDC)); 
	SetColorName(_T("ghostwhite"), RGB(0xF8, 0xF8, 0xFF)); 
	SetColorName(_T("goldenrod"), RGB(0xDA, 0xA5, 0x20)); 
	SetColorName(_T("greenyellow"), RGB(0xAD, 0xFF, 0x2F)); 
	SetColorName(_T("honeydew"), RGB(0xF0, 0xFF, 0xF0)); 
	SetColorName(_T("hotpink"), RGB(0xFF, 0x69, 0xB4)); 
	SetColorName(_T("indianred"), RGB(0xCD, 0x5C, 0x5C)); 
	SetColorName(_T("indigo"), RGB(0x4B, 0x00, 0x82)); 
	SetColorName(_T("ivory"), RGB(0xFF, 0xFF, 0xF0)); 
	SetColorName(_T("khaki"), RGB(0xF0, 0xE6, 0x8C)); 
	SetColorName(_T("lavender"), RGB(0xE6, 0xE6, 0xFA)); 
	SetColorName(_T("lavenderblush"), RGB(0xFF, 0xF0, 0xF5)); 
	SetColorName(_T("lawngreen"), RGB(0x7C, 0xFC, 0x00)); 
	SetColorName(_T("lemonchiffon"), RGB(0xFF, 0xFA, 0xCD)); 
	SetColorName(_T("lightblue"), RGB(0xAD, 0xD8, 0xE6)); 
	SetColorName(_T("lightcoral"), RGB(0xF0, 0x80, 0x80)); 
	SetColorName(_T("lightcyan"), RGB(0xE0, 0xFF, 0xFF));
	SetColorName(_T("lightgoldenrodyellow"), RGB(0xFA, 0xFA, 0xD2)); 
	SetColorName(_T("lightgreen"), RGB(0x90, 0xEE, 0x90)); 
	SetColorName(_T("lightgrey"), RGB(0xD3, 0xD3, 0xD3)); 
	SetColorName(_T("lightpink"), RGB(0xFF, 0xB6, 0xC1)); 
	SetColorName(_T("lightsalmon"), RGB(0xFF, 0xA0, 0x7A)); 
	SetColorName(_T("lightseagreen"), RGB(0x20, 0xB2, 0xAA)); 
	SetColorName(_T("lightskyblue"), RGB(0x87, 0xCE, 0xFA)); 
	SetColorName(_T("lightslategray"), RGB(0x77, 0x88, 0x99)); 
	SetColorName(_T("lightsteelblue"), RGB(0xB0, 0xC4, 0xDE));
	SetColorName(_T("lightyellow"), RGB(0xFF, 0xFF, 0xE0)); 
	SetColorName(_T("lime"), RGB(0x00, 0xFF, 0x00)); 
	SetColorName(_T("limegreen"), RGB(0x32, 0xCD, 0x32)); 
	SetColorName(_T("linen"), RGB(0xFA, 0xF0, 0xE6)); 
	SetColorName(_T("mediumaquamarine"), RGB(0x66, 0xCD, 0xAA)); 
	SetColorName(_T("mediumblue"), RGB(0x00, 0x00, 0xCD)); 
	SetColorName(_T("mediumorchid"), RGB(0xBA, 0x55, 0xD3)); 
	SetColorName(_T("mediumpurple"), RGB(0x93, 0x70, 0xDB)); 
	SetColorName(_T("mediumseagreen"), RGB(0x3C, 0xB3, 0x71)); 
	SetColorName(_T("mediumslateblue"), RGB(0x7B, 0x68, 0xEE)); 
	SetColorName(_T("mediumspringgreen"), RGB(0x00, 0xFA, 0x9A)); 
	SetColorName(_T("mediumturquoise"), RGB(0x48, 0xD1, 0xCC)); 
	SetColorName(_T("mediumvioletred"), RGB(0xC7, 0x15, 0x85)); 
	SetColorName(_T("midnightblue"), RGB(0x19, 0x19, 0x70)); 
	SetColorName(_T("mintcream"), RGB(0xF5, 0xFF, 0xFA)); 
	SetColorName(_T("mistyrose"), RGB(0xFF, 0xE4, 0xE1)); 
	SetColorName(_T("moccasin"), RGB(0xFF, 0xE4, 0xB5)); 
	SetColorName(_T("navajowhite"), RGB(0xFF, 0xDE, 0xAD)); 
	SetColorName(_T("oldlace"), RGB(0xFD, 0xF5, 0xE6)); 
	SetColorName(_T("olivedrab"), RGB(0x6B, 0x8E, 0x23)); 
	SetColorName(_T("orangered"), RGB(0xFF, 0x45, 0x00)); 
	SetColorName(_T("orchid"), RGB(0xDA, 0x70, 0xD6)); 
	SetColorName(_T("palegoldenrod"), RGB(0xEE, 0xE8, 0xAA)); 
	SetColorName(_T("palegreen"), RGB(0x98, 0xFB, 0x98)); 
	SetColorName(_T("paleturquoise"), RGB(0xAF, 0xEE, 0xEE)); 
	SetColorName(_T("palevioletred"), RGB(0xDB, 0x70, 0x93)); 
	SetColorName(_T("papayawhip"), RGB(0xFF, 0xEF, 0xD5));
	SetColorName(_T("peachpuff"), RGB(0xFF, 0xDA, 0xB9)); 
	SetColorName(_T("peru"), RGB(0xCD, 0x85, 0x3F)); 
	SetColorName(_T("plum"), RGB(0xDD, 0xA0, 0xDD)); 
	SetColorName(_T("powderblue"), RGB(0xB0, 0xE0, 0xE6)); 
	SetColorName(_T("rosybrown"), RGB(0xBC, 0x8F, 0x8F)); 
	SetColorName(_T("royalblue"), RGB(0x41, 0x69, 0xE1)); 
	SetColorName(_T("saddlebrown"), RGB(0x8B, 0x45, 0x13)); 
	SetColorName(_T("salmon"), RGB(0xFA, 0x80, 0x72)); 
	SetColorName(_T("sandybrown"), RGB(0xF4, 0xA4, 0x60)); 
	SetColorName(_T("seagreen"), RGB(0x2E, 0x8B, 0x57)); 
	SetColorName(_T("seashell"), RGB(0xFF, 0xF5, 0xEE)); 
	SetColorName(_T("sienna"), RGB(0xA0, 0x52, 0x2D)); 
	SetColorName(_T("skyblue"), RGB(0x87, 0xCE, 0xEB)); 
	SetColorName(_T("slateblue"), RGB(0x6A, 0x5A, 0xCD)); 
	SetColorName(_T("slategray"), RGB(0x70, 0x80, 0x90)); 
	SetColorName(_T("springgreen"), RGB(0x00, 0xFF, 0x7F)); 
	SetColorName(_T("steelblue"), RGB(0x46, 0x82, 0xB4)); 
	SetColorName(_T("tan"), RGB(0xD2, 0xB4, 0x8C)); 
	SetColorName(_T("teal"), RGB(0x00, 0x80, 0x80)); 
	SetColorName(_T("thistle"), RGB(0xD8, 0xBF, 0xD8)); 
	SetColorName(_T("tomato"), RGB(0xFF, 0x63, 0x47)); 
	SetColorName(_T("turquoise"), RGB(0x40, 0xE0, 0xD0)); 
	SetColorName(_T("wheat"), RGB(0xF5, 0xDE, 0xB3)); 
	SetColorName(_T("whitesmoke"), RGB(0xF5, 0xF5, 0xF5)); 
	SetColorName(_T("yellowgreen"), RGB(0x9A, 0xCD, 0x32));

	//Systems colors
	SetColorName(_T("activeborder"), ::GetSysColor(COLOR_ACTIVEBORDER)); 
	SetColorName(_T("activecaption"), ::GetSysColor(COLOR_ACTIVECAPTION)); 
	SetColorName(_T("appworkspace"), ::GetSysColor(COLOR_APPWORKSPACE)); 
	SetColorName(_T("background"), ::GetSysColor(COLOR_BACKGROUND)); 
	SetColorName(_T("buttonface"), ::GetSysColor(COLOR_BTNFACE)); 
	SetColorName(_T("buttonhighlight"), ::GetSysColor(COLOR_BTNHILIGHT)); 
	SetColorName(_T("buttonshadow"), ::GetSysColor(COLOR_BTNSHADOW)); 
	SetColorName(_T("buttontext"), ::GetSysColor(COLOR_BTNTEXT)); 
	SetColorName(_T("captiontext"), ::GetSysColor(COLOR_CAPTIONTEXT)); 
	SetColorName(_T("graytext"), ::GetSysColor(COLOR_GRAYTEXT)); 
	SetColorName(_T("highlight"), ::GetSysColor(COLOR_HIGHLIGHT)); 
	SetColorName(_T("highlighttext"), ::GetSysColor(COLOR_HIGHLIGHTTEXT)); 
	SetColorName(_T("inactiveborder"), ::GetSysColor(COLOR_INACTIVEBORDER)); 
	SetColorName(_T("inactivecaption"), ::GetSysColor(COLOR_INACTIVECAPTION)); 
	SetColorName(_T("inactivecaptiontext"), ::GetSysColor(COLOR_INACTIVECAPTIONTEXT)); 
	SetColorName(_T("infobackground"), ::GetSysColor(COLOR_INFOBK)); 
	SetColorName(_T("infotext"), ::GetSysColor(COLOR_INFOTEXT)); 
	SetColorName(_T("menu"), ::GetSysColor(COLOR_MENU)); 
	SetColorName(_T("menutext"), ::GetSysColor(COLOR_MENUTEXT)); 
	SetColorName(_T("scrollbar"), ::GetSysColor(COLOR_SCROLLBAR)); 
	SetColorName(_T("threeddarkshadow"), ::GetSysColor(COLOR_3DDKSHADOW)); 
	SetColorName(_T("threedface"), ::GetSysColor(COLOR_3DFACE)); 
	SetColorName(_T("threedhighlight"), ::GetSysColor(COLOR_3DHIGHLIGHT)); 
	SetColorName(_T("threedlightshadow"), ::GetSysColor(COLOR_3DLIGHT)); 
	SetColorName(_T("threedshadow"), ::GetSysColor(COLOR_3DSHADOW)); 
	SetColorName(_T("window"), ::GetSysColor(COLOR_WINDOW)); 
	SetColorName(_T("windowframe"), ::GetSysColor(COLOR_WINDOWFRAME)); 
	SetColorName(_T("windowtext"), ::GetSysColor(COLOR_WINDOWTEXT)); 
} //End SetTableOfColors

void CPPHtmlDrawer::SetColorName(LPCTSTR lpszColorName, COLORREF color)
{
	iterMapColors iterMap = m_mapColors.find((CPPString)lpszColorName);
	
	if (iterMap != m_mapColors.end())
		iterMap->second = color; //Modifies
	else
		m_mapColors.insert(std::make_pair((CPPString)lpszColorName, color)); //Add new
} //End SetColorName

COLORREF CPPHtmlDrawer::GetColorByName(LPCTSTR lpszColorName, COLORREF crDefColor /* = RGB(0, 0, 0) */)
{
	if (m_bIsEnable)
	{
		iterMapColors iterMap = m_mapColors.find((CPPString)lpszColorName);
		
		if (iterMap != m_mapColors.end())
			crDefColor = iterMap->second;
	}
	else
	{
		//For disabled output
		crDefColor = m_crDisabled;
	} //if
	return crDefColor;
} //End GetColorByName

/////////////////////////////////////////////////////////////////
// Gets the system tooltip's logfont
/////////////////////////////////////////////////////////////////
LPLOGFONT CPPHtmlDrawer::GetSystemToolTipFont() const
{
    static LOGFONT lf;
	
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
        return FALSE;
	
    memcpy(&lf, &(ncm.lfStatusFont), sizeof(LOGFONT));
	
    return &lf; 
} //End GetSystemToolTipFont

////////////////////////////////////////////
// Check a pointer over the hyperlink
//   In: lpPoint - the coordinates of the mouse pointer 
//  Out: -1 - hyperlink not found
//       index of the hyperlink
////////////////////////////////////////////
int CPPHtmlDrawer::PtInHyperlink(LPPOINT lpPoint)
{
	if (m_arrLinks.size() > 0)
	{
		STRUCT_HYPERLINK link;
		for (int i = 0; i < (int)m_arrLinks.size(); i++)
		{
			link = m_arrLinks [i];
			if (link.rcArea.PtInRect(*lpPoint))
				return i;
		} //for
	} //if
	return -1;
} //End PtInHyperlink

void CPPHtmlDrawer::JumpToHyperlink(int nLink)
{
	STRUCT_HYPERLINK link = m_arrLinks [nLink];
	TRACE(_T("Jump to Hyperlink number = %d\n"), nLink);
	if (!link.sHyperlink.IsEmpty())
	{
		switch (link.nTypeLink)
		{
		case LINK_HREF:
			GotoURL(link.sHyperlink);
			break;
		case LINK_MESSAGE:
			CallbackOnClickHyperlink(link.sHyperlink);
			break;
		} //switch
	} //if
} //End JumpToHyperlink

void CPPHtmlDrawer::OnLButtonDown(LPPOINT lpClient)
{
	TRACE (_T("CPPHtmlDrawer::OnLButtonDown()\n"));
	
	int nLink = PtInHyperlink(lpClient);
	if (nLink >= 0)
	{
		//Hyperlink under the mouse pointer
		JumpToHyperlink(nLink);
	} //if
} //End OnLButtonDown

BOOL CPPHtmlDrawer::OnSetCursor(LPPOINT lpClient)
{
	int nLink = PtInHyperlink(lpClient);
	if (nLink >= 0)
	{
		STRUCT_HYPERLINK link = m_arrLinks [nLink];
		if (m_nHoverIndexLink != link.nIndexLink)
		{
			m_nHoverIndexLink = link.nIndexLink;
			CallbackOnRepaint(m_nHoverIndexLink);
			//Redraw Window
		} //if
		
		if (!link.sHyperlink.IsEmpty() && (NULL != m_hLinkCursor))
		{
			::SetCursor(m_hLinkCursor);
			return TRUE;
		} //if
	}
	else if (m_nHoverIndexLink != PPHTMLDRAWER_NO_HOVERLINK)
	{
		m_nHoverIndexLink = PPHTMLDRAWER_NO_HOVERLINK;
		CallbackOnRepaint(m_nHoverIndexLink);
		//Redraw Window
	} //if
	
    return FALSE;
} //End OnSetCursor

BOOL CPPHtmlDrawer::OnTimer()
{
	BOOL bRedraw = FALSE;
	if (m_arrAni.size() > 0)
	{
		STRUCT_ANIMATION sa;
		for (int i = 0; i < (int)m_arrAni.size(); ++i)
		{
			sa = m_arrAni [i];
			if (sa.nMaxImages > 0)
			{
				sa.nTimerCount ++;
				if (sa.nTimerCount >= sa.nSpeed)
				{
					sa.nTimerCount = 0;
					sa.nIndex ++;
					if (sa.nIndex >= sa.nMaxImages)
						sa.nIndex = 0;
					bRedraw = TRUE;
				} //if
				m_arrAni [i] = sa;
			} //if
		} //for
	} //if

	return bRedraw;
} //End of OnTimer

void CPPHtmlDrawer::CallbackOnRepaint(int nIndexLink)
{
	TRACE(_T("CPPHtmlDrawer::CallbackOnRepaint()\n")); 

	if ((NULL == m_csCallbackRepaint.hWnd) || !m_csCallbackRepaint.nMessage)
		return; 
 	
	::SendMessage(m_csCallbackRepaint.hWnd, m_csCallbackRepaint.nMessage, (LPARAM)nIndexLink, m_csCallbackRepaint.lParam);  
} //End CallbackOnRepaint

void CPPHtmlDrawer::CallbackOnClickHyperlink(LPCTSTR sLink)
{
	TRACE(_T("CPPHtmlDrawer::CallbackOnClickHyperlink()\n")); 

	if ((NULL == m_csCallbackLink.hWnd) || !m_csCallbackLink.nMessage)
		return; 
	
	::SendMessage(m_csCallbackLink.hWnd, m_csCallbackLink.nMessage, (LPARAM)sLink, m_csCallbackLink.lParam);  	
} //if CallbackOnClickHyperlink

HINSTANCE CPPHtmlDrawer::GotoURL(LPCTSTR url, int showcmd /* = SW_SHOW */)
{
	SetHyperlinkCursor(NULL);

    TCHAR key[MAX_PATH + MAX_PATH];

    // First try ShellExecute()
    HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

    // If it failed, get the .htm regkey and lookup the program
    if ((UINT)result <= HINSTANCE_ERROR) 
	{

        if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) 
		{
            lstrcat(key, _T("\\shell\\open\\command"));

            if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) 
			{
                TCHAR *pos;
                pos = _tcsstr(key, _T("\"%1\""));
                if (pos == NULL) 
				{                     // No quotes found
                    pos = _tcsstr(key, _T("%1"));      // Check for %1, without quotes 
                    if (pos == NULL)                   // No parameter at all...
                        pos = key+lstrlen(key)-1;
                    else
                        *pos = '\0';                   // Remove the parameter
                }
                else
                    *pos = '\0';                       // Remove the parameter

                lstrcat(pos, _T(" "));
                lstrcat(pos, url);

                USES_CONVERSION;
                result = (HINSTANCE) WinExec(T2A(key),showcmd);
            } //if
        } //if
    } //if
    return result;
} //End GotoURL

LONG CPPHtmlDrawer::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];
        RegQueryValue(hkey, NULL, data, &datasize);
        lstrcpy(retdata,data);
        RegCloseKey(hkey);
    } //if

    return retval;
} //End GetRegKey

/////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::DrawHtml (LPSIZE lpSize, LPCRECT lpRect)
{
	//ENG: Bounding rectangle of a current area for output
	//RUS: �������������� ������������� ��� ������� ������� ������
	RECT rcArea;
	rcArea.left = lpRect->left;
	rcArea.right = lpRect->right;
	rcArea.top = lpRect->top;
	rcArea.bottom = lpRect->bottom;

	SIZE szArea;
	szArea.cx = szArea.cy = 0;
	
	if (m_bIsCalculate)
	{
		//ENG: In preparing mode clears an auxiliary tables
		//RUS: � ������ ���������� ������� ��������������� ������� 
		m_arrLinks.clear();
		m_arrTable.clear();
		m_arrHtmlLine.clear();
		m_arrTableSizes.clear();
		m_arrAni.clear();
	} //if

	m_nCurLine = 0;
	m_nCurTableIndex = 0;
	m_nNumCurTable = -1;
	m_nCurIndexLink = -1;
	m_nCurIndexAni = -1;
	
	//ENG: Clear stack of tags
	//RUS: ������� ���� �����
	m_arrStack.clear();
	
	int nIndex = 0;
	int nBegin;
	CPPString strText;
	
	//ENG: Applies a default styles
	//RUS: ��������� ����� ��-���������
	SetDefaultStyles(m_defStyle);
	SelectNewHtmlStyle(_T("body"), m_defStyle);
	
	//ENG: Creates a default font
	//RUS: ������� ����� �� ���������
	m_lfDefault.lfHeight = m_defStyle.nSizeFont;
	m_lfDefault.lfWidth = 0;
	m_lfDefault.lfOrientation = 0;
	m_lfDefault.lfEscapement = 0;
	m_lfDefault.lfWeight = m_defStyle.nWeightFont;
	m_lfDefault.lfItalic = m_defStyle.bItalicFont;
	m_lfDefault.lfStrikeOut = m_defStyle.bStrikeOutFont;
	m_lfDefault.lfUnderline = m_defStyle.bUnderlineFont;
	m_lfDefault.lfCharSet = DEFAULT_CHARSET;
	m_lfDefault.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lfDefault.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lfDefault.lfQuality = DEFAULT_QUALITY;
	m_lfDefault.lfPitchAndFamily = FF_DONTCARE;
	_tcscpy (m_lfDefault.lfFaceName, m_defStyle.sFaceFont);
	m_hFont = ::CreateFontIndirect(&m_lfDefault);
	
	//ENG: Remember a current context setting
	//RUS: ���������� ������� ��������� �������� ����������
	m_hOldFont = (HFONT)::SelectObject(m_hDC, &m_hFont);
	m_nOldBkMode = ::SetBkMode(m_hDC, m_defStyle.nBkMode);
	m_crOldText = ::SetTextColor(m_hDC, m_defStyle.crText);
	m_crOldBk = ::SetBkColor(m_hDC, m_defStyle.crBkgnd);
	::GetTextMetrics(m_hDC, &m_tm);
	
	while (nIndex < m_csHtmlText.GetLength())
	{
		//ENG: Searching a begin of table
		//RUS: ���� ������ �������
		nBegin = nIndex;
		BOOL bFoundTable = SearchTag(m_csHtmlText, nIndex, _T("table"));

		//ENG: Gets a text before a table
		//RUS: �������� ����� �� �������
		strText = m_csHtmlText.Mid(nBegin, nIndex - nBegin);

		//ENG: If text before a table is exist
		//RUS: ���� ����� ����� �������� ����������
		if (!strText.IsEmpty())
		{
			//ENG: Add a tag BODY around of a output text
			//RUS: ��������� ��� BODY ������ ���������� ������
//			strText = _T("<body>") + strText + _T("</body>");

			//ENG: Output a text before of a table
			//RUS: ������� ����� ����� ��������
			DrawHtmlString(strText, &rcArea, &szArea);

			//ENG: Updates a output area size
			//RUS: ��������� ������ ������� ������
			lpSize->cx = max(lpSize->cx, szArea.cx);
			lpSize->cy += szArea.cy;
			if (!m_bIsCalculate)
				rcArea.top += szArea.cy;
		} //if
		
		//ENG: If table was found
		//RUS: ���� ������� ���� �������
		if (bFoundTable)
		{
			//ENG: Searching an end of the table
			//RUS: ���� ��������� �������
			nBegin = nIndex;
			nIndex += 6;
			SearchEndOfTable(m_csHtmlText, nIndex);

			//ENG: Cuts a text of a table
			//RUS: �������� ����� �������
			strText = m_csHtmlText.Mid(nBegin, nIndex - nBegin);

			//ENG: Output a table
			//RUS: ����� �������
			DrawHtmlTable(strText, &rcArea, &szArea);
			
			//ENG: Updates a output area size
			//RUS: ��������� ������ ������� ������
			lpSize->cx = max(lpSize->cx, szArea.cx);
			lpSize->cy += szArea.cy;
			if (!m_bIsCalculate)
				rcArea.top += szArea.cy;
		} //if
	} //while
	
	//ENG: Restore context setting
	//RUS: ��������������� �������� ��������� ����������
	::SetBkMode(m_hDC, m_nOldBkMode);
	::SetBkColor(m_hDC, m_crOldBk);
	::SetTextColor(m_hDC, m_crOldText);
	::SelectObject(m_hDC, m_hOldFont);
	
	//ENG: Clear stack of tags
	//RUS: ������� ���� �����
	m_arrStack.clear();
	
	//ENG: Delete a font
	//RUS: ������� �����
	::DeleteObject(m_hFont);
} //End of DrawHtml

void CPPHtmlDrawer::DrawHtmlTable (LPCTSTR lpszHtml, LPCRECT lpRect, LPSIZE lpSize)
{
	CPPString str = lpszHtml;
	CRect rect = lpRect;
	_STRUCT_CELL sc;
	vecCol row;
	vecSize rowHeight;
	vecSize colWidth;
	CSize szTable; //The size of the table
	m_nNumCurTable++;
	int nNumOfTable = m_nNumCurTable;
	
	//Creates table
	if (m_bIsCalculate)
	{
		//Get size of the table
		szTable = GetTableDimensions(str);

		sc.nColSpan = 1;
		sc.nRowSpan = 1;
		sc.szText = szTable;
		row.push_back(sc);
		m_arrTable.push_back(row); //First stores the size of the table

		if (szTable.cx && szTable.cy)
		{
			//Creates one row of the table
			row.clear();
			sc.szText = CSize(0, 0);
			for (int i = 0; i < szTable.cx; i++)
			{
				row.push_back(sc);
				colWidth.push_back(0);
			} //for

			//Creates table
			for (i = 0; i < szTable.cy; i++)
			{
				m_arrTable.push_back(row);
				rowHeight.push_back(0);
			} //for
			m_arrTableSizes.push_back(colWidth);
			m_arrTableSizes.push_back(rowHeight);
		} //if
	}
	else
	{
		ASSERT (m_arrTable.size());
		row = m_arrTable [m_nCurTableIndex];
		sc = row [0];
		szTable = sc.szText;
		colWidth = m_arrTableSizes [nNumOfTable * 2];
		rowHeight = m_arrTableSizes [nNumOfTable * 2 + 1];
	}//if

	m_nCurTableIndex ++;
	if (!szTable.cx || !szTable.cy)
	{
		if (NULL != lpSize)
		{
			lpSize->cx = lpSize->cy = 0;
		} //if
	} //if

	int nBeginTable = m_nCurTableIndex;
	m_nCurTableIndex += szTable.cy;

	//Unpacked <table> tag style
	m_defStyle.strTag = _T("table");
	StoreRestoreStyle(FALSE);
	SetDefaultStyles(m_defStyle);
	SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

	CRect rcTable = rect;
	CRect rcCell = rcTable;
	CRect rcText;

	int nIndex = 0;
	int nRow = 0;
	int nCol = 0;
	CSize szSpan;
	CPPString strTag = _T("");
	CPPString strProperty = _T("");
	CPPString strParameter = _T("");
	CPPString strText = _T("");
	CPPString strTable = _T("");
	SearchBodyOfTag(str, strTable, nIndex);
	AnalyseCellParam(strTable, m_defStyle);
	UpdateContext();
	
	int nWidthTable = 0;
	int nHeightTable = 0;
	int i = 0;
	//Draw table border
	if (!m_bIsCalculate)
	{
		//Calculate the real column's width and row's height
		for (i = 0; i < szTable.cx; i++)
			nWidthTable += colWidth [i];
		for (i = 0; i < szTable.cy; i++)
			nHeightTable += rowHeight [i];

		rcTable.right = rcTable.left + GetTableWidth(strTable, rcTable.Width(), nWidthTable);
		//horizontal align of the table
		if (rcTable.Width() < rect.Width())
		{
			switch (m_defStyle.nHorzAlign)
			{
			case ALIGN_RIGHT:
				rcTable.OffsetRect(rect.Width() - rcTable.Width(), 0);
				break;
			case ALIGN_CENTER:
				rcTable.OffsetRect((rect.Width() - rcTable.Width()) / 2, 0);
				break;
			} //switch
		} //if

		if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			nHeightTable += m_defStyle.nBorderWidth * 6;
		else
			nHeightTable += m_defStyle.nBorderWidth * 2;

		rcTable.bottom = rcTable.top + nHeightTable;

		if (m_defStyle.nFillBkgnd >= 0)
		{
			m_drawmanager.FillEffect(m_hDC, m_defStyle.nFillBkgnd, rcTable, 
				m_defStyle.crBkgnd, m_defStyle.crMidBkgnd, m_defStyle.crEndBkgnd,
				5);
		}
		else if (!m_defStyle.strNameResBk.IsEmpty())
		{
			DrawBackgroundImage(m_hDC, rcTable.left, rcTable.top, rcTable.Width(), rcTable.Height(), m_defStyle.strNameResBk);
		} //if

		m_drawmanager.DrawRectangle(m_hDC, rcTable, m_defStyle.crBorderLight, m_defStyle.crBorderDark,
			m_defStyle.nBorderStyle, m_defStyle.nBorderWidth);

		if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			rcTable.DeflateRect(m_defStyle.nBorderWidth * 3, m_defStyle.nBorderWidth * 3);
		else
			rcTable.DeflateRect(m_defStyle.nBorderWidth, m_defStyle.nBorderWidth);

		if (nWidthTable < rcTable.Width())
		{
			int nStep = (rcTable.Width() - nWidthTable) / szTable.cx;
			colWidth [0] += (rcTable.Width() - nWidthTable) % szTable.cx;
			for (i = 0; i < szTable.cx; i++)
				colWidth [i] += nStep;
		} //if
	} //if

	//Analize the table
	BOOL bEndOfTable = FALSE;
	while (!bEndOfTable && (nIndex < str.GetLength()))
	{
		int nNewRow = nIndex;
		int nEndTable = nIndex;
		SearchTag(str, nNewRow, _T("tr"));
		SearchTag(str, nEndTable, _T("/table"));
		if (nNewRow < nEndTable)
		{
			//Applies styles of <tr> tag
			m_defStyle.strTag = _T("tr");
			StoreRestoreStyle(FALSE);
			SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

			//Gets data of the current row
			row = m_arrTable [nRow + nBeginTable];
			nCol = 0;
			
			SearchBodyOfTag(str, strTag, nIndex);
			//Analizing <tr> tag
			AnalyseCellParam(strTag, m_defStyle);
			UpdateContext();
			
			BOOL bEndOfRow = FALSE;
			while (!bEndOfRow && (nIndex < str.GetLength()))
			{
				int nEndRow = nIndex;
				int nNewCell = nIndex;
				SearchTag(str, nEndRow, _T("/tr"));
				SearchTag(str, nNewCell, _T("td"));
				if (nNewCell < nEndRow)
				{
					//Applies styles of <td> tag
					m_defStyle.strTag = _T("td");
					StoreRestoreStyle(FALSE);
					SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

					nIndex = nNewCell;
					//Gets data of the current cell
					sc = row [nCol];
					while ((sc.nColSpan < 0) && (nCol < (szTable.cx - 1)))
					{
						nCol ++;
						sc = row.at(nCol);
					} //while
					
					CSize szCell(0, 0);
//					nInTag = 0;
					SearchBodyOfTag(str, strTag, nIndex);
//					GetNameOfTag(strTag, nInTag);
					//Analizing <td> tag
					szSpan = AnalyseCellParam(strTag, m_defStyle);
					UpdateContext();

					if (m_bIsCalculate)
					{
						//Set colspan and rowspan
						sc.nColSpan = szSpan.cx;
						sc.nRowSpan = szSpan.cy;
						_STRUCT_CELL scTemp;
						if (szSpan.cx > 1)
						{
							szSpan.cx += nCol;
							for (int i = nCol + 1; i < szSpan.cx; i++)
							{
								scTemp = row [i];
								scTemp.nColSpan = -1;
								scTemp.nRowSpan = -1;
								row [i] = scTemp;
							} //for
						} //if
						if (szSpan.cy > 1)
						{
							vecCol rowTemp;
							szSpan.cy += nRow;
							for (int i = nRow + 1; i < szSpan.cy; i++)
							{
								rowTemp = m_arrTable [i + nBeginTable];
								scTemp = rowTemp [nCol];
								scTemp.nColSpan = -1;
								scTemp.nRowSpan = -1;
								rowTemp [nCol] = scTemp;
								m_arrTable [i + nBeginTable] = rowTemp;
							} //for
						} //if
						sc.szText = CSize(0, 0);
						rcCell.SetRect(0, 0, 0, 0);
						rcText = rcCell;
					}
					else
					{
						//Draws the cell
						rcCell = rcTable;
						for (i = 0; i < nCol; i++)
							rcCell.left += colWidth [i];
						rcCell.right = rcCell.left;
						for (i = 0; i < sc.nColSpan; i++)
							rcCell.right += colWidth [nCol + i];

						for (i = 0; i < nRow; i++)
							rcCell.top += rowHeight [i];
						rcCell.bottom = rcCell.top;
						for (i = 0; i < sc.nRowSpan; i++)
							rcCell.bottom += rowHeight [nRow + i];

						//Cellspacing
						rcText = rcCell;
						rcText.DeflateRect(m_defStyle.nPaddingLeft, m_defStyle.nPaddingTop, 
							m_defStyle.nPaddingRight, m_defStyle.nPaddingBottom);
						if (m_defStyle.nFillBkgnd >= 0)
						{
							m_drawmanager.FillEffect(m_hDC, m_defStyle.nFillBkgnd, rcText, 
								m_defStyle.crBkgnd, m_defStyle.crMidBkgnd, m_defStyle.crEndBkgnd,
								5);
						} //if

						//Draws the border
						m_drawmanager.DrawRectangle(m_hDC, rcText, m_defStyle.crBorderLight, m_defStyle.crBorderDark, m_defStyle.nBorderStyle);

						if (m_defStyle.nBorderWidth)
						{
							if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
								rcText.DeflateRect(3, 3, 3, 3);
							else
								rcText.DeflateRect(1, 1, 1, 1);
						} //if
							
						//Cellpadding
						rcText.DeflateRect(m_defStyle.nMarginLeft, m_defStyle.nMarginTop, 
							m_defStyle.nMarginRight, m_defStyle.nMarginBottom);

						//Vertical align
						switch (m_defStyle.nVertAlign)
						{
						case ALIGN_BOTTOM:
							rcText.top += rcText.Height() - sc.szText.cy;
							break;
						case ALIGN_VCENTER:
							rcText.top += (rcText.Height() - sc.szText.cy) / 2;
							break;
						}
					} //if
					
					BOOL bEndOfCell = FALSE;
					while(!bEndOfCell && (nIndex < str.GetLength()))
					{
						int nEndCell = nIndex;
						int nNewTable = nIndex;
						SearchTag(str, nEndCell, _T("/td"));
						SearchTag(str, nNewTable, _T("table"));
						int nNearlyTag = min(nEndCell, nNewTable);
						szCell = CSize(0, 0);
						if (nNearlyTag > nIndex)
						{
							//Exist output text
							strParameter = str.Mid(nIndex, nNearlyTag - nIndex);
							DrawHtmlString(strParameter, rcText, &szCell);
							nIndex = nNearlyTag;
						} //if
						else if (nNewTable < nEndCell)
						{
							//Include new table
							nIndex = nNewTable;
							SearchEndOfTable(str, nIndex);
							strParameter = str.Mid(nNewTable, nIndex - nNewTable);
							DrawHtmlTable(strParameter, rcText, &szCell); 
						}
						else
						{
							bEndOfCell = TRUE;
							nIndex = nEndCell;
							GetIndexNextChars(str, nIndex, _T(">"));
						} //if
						
						if (m_bIsCalculate)
						{
							sc.szText.cx = max(sc.szText.cx, szCell.cx);
							sc.szText.cy += szCell.cy;
						} //if
						rcText.top += szCell.cy;
					} //while
					
					if (m_bIsCalculate)
					{
						//Adding cellpadding and cellspacing
						sc.szCell = sc.szText;
						sc.szCell.cx += m_defStyle.nMarginLeft + m_defStyle.nMarginRight;
						sc.szCell.cx += m_defStyle.nPaddingLeft + m_defStyle.nPaddingRight;
						sc.szCell.cy += m_defStyle.nMarginTop + m_defStyle.nMarginBottom;
						sc.szCell.cy += m_defStyle.nPaddingTop + m_defStyle.nPaddingBottom;
						
						if (m_defStyle.nBorderWidth)
						{
							if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
							{
								sc.szCell.cx += 6;
								sc.szCell.cy += 6;
							}
							else
							{
								sc.szCell.cx += 2;
								sc.szCell.cy += 2;
							} //if
						} //if
						row [nCol] = sc;
					} //if
					nCol += sc.nColSpan;
					//Restore styles before <td> tag
					m_defStyle.strTag = _T("td");
					if (StoreRestoreStyle(TRUE))
						UpdateContext();
				}
				else
				{
					bEndOfRow = TRUE;
					nIndex = nEndRow;
					GetIndexNextChars(str, nIndex, _T(">"));
				} //if
			} //while

			m_arrTable [nRow + nBeginTable] = row;
			nRow++;
			//Restore styles before <tr> tag
			m_defStyle.strTag = _T("tr");
			if (StoreRestoreStyle(TRUE))
				UpdateContext();
		}
		else
		{
			bEndOfTable = TRUE;
			nIndex = nEndTable;
			GetIndexNextChars(str, nIndex, _T(">"));
		} //if
	} //while

	CSize size(0, 0);
	if (m_bIsCalculate)
	{
		//Analysing cell's width
		for (i = 1; i <= szTable.cx; i++)
		{
			for (int y = 0; y < szTable.cy; y++)
			{
				row = m_arrTable [nBeginTable + y];
				for (int x = 0; x < szTable.cx; x++)
				{
					sc = row [x];
					if (sc.nColSpan == i)
					{
						if (i == 1)
							colWidth [x] = max (colWidth [x], sc.szCell.cx);
						else
						{
							int span_width = 0;
							for (int z = 0; z < i; z++)
								span_width += colWidth [x + z];
							
							if (span_width < sc.szText.cx)
							{
								int step = (sc.szCell.cx - span_width) / i;
								colWidth [x + i - 1] += (sc.szCell.cx - span_width) % i;
								for (z = 0; z < i; z++)
									colWidth [x + z] += step;
							} //if
						} //if
					} //if
				} //for
			} //for
		} //for

		//Analysing cell's height
		for (i = 1; i <= szTable.cy; i++)
		{
			for (int y = 0; y < szTable.cy; y++)
			{
				row = m_arrTable [nBeginTable + y];
				for (int x = 0; x < szTable.cx; x++)
				{
					sc = row [x];
					if (sc.nRowSpan == i)
					{
						if (i == 1)
							rowHeight [y] = max (rowHeight [y], sc.szCell.cy);
						else
						{
							int span_height = 0;
							for (int z = 0; z < i; z++)
								span_height += rowHeight [y + z];
							
							if (span_height < sc.szCell.cy)
							{
								int step = (sc.szCell.cy - span_height) / i;
								rowHeight [y] += (sc.szCell.cy - span_height) % i;
								for (z = 0; z < i; z++)
									rowHeight [y + z] += step;
							} //if
						} //if
					} //if
				} //for
			} //for
		} //for

		//Stores width and height of the table's cells
		m_arrTableSizes [nNumOfTable * 2] = colWidth;
		m_arrTableSizes [nNumOfTable * 2 + 1] = rowHeight;

		for (i = 0; i < szTable.cx; i++)
			size.cx += colWidth [i];
		for (i = 0; i < szTable.cy; i++)
			size.cy += rowHeight [i];
		
		if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
		{
			size.cx += m_defStyle.nBorderWidth * 6;
			size.cy += m_defStyle.nBorderWidth * 6;
		}
		else
		{
			size.cx += m_defStyle.nBorderWidth * 2;
			size.cy += m_defStyle.nBorderWidth * 2;
		} //if

		size.cx = GetTableWidth(strTable, 0, size.cx, TRUE);
	}
	else
	{
		size.cx = rect.Width();
		size.cy = nHeightTable;
	} //if

	//Restore styles before <table> tag
	m_defStyle.strTag = _T("table");
	if (StoreRestoreStyle(TRUE))
		UpdateContext();

	if (NULL != lpSize)
	{
		lpSize->cx = size.cx;
		lpSize->cy = size.cy;
	} //if
} //End DrawHtmlTable

void CPPHtmlDrawer::DrawHtmlRow(LPCTSTR lpszHtml, LPCRECT lpRect, LPSIZE lpSize)
{
}

void CPPHtmlDrawer::DrawHtmlCell(LPCTSTR lpszHtml, LPCRECT lpRect, LPSIZE lpSize)
{
}
	
void CPPHtmlDrawer::DrawHtmlString (LPCTSTR lpszHtml, LPCRECT lpRect, LPSIZE lpSize)
{
	//ENG: Gets a text to output
	//RUS: �������� ����� ��� ������
	CPPString strHtml = lpszHtml;

	COLORREF clrShadow = m_bIsEnable ? m_crShadow : GetColorByName("");

	//ENG: For any string we are add a <body> tag as wrapper
	//RUS: ��� ����� ������ ��������� ��� <body>
	strHtml = _T("<body>") + strHtml;
	strHtml += _T("</body>");

	//ENG: Bounding rectangle for a full text
	//RUS: �������������� ������������� ��� ������ ����� ������
	m_rcOutput.top = lpRect->top;
	m_rcOutput.left = lpRect->left;
	m_rcOutput.bottom = lpRect->bottom;
	m_rcOutput.right = lpRect->right;

	//ENG: A current position for output
	//RUS: ������� ������� ��� ������
	POINT ptOutput;
	ptOutput.x = lpRect->left;
	ptOutput.y = lpRect->top;

	lpSize->cx = lpSize->cy = 0;
//	m_szOutput.cx = m_szOutput.cy = 0;

//	m_szOutput = CSize(0, 0);

	//ENG: If a text is empty
	//RUS: ���� ������ ��� ������ ���
//	if (str.IsEmpty())
//	{
//		lpSize->cx = lpSize->cy = 0;
//		return;
//	} //if


	int nFirstLine = m_nCurLine;

//	POINT pt;
//	pt.x = lpRect->left;
//	pt.y = lpRect->top;

	int y;
	SIZE sz;

	CPPString strText = _T("");
	CPPString strTag = _T(""); //String of the tag
	CPPString strProperty = _T(""); //String of the tag's property
	CPPString strParameter = _T("");

	BOOL bCloseTag = FALSE; //TRUE if tag have symbol '\'

	//ENG: Initializing a new line
	//RUS: ������������� ����� ������
	ptOutput.x = InitNewLine(ptOutput.x);

	int nIndex = 0;
	int nBegin = 0;
	int i = 0;
	while (i < strHtml.GetLength())
	{
		//ENG: Searching a first tag
		//RUS: ����� ������� ����
		strText = SearchBodyOfTag(strHtml, strTag, i);

		//ENG: Before a tag was exist a text
		//RUS: ����� ����� ���� ����� ��� ������
		if (!strText.IsEmpty())
		{
			//ENG: Transform text
			//RUS: ����������� �����
			switch (m_defStyle.nTextTransform)
			{
			case TEXT_TRANSFORM_UPPERCASE:
				//ENG: All chars make upper
				//RUS: ��� ������� ��������� � ������� �������
				strText.MakeUpper();
				break;
			case TEXT_TRANSFORM_LOWERCASE:
				//ENG: All chars make lower
				//RUS: ��� ������� ��������� � ������ �������
				strText.MakeLower();
				break;
			case TEXT_TRANSFORM_CAPITALIZE:
				//ENG: Each first char of a word to upper
				//RUS: ����� ������ ������ ����� � ������� �������, ��������� � ������
				strText.MakeLower();
				for (nIndex = 0; nIndex < strText.GetLength(); nIndex++)
				{
					if ((strText.GetAt(nIndex) >= _T('a')) && (strText.GetAt(nIndex) <= _T('z')))
					{
						if ((0 == nIndex) || (_T(' ') == strText.GetAt(nIndex - 1)))
							strText.SetAt(nIndex, strText.GetAt(nIndex) - _T('a') + _T('A'));
					} //if
				} //if
				break;
			} //switch

			//ENG: Gets a size a output text
			//RUS: �������� ������ ���������� ������
			::GetTextExtentPoint(m_hDC, strText, strText.GetLength(), &sz);

			//ENG: Gets a real top coordinate to output with vertical alignment
			//RUS: �������� �������� ��������� ����� ������ � ������ ������������� ������������
			y = VerticalAlignText(ptOutput.y, sz.cy);

			if (!m_bIsCalculate)
			{
				//ENG: Stores a current area as a hyperlink area if it available
				//RUS: ��������� ������� ������� ��� ������� ���������� ���� �� ����������
				StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

				//ENG: Real output a text
				//RUS: ����� ������
				::TextOut(m_hDC, ptOutput.x, y, strText, strText.GetLength());

				//ENG: If sets an overline style then draw a line over the text
				//RUS: ���� ���������� ����� overline, �� ������ ����� ��� �������
				if (m_defStyle.bOverlineFont)
				{
					HPEN hpenOverline = ::CreatePen(PS_SOLID, (m_defStyle.nWeightFont >= FW_BOLD) ? 2 : 1, m_defStyle.crText);
					HPEN hOldPen = (HPEN)::SelectObject(m_hDC, hpenOverline);
					::MoveToEx(m_hDC, ptOutput.x, y, NULL);
					::LineTo(m_hDC, ptOutput.x + sz.cx, y);
					::SelectObject(m_hDC, hOldPen);
				} //if
			}
			else
			{
				//ENG: Stores a last horizontal alignment
				//RUS: ��������� ��������� �������������� ������������
				m_hline.nHorzAlign = m_defStyle.nHorzAlign;
			} //if

			//ENG: Moves to a right of the outputed text
			//RUS: ������������ ������ �� ����������� ������
			ptOutput.x += sz.cx;
			strText.Empty();
		} //if

		//ENG: If tag was found then analyzing ...
		//RUS: ���� ��� ������, ����������� ...
		if (!strTag.IsEmpty())
		{
			//ENG: Reset temporary parameters
			//RUS: ����� ��������� ����������
			strTag.MakeLower();
			m_defStyle.strTag.Empty();
			bCloseTag = FALSE;
			
			//ENG: Get Tag's name
			//RUS: �������� ��� ����
			nIndex = 0;
			strProperty = GetNameOfTag(strTag, nIndex);

			//ENG: If tag's name exist
			//RUS: ���� ����� ���� ����������
			if (!strProperty.IsEmpty())
			{
				//ENG: Searching a tag's value
				//RUS: ����� �������� ����
				DWORD dwTag = GetTagFromList(strProperty, m_defStyle.strTag, bCloseTag);
				
				//ENG: If a tag was found in a list of the tags
				//RUS: ���� ��� ������ � ������
				if (TAG_NONE != dwTag)
				{
					//ENG: If it is a style tag 
					//RUS: ���� ������� ��� ��� ������ �� �������
					if (!m_defStyle.strTag.IsEmpty())
					{
						//ENG: Checks on permissibility of tag
						//RUS: ��������� �� ������������ ����
						if (StoreRestoreStyle(bCloseTag))
						{
							//ENG: If it isn't a close tag
							//RUS: ���� ��� �� ��������� ����
							if (!bCloseTag)
							{
								//ENG: Processing a tag
								//RUS: ��������� ����
								switch (dwTag)
								{
								case TAG_BOLD:
									m_defStyle.nWeightFont <<= 1;
									if (m_defStyle.nWeightFont > FW_BLACK)
										m_defStyle.nWeightFont = FW_BLACK;
									break;
								case TAG_ITALIC:
									m_defStyle.bItalicFont = m_defStyle.bItalicFont ? FALSE : TRUE;
									break;
								case TAG_UNDERLINE:
									m_defStyle.bUnderlineFont = m_defStyle.bUnderlineFont ? FALSE : TRUE;
									break;
								case TAG_STRIKEOUT:
									m_defStyle.bStrikeOutFont = m_defStyle.bStrikeOutFont ? FALSE : TRUE;
									break;
								case TAG_FONT:
									//Search parameters
									while (nIndex < strTag.GetLength())
									{
										//ENG: Searching a parameters of a tag
										//RUS: ����� ���������� ����
										strProperty = SearchPropertyOfTag(strTag, nIndex);
										//ENG: If a parameter was found
										//RUS: ���� �������� ������
										if (!strProperty.IsEmpty())
										{
											//ENG: Searching a parameter's value
											//RUS: ����� �������� ���������
											strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
											//ENG: Processing a parameters of a tag
											//RUS: ��������� ���������� ����
											if (strProperty == _T("face"))
												m_defStyle.sFaceFont = GetStyleString(strParameter, m_defStyle.sFaceFont);
											else if (strProperty == _T("size"))
												m_defStyle.nSizeFont = GetLengthUnit(strParameter, m_defStyle.nSizeFont, TRUE);
											else if (strProperty == _T("color"))
											{
												if (m_bIsEnable)
													m_defStyle.crText = GetStyleColor(strParameter, m_defStyle.crText);
												else
													m_defStyle.crText = GetColorByName("");
											}
											else if (strProperty == _T("style"))
												GetStyleFontShortForm(strParameter);
											else if (strProperty == _T("weight"))
												m_defStyle.nWeightFont = GetStyleFontWeight(strParameter, m_defStyle.nWeightFont);
											else if (strProperty == _T("bkgnd"))
											{
												if (((strParameter == _T("transparent")) && strParameter.IsEmpty()) || !m_bIsEnable)
												{
													m_defStyle.nBkMode = TRANSPARENT;
												}
												else
												{
													m_defStyle.nBkMode = OPAQUE;
													m_defStyle.crBkgnd = GetStyleColor(strParameter, m_defStyle.crBkgnd);
												} //if
											} //if
										} //if
									} //while
									break;
								case TAG_LEFT:
									m_defStyle.nHorzAlign = ALIGN_LEFT;
									break;
								case TAG_CENTER:
									m_defStyle.nHorzAlign = ALIGN_CENTER;
									break;
								case TAG_RIGHT:
									m_defStyle.nHorzAlign = ALIGN_RIGHT;
									break;
								case TAG_BASELINE:
									m_defStyle.nVertAlign = ALIGN_BASELINE;
									break;
								case TAG_TOP:
									m_defStyle.nVertAlign = ALIGN_TOP;
									break;
								case TAG_VCENTER:
									m_defStyle.nVertAlign = ALIGN_VCENTER;
									break;
								case TAG_BOTTOM:
									m_defStyle.nVertAlign = ALIGN_BOTTOM;
									break;
								case TAG_NEWSTYLE:
									SelectNewHtmlStyle(strProperty, m_defStyle);
									break;
								case TAG_SPAN:
									//ENG: Searching a parameters of a tag
									//RUS: ����� ���������� ����
									strProperty = SearchPropertyOfTag(strTag, nIndex);
									//ENG: If a parameter was found
									//RUS: ���� �������� ������
									if (!strProperty.IsEmpty())
									{
										//ENG: Searching a parameter's value
										//RUS: ����� �������� ���������
										strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
										//ENG: Processing a parameters of a tag
										//RUS: ��������� ���������� ����
										if (strProperty == _T("class"))
										{
											SelectNewHtmlStyle(_T(".") + GetStyleString(strParameter, _T("")), m_defStyle);
										} //if
									} //if
									break;
								case TAG_HYPERLINK:
									//ENG: A default values
									//RUS: �������� �� ���������
									m_defStyle.nTypeLink = LINK_MESSAGE;
									m_defStyle.sHyperlink.Empty();
									while (nIndex < strTag.GetLength())
									{
										//ENG: Searching a parameters of a tag
										//RUS: ����� ���������� ����
										strProperty = SearchPropertyOfTag(strTag, nIndex);
										//ENG: If a parameter was found
										//RUS: ���� �������� ������
										if (!strProperty.IsEmpty())
										{
											//ENG: Searching a parameter's value
											//RUS: ����� �������� ���������
											strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
											//ENG: Processing a parameters of a tag
											//RUS: ��������� ���������� ����
											if (strProperty == _T("href"))
											{
												m_defStyle.nTypeLink = LINK_HREF;
												m_defStyle.sHyperlink = GetStyleString(strParameter, _T(""));
											} //if
											if (strProperty == _T("msg"))
											{
												m_defStyle.nTypeLink = LINK_MESSAGE;
												m_defStyle.sHyperlink = GetStyleString(strParameter, _T(""));
											} //if
										} //if
									} //while
									//ENG: Gets a index of a current link
									//RUS: �������� ������ ������� �����������
									m_nCurIndexLink ++;
									//ENG: If a mouse over this link
									//RUS: ���� ��� ��� ���� �����
									if (m_nCurIndexLink == m_nHoverIndexLink)
										SelectNewHtmlStyle(_T("a:hover"), m_defStyle);
									else
										SelectNewHtmlStyle(_T("a:link"), m_defStyle);
									break;
								} //switch
							} //if
							//ENG: Update a device context
							//RUS: ���������� ��������� ����������
							UpdateContext();
						} //if
					}
					else 
					{
						BOOL bPercent;
						BOOL bShadow;
						BOOL bAutoDelete;
						int nWidth, nNum;
						
						STRUCT_IMAGE si;
						STRUCT_CHANGESTYLE csTemp; //Temporary structure
						STRUCT_ANIMATION sa;
						
						SIZE szReal;
						
//						HDC hSrcDC = NULL;
//						HDC hDestDC = NULL;
						
						HBITMAP hBitmap = NULL;;
//						HBITMAP hIconBmp = NULL;
//						HBITMAP hOldSrcBmp = NULL;
//						HBITMAP hOldDestBmp = NULL;
						
						HICON hIcon = NULL;
						
						DWORD nMaxCol, nMaxRow;
						UINT nIdRes, nIdDll;
						//CPPString str;

						//ENG: Processing a tag
						//RUS: ��������� ����
						switch (dwTag)
						{
						case TAG_HLINE:
							//ENG: Draws the horizontal line
							//RUS: ��������� �������������� �����
							csTemp = m_defStyle;
							csTemp.nBorderWidth = 1;
							//ENG: Applies a new styles for <hr> tag
							SelectNewHtmlStyle(_T("hr"), csTemp);
							nWidth = 100;
							bPercent = TRUE;
							
							while (nIndex < strTag.GetLength())
							{
								//ENG: Searching a parameters of a tag
								//RUS: ����� ���������� ����
								strProperty = SearchPropertyOfTag(strTag, nIndex);
								//ENG: If a parameter was found
								//RUS: ���� �������� ������
								if (!strProperty.IsEmpty())
								{
									//ENG: Searching a parameter's value
									//RUS: ����� �������� ���������
									strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
									//ENG: Processing a parameters of a tag
									//RUS: ��������� ���������� ����
									if (strProperty == _T("width"))
									{
										bPercent = IsPercentableValue(strParameter);
										nWidth = GetLengthUnit(strParameter, 100);
									}
									else if (strProperty == _T("size"))
										csTemp.nBorderWidth = GetLengthUnit(strParameter, csTemp.nBorderWidth);
									else if (strProperty == _T("color"))
									{
										if (m_bIsEnable)
											csTemp.crText = GetStyleColor(strParameter, csTemp.crText);
										else
											csTemp.crText = GetColorByName("");
									}
								} //if
							} //while
							
							if (bPercent)
							{
								if (m_bIsCalculate)
								{
									m_hline.nAddPercentWidth += nWidth;
									nWidth = 0;
								}
								else nWidth = ::MulDiv(lpRect->right - lpRect->left, nWidth, 100);
							} //if
							
							if (m_bIsCalculate)
							{
								m_hline.nHeightLine = max(m_hline.nHeightLine, csTemp.nBorderWidth + 8);
								m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
							}
							else
							{
								m_drawmanager.DrawLine(m_hDC, ptOutput.x, ptOutput.y + m_hline.nHeightLine / 2, 
									ptOutput.x + nWidth, ptOutput.y + m_hline.nHeightLine / 2, 
									csTemp.crText, CPPDrawManager::PEN_SOLID, csTemp.nBorderWidth);
							} //if
							ptOutput.x += nWidth;
							break;
						case TAG_NEWLINE:
							//ENG: New line
							//RUS: ����� ������
							nNum = 1;
							if (strProperty == _T("br"))
							{
								strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
								nNum = GetLengthUnit(strParameter, nNum);
							}
							Tag_NewLine(&ptOutput, nNum, lpSize);
							break;
						case TAG_TABULATION:
							//ENG: Tabulation
							//RUS: ���������
							nNum = 1;
							if (strProperty == _T("t"))
							{
								strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
								nNum = GetLengthUnit(strParameter, nNum);
							}
							Tag_Tabulation(&ptOutput, nNum);
							break;
						case TAG_BITMAP:
							//-----------------------------
							//Draws the bitmap 
							//-----------------------------
							//ENG: Default Parameters
							//RUS: ��������� �� ���������
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.crMask = RGB(255, 0, 255);
							si.bUseMask = FALSE;
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSrcFile.Empty();
							si.strPathDll.Empty();
							
							//ENG: Searching image parameters
							//RUS: ����� ���������� �����������
							AnalyseImageParam(strTag, si);
							
							//ENG: If a image's source was specified
							//RUS: ���� ������ �������� �����������
							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								//RUS: ���������� ���� ��������������� �������� ������� �����������
								bAutoDelete = TRUE;
								
								//ENG: Gets a handle of the image
								//RUS: �������� ���������� �����������
								if (si.nIdRes)
									hBitmap = GetBitmapFromResources(si.nIdRes);
								else if (!si.strSrcFile.IsEmpty())
									hBitmap = GetBitmapFromFile(si.strSrcFile);
								else if (si.nIdDll)
									hBitmap = GetBitmapFromDll(si.nIdDll, si.strPathDll);
								else if (si.nHandle)
								{
									hBitmap = (HBITMAP)si.nHandle;
									//ENG: If an image handle specified, disables autodelete
									//RUS: ���� ������ ���������� �����������, �� ��������� ��������
									bAutoDelete = FALSE;
								} //if

								//ENG: If a handle of an image was retrieved
								//RUS: ���� ���������� ����������� �������
								if (NULL != hBitmap)
								{
									//ENG: Image with shadow or not?
									//RUS: ����������� � ����� ��� ���
									bShadow = IsImageWithShadow(si);

									//ENG: Retrieves an original size of an image
									//RUS: �������� ������������ ������ �����������
									m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);
					
									//ENG: Retrieves an output size
									//RUS: �������� ������� ��� ���������
									if (si.bPercentWidth) si.nWidth = ::MulDiv(sz.cx, si.nWidth, 100);
									if (si.bPercentHeight) si.nHeight = ::MulDiv(sz.cy, si.nHeight, 100);
									
									//ENG: If a shadow was enabled then set a real size
									//RUS: ���� ���� ��������, �� ������������� �������� ������
									if (si.nWidth && si.nHeight && bShadow)
									{
										sz.cx = si.nWidth + m_szOffsetShadow.cx;
										sz.cy = si.nHeight + m_szOffsetShadow.cy;
									} //if

									//ENG: Store a last horizontal alignment
									//RUS: ���������� ��������� �������������� ������������
									if (m_bIsCalculate) 
										m_hline.nHorzAlign = m_defStyle.nHorzAlign;

									//ENG: Retrieves a vertical coordinates of drawing area
									//RUS: �������� ������������ ���������� ������� ���������
									y = VerticalAlignImage(ptOutput.y, si.nHeight);

									//ENG: If an image is exist and not prepare mode
									//RUS: ���� ����������� �������� � �� ���������� ����� ����������
									if (si.nWidth && si.nHeight && !m_bIsCalculate)
									{
										//ENG: Add an output area to hyperlink list if needed
										//RUS: ���� ���������� ��������� ������� ������ � ������ �����������
										StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

										//ENG: If a mouse over an image then applies a hot styles
										//RUS: ���� ���� ��� ������������, �� ��������� �������������� �����
										if (m_defStyle.nTypeLink != LINK_NONE)
										{
											if (m_nCurIndexLink == m_nHoverIndexLink)
												si.nStyles = si.nHotStyles;
										} //if

										if (!m_bIsEnable)
											si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

										//ENG: Drawing an image
										//RUS: ��������� �����������
										m_drawmanager.DrawBitmap(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hBitmap, 
											si.bUseMask, si.crMask, si.nStyles, 
											bShadow, 
											m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
											m_szDepthShadow.cx, m_szDepthShadow.cy, 
											clrShadow);
									} //if
									
									//ENG: Moves to a right of the outputed image
									//RUS: ������������ ������ �� ����������� �����������
									ptOutput.x += si.nWidth;
									
									//ENG: If needed delete a handle of an image
									//RUS: ���� ���������� ������� ���������� �����������
									if (bAutoDelete)
										::DeleteObject(hBitmap);
								} //if
							} //if
							break;
						case TAG_ICON:
							//-----------------------------
							//Draws the icon
							//-----------------------------
							//ENG: Default Parameters
							//RUS: ��������� �� ���������
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSrcFile.Empty();
							si.strPathDll.Empty();
							
							//ENG: Searching image parameters
							//RUS: ����� ���������� �����������
							AnalyseImageParam(strTag, si);
							
							//ENG: If a image's source was specified
							//RUS: ���� ������ �������� �����������
							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								//RUS: ���������� ���� ��������������� �������� ������� �����������
								bAutoDelete = TRUE;

								//RUS: �������� ��������� ������ ������
								sz.cx = si.nWidth;
								sz.cy = si.nHeight;
								if (si.bPercentWidth) sz.cx = ::MulDiv(::GetSystemMetrics(SM_CXICON), si.nWidth, 100);
								if (si.bPercentHeight) sz.cy = ::MulDiv(::GetSystemMetrics(SM_CYICON), si.nHeight, 100);
								
								//ENG: Gets a handle of the image
								//RUS: �������� ���������� �����������
								if (si.nIdRes)
									hIcon = GetIconFromResources(si.nIdRes, sz.cx, sz.cy);
								else if (!si.strSrcFile.IsEmpty())
									hIcon = GetIconFromFile(si.strSrcFile, sz.cx, sz.cy);
								else if (si.nIdDll)
									hIcon = GetIconFromDll(si.nIdDll, sz.cx, sz.cy, si.strPathDll);
								else if (si.nHandle)
								{
									hIcon = (HICON)si.nHandle;

									//ENG: If an image handle specified, disables autodelete
									//RUS: ���� ������ ���������� �����������, �� ��������� ��������
									bAutoDelete = FALSE;
								} //if
								
								//ENG: If a handle of an image was retrieved
								//RUS: ���� ���������� ����������� �������
								if (NULL != hIcon)
								{
									//ENG: Image with shadow or not?
									//RUS: ����������� � ����� ��� ���
									BOOL bShadow = IsImageWithShadow(si);
									
									//ENG: Retrieves an original size of an image
									//RUS: �������� ������������ ������ �����������
									m_drawmanager.GetSizeOfIcon(hIcon, &sz);
									si.nWidth = sz.cx;
									si.nHeight = sz.cy;
									
									//ENG: Retrieves an output size
									//RUS: �������� ������� ��� ���������
//									if (si.bPercentWidth) si.nWidth = ::MulDiv(sz.cx, si.nWidth, 100);
//									if (si.bPercentHeight) si.nHeight = ::MulDiv(sz.cy, si.nHeight, 100);
									
									//ENG: If a shadow was enabled then set a real size
									//RUS: ���� ���� ��������, �� ������������� �������� ������
									if (si.nWidth && si.nHeight && bShadow)
									{
										sz.cx = si.nWidth + m_szOffsetShadow.cx;
										sz.cy = si.nHeight + m_szOffsetShadow.cy;
									} //if

									//ENG: Store a last horizontal alignment
									//RUS: ���������� ��������� �������������� ������������
									if (m_bIsCalculate) 
										m_hline.nHorzAlign = m_defStyle.nHorzAlign;

									//ENG: Retrieves a vertical coordinates of drawing area
									//RUS: �������� ������������ ���������� ������� ���������
									y = VerticalAlignImage(ptOutput.y, si.nHeight);

									//ENG: If an image is exist and not prepare mode
									//RUS: ���� ����������� �������� � �� ���������� ����� ����������
									if (si.nWidth && si.nHeight && !m_bIsCalculate)
									{
										//ENG: Add an output area to hyperlink list if needed
										//RUS: ���� ���������� ��������� ������� ������ � ������ �����������
										StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

										//ENG: If a mouse over an image then applies a hot styles
										//RUS: ���� ���� ��� ������������, �� ��������� �������������� �����
										if (m_defStyle.nTypeLink != LINK_NONE)
										{
											if (m_nCurIndexLink == m_nHoverIndexLink)
												si.nStyles = si.nHotStyles;
										} //if

										if (!m_bIsEnable)
											si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

										//ENG: Drawing an image
										//RUS: ��������� �����������
										m_drawmanager.DrawIcon(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hIcon, si.nStyles, 
											bShadow, 
											m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
											m_szDepthShadow.cx, m_szDepthShadow.cy, 
											clrShadow);
									} //if
									//ENG: Moves to a right of the outputed image
									//RUS: ������������ ������ �� ����������� �����������
									ptOutput.x += si.nWidth;
									
									//ENG: If needed delete a handle of an image
									//RUS: ���� ���������� ������� ���������� �����������
									if (bAutoDelete) 
										::DestroyIcon(hIcon);
								} //if
							} //if
							break;
						case TAG_IMAGELIST:
							//-----------------------------
							//Draws the icon from image list
							//-----------------------------
							//ENG: Default Parameters
							//RUS: ��������� �� ���������
							si.nIndexImageList = 0;
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.nSpeed = 0;
							si.bUseMask = FALSE;
							si.crMask = RGB(255, 0, 255);
							si.cx = GetSystemMetrics(SM_CXICON);
							si.cy = GetSystemMetrics(SM_CYICON);
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSrcFile.Empty();
							si.strPathDll.Empty();
							
							//ENG: Searching image parameters
							//RUS: ����� ���������� �����������
							AnalyseImageParam(strTag, si);
							
							//ENG: Image with shadow or not?
							//RUS: ����������� � ����� ��� ���
							bShadow = IsImageWithShadow(si);
							
							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								//RUS: ���������� ���� ��������������� �������� ������� �����������
								bAutoDelete = TRUE;
								
								//ENG: Gets a handle of the image
								//RUS: �������� ���������� �����������
								if (si.nIdRes)
									hBitmap = GetBitmapFromResources(si.nIdRes);
								else if (!si.strSrcFile.IsEmpty())
									hBitmap = GetBitmapFromFile(si.strSrcFile);
								else if (si.nIdDll)
									hBitmap = GetBitmapFromDll(si.nIdDll, si.strPathDll);
								else if (si.nHandle)
								{
									hBitmap = (HBITMAP)si.nHandle;
									//ENG: If an image handle specified, disables autodelete
									//RUS: ���� ������ ���������� �����������, �� ��������� ��������
									bAutoDelete = FALSE;
								} //if
								
								//ENG: If a handle of an image was retrieved
								//RUS: ���� ���������� ����������� �������
								if ((NULL != hBitmap) && si.cx && si.cy)
								{
									//ENG: Retrieves an original size of an image
									//RUS: �������� ������������ ������ �����������
									m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);
					
									//ENG: Retrieves an output size
									//RUS: �������� ������� ��� ���������
									if (si.bPercentWidth) si.nWidth = ::MulDiv(si.cx, si.nWidth, 100);
									if (si.bPercentHeight) si.nHeight = ::MulDiv(si.cy, si.nHeight, 100);
									
									//ENG: If a shadow was enabled then set a real size
									//RUS: ���� ���� ��������, �� ������������� �������� ������
									szReal.cx = si.nWidth;
									szReal.cy = si.nHeight;
									if (si.nWidth && si.nHeight && bShadow)
									{
										szReal.cx += m_szOffsetShadow.cx;
										szReal.cy += m_szOffsetShadow.cy;
									} //if
									
									//ENG: Gets a max columns and rows of the images on the bitmap
									//RUS: �������� ������������ ����� ������� � ����� ����������� �� ��������
									nMaxCol = sz.cx / si.cx;
									nMaxRow = sz.cy / si.cy;

									if (si.nSpeed)
									{
										if (m_bIsCalculate)
										{
											sa.nIndex = si.nIndexImageList;
											sa.nMaxImages = nMaxCol * nMaxRow;
											sa.nSpeed = si.nSpeed;
											sa.nTimerCount = 0;
											m_arrAni.push_back(sa);
										}
										else
										{
											m_nCurIndexAni ++;
											sa = m_arrAni [m_nCurIndexAni];
											si.nIndexImageList = sa.nIndex;
										} //if
									} //if
									
									//ENG: If a specified index of image is a legitimate value
									//RUS: ���� ��������� ������ ����������� ��������
									if ((si.nIndexImageList < (int)(nMaxCol * nMaxRow)) && nMaxCol && nMaxRow)
									{
										//ENG: Store a last horizontal alignment
										//RUS: ���������� ��������� �������������� ������������
										if (m_bIsCalculate) 
											m_hline.nHorzAlign = m_defStyle.nHorzAlign;

										//ENG: Retrieves a vertical coordinates of drawing area
										//RUS: �������� ������������ ���������� ������� ���������
										y = VerticalAlignImage(ptOutput.y, szReal.cy);

										//ENG: If an image is exist and not prepare mode
										//RUS: ���� ����������� �������� � �� ���������� ����� ����������
										if (si.nWidth && si.nHeight && !m_bIsCalculate)
										{
											//ENG: Add an output area to hyperlink list if needed
											//RUS: ���� ���������� ��������� ������� ������ � ������ �����������
											StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + szReal.cx, y + szReal.cy);
											
											//ENG: If a mouse over an image then applies a hot styles
											//RUS: ���� ���� ��� ������������, �� ��������� �������������� �����
											if (m_defStyle.nTypeLink != LINK_NONE)
											{
												if (m_nCurIndexLink == m_nHoverIndexLink)
													si.nStyles = si.nHotStyles;
											} //if

											if (!m_bIsEnable)
												si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

											//ENG: Drawing an image
											//RUS: ��������� �����������
											m_drawmanager.DrawImageList(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hBitmap,
												si.nIndexImageList, si.cx, si.cy,
												si.bUseMask, si.crMask, si.nStyles, 
												bShadow, 
												m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
												m_szDepthShadow.cx, m_szDepthShadow.cy, 
												clrShadow);
										} //if

										//ENG: Moves to a right of the outputed image
										//RUS: ������������ ������ �� ����������� �����������
										ptOutput.x += szReal.cx;
									} //if
									
									//ENG: If needed delete a handle of an image
									//RUS: ���� ���������� ������� ���������� �����������
									if (bAutoDelete)
										::DeleteObject(hBitmap);
								} //if
							}
							else if (NULL != m_ImageList.GetSafeHandle())
							{
								if ((int)si.nIndexImageList < m_ImageList.GetImageCount())
								{
									hIcon = m_ImageList.ExtractIcon(si.nIndexImageList);
									if (NULL != hIcon)
									{
										sz.cx = si.nWidth;
										sz.cy = si.nHeight;
										if (si.bPercentWidth) sz.cx = ::MulDiv(m_szImageList.cx, si.nWidth, 100);
										if (si.bPercentHeight) sz.cy = ::MulDiv(m_szImageList.cy, si.nHeight, 100);
										
										szReal.cx = sz.cx;
										szReal.cy = sz.cy;
										if (sz.cx && sz.cy && bShadow)
										{
											szReal.cx += m_szOffsetShadow.cx;
											szReal.cy += m_szOffsetShadow.cy;
										} //if
										
										if (m_bIsCalculate) m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
										y = VerticalAlignImage(ptOutput.y, szReal.cy);
										if (sz.cx && sz.cy && !m_bIsCalculate)
										{
											StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + szReal.cx, y + szReal.cy);
											
											if (m_defStyle.nTypeLink != LINK_NONE)
											{
												if (m_nCurIndexLink == m_nHoverIndexLink)
													si.nStyles = si.nHotStyles;
											} //if
											
											if (!m_bIsEnable)
												si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

											m_drawmanager.DrawIcon(m_hDC, ptOutput.x, y, 
												sz.cx, sz.cy, hIcon, si.nStyles, 
												bShadow, 
												m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
												m_szDepthShadow.cx, m_szDepthShadow.cy, 
												clrShadow);
											::DestroyIcon(hIcon);
										} //if
										ptOutput.x += szReal.cx;
									} //if
								} //if
							} //if
							break;
						case TAG_STRING:
							//-----------------------------
							//Draws the string
							//-----------------------------
							nIdRes = 0;
							nIdDll = 0;
							strText.Empty();
							while (nIndex < strTag.GetLength())
							{
								strProperty = SearchPropertyOfTag(strTag, nIndex);
								if (!strProperty.IsEmpty())
								{
									strParameter = GetParameterString(strTag, nIndex, _T('='), _T(" "));
									if (strProperty == _T("idres"))
										nIdRes = GetLengthUnit(strParameter, nIdRes);
									else if (strProperty == _T("iddll"))
										nIdRes = GetLengthUnit(strParameter, nIdDll);
									else if (strProperty == _T("srcdll"))
										strText = GetStyleString(strParameter, strText);
								} //if
							} //while
							if (nIdRes || nIdDll)
							{
								if (nIdRes)
									strText = GetStringFromResource(nIdRes);
								else if (nIdDll)
									strText = GetStringFromDll(nIdDll, strText);
								
								if (!strText.IsEmpty())
								{
									::GetTextExtentPoint(m_hDC, strText, strText.GetLength(), &sz);
									if (m_bIsCalculate) m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
									y = VerticalAlignText(ptOutput.y, sz.cy);
									if (!m_bIsCalculate)
									{
										StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);
										::TextOut(m_hDC, ptOutput.x, y, strText, strText.GetLength());
									} //if
									ptOutput.x += sz.cx;
								} //if
							} //if
							break;
						} //switch
					} //if
				} //if
			} //if
		} //if
	} //for
	Tag_NewLine(&ptOutput, 1, lpSize);

	lpSize->cy = ptOutput.y - lpRect->top;

	//Adds the percent's length to the line's length
	for (i = nFirstLine; i < m_nCurLine; i++)
	{
		m_hline = m_arrHtmlLine [i];
		if (0 != m_hline.nAddPercentWidth)
		{
			m_hline.nWidthLine += ::MulDiv(m_hline.nAddPercentWidth, lpSize->cx, 100);
			lpSize->cx = max(lpSize->cx, m_hline.nWidthLine);
		} //if
	} //for
//
//	if (NULL != lpSize)
//	{
//		lpSize->cx = m_szOutput.cx;
//		lpSize->cy = m_szOutput.cy;
//	} //if
} //End DrawHtmlString

void CPPHtmlDrawer::StoreHyperlinkArea(int left, int top, int right, int bottom)
{
	if (m_defStyle.nTypeLink != LINK_NONE)
	{
		STRUCT_HYPERLINK link;
		link.rcArea = CRect(left, top, right, bottom);
		link.sHyperlink = m_defStyle.sHyperlink;
		link.nTypeLink = m_defStyle.nTypeLink;
		link.nIndexLink = m_nCurIndexLink;
		m_arrLinks.push_back(link);
	} //if
} //StoreHyperlinkArea

void CPPHtmlDrawer::SelectNewHtmlStyle(LPCTSTR lpszNameStyle, STRUCT_CHANGESTYLE & cs)
{
	//Unpack a new styles
	UnpackTextStyle(GetTextStyle(lpszNameStyle), cs);
}

BOOL CPPHtmlDrawer::StoreRestoreStyle(BOOL bRestore)
{
	BOOL bOk = FALSE;
	if (bRestore)
	{
		//Restore styles
		if (m_arrStack.size() > 0)
		{
			STRUCT_CHANGESTYLE cs = m_arrStack.back();
			if (cs.strTag == m_defStyle.strTag)
			{
				m_defStyle = cs;
				m_arrStack.pop_back();
				bOk = TRUE;
			} //if
		} //if
		m_defStyle.strTag.Empty();
	}
	else 
	{
		m_arrStack.push_back(m_defStyle);
		bOk = TRUE;
	} //if

	return bOk;
} //End StoreRestoreStyle

void CPPHtmlDrawer::UpdateContext()
{
	::SelectObject(m_hDC, m_hOldFont);
	::DeleteObject(m_hFont);
	m_lfDefault.lfHeight = m_defStyle.nSizeFont;
	m_lfDefault.lfWeight = m_defStyle.nWeightFont;
	m_lfDefault.lfItalic = m_defStyle.bItalicFont;
	m_lfDefault.lfStrikeOut = m_defStyle.bStrikeOutFont;
	m_lfDefault.lfUnderline = m_defStyle.bUnderlineFont;
	_tcscpy (m_lfDefault.lfFaceName, m_defStyle.sFaceFont);
	m_hFont = ::CreateFontIndirect(&m_lfDefault);
	m_hOldFont = (HFONT)::SelectObject(m_hDC, m_hFont);
	::GetTextMetrics(m_hDC, &m_tm);
	
	::SetBkMode(m_hDC, m_defStyle.nBkMode);
	::SetTextColor(m_hDC, m_defStyle.crText);
	::SetBkColor(m_hDC, m_defStyle.crBkgnd);
} //End UpdateContext

int CPPHtmlDrawer::VerticalAlignText(int y, int nHeight)
{
	//Vertical align
	if (m_bIsCalculate)
	{
		//If calculate then don't output text
		m_hline.nDescentLine = max(m_hline.nDescentLine, nHeight - m_tm.tmAscent);
		m_hline.nHeightLine = max(m_hline.nHeightLine, m_tm.tmAscent);
	}
	else
	{
		switch (m_defStyle.nVertAlign)
		{
		case ALIGN_VCENTER:
			y += (m_hline.nHeightLine - m_tm.tmHeight) / 2;
			break;
		case ALIGN_BASELINE:
			y += m_hline.nHeightLine - m_hline.nDescentLine - m_tm.tmAscent;
			break;
		case ALIGN_BOTTOM:
			y += m_hline.nHeightLine - m_tm.tmAscent;
			break;
		} //switch
	} //if
	return y;
} //End VerticalAlignText

int CPPHtmlDrawer::VerticalAlignImage(int y, int nHeight)
{
	//Vertical align
	if (m_bIsCalculate)
	{
		//If calculate then don't output text
		m_hline.nHeightLine = max(m_hline.nHeightLine, nHeight);
	}
	else
	{
		switch (m_defStyle.nVertAlign)
		{
		case ALIGN_VCENTER:
			y += (m_hline.nHeightLine - nHeight) / 2;
			break;
		case ALIGN_BASELINE:
			y += m_hline.nHeightLine - m_hline.nDescentLine - nHeight;
			break;
		case ALIGN_BOTTOM:
			y += m_hline.nHeightLine - nHeight;
			break;
		} //switch
	} //if
	return y;
} //End VerticalAlignImage

int CPPHtmlDrawer::HorizontalAlign(int x)
{
	int nWidth = m_rcOutput.right - m_rcOutput.left;
	switch (m_hline.nHorzAlign)
	{
	case ALIGN_CENTER:
		x = m_rcOutput.left + (nWidth - m_hline.nWidthLine) / 2;
		break;
	case ALIGN_RIGHT:
		x = m_rcOutput.left + nWidth - m_hline.nWidthLine;
		break;
	} //switch
	return x;
} //End of HorizontalAlign

void CPPHtmlDrawer::Tag_NewLine(LPPOINT lpPoint, int nNum, LPSIZE lpSize)
{
	//New line
	if (nNum <= 0)
		nNum = 1;

	if (m_bIsCalculate)
	{
		if (!m_hline.nHeightLine)
			m_hline.nHeightLine = m_tm.tmHeight;
		lpSize->cx = max(lpSize->cx, lpPoint->x - m_rcOutput.left);
		m_hline.nWidthLine = lpPoint->x - m_rcOutput.left; //Adds the real length of the lines
		m_hline.nHeightLine += m_hline.nDescentLine; //Adds the real height of the lines
		m_arrHtmlLine [m_nCurLine] = m_hline;
	} //if
	
	m_nCurLine ++;

	lpPoint->y += m_hline.nHeightLine * nNum;
	lpPoint->x = InitNewLine(m_rcOutput.left);	
} //End Tag_NewLine

int CPPHtmlDrawer::InitNewLine(int x)
{
	if (m_bIsCalculate)
	{
		//ENG: Creates a new line with default parameters
		//RUS: �������� ����� ����� � ����������� ��-���������
		m_hline.nAddPercentWidth = 0;
		m_hline.nDescentLine = 0;
		m_hline.nHeightLine = 0;
		m_hline.nWidthLine = 0;
		m_hline.nHorzAlign = m_defStyle.nHorzAlign;
		m_arrHtmlLine.push_back(m_hline);
	}
	else
	{
		//ENG: Gets the data of the first line and converts the percent value to the real width
		//RUS: �������� ������ ������ ������ � ����������� ���������� ������ � ��������
		m_hline = m_arrHtmlLine [m_nCurLine];
		if (m_hline.nAddPercentWidth)
			m_hline.nWidthLine += ::MulDiv(m_rcOutput.right - m_rcOutput.left, m_hline.nAddPercentWidth, 100);
		x = HorizontalAlign(x);
	} //if
	return x;
} //End of InitNewLine

void CPPHtmlDrawer::Tag_Tabulation(LPPOINT lpPoint, int nNum)
{
	//Tabulation
	if (!nNum)
		nNum = 1;
	int nWidth = (lpPoint->x - m_rcOutput.left) % 32;
	if (nWidth)
	{
		//aligns with tab
		lpPoint->x += 32 - nWidth;
		nNum --;
	} //if
	lpPoint->x += (nNum * 32);
} //End Tag_Tabulation

/////////////////////////////////////////////////////////////////////////////////////////

void CPPHtmlDrawer::Draw(HDC hDC, LPCTSTR lpszHtml, LPPOINT lpPoint)
{
	//ENG: Preparing an output text
	//RUS: ���������� ������ � ������
	SIZE size;
	PrepareOutput(hDC, lpszHtml, &size);

	//ENG: If output was disabled
	//RUS: ���� ����� ��������
	if (!size.cx || !size.cy)
		return;
	
	//ENG: Calculates an output area
	//RUS: ������� ������� ������
	RECT rect;
	rect.left = lpPoint->x;
	rect.top = lpPoint->y;
	rect.right = rect.left + size.cx;
	rect.bottom = rect.top + size.cy;
	
	//ENG: Output a prepared text
	//RUS: ����� ��������������� ������
	DrawPreparedOutput(hDC, &rect);
} //End Draw

void CPPHtmlDrawer::PrepareOutput(HDC hDC, LPCTSTR lpszHtml, LPSIZE lpSize)
{
	//ENG: Copy initial parameters
	//RUS: ����������� ��������� ����������
	m_hDC = hDC;
	RECT rect;
	rect.left = rect.right = rect.top = rect.bottom = 0;
	m_csHtmlText = lpszHtml;
	lpSize->cx = lpSize->cy = 0;
	
	//ENG: If prepared text wasn't empty then return
	//RUS: ���� �������������� ����� �� ������, �� �����
	if (!m_csHtmlText.IsEmpty())
	{
		//ENG: Sets a prepare mode
		//RUS: ������������� ����� ����������
		m_bIsCalculate = TRUE;
		
		//ENG: Prepares to real output
		//RUS: ���������� � ��������� ������
		DrawHtml(lpSize, &rect);
		
		if (!lpSize->cx && !lpSize->cy)
			m_csHtmlText.Empty();
		
		lpSize->cx ++;
		lpSize->cy ++;
	} //if
} //End PrepareOutput

////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer::DrawPreparedOutput()
//		Draw a string prepared by PrepareOutput method.
//------------------------------------------------------------------
// Parameters:
//		hDC				- Device Context to drawing 
//		lpRect			- Pointer to RECT structure contains a bounding rectangle of
//						  drawing area.
////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::DrawPreparedOutput(HDC hDC, LPCRECT lpRect)
{
	//ENG: If prepared text was empty then return
	//RUS: ���� �������������� ����� ������, �� �����
	if (m_csHtmlText.IsEmpty())
		return;

	//ENG: Copy initial parameters
	//RUS: ����������� ��������� ����������
	m_hDC = hDC;
	SIZE size;
	size.cx = size.cy = 0;

	//ENG: Sets a output mode
	//RUS: ������������� ����� ������
	m_bIsCalculate = FALSE;
	
	//ENG: Real output the prepared string
	//RUS: ����� �������������� ������
	DrawHtml(&size, lpRect);
} //End of DrawPreparedOutput

// The following appeared in Paul DiLascia's Jan 1998 MSJ articles.
// It loads a "hand" cursor from the winhlp32.exe module
void CPPHtmlDrawer::SetDefaultCursor()
{
	if (m_hLinkCursor == NULL)                // No cursor handle - load our own
    {
#ifdef IDC_HAND
		//This code was added from Zorglab's comments to Hyperlink control from Chris Maunder
		m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND); // Load Windows' hand cursor
		if (m_hLinkCursor != NULL)                    // if not available, load it from winhlp32.exe
			return;
#endif
		// Get the windows directory
        CPPString strWndDir;
        GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
        strWndDir.ReleaseBuffer();

        strWndDir += _T("\\winhlp32.exe");
        // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
        HMODULE hModule = LoadLibrary(strWndDir);
        if (hModule) 
		{
            HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
            if (hHandCursor)
                m_hLinkCursor = CopyCursor(hHandCursor);
        } //if
        FreeLibrary(hModule);
    } //if
} //End SetDefaultCursor

void CPPHtmlDrawer::SetHyperlinkCursor(HCURSOR hCursor /* = NULL */)
{
	if ((m_hLinkCursor == hCursor) && (NULL != m_hLinkCursor))
		return;

	if (NULL != m_hLinkCursor)
	{
		::DestroyCursor(m_hLinkCursor);
		m_hLinkCursor = NULL;
	} //if
	

    if (NULL == hCursor)
		SetDefaultCursor();
	else
		m_hLinkCursor = hCursor;
} //End SetHyperlinkCursor

HCURSOR CPPHtmlDrawer::GetHyperlinkCursor() const
{
    return m_hLinkCursor;
} //End GetHyperlinkCursor

/////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer::SetCallbackHyperlink
// This function sets or removes the notification messages from the control before display.
//
// Parameters:
//	hWnd [in] -    If non-NULL the control will be send the notification 
//				   to specified window
//				   Else the notification will not send
///////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::SetCallbackHyperlink(HWND hWnd, UINT nMessage, LPARAM lParam /* = 0 */)
{
	TRACE(_T("CPPHtmlDrawer::SetCallbackHyperlink()\n"));

	m_csCallbackLink.hWnd = hWnd;
	if (NULL == hWnd)
	{
		m_csCallbackLink.nMessage = 0;
		m_csCallbackLink.lParam = 0;
	}
	else
	{
		m_csCallbackLink.nMessage = nMessage;
		m_csCallbackLink.lParam = lParam;
	} //if
} //End SetCallbackHyperlink

void CPPHtmlDrawer::SetCallbackRepaint(HWND hWnd, UINT nMessage, LPARAM lParam /* = 0 */)
{
	TRACE(_T("CPPHtmlDrawer::SetCallbackRepaint()\n"));

	m_csCallbackRepaint.hWnd = hWnd;
	if (NULL == hWnd)
	{
		m_csCallbackRepaint.nMessage = 0;
		m_csCallbackRepaint.lParam = 0;
	}
	else
	{
		m_csCallbackRepaint.nMessage = nMessage;
		m_csCallbackRepaint.lParam = lParam;
	} //if
} //End SetCallbackRepaint

/////////////////////////////////////////////////////////////////////////////
//  CPPToolTip::SetImageList (public member function)
//    sets the image list to tooltip
//
//  Parameters :
//		nIdBitmap	[in] - Resource IDs of the bitmap to be associated with the image list
//		cx			[in] - Dimensions of each image, in pixels.
//		cy			[in] - Dimensions of each image, in pixels.
//		nCount		[in] - Number of images that the image list initially contains.
//		crMask		[in] - Color used to generate a mask. Each pixel of this color in the 
//						   specified bitmap is changed to black, and the corresponding 
//						   bit in the mask is set to one.
//  Returns :
//		None
//
/////////////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::SetImageList(UINT nIdBitmap, int cx, int cy, int nCount, COLORREF crMask /* = RGB(255, 0, 255) */)
{
	// Load bitmap
	HBITMAP hBitmap = GetBitmapFromResources(nIdBitmap);
	SetImageList(hBitmap, cx, cy, nCount, crMask);
} //End SetImageList

/////////////////////////////////////////////////////////////////////////////
//  CPPToolTip::SetImageList (public member function)
//    sets the image list to tooltip
//
//  Parameters :
//		hBitmap		[in] - Handle of the bitmap to be associated with the image list
//		cx			[in] - Dimensions of each image, in pixels.
//		cy			[in] - Dimensions of each image, in pixels.
//		nCount		[in] - Number of images that the image list initially contains.
//		crMask		[in] - Color used to generate a mask. Each pixel of this color in the 
//						   specified bitmap is changed to black, and the corresponding 
//						   bit in the mask is set to one.
//  Returns :
//		None
//
/////////////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::SetImageList(HBITMAP hBitmap, int cx, int cy, int nCount, COLORREF crMask /* = RGB(255, 0, 255) */)
{
	//ENG: Removes previously image list
	//RUS: ������� ���������� ������ �����������
	if (NULL != m_ImageList.GetSafeHandle())
		m_ImageList.DeleteImageList();

	//ENG: If don't need to create a new image list
	//RUS: ���� �� ����� ��������� ����� ������ �����������
	if (NULL == hBitmap)
		return;
	
    m_ImageList.Create(cx, cy, ILC_COLOR32 | ILC_MASK, nCount, 1);
	m_ImageList.Add(CBitmap::FromHandle(hBitmap), crMask);
	m_szImageList = CSize(cx, cy);
} //End SetImageList

/////////////////////////////////////////////////////////////////////////////
//  CPPToolTip::GetImageList (public member function)
//    gets the image list from tooltip
//
//  Parameters :
//		sz		   [out] - Dimensions of each image, in pixels.
//  Returns :
//		A pointer to a CImageList object
//
/////////////////////////////////////////////////////////////////////////////
//CImageList * CPPHtmlDrawer::GetImageList(CSize & sz)
//{
//	sz = m_szImageList;
//	return &m_ImageList;
//} //End GetImageList

void CPPHtmlDrawer::EnableEscapeSequences(BOOL bEnable /* = TRUE */)
{
	m_bEnableEscapeSequences = bEnable;
}

void CPPHtmlDrawer::LoadResourceDll(LPCTSTR lpszPathDll, DWORD dwFlags /* = 0 */)
{
	HINSTANCE hInst = NULL;
	if (NULL != lpszPathDll)
		hInst = ::LoadLibraryEx(lpszPathDll, NULL, dwFlags);
	
	SetResourceDll(hInst);

	if (NULL != hInst)
		m_bFreeInstDll = TRUE;
} //End LoadResourceDll

void CPPHtmlDrawer::SetResourceDll(HINSTANCE hInstDll /* = NULL */)
{
	if (NULL != m_hInstDll)
	{
		if (!m_bFreeInstDll)
			return;
		::FreeLibrary(m_hInstDll);
		m_hInstDll = NULL;
	} //if

	m_bFreeInstDll = FALSE;

	if (NULL != hInstDll)
		m_hInstDll = hInstDll;
} //End SetResourceDll

/////////////////////////////////////////////////////
// Gets dimensions of the table
//---------------------------------------------------
//  In: str - the string contains a HTML table
// Return: CSize::cx - number of the columns
//         CSize::cy - number of the row
/////////////////////////////////////////////////////
CSize CPPHtmlDrawer::GetTableDimensions(CPPString str)
{
	CSize szTable(0, 0);
	CPPString strTag = _T("");
	CPPString strName = _T("");
	CPPString strProperty = _T("");
	CPPString strParameter = _T("");
	int nIndex = 0;
	int nBegin = 0;
	int nCols = 0;
	int nRows = 0;
	int nTable = 0;
	BOOL bEndOfTable = FALSE;

	SearchBodyOfTag(str, strTag, nIndex);

	while (!bEndOfTable && (nIndex < str.GetLength()))
	{
		SearchBodyOfTag(str, strTag, nIndex);
		nBegin = 0;
		strName = GetNameOfTag(strTag, nBegin);
		if (strName == _T("tr"))
		{
			//New row
			nCols = 0;
			szTable.cy ++;
		}
		else if (strName == _T("/tr"))
		{
			//End of the row
			szTable.cx = max(szTable.cx, nCols);
		}
		else if (strName == _T("td"))
		{
			//New cell
			BOOL bColspan = FALSE;
			while (nBegin < strTag.GetLength())
			{
				strProperty = SearchPropertyOfTag(strTag, nBegin);
				if (strProperty == _T("colspan"))
				{
					strParameter = GetParameterString(strTag, nBegin, _T('='), _T(" "));
					nCols += GetLengthUnit(strParameter, 0);
					nBegin = strTag.GetLength();
					bColspan = TRUE;
				} //if
			} //while
			if (!bColspan)
				nCols ++;
		}
		else if (strName == _T("table"))
		{
			//Nested table(s)
			SearchEndOfTable(str, nIndex);
		}
		else if (strName == _T("/table"))
		{
			//End of the table
			bEndOfTable = TRUE;
		} //if
	} //while
	return szTable;
} //End GetTableDimensions

/////////////////////////////////////////////////////
// Searching the end of the table
//---------------------------------------------------
//  In:    str - the string contains a HTML table
//		nIndex - index of the begin char of the <table> tag
// Out: nIndex - index of the first char after end of the table (</table> tag)
/////////////////////////////////////////////////////
void CPPHtmlDrawer::SearchEndOfTable(CPPString & str, int & nIndex)
{
	CPPString strTag = _T("");
	CPPString strName = _T("");
	int nBegin;
	int nTable;
	SearchBodyOfTag(str, strTag, nIndex); //Passes first <table> tag
	//Nested table(s)
	nTable = 1;
	while (nTable && (nIndex < str.GetLength()))
	{
		SearchBodyOfTag(str, strTag, nIndex);
		nBegin = 0;
		strName = GetNameOfTag(strTag, nBegin);
		if (strName == _T("table"))
			nTable ++;
		else if (strName == _T("/table"))
			nTable --;
	} //while
} //End SearchEndOfTable

CPPDrawManager * CPPHtmlDrawer::GetDrawManager()
{
	return &m_drawmanager;
} //End GetDrawManager

BOOL CPPHtmlDrawer::IsImageWithShadow(_STRUCT_IMAGE & si)
{
	DWORD dwStyles = si.nStyles | si.nHotStyles;
	if ((dwStyles & IMAGE_EFFECT_MONO_SHADOW) || 
		(dwStyles & IMAGE_EFFECT_GRADIENT_SHADOW))
		return TRUE;
	
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
// Map of the styles
void CPPHtmlDrawer::SetDefaultCssStyles()
{
	CPPString str = _T("");
	str += _T("body {font-size: 10pt; color:black; font-family:Verdana}\r\n");
	str += _T("p {font-size: 10pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
	str += _T("h1 {font-size: 14pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
	str += _T("h2 {font-size: 13pt; color:#ff9900; font-family:Verdana; font-weight:bold}\r\n");
	str += _T("h3 {font-size: 12pt; color:#ff9900; font-family:Arial; font-weight:bold}\r\n");
	str += _T("h4 {font-size: 10pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
	str += _T("h5 {font-size: 9pt; color:#ff9900; font-family:Verdana; font-weight:bold}\r\n");
	str += _T("h6 {font-size: 65%; color:#626262; font-family:Verdana; font-weight:normal}\r\n");
	str += _T("pre {font-size: 9pt; font-family:\"Courier\"; background-color:#fbedbb}\r\n");
	str += _T("code {color:#990000; font-family:Arial}\r\n");
	str += _T("a:link {text-decoration:none; color:blue}\r\n");
	str += _T("a:hover {text-decoration:underline; color:blue}\r\n");
	str += _T("sub {font-size:65%; vertical-align:bottom}\r\n");
	str += _T("sup {font-size:65%; vertical-align:top}\r\n");
	str += _T("big {font-size:125%}\r\n");
	str += _T("small {font-size:75%}\r\n");
	str += _T(".cpp-comment {color:green; font-style:italic}\r\n");

	SetCssStyles(str);
} //End SetDefaultCssStyle

void CPPHtmlDrawer::SetCssStyles(DWORD dwIdCssString, LPCTSTR lpszPathDll /* = NULL */)
{
	CPPString str;
	if (NULL == lpszPathDll)
		str = GetStringFromResource(dwIdCssString);
	else
		str = GetStringFromDll(dwIdCssString, lpszPathDll);
	SetCssStyles(str);
} //End SetCssStyles

void CPPHtmlDrawer::SetCssStyles(LPCTSTR lpszCssString /* = NULL */)
{
	m_mapStyles.clear(); //removes previously styles

	if (NULL == lpszCssString)
	{
		SetDefaultCssStyles();
	}
	else
	{
		CPPString str = (CPPString)lpszCssString;
		m_strCssStyles = str;
		
		CPPString strName;
		CPPString strProperty;
		
		int nBegin;
		TCHAR chSymbol;
		int nIndex = 0;
		
		while (nIndex < str.GetLength())
		{
			//Passes a space in begin string
			if (GetIndexNextAlphaNum(str, nIndex))
			{
				nBegin = nIndex;
				//Searching end of the style name
				chSymbol = GetIndexNextChars(str, nIndex, _T(" {"));
				if ((nIndex > nBegin) && (0 != chSymbol))
				{
					strName = str.Mid(nBegin, nIndex - nBegin);
					if (!strName.IsEmpty())
					{
						if (chSymbol != _T(' '))
							nIndex --;
						chSymbol = GetIndexNextChars(str, nIndex, _T("{"));
						if (0 != chSymbol)
						{
							nBegin = nIndex + 1;
							chSymbol = GetIndexNextChars(str, nIndex, _T("}"));
							if ((nIndex > nBegin) && (0 != chSymbol))
							{
								strProperty = str.Mid(nBegin, nIndex - nBegin);
								SetTextStyle(strName, strProperty);
							} //if
						} //if
					} //if
				} //if
			} //if
		} //while
	} //if
} //End SetCssStyles

LPCTSTR CPPHtmlDrawer::GetCssStyles()
{
	return (LPCTSTR)m_strCssStyles;
} //End GetCssStyles

LPCTSTR CPPHtmlDrawer::GetTextStyle(LPCTSTR lpszStyleName)
{
	CPPString name = (CPPString)lpszStyleName;
	name.MakeLower();
	iter_mapStyles iterMap = m_mapStyles.find(name);
	
	if (iterMap != m_mapStyles.end())
		return (LPCTSTR)iterMap->second;

	//Not found
	return NULL;
} //End GetTextStyle

void CPPHtmlDrawer::SetTextStyle(LPCTSTR lpszStyleName, LPCTSTR lpszStyleValue)
{
	CPPString name = (CPPString)lpszStyleName;
	name.MakeLower();
	iter_mapStyles iterMap = m_mapStyles.find(name);
	
	if (iterMap != m_mapStyles.end())
	{
		//Modifies 
		iterMap->second = (CPPString)lpszStyleValue;
	}
	else
	{
		//Add new
		m_mapStyles.insert(std::make_pair(name, (CPPString)lpszStyleValue));
	} //if
} //End SetTextStyle

void CPPHtmlDrawer::RemoveTextStyle(LPCTSTR lpszStyleName)
{
	CPPString name = (CPPString)lpszStyleName;
	name.MakeLower();
	iter_mapStyles iterMap = m_mapStyles.find(name);
	
	if (iterMap == m_mapStyles.end())
		return; //item was not found
	
	m_mapStyles.erase(iterMap);
} //End RemoveTextStyle

void CPPHtmlDrawer::AddToTextStyle(LPCTSTR lpszStyleName, LPCTSTR lpszAddStyle)
{
} //End AddToTextStyle

void CPPHtmlDrawer::UnpackTextStyle(CPPString strStyle, _STRUCT_CHANGESTYLE & cs)
{
	//Gets a string
	strStyle.MakeLower();
	if (strStyle.IsEmpty())
		return;

	CPPString strName;
	CPPString strParameter;

	int nBegin;
	TCHAR chSymbol;
	int nIndex = 0;
	CPPString str;

	while (nIndex < strStyle.GetLength())
	{
		//Passes a space in begin string
		if (GetIndexNextAlphaNum(strStyle, nIndex))
		{
			nBegin = nIndex;
			//Searching end of the style name
			chSymbol = GetIndexNextChars(strStyle, nIndex, _T(" :"));
			if (0 != chSymbol)
			{
				//Gets a property's name
				strName = strStyle.Mid(nBegin, nIndex - nBegin);

				//Gets a property's value
				strParameter = GetParameterString(strStyle, nIndex, _T(':'));

				//Analyzing name
				if (strName == _T("font-size"))
				{
					cs.nSizeFont = GetLengthUnit(strParameter, cs.nSizeFont, TRUE);
				}
				else if (strName == _T("font-family"))
				{
					if (!strParameter.IsEmpty())
						cs.sFaceFont = strParameter;
				}
				else if (strName == _T("font-style"))
				{
					cs.bItalicFont = GetStyleFontStyle(strParameter, cs.bItalicFont);
				}
				else if (strName == _T("font-weight"))
				{
					cs.nWeightFont = GetStyleFontWeight(strParameter, cs.nWeightFont);
				}
				else if (strName == _T("text-align"))
				{
					cs.nHorzAlign = GetStyleHorzAlign(strParameter, cs.nHorzAlign);
				}
				else if (strName == _T("text-transform"))
				{
					cs.nTextTransform = GetStyleTextTransform(strParameter, cs.nTextTransform);
				}
				else if (strName == _T("color"))
				{
					if (m_bIsEnable)
						cs.crText = GetStyleColor(strParameter, cs.crText);
					else
						cs.crText = GetColorByName("");
				}
				else if (strName == _T("background-color"))
				{
					if (((strParameter == _T("transparent")) && strParameter.IsEmpty()) || !m_bIsEnable)
					{
						cs.nBkMode = TRANSPARENT;
					}
					else
					{
						cs.nBkMode = OPAQUE;
						cs.crBkgnd = GetStyleColor(strParameter, cs.crBkgnd);
					} //if
				}
				else if (strName == _T("text-decoration"))
				{
					StyleTextDecoration(strParameter, cs);
				}
				else if (strName == _T("vertical-align"))
				{
					cs.nVertAlign = GetStyleVertAlign(strParameter, cs.nVertAlign);
				}
				else if (strName == _T("border-color"))
				{
					if (m_bIsEnable)
						cs.crBorderLight = GetStyleColor(strParameter, cs.crBorderLight);
					else
						cs.crBorderLight = GetColorByName("");
					cs.crBorderDark = cs.crBorderLight;
				}
				else if ((strName == _T("border-width")) || (strName == _T("size")))
				{
					cs.nBorderWidth = StyleBorderWidth(strParameter, cs.nBorderWidth);
				}
				else if (strName == _T("border-style"))
				{
					cs.nBorderStyle = StyleBorder(strParameter, cs.nBorderStyle);
				}
				else if (strName == _T("margin-left"))
				{
					cs.nMarginLeft = GetLengthUnit(strParameter, cs.nMarginLeft);
				}
				else if (strName == _T("margin-top"))
				{
					cs.nMarginTop = GetLengthUnit(strParameter, cs.nMarginTop);
				}
				else if (strName == _T("margin-right"))
				{
					cs.nMarginRight = GetLengthUnit(strParameter, cs.nMarginRight);
				}
				else if (strName == _T("margin-bottom"))
				{
					cs.nMarginBottom = GetLengthUnit(strParameter, cs.nMarginBottom);
				}
				else if (strName == _T("padding-left"))
				{
					cs.nPaddingLeft = GetLengthUnit(strParameter, cs.nPaddingLeft);
				}
				else if (strName == _T("padding-top"))
				{
					cs.nPaddingTop = GetLengthUnit(strParameter, cs.nPaddingTop);
				}
				else if (strName == _T("padding-right"))
				{
					cs.nPaddingRight = GetLengthUnit(strParameter, cs.nPaddingRight);
				}
				else if (strName == _T("padding-bottom"))
				{
					cs.nPaddingBottom = GetLengthUnit(strParameter, cs.nPaddingBottom);
				} //if
			} //if
		} //if
	} //while
} //End UnpackTextStyle

BOOL CPPHtmlDrawer::GetStyleFontStyle(CPPString & str, BOOL bDefault)
{
	if ((str == _T("normal")) || str.IsEmpty())
	{
		bDefault = FALSE;
	}
	else if ((str == _T("italic")) || (str == _T("oblique"))) 
	{
		bDefault = TRUE;
	} //if

	return bDefault;
} //End GetStyleFontStyle

int CPPHtmlDrawer::GetStyleFontWeight(CPPString & str, int nDefault)
{
	if ((str == _T("normal")) || str.IsEmpty())
	{
		nDefault = FW_NORMAL;
	}
	else if (str == _T("bold"))
	{
		nDefault = FW_BOLD;
	}
	else if (str == _T("bolder"))
	{
		nDefault = 900;
	}
	else if (str == _T("lighter"))
	{
		nDefault = 100;
	}
	else
	{
		nDefault = _ttoi(str);
	} //if

	return nDefault;
} //End GetStyleFontWeight

int CPPHtmlDrawer::GetStyleHorzAlign(CPPString & str, int nDefault)
{
	if ((str == _T("left")) || str.IsEmpty())
	{
		nDefault = ALIGN_LEFT;
	}
	else if (str == _T("center"))
	{
		nDefault = ALIGN_CENTER;
	}
	else if (str == _T("right"))
	{
		nDefault = ALIGN_RIGHT;
	}

	return nDefault;
} //End GetStyleHorzAlign

int CPPHtmlDrawer::GetStyleVertAlign(CPPString & str, int nDefault)
{
	if ((str == _T("baseline")) || str.IsEmpty())
	{
		nDefault = ALIGN_BASELINE;
	}
	else if ((str == _T("middle")) || (str == _T("vcenter")))
	{
		nDefault = ALIGN_VCENTER;
	}
	else if (str == _T("top"))
	{
		nDefault = ALIGN_TOP;
	}
	else if (str == _T("bottom"))
	{
		nDefault = ALIGN_BOTTOM;
	}
	
	return nDefault;
} //End GetStyleVertAlign

int CPPHtmlDrawer::GetStyleTextTransform(CPPString & str, int nDefault)
{
	if ((str == _T("none")) || str.IsEmpty())
	{
		nDefault = TEXT_TRANSFORM_NONE;
	}
	else if (str == _T("uppercase"))
	{
		nDefault = TEXT_TRANSFORM_UPPERCASE;
	}
	else if (str == _T("lowercase"))
	{
		nDefault = TEXT_TRANSFORM_LOWERCASE;
	}
	else if (str == _T("capitalize"))
	{
		nDefault = TEXT_TRANSFORM_CAPITALIZE;
	}
	
	return nDefault;
}

COLORREF CPPHtmlDrawer::GetStyleColor(CPPString & str, COLORREF crDefault)
{
//	if (!m_bIsEnable)
//		return GetColorByName("");
	
	if (!str.IsEmpty())
	{
		if (str.GetAt(0) == _T('#'))
		{
			if (str.GetLength() == 7)
			{
				CPPString strHex = _T("0x");
				strHex += str.Mid(5, 2);
				strHex += str.Mid(3, 2);
				strHex += str.Mid(1, 2);
				crDefault = (COLORREF)_tcstoul(strHex, 0, 0);
			} //if
		}
		else if ((str.GetAt(0) >= '0') && (str.GetAt(0) <= '9'))
			crDefault = (COLORREF)_tcstoul(str, 0, 0);
		else
			crDefault = GetColorByName(str, crDefault);
	} //if 

	return crDefault;
} //End GetStyleColor

int CPPHtmlDrawer::GetLengthUnit(CPPString & str, int nDefault, BOOL bFont /* = FALSE */)
{
	if (str.IsEmpty())
		return nDefault;
	
	if (IsPercentableValue(str))
	{
		//Percent value
		int percent = _ttoi(str.Left(str.GetLength() - 1));
		return ::MulDiv(nDefault, percent, 100);
	} //if

	int nSign = 0;
	if (str.GetAt(0) == _T('+')) nSign = 1;
	else if (str.GetAt(0) == _T('-')) nSign = -1;
	
	if (0 != nSign) str = str.Right(str.GetLength() - 1);
	
	int nValue = _ttoi(str.Left(str.GetLength() - 2));
	CPPString strUnit = str.Right(2);
	if (strUnit == _T("px"))		nDefault = nValue;
	else if (strUnit == _T("ex"))
	{
		SIZE szText;
		CPPString strText = _T("x");
		::GetTextExtentPoint(m_hDC, strText, strText.GetLength(), &szText);
		nDefault = nValue * szText.cy;
	}
	else if (strUnit == _T("em"))	nDefault = nValue * m_tm.tmHeight;
	else
	{
		//Gets pixel in inch
		nValue *= ::GetDeviceCaps(m_hDC, LOGPIXELSY);
		if (strUnit == _T("in"))		nDefault = nValue;
		else if (strUnit == _T("cm"))	nDefault = (int)((double)nValue / 2.54);
		else if (strUnit == _T("mm"))	nDefault = (int)((double)nValue / 25.4);
		else if (strUnit == _T("pt"))	nDefault = nValue / 72;
		else if (strUnit == _T("pc"))	nDefault = nValue / 6;
		else
		{
			nValue = _tcstoul(str, 0, 0);//_ttoi(str);
			if ((nValue > 0) && (nValue < 8) && bFont)
			{
				int nSize [] = {8, 10, 12, 14, 18, 24, 36};
				nDefault = nSize [nValue - 1];
			}
			else
			{
				nDefault = nValue;
			} //if
		} //if
	} //if
	
	return nDefault;
} //End GetLengthUnit

void CPPHtmlDrawer::StyleTextDecoration(CPPString & str, _STRUCT_CHANGESTYLE & cs)
{
	if (str.IsEmpty())
		str = _T("none");
	
	int nBegin = 0;
	int nEnd = 0;
	CPPString strTemp;
	while (nBegin < str.GetLength())
	{
		if (GetIndexNextAlphaNum(str, nBegin))
		{
			nEnd = nBegin;
			GetIndexNextChars(str, nEnd, _T(" ,"));
			strTemp = str.Mid(nBegin, nEnd - nBegin);
			nBegin = nEnd;
			if (strTemp == _T("none"))
			{
				cs.bUnderlineFont = FALSE;
				cs.bStrikeOutFont = FALSE;
				cs.bOverlineFont = FALSE;
			}
			else if (strTemp == _T("underline"))
			{
				cs.bUnderlineFont = TRUE;
			}
			else if (strTemp == _T("line-through"))
			{
				cs.bStrikeOutFont = TRUE;
			}
			else if (strTemp == _T("overline"))
			{
				cs.bOverlineFont = TRUE;
			}  //if
		} //if
	} //while
} //End StyleTextDecoration

int CPPHtmlDrawer::StyleBorderWidth(CPPString & str, int nDefault)
{
	if (str ==_T("thin"))		nDefault = ::MulDiv(75, nDefault, 100);
	else if (str ==_T("thick"))	nDefault = ::MulDiv(125, nDefault, 100);
	else if (str !=_T("medium"))nDefault = GetLengthUnit(str, nDefault);

	return nDefault;
} //End StyleBorderWidth

int CPPHtmlDrawer::StyleBorder(CPPString & str, int nDefault)
{
	if ((str == _T("none")) || str.IsEmpty()) nDefault = CPPDrawManager::PEN_NULL;
	else if (str == _T("solid")) nDefault = CPPDrawManager::PEN_SOLID;
	else if (str == _T("dotted")) nDefault = CPPDrawManager::PEN_DOT;
	else if (str == _T("dashed")) nDefault = CPPDrawManager::PEN_DASH;
	else if (str == _T("double")) nDefault = CPPDrawManager::PEN_DOUBLE;

	return nDefault;
} //End StyleBorder

void CPPHtmlDrawer::SetDefaultStyles(_STRUCT_CHANGESTYLE & cs)
{
	m_defStyle.strTag.Empty();		//The name of the last opened tag
	
	//Font
	m_defStyle.nSizeFont = 16;		//The height of the logic font
	m_defStyle.nWeightFont = FW_NORMAL;	//The weight of the logic font
	m_defStyle.bItalicFont = FALSE;	//Is italic logic font?
	m_defStyle.bUnderlineFont = FALSE;//Is underline logic font?
	m_defStyle.bStrikeOutFont = FALSE;//Is strikeout logic font?
	m_defStyle.bOverlineFont = FALSE; //Is overline logic font?
	m_defStyle.sFaceFont = _T("Verdana");  //The face name of the logic font
	
	//Color		
	m_defStyle.crText = RGB (0, 0, 0);	//The foreground color 
	m_defStyle.crBkgnd = RGB (255, 255, 255);	//The background color (also begin for the gradient)
	m_defStyle.crBorderLight = RGB (0, 0, 0);	//The border color
	m_defStyle.crBorderDark = RGB (0, 0, 0);	//The border color
	m_defStyle.crMidBkgnd = RGB (255, 255, 255);//The middle background color
	m_defStyle.crEndBkgnd = RGB (255, 255, 255);//The end background color
	
	//Fill
	m_defStyle.nBkMode = TRANSPARENT;		//The background mode for the text (TRANSPARENT, OPAQUE)
	m_defStyle.nFillBkgnd = -1;	//The fill effect of the background
	m_defStyle.strNameResBk.Empty();
	
	//Align
	m_defStyle.nHorzAlign = ALIGN_LEFT;	//The horizontal align
	m_defStyle.nVertAlign = ALIGN_BASELINE;	//The vertical align
	
	//Border
	m_defStyle.nBorderStyle = CPPDrawManager::PEN_NULL;	//The border style
	m_defStyle.nBorderWidth = 0;	//The width of the border
	
	//Text
	m_defStyle.nTextTransform = TEXT_TRANSFORM_NONE;//Transformation of the text (NONE, UPPERCASE, LOWERCASE, CAPITALIZE)
	
	//Margins
	m_defStyle.nMarginLeft = 2;
	m_defStyle.nMarginRight = 2;
	m_defStyle.nMarginTop = 2;
	m_defStyle.nMarginBottom = 2;
	
	//Padding
	m_defStyle.nPaddingLeft = 0;
	m_defStyle.nPaddingRight = 0;
	m_defStyle.nPaddingTop = 0;
	m_defStyle.nPaddingBottom = 0;
	
	//Hyperlink
	m_defStyle.nTypeLink = LINK_NONE;		//The type of the link (NONE, HREF, MESSAGE)
	m_defStyle.sHyperlink.Empty(); //The additional parameter for the link
} //SetDefaultStyles

/////////////////////////////////////////////////////////////////
// Search body of the next tag
//---------------------------------------------------------------
// Parameters:
//     In: str    - a string with html text
//         nIndex - an index of the first char to the searching in the string
//    Out: nIndex - an index of the char in the string after found tag's text
//         strTag - a string contained the tag's text if was found
// Return: A string before found tag's text 
/////////////////////////////////////////////////////////////////
CPPString CPPHtmlDrawer::SearchBodyOfTag(CPPString & str, CPPString & strTag, int & nIndex)
{
	int nBegin;
	TCHAR chFound;
	CPPString sText = _T("");
	strTag.Empty();

	while (nIndex < str.GetLength())
	{
		nBegin = nIndex;
		//Searching a chars of the begin tag
		chFound = GetIndexNextChars(str, nIndex, _T("<\n\r\t"));
		sText += str.Mid(nBegin, nIndex - nBegin);
		switch (chFound)
		{
		case 0:
			//End of the searching string
			return sText; 
		case _T('\n'):
		case _T('\r'):
		case _T('\t'):
			nIndex ++; //Jump to the char after the tag
			if (m_bEnableEscapeSequences)
			{
				//If escape sequnces was enabled then tag was found
				strTag += chFound;
				return sText;
			}
			break;
		case _T('<'):
			//May be it is a begin of the tag?
			if ((nIndex < (str.GetLength() - 1)) && (_T('<') != str.GetAt(nIndex + 1)))
			{
				//Yes of cause!!!
				nIndex++;
				nBegin = nIndex;
				//Searching end of the tag
				GetIndexNextChars(str, nIndex, _T(">"));
				//Retrive string of the tag
				strTag = str.Mid(nBegin, nIndex - nBegin);
				nIndex ++; //Jump to the char after the tag
				return sText;
			}
			//No, it is a char '<'
			sText += chFound;
			nIndex += 2;
			break;
		} //switch
	} //while
	return sText;
} //End SearchBodyOfTag

/////////////////////////////////////////////////////////////////
// Searching the next property of the tag
//---------------------------------------------------------------
// Parameters:
//     In: str    - a string with html text
//         nIndex - an index of the first char to the searching in the string
//    Out: nIndex - an index of the char in the string after found tag's text
// Return: A property's string 
/////////////////////////////////////////////////////////////////
CPPString CPPHtmlDrawer::SearchPropertyOfTag(CPPString & str, int & nIndex)
{
	CPPString sText = _T("");
	
	//Passes the spaces before a property
	if (GetIndexNextAlphaNum(str, nIndex))
	{
		//The begin of the property was found
		int nBegin = nIndex;
		//Searching end of the property
		TCHAR chFound = GetIndexNextChars(str, nIndex, _T(" ="));
		//Gets a property's string
		sText = str.Mid(nBegin, nIndex - nBegin);
	} //if
	return sText;
} //End SearchPropertyOfTag

/////////////////////////////////////////////////////////////////
// Search a tag
//---------------------------------------------------------------
// Parameters:
//     In: str    - a string with html text
//         nIndex - an index of the first char to the searching in the string
//    Out: nIndex - an index of the first char of the tag
// Return: TRUE if specified tag was found 
//---------------------------------------------------------------
// Example: (strTag = "table") or (strTag = "/table")
/////////////////////////////////////////////////////////////////
BOOL CPPHtmlDrawer::SearchTag(CPPString & str, int & nIndex, CPPString strTag)
{
	strTag = _T("<") + strTag;
	int nBegin = nIndex;
	nIndex = str.Find(strTag, nBegin);
	if (nIndex < 0)
	{
		nIndex = str.GetLength();
		return FALSE;
	}

	return TRUE;
} //End SearchTag

/////////////////////////////////////////////////////////////////
// Search a first alpha_num chars or first arithmetic char
//---------------------------------------------------------------
// Parameters:
//     In: str    - a string with html text
//         nIndex - an index of the first char to the searching in the string
//    Out: nIndex - an index of the first found char
// Return: TRUE if specified char was found 
/////////////////////////////////////////////////////////////////
BOOL CPPHtmlDrawer::GetIndexNextAlphaNum(CPPString & str, int & nIndex, BOOL bArithmetic /* = FALSE */)
{
	TCHAR ch;
	for (; nIndex < str.GetLength(); nIndex++)
	{
		ch = str.GetAt(nIndex);
		if ((ch >= _T('0')) && (ch <= _T('9')))
			return TRUE;
		if ((ch >= _T('A')) && (ch <= _T('Z')))
			return TRUE;
		if ((ch >= _T('a')) && (ch <= _T('z')))
			return TRUE;
		if (ch == _T('.'))
			return TRUE;
		if (bArithmetic)
		{
			if ((_T('+') == ch) || (_T('-') == ch) || 
				(_T('*') == ch) || (_T('/') == ch))
				return TRUE;
		} //if
	} //for
	return FALSE;
} //End GetIndexNextAlphaNum

/////////////////////////////////////////////////////////////////
// Search a first char of the chars set
//---------------------------------------------------------------
// Parameters:
//     In: str      - a string with html text
//         nIndex   - an index of the first char to the searching in the string
//		   strChars - the set of the chars
//    Out: nIndex   - an index of the first found char
// Return: A found char or zero if chars was not found  
/////////////////////////////////////////////////////////////////
TCHAR CPPHtmlDrawer::GetIndexNextChars(CPPString & str, int & nIndex, CPPString strChars)
{
	int i;
	for (; nIndex < str.GetLength(); nIndex++)
	{
		for (i = 0; i < strChars.GetLength(); i++)
		{
			if (str.GetAt(nIndex) == strChars.GetAt(i))
				return str.GetAt(nIndex);
		} //for
	} //for
	return 0;
} //End GetIndexNextChars

/////////////////////////////////////////////////////////////////
// Search a first char isn't specified in chars set
//---------------------------------------------------------------
// Parameters:
//     In: str      - a string with html text
//         nIndex   - an index of the first char to the searching in the string
//		   strChars - the set of the chars
//    Out: nIndex   - an index of the first char isn't from chars set
// Return: A found char or zero if all chars was specified in the chars set  
/////////////////////////////////////////////////////////////////
TCHAR CPPHtmlDrawer::GetIndexNextNoChars(CPPString & str, int & nIndex, CPPString strChars)
{
	int i;
	BOOL bFound;
	for (; nIndex < str.GetLength(); nIndex++)
	{
		bFound = FALSE;
		for (i = 0; (i < strChars.GetLength()) && !bFound; i++)
		{
			if (str.GetAt(nIndex) == strChars.GetAt(i))
				bFound = TRUE;
		} //for
		if (!bFound)
			return str.GetAt(nIndex);
	} //for
	return 0;
} //End GetIndexNextNoChars

/////////////////////////////////////////////////////////////////
// Is exist a property's parameter?
//---------------------------------------------------------------
// Parameters:
//     In: str         - a string with html text
//         nIndex      - an index of the first char to the searching in the string
//		   chSeparator - the char is a begin of the parameter
//    Out: nIndex   - an index of the begin parameter (if it exist) or the begin of the next property
// Return: TRUE if parameter was found  
/////////////////////////////////////////////////////////////////
BOOL CPPHtmlDrawer::GetBeginParameter(CPPString & str, int & nIndex, TCHAR chSeparator /* = _T(':') */)
{
	TCHAR ch;
	for (; nIndex < str.GetLength(); nIndex++) 
	{
		//Gets a current char
		ch = str.GetAt(nIndex);
		if (_T(' ') != ch)
		{
			//if it is not space char
			if (chSeparator == ch)
			{
				//if begin of the property's parameter was found
				nIndex ++; //jump to the next char after a begin parameter
				return TRUE;
			}
			else
			{
				return FALSE;
			}//if
		} //if
	} //for
	return FALSE;
} //End GetBeginParameter

/////////////////////////////////////////////////////////////////
// Gets a parameter for the currrent property
//---------------------------------------------------------------
// Parameters:
//     In: str         - a string with html text
//         nIndex      - an index of the first char to the searching in the string
//		   chSeparator - the char is a begin of the parameter
//    Out: nIndex   - an index of the first char after the parameter
// Return: String of the property's parameter (empty if it is not exist)  
/////////////////////////////////////////////////////////////////
CPPString CPPHtmlDrawer::GetParameterString(CPPString & str, int & nIndex, TCHAR chBeginParam /* = _T(':') */, CPPString strSeparators /* = _T(";") */)
{
	if (GetBeginParameter(str, nIndex, chBeginParam))
	{
		//Parameter for the current property was found
		TCHAR ch = GetIndexNextNoChars(str, nIndex, strSeparators + _T(" "));
		if (0 != ch)
		{
			int nBegin = nIndex;
			if (_T('"') == str.GetAt(nIndex))
			{
				nIndex++;
				TCHAR ch = GetIndexNextChars(str, nIndex, _T("\""));
				if (_T('"') == ch)
				{
					nIndex ++;
					return str.Mid(nBegin + 1, nIndex - nBegin - 2);
				} //if
			}
			else
			{
				GetIndexNextChars(str, nIndex, strSeparators);
				return str.Mid(nBegin, nIndex - nBegin);
			} //if
		} //if
	} //if
	return _T("");
} //End GetParameterString

/////////////////////////////////////////////////////////////////
// Gets a name of the tag
//---------------------------------------------------------------
// Parameters:
//     In: str         - a tag's string
//         nIndex      - an index of the first char to the searching in the string
//    Out: nIndex   - an index of the first char after the parameter
// Return: Name of the tag (empty if it is not exist)  
/////////////////////////////////////////////////////////////////
CPPString CPPHtmlDrawer::GetNameOfTag(CPPString & str, int & nIndex)
{
	CPPString strName = _T("");
	GetIndexNextNoChars(str, nIndex, _T(" "));
	int nBegin = nIndex;
	GetIndexNextChars(str, nIndex, _T(" ="));
	if (nIndex > nBegin)
		strName = str.Mid(nBegin, nIndex - nBegin);
	
	return strName;
} //End GetNameOfTag

///////////////////////////////////////////////////////////////////////
// Analysing the cell parameters
//---------------------------------------------------------------------
// Parameters:
//   In: strTag - str string contains parameters of the <table>, <td> or <tr> tags
//           cs - the structures contains the current styles
//  Out:     cs - the structures contains the new styles
///////////////////////////////////////////////////////////////////////
CSize CPPHtmlDrawer::AnalyseCellParam(CPPString & strTag, _STRUCT_CHANGESTYLE & cs)
{
	CSize szSpan(1, 1);
	if (strTag.IsEmpty())
		return szSpan;
	
	int i = 0;
	CPPString strProperty;
	CPPString strParameter;
	
	while (i < strTag.GetLength())
	{
		strProperty = SearchPropertyOfTag(strTag, i);
		strParameter = GetParameterString(strTag, i, _T('='), _T(" "));
			
		if (strProperty == _T("rowspan"))
		{
			szSpan.cy = GetLengthUnit(strParameter, szSpan.cy);
		}
		else if (strProperty == _T("colspan"))
		{
			szSpan.cx = GetLengthUnit(strParameter, szSpan.cx);
		}
		else if (strProperty == _T("border"))
		{
			cs.nBorderWidth = GetLengthUnit(strParameter, cs.nBorderWidth);
			if (!cs.nBorderWidth)
				cs.nBorderStyle = CPPDrawManager::PEN_NULL;
			else if (CPPDrawManager::PEN_NULL == cs.nBorderStyle)
				cs.nBorderStyle = CPPDrawManager::PEN_SOLID;
		}
		else if (strProperty == _T("borderstyle"))
		{
			cs.nBorderStyle = StyleBorder(strParameter, cs.nBorderStyle);
		}
		else if (strProperty == _T("bordercolor"))
		{
			if (m_bIsEnable)
				cs.crBorderLight = GetStyleColor(strParameter, cs.crBorderLight);
			else
				cs.crBorderLight = GetColorByName("");
			cs.crBorderDark = cs.crBorderLight;
		}
		else if (strProperty == _T("bordercolorlight"))
		{
			if (m_bIsEnable)
				cs.crBorderLight = GetStyleColor(strParameter, cs.crBorderLight);
			else
				cs.crBorderLight = GetColorByName("");
		}
		else if (strProperty == _T("bordercolordark"))
		{
			if (m_bIsEnable)
				cs.crBorderDark = GetStyleColor(strParameter, cs.crBorderDark);
			else
				cs.crBorderDark = GetColorByName("");
		}
		else if (strProperty == _T("bgcolor"))
		{
			if (m_bIsEnable)
			{
				cs.crBkgnd = GetStyleColor(strParameter, cs.crBkgnd);
				if (cs.nFillBkgnd < 0)
					cs.nFillBkgnd = CPPDrawManager::EFFECT_SOLID;
			} //if
		}
		else if (strProperty == _T("bgmidcolor"))
		{
			if (m_bIsEnable)
				cs.crMidBkgnd = GetStyleColor(strParameter, cs.crMidBkgnd);
		}
		else if (strProperty == _T("bgendcolor"))
		{
			if (m_bIsEnable)
				cs.crEndBkgnd = GetStyleColor(strParameter, cs.crEndBkgnd);
		}
		else if (strProperty == _T("bgeffect"))
		{
			if (m_bIsEnable)
				cs.nFillBkgnd = GetStyleBkgndEffect(strParameter, cs.nFillBkgnd);
		}
		else if (strProperty == _T("align"))
		{
			cs.nHorzAlign = GetStyleHorzAlign(strParameter, cs.nHorzAlign);
		}
		else if (strProperty == _T("valign"))
		{
			cs.nVertAlign = GetStyleVertAlign(strParameter, cs.nVertAlign);
		}
		else if (strProperty == _T("cellpadding"))
		{
			cs.nMarginLeft = cs.nMarginTop = 
			cs.nMarginRight = cs.nMarginBottom = GetLengthUnit(strParameter, cs.nMarginLeft);
		}
		else if (strProperty == _T("cellspacing"))
		{
			cs.nPaddingLeft = cs.nPaddingTop = 
			cs.nPaddingRight = cs.nPaddingBottom = GetLengthUnit(strParameter, cs.nPaddingLeft);
		} 
		else if (strProperty == _T("background"))
		{
			cs.strNameResBk = strParameter;
		} //if
	} //for

	return szSpan;
} //End AnalyseCellParam

///////////////////////////////////////////////////////////////////////
// Analysing the image parameters
//---------------------------------------------------------------------
// Parameters:
//   In: strTag - the string contains
//           si - the structures contains the image parameters
//  Out:     si - the structures contains the image parameters
///////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::AnalyseImageParam(CPPString & strTag, _STRUCT_IMAGE & si)
{
	if (strTag.IsEmpty())
		return;
	
	int i = 0;
	CPPString strProperty;
	CPPString strParameter;
	
	while (i < strTag.GetLength())
	{
		strProperty = SearchPropertyOfTag(strTag, i);
		strParameter = GetParameterString(strTag, i, _T('='), _T(" "));
			
		if (strProperty == _T("index"))
		{
			si.nIndexImageList = GetLengthUnit(strParameter, si.nIndexImageList);
		}
		else if (strProperty == _T("idres"))
		{
			si.nIdRes = GetLengthUnit(strParameter, si.nIdRes);
		}
		else if (strProperty == _T("iddll"))
		{
			si.nIdDll = GetLengthUnit(strParameter, si.nIdDll);
		}
		else if (strProperty == _T("handle"))
		{
			si.nHandle = GetLengthUnit(strParameter, si.nHandle);
		}
		else if (strProperty == _T("file"))
		{
			si.strSrcFile = GetStyleString(strParameter, si.strSrcFile);
		}
		else if (strProperty == _T("srcdll"))
		{
			si.strPathDll = GetStyleString(strParameter, si.strPathDll);
		}
		else if (strProperty == _T("mask"))
		{
			si.crMask = GetStyleColor(strParameter, si.crMask);
			si.bUseMask = TRUE;
		}
		else if (strProperty == _T("style"))
		{
			si.nStyles = GetStyleImageShortForm(strParameter);
			si.nHotStyles = si.nStyles;
		}
		else if (strProperty == _T("hotstyle"))
		{
			si.nHotStyles = GetStyleImageShortForm(strParameter);
		}
		else if (strProperty == _T("cx"))
		{
			si.cx = GetLengthUnit(strParameter, si.cx);
		}
		else if (strProperty == _T("cy"))
		{
			si.cy = GetLengthUnit(strParameter, si.cy);
		}
		else if (strProperty == _T("width"))
		{
			si.bPercentWidth = IsPercentableValue(strParameter);
			si.nWidth = GetLengthUnit(strParameter, si.nWidth);
		}
		else if (strProperty == _T("height"))
		{
			si.bPercentHeight = IsPercentableValue(strParameter);
			si.nHeight = GetLengthUnit(strParameter, si.nHeight);
		}
		else if (strProperty == _T("speed"))
		{
			si.nSpeed = GetLengthUnit(strParameter, si.nSpeed);
		} //if
	} //for
} //End AnalyseImageParam

CPPString CPPHtmlDrawer::GetStyleString(CPPString str, CPPString strDefault)
{
	if (!str.IsEmpty())
		strDefault = str;
	return str;
}

///////////////////////////////////////////////////////////////////////
// Analysing the short form of the font style
//---------------------------------------------------------------------
// Parameters:
//   In: str - string contains parameters of the font in the short form
// Short form styles
//       [+] - positive style
//       [-] - inverse style
//       [b] - bold
//       [i] - italic
//       [u] - underlined
//       [s] - strikeout
//       [o] - overline
///////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::GetStyleFontShortForm(CPPString & str)
{
	if (!str.IsEmpty())
	{
		BOOL bSetValue = TRUE;
		for (int i = 0; i < str.GetLength(); i++)
		{
			switch (str.GetAt(i))
			{
			case _T('-'):
				bSetValue = FALSE;
				break;
			case _T('+'):
				bSetValue = TRUE;
				break;
			case _T('b'):
				m_defStyle.nWeightFont = (bSetValue) ? FW_BOLD : FW_NORMAL;
				bSetValue = TRUE;
				break;
			case _T('i'):
				m_defStyle.bItalicFont = bSetValue;
				bSetValue = TRUE;
				break;
			case _T('u'):
				m_defStyle.bUnderlineFont = bSetValue;
				bSetValue = TRUE;
				break;
			case _T('s'):
				m_defStyle.bStrikeOutFont = bSetValue;
				bSetValue = TRUE;
				break;
			case _T('o'):
				m_defStyle.bOverlineFont = bSetValue;
				bSetValue = TRUE;
				break;
			} //switch
		} //for
	} //if
} //End GetStyleFontShortForm

//Get font style value
UINT CPPHtmlDrawer::GetStyleImageShortForm(CPPString & str)
{
	UINT uStyle = 0; //Original image
	
	if (!str.IsEmpty())
	{
		for (int i = 0; i < str.GetLength(); i++)
		{
			switch (str.GetAt(i))
			{
			case _T('d'):
				uStyle |= IMAGE_EFFECT_DARKEN;
				break;
			case _T('g'):
				uStyle |= IMAGE_EFFECT_GRAYEN;
				break;
			case _T('s'):
				if (m_szOffsetShadow.cx || m_szOffsetShadow.cy)
				{
					if (m_bGradientShadow)
						uStyle |= IMAGE_EFFECT_GRADIENT_SHADOW;
					else uStyle |= IMAGE_EFFECT_MONO_SHADOW;
				} //if
				break;
			case _T('l'):
				uStyle |= IMAGE_EFFECT_LIGHTEN;
				break;
			} //switch
		} //for
	} //if
	
	return uStyle;
} //End GetStyleImageShortForm

BOOL CPPHtmlDrawer::IsPercentableValue(CPPString & str)
{
	if (!str.IsEmpty())
	{
		if (str.GetAt(str.GetLength() - 1) == _T('%'))
			return TRUE;
	}
	return FALSE;
}

int CPPHtmlDrawer::GetStyleBkgndEffect(CPPString & str, int nDefault)
{
	if (!str.IsEmpty())
	{
		if (str == _T("transparent"))
			nDefault = -1;
		else if (str == _T("solid"))
			nDefault = CPPDrawManager::EFFECT_SOLID;
		else if (str == _T("hgradient"))
			nDefault = CPPDrawManager::EFFECT_HGRADIENT;
		else if (str == _T("vgradient"))
			nDefault = CPPDrawManager::EFFECT_VGRADIENT;
		else if (str == _T("hcgradient"))
			nDefault = CPPDrawManager::EFFECT_HCGRADIENT;
		else if (str == _T("vcgradient"))
			nDefault = CPPDrawManager::EFFECT_VCGRADIENT;
		else if (str == _T("3hgradient"))
			nDefault = CPPDrawManager::EFFECT_3HGRADIENT;
		else if (str == _T("3vgradient"))
			nDefault = CPPDrawManager::EFFECT_3VGRADIENT;
#ifdef USE_SHADE
		else if (str == _T("noise"))
			nDefault = CPPDrawManager::EFFECT_NOISE;
		else if (str == _T("diagshade"))
			nDefault = CPPDrawManager::EFFECT_DIAGSHADE;
		else if (str == _T("hshade"))
			nDefault = CPPDrawManager::EFFECT_HSHADE;
		else if (str == _T("vshade"))
			nDefault = CPPDrawManager::EFFECT_VSHADE;
		else if (str == _T("hbump"))
			nDefault = CPPDrawManager::EFFECT_HBUMP;
		else if (str == _T("vbump"))
			nDefault = CPPDrawManager::EFFECT_VBUMP;
		else if (str == _T("softbump"))
			nDefault = CPPDrawManager::EFFECT_SOFTBUMP;
		else if (str == _T("hardbump"))
			nDefault = CPPDrawManager::EFFECT_HARDBUMP;
		else if (str == _T("metal"))
			nDefault = CPPDrawManager::EFFECT_METAL;
#endif
		else nDefault = GetLengthUnit(str, nDefault);
	} //if

	return nDefault;
} //End GetStyleBkgndEffect

int CPPHtmlDrawer::GetTableWidth(CPPString & str, int nClientWidth, int nMinWidth, BOOL bSet /* = FALSE */)
{
	if (!str.IsEmpty())
	{
		int i = 0;
		CPPString strProperty;
		CPPString strParameter;
		
		while (i < str.GetLength())
		{
			strProperty = SearchPropertyOfTag(str, i);
			strParameter = GetParameterString(str, i, _T('='), _T(" "));
			strProperty.MakeLower();
			
			if (strProperty == _T("width"))
			{
				if (IsPercentableValue(strParameter))
				{
					int nWidth = GetLengthUnit(strParameter, 100);
					if (bSet)
					{
						if (nWidth <= 100)
							nClientWidth = ::MulDiv(nMinWidth, 100, nWidth);
						else
							nClientWidth = ::MulDiv(nMinWidth, nWidth, 100);
					}
					else
					{
						if (nWidth < 100)
							nClientWidth = ::MulDiv(nClientWidth, nWidth, 100);
					} //if
				}
				else
				{
					nClientWidth = GetLengthUnit(strParameter, nMinWidth);
				} //if
				break;
			} //if
		} //while
	} //if

	if (nClientWidth < nMinWidth)
		nClientWidth = nMinWidth;

	return nClientWidth;
} //End GetTableWidth

void CPPHtmlDrawer::DrawBackgroundImage(HDC hDC, int nDestX, int nDestY, int nWidth, int nHeight, CPPString strNameImage)
{
	if (!m_bIsEnable)
		return;
	if (strNameImage.IsEmpty())
		return;
	if (strNameImage.GetLength() < 6)
		return;

	HBITMAP hBitmap = NULL;

	int nIndex = 0;
	if (GetIndexNextAlphaNum(strNameImage, nIndex))
	{
		int nBegin = nIndex;
		//Searching end of the style name
		TCHAR chSymbol = GetIndexNextChars(strNameImage, nIndex, _T(" :"));
		if (0 != chSymbol)
		{
			//Gets a property's name
			CPPString strName = strNameImage.Mid(nBegin, nIndex - nBegin);
			//Gets a property's value
			CPPString strParameter = GetParameterString(strNameImage, nIndex, _T(':'));
			
			if (strName == _T("idres"))
			{
				UINT nID = (UINT)GetLengthUnit(strParameter, 0);
				hBitmap = GetBitmapFromResources(nID);
			}
			else if (strName == _T("iddll"))
			{
				UINT nID = (UINT)GetLengthUnit(strParameter, 0);
				hBitmap = GetBitmapFromDll(nID);
			}
			else if (strName == _T("file"))
			{
				hBitmap = GetBitmapFromFile(strParameter);
			} //if
		} //if
	} //if

	if (NULL == hBitmap)
		return;

	SIZE sz;
	m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);
	HDC hSrcDC = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hSrcDC, hBitmap);
	m_drawmanager.MultipleCopy(hDC, nDestX, nDestY, nWidth, nHeight, hSrcDC, 0, 0, sz.cx, sz.cy);
	::SelectObject(hSrcDC, hOldBitmap);
	::DeleteDC(hSrcDC);

	::DeleteObject(hBitmap);
	hBitmap = NULL;
} //End of DrawBackgroundImage

////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer::SetTooltipShadow()
//		Sets a image's shadow.
//------------------------------------------------------------------
// Parameters:
//		nOffsetX, 
//		nOffsetY		- The offsets of the tooltip's shadow from the tooltip's window.
//		nDarkenPercent	- So far as colors under the shadow will be darken (0 - 100)
//      bGradient		- TRUE to use a gradient shadow.
//		nDepthX,
//		nDepthY			- The gradient depths of the tooltip's shadow.
////////////////////////////////////////////////////////////////////
void CPPHtmlDrawer::SetImageShadow(int nOffsetX, int nOffsetY, BYTE nDarkenPercent /* = 50 */, 
								  BOOL bGradient /* = TRUE */, int nDepthX /* = 7 */, int nDepthY /* = 7 */)
{
	m_szOffsetShadow.cx = nOffsetX;
	m_szOffsetShadow.cy = nOffsetY;
	m_szDepthShadow.cx = nDepthX;
	m_szDepthShadow.cy = nDepthY;
	m_nDarkenShadow = min(100, nDarkenPercent);
	m_bGradientShadow = bGradient;
	BYTE nColor = ::MulDiv(255, 100 - m_nDarkenShadow, 100);
	m_crShadow = RGB(nColor, nColor, nColor);
} //End of SetTooltipShadow
