// demoView.cpp : implementation of the CDemoView class
//

#include "stdafx.h"
#include "ximage.h"
#include "MainFrm.h"
#include "demo.h"

#include "demoDoc.h"
#include "demoView.h"
#include "memdc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CDemoView

IMPLEMENT_DYNCREATE(CDemoView, CScrollView)

BEGIN_MESSAGE_MAP(CDemoView, CScrollView)
	//{{AFX_MSG_MAP(CDemoView)
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_MESSAGE(WM_USER_NEWIMAGE, OnNewImage)
	ON_MESSAGE(WM_USER_PROGRESS, OnNewProgress)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
// CDemoView construction/destruction
CDemoView::CDemoView()
{
	SetScrollSizes(MM_TEXT, CSize(0, 0));
   VERIFY(m_brHatch.CreateHatchBrush(HS_DIAGCROSS, RGB(191, 191, 191)));
}
//////////////////////////////////////////////////////////////////////////////
CDemoView::~CDemoView()
{
}
//////////////////////////////////////////////////////////////////////////////
BOOL CDemoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CScrollView::PreCreateWindow(cs);
}
//////////////////////////////////////////////////////////////////////////////
// CDemoView drawing
void CDemoView::OnDraw(CDC* pDC)
{
	CDemoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	BOOL bPrinting = pDC->IsPrinting();

	CMemDC* pMemDC = NULL;
	if (!bPrinting) pDC = pMemDC = new CMemDC(pDC);

	if (!bPrinting && m_brHatch.m_hObject){
		CRect rect;
		GetClientRect(&rect);
		rect.right  = max(rect.right , m_totalDev.cx);
		rect.bottom = max(rect.bottom, m_totalDev.cy);
		m_brHatch.UnrealizeObject();
		CPoint pt(0, 0);
		pDC->LPtoDP(&pt);
		pt = pDC->SetBrushOrg(pt.x % 8, pt.y % 8);
		CBrush* old = pDC->SelectObject(&m_brHatch);
		pDC->FillRect(&rect, &m_brHatch);
		pDC->SelectObject(old);
	}

	CxImage* ima = pDoc->GetImage(); 
	if (ima) {
		if (bPrinting) {
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			//int dcbpp = pDC->GetDeviceCaps(BITSPIXEL);
			//int dcnc = pDC->GetDeviceCaps(NUMCOLORS);
			//int dcp = pDC->GetDeviceCaps(PLANES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);
			// Best Fit case: create a rectangle which preserves the aspect ratio
			int cx=(ima->GetWidth()*cxInch)/ima->GetXDPI();
			int cy=(ima->GetHeight()*cyInch)/ima->GetYDPI();
			// print it!
			/*HDC TmpDC=CreateCompatibleDC(pDC->GetSafeHdc());
			HBITMAP bm =::CreateCompatibleBitmap(pDC->GetSafeHdc(), cx, cy);
			HBITMAP oldbm = (HBITMAP)::SelectObject(TmpDC,bm);
			BitBlt(TmpDC,0,0,cx,cy,0,0,0,WHITENESS);
			ima->Draw(TmpDC,CRect(0,0,cx,cy));
			BitBlt(pDC->GetSafeHdc(),100,100,cx,cy,TmpDC,0,0,SRCCOPY);
			DeleteObject(SelectObject(TmpDC,oldbm));
			DeleteDC(TmpDC);*/
			CxImage tmp;
			tmp.Copy(*ima);
			RGBQUAD c={255,255,255,0};
			tmp.SetTransColor(c);
			tmp.AlphaStrip();
			tmp.Stretch(pDC->GetSafeHdc(), CRect(100,100,cx,cy));
		}
		else if (pDoc->GetStretchMode()) {
			CRect rect;
			GetClientRect(&rect);
			ima->Draw(pDC->GetSafeHdc(), rect);
		} else {
			float zoom=pDoc->GetZoomFactor();
			if (zoom==1)
				ima->Draw(pDC->GetSafeHdc());
			else
				ima->Draw(pDC->GetSafeHdc(),
						CRect(0,0,(int)(ima->GetWidth()*zoom),(int)(ima->GetHeight()*zoom)));
		}
	}

	delete pMemDC;
}
//////////////////////////////////////////////////////////////////////////////
// CDemoView printing
BOOL CDemoView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	pInfo->SetMaxPage(1);
	return DoPreparePrinting(pInfo);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}
//////////////////////////////////////////////////////////////////////////////
// CDemoView diagnostics
#ifdef _DEBUG
void CDemoView::AssertValid() const
{
	CScrollView::AssertValid();
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
//////////////////////////////////////////////////////////////////////////////
CDemoDoc* CDemoView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDemoDoc)));
	return (CDemoDoc*)m_pDocument;
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////
void CDemoView::GetImageXY(CDemoDoc *pDoc, CxImage *ima, long *x, long *y)
{
	if (!ima || !pDoc) return;

	CPoint point=GetScrollPosition();
	float fx =(float)(*x + point.x);
	float fy =(float)(*y + point.y);

	fx/=pDoc->GetZoomFactor();
	fy/=pDoc->GetZoomFactor();

	if (pDoc->GetStretchMode())	{
		CRect rect;
		GetClientRect(&rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		fx *= ima->GetWidth()/(float)width;
		fy *= ima->GetHeight()/(float)height;
	}
	*x = (long)fx;
	*y = (long)fy;
}
//////////////////////////////////////////////////////////////////////////////
// CDemoView message handlers
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDemoDoc* pDoc = GetDocument();
	CxImage*  ima  = pDoc->GetImage();
	if (!ima)	return;

	// We'll get the RGB values at the point the user selects
	long x = point.x;
	long y = point.y;
	GetImageXY(pDoc, ima, &x,&y);

	char s[80];
	if (ima->IsInside(x,y))	{

		long yflip = ima->GetHeight() - y - 1;
		sprintf(s,"x: %d y: %d  idx: %d", x, y, ima->GetPixelIndex(x,yflip));
		RGBQUAD rgb=ima->GetPixelColor(x,yflip);
		if (ima->HasAlpha()) rgb.rgbReserved=ima->AlphaGet(x,yflip);
		else rgb.rgbReserved=ima->GetPaletteColor(ima->GetPixelIndex(x,yflip)).rgbReserved;
		sprintf(&s[strlen(s)],"  RGBA: (%d, %d, %d, %d)", rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue, rgb.rgbReserved);

		//Enable these lines if you want draw over the image	
		//if ((nFlags & MK_LBUTTON)==MK_LBUTTON){
		//	ima->SetPixelColor(x,yflip,RGB(rand()/(RAND_MAX/256),rand()/(RAND_MAX/256),rand()/(RAND_MAX/256)));
		//	Invalidate(0);
		//}

		if (nFlags & MK_LBUTTON)
			switch (pDoc->m_tool){
			case 0:
				SetCursor(LoadCursor(0,IDC_SIZEALL));
				SetScrollPos(SB_HORZ,m_RefScroll.x - point.x + m_RefPoint.x);
				SetScrollPos(SB_VERT,m_RefScroll.y - point.y + m_RefPoint.y);
				Invalidate(0);
				break;
			case 1:	//selection
				SetCursor(LoadCursor(0,IDC_CROSS));
#if CXIMAGE_DEMO_SELECT
				CDemoDoc* pDoc = GetDocument();
				float zoom=pDoc->GetZoomFactor();
				CPoint pos(GetScrollPosition());
				int i=pDoc->m_NumSel;
				pDoc->m_Sel[i].x = (long)((point.x + pos.x)/zoom);
				pDoc->m_Sel[i].y = (long)((point.y + pos.y)/zoom);
				if (i<(MAX_SEL_POINTS-2)) pDoc->m_NumSel++;
				DrawSelection();
#endif
				break;
		}

	} else strcpy(s," ");
	
	CStatusBar& statusBar = ((CMainFrame *)(AfxGetApp()->m_pMainWnd))->GetStatusBar();
	statusBar.SetPaneText(0, s);
	
	CScrollView::OnMouseMove(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////////
BOOL CDemoView::OnEraseBkgnd(CDC* pDC) 
{
	return 1;
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	CDemoDoc* pDoc = GetDocument();
	if (pDoc) pDoc->UpdateStatusBar();
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CDemoDoc* pDoc = GetDocument();
	if (pDoc) {
		switch(pDoc->m_tool){
		case 0:
			{
				CxImage* ima = pDoc->GetImage();
				if (ima) {
					m_RefScroll = GetScrollPosition();
					m_RefPoint.x = point.x;
					m_RefPoint.y = point.y;
				}
			}
			break;
		case 2:
			if (!pDoc->GetWaitingClick()) PostMessage(WM_COMMAND,ID_VIEW_ZOOMIN);
			break;
		}

		if (pDoc->GetWaitingClick()){
			pDoc->SetWaitingClick(0);
			CxImage* ima = pDoc->GetImage();
			if (ima) {
				long x = point.x;
				long y = point.y;
				GetImageXY(pDoc, ima, &x,&y);
				if (ima->IsInside(x,y))	{
					pDoc->SubmitUndo();
					long yflip = ima->GetHeight() - y - 1;
					ima->SetTransIndex(ima->GetPixelIndex(x,yflip));
					// <DP> RGB transparency
					ima->SetTransColor(ima->GetPixelColor(x,yflip));
					pDoc->UpdateAllViews(NULL);
				}
			}
		}
#if CXIMAGE_DEMO_SELECT
		 else {
			KillTimer(1);
			pDoc->m_NumSel=0;
			m_SelShift=0;
		}
#endif
	}
	CScrollView::OnLButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnLButtonUp(UINT nFlags, CPoint point) 
{
#if CXIMAGE_DEMO_SELECT
	CDemoDoc* pDoc = GetDocument();
	if (pDoc){
		CxImage* ima=pDoc->GetImage();
		if (ima){
			int i=pDoc->m_NumSel;
			if (i>2){
				pDoc->m_Sel[i].x = pDoc->m_Sel[0].x;
				pDoc->m_Sel[i].y = pDoc->m_Sel[0].y;
				pDoc->m_NumSel++;
				SetTimer(1,333,NULL);
				
				long i;
				long n=pDoc->m_NumSel;
				POINT* p = (POINT*)malloc(n*sizeof(POINT));
				long h=ima->GetHeight();
				for(i=0;i<n;i++){
					p[i].x = pDoc->m_Sel[i].x;
					p[i].y = h-1-pDoc->m_Sel[i].y;
				}
				ima->SelectionClear();
				ima->SelectionAddPolygon(p,n);
				free(p);
			} else {
				ima->SelectionDelete();
			}
			Invalidate(0);
		}
	}
#endif
	CScrollView::OnLButtonUp(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch (lHint)
	{
	case WM_USER_NEWIMAGE:
		{
			CDemoDoc* pDoc = GetDocument();
			CxImage*  ima  = pDoc->GetImage();
			int px=GetScrollPos(SB_HORZ);
			int py=GetScrollPos(SB_VERT);
			CSize sz(GetTotalSize());
			int x=(int)(ima->GetWidth()*pDoc->GetZoomFactor());
			int y=(int)(ima->GetHeight()*pDoc->GetZoomFactor());
			SetScrollSizes(MM_TEXT,	CSize(x,y));
			CSize sz2(GetTotalSize());
			if (sz.cx!=0 && sz.cy!=0){
				SetScrollPos(SB_HORZ,sz2.cx*px/sz.cx);
				SetScrollPos(SB_VERT,sz2.cy*py/sz.cy);
			}
			CWnd* pFrame=GetParentFrame();
			if (!(pFrame->IsIconic()||pFrame->IsZoomed())){
				RECT rClient,rMainCl,rFrame,rMainFr;
				((CMainFrame *)(AfxGetApp()->m_pMainWnd))->GetClientRect(&rMainCl);
				((CMainFrame *)(AfxGetApp()->m_pMainWnd))->GetWindowRect(&rMainFr);
				pFrame->GetClientRect(&rClient);
				pFrame->GetWindowRect(&rFrame);
				pFrame->SetWindowPos(0,0,0,
					(4+rFrame.right-rFrame.left-rClient.right+rClient.left)+
					min(rMainCl.right-(rFrame.left-rMainFr.left+12),x),
					(4+rFrame.bottom-rFrame.top-rClient.bottom+rClient.top)+
					min(rMainCl.bottom-(rFrame.top-rMainFr.top+12),y),
					SWP_NOMOVE|SWP_NOZORDER);
				//ResizeParentToFit(1);
			}
			if (!ima->HasSelection()) KillTimer(1);

			CMainFrame* pMain = (CMainFrame*) AfxGetMainWnd();
			if (pMain->m_HistoBar.IsWindowVisible()){
				pDoc->m_hmax=ima->Histogram(pDoc->m_hr,pDoc->m_hg,pDoc->m_hb,pDoc->m_hgray);
				pMain->m_HistoBar.Invalidate();
			}

			break;
		}
	default:
		{
			CDemoDoc* pDoc = GetDocument();
			if (pDoc){
				CxImage*  ima  = pDoc->GetImage();
				if (ima){
					if (pDoc->GetStretchMode()) SetScrollSizes(MM_TEXT,	CSize(0,0));
					else SetScrollSizes(MM_TEXT,CSize((int)(ima->GetWidth()*pDoc->GetZoomFactor()),
													  (int)(ima->GetHeight()*pDoc->GetZoomFactor())));
				}
			}
		}
	}
	CScrollView::OnUpdate(pSender, lHint, pHint);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CDemoDoc* pDoc = GetDocument();
	if (pDoc && pDoc->m_tool==2) return;

	static UINT toolbars[]={
		IDR_MAINFRAME,
		IDR_TOOLBAR1,
	};
	BCMenu menu;
	menu.LoadMenu(IDM_CXIMAGE1);
	menu.LoadToolbars(toolbars,2);
	CMenu *pContextMenu=menu.GetSubMenu(0);
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,point.x,point.y,AfxGetMainWnd());
}
//////////////////////////////////////////////////////////////////////////////
LRESULT CDemoView::OnNewImage(WPARAM wParam, LPARAM lParam)
{
	CDemoDoc* pDoc = GetDocument();
	pDoc->UpdateAllViews(0,WM_USER_NEWIMAGE);
	pDoc->UpdateStatusBar();
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
LRESULT CDemoView::OnNewProgress(WPARAM wParam, LPARAM lParam)
{
	CStatusBar& statusBar = ((CMainFrame *)(AfxGetApp()->m_pMainWnd))->GetStatusBar();
	CString s;
	s.Format("%d %%",(int)wParam);
	statusBar.SetPaneText(2, s);
	statusBar.Invalidate(1);

/*	CProgressCtrl& progressBar = ((CMainFrame *)(AfxGetApp()->m_pMainWnd))->GetProgressBar();
	progressBar.SetPos((int)wParam);
	progressBar.Invalidate(1);*/

	return 0;
}
//////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_DEMO_SELECT
void CALLBACK LineDDAProc(int x, int y, CDemoView *pview)
{
	CDC* pDC = pview->GetDC();
	if (pDC) {
		pview->m_SelCount++;
		if (pview->m_SelCount <= 4)	pDC->SetPixelV(x,y,RGB(255,255,255));
		else pDC->SetPixelV(x,y,RGB(0,0,0));
		if (pview->m_SelCount == 8) pview->m_SelCount=0;
	}
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::DrawSelection()
{
	CDemoDoc* pDoc = GetDocument();
	if (pDoc->m_NumSel>2){
		CPoint pos(GetScrollPosition());
		float zoom=pDoc->GetZoomFactor();
		m_SelCount=m_SelShift;
//		pDC->MoveTo(zoom*pDoc->m_Sel[0].x - pos.x,zoom*pDoc->m_Sel[0].y - pos.y);
		for(int i=1;i<pDoc->m_NumSel;i++){
//			pDC->LineTo(zoom*pDoc->m_Sel[i].x - pos.x,zoom*pDoc->m_Sel[i].y - pos.y);
			LineDDA((int)(zoom*pDoc->m_Sel[i-1].x - pos.x),
				    (int)(zoom*pDoc->m_Sel[i-1].y - pos.y),
					(int)(zoom*pDoc->m_Sel[i].x - pos.x),
					(int)(zoom*pDoc->m_Sel[i].y - pos.y),
					(LINEDDAPROC)LineDDAProc,(LPARAM)this);
		}
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnDestroy() 
{
	CScrollView::OnDestroy();
	((CMainFrame*)AfxGetMainWnd())->m_HistoBar.Invalidate();
#if CXIMAGE_DEMO_SELECT
	KillTimer(1);
#endif
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnTimer(UINT nIDEvent) 
{
#if CXIMAGE_DEMO_SELECT
	m_SelShift++;
	if (m_SelShift==8) m_SelShift=0;
	DrawSelection();
#endif
	CScrollView::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE){
		CMainFrame* pMain = (CMainFrame*) AfxGetMainWnd();
		if (pMain->m_fullscreen){
			pMain->PostMessage(WM_COMMAND, ID_VIEW_FULLSCREEN);
		}
	}
	
	CScrollView::OnChar(nChar, nRepCnt, nFlags);
}
//////////////////////////////////////////////////////////////////////////////
void CDemoView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CDemoDoc* pDoc = GetDocument();
	if (pDoc) {
		switch(pDoc->m_tool){
		case 2:
			if (!pDoc->GetWaitingClick()) PostMessage(WM_COMMAND,ID_VIEW_ZOOMOUT);
			return;
		}
	}
	
	CScrollView::OnRButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////////

void CDemoView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if (bActivate){
		((CMainFrame*)AfxGetMainWnd())->m_HistoBar.Invalidate();
	}
	
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
