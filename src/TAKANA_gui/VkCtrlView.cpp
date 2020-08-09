#include "pch.h"
#include "VkCtrlView.h"

IMPLEMENT_DYNCREATE(CVkCtrlView, CVkView)

CVkCtrlView::CVkCtrlView()
{
}

CVkCtrlView::~CVkCtrlView()
{
}

BEGIN_MESSAGE_MAP(CVkCtrlView, CVkView)
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CVkCtrlView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CVkCtrlView diagnostics

#ifdef _DEBUG
void CVkCtrlView::AssertValid() const
{
	CVkView::AssertValid();
}

#ifndef _WIN32_WCE
void CVkCtrlView::Dump(CDumpContext& dc) const
{
	CVkView::Dump(dc);
}
#endif
#endif //_DEBUG



// CVkCtrlView message handlers

void CVkCtrlView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CVkView::OnLButtonDown(nFlags, point);
}

void CVkCtrlView::OnMButtonDown(UINT nFlags, CPoint point)
{
	CVkView::OnMButtonDown(nFlags, point);
}

void CVkCtrlView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CVkView::OnRButtonDown(nFlags, point);
}

void CVkCtrlView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CVkView::OnLButtonUp(nFlags, point);
}

void CVkCtrlView::OnMButtonUp(UINT nFlags, CPoint point)
{
	Invalidate(FALSE);
}

void CVkCtrlView::OnRButtonUp(UINT nFlags, CPoint point)
{
	Invalidate(FALSE);
}

void CVkCtrlView::OnMouseMove(UINT nFlags, CPoint point)
{
	CVkView::OnLButtonUp(nFlags, point);
}

BOOL CVkCtrlView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CVkView::OnMouseWheel(nFlags, zDelta, pt);
}

void CVkCtrlView::OnSize(UINT nType, int cx, int cy)
{
	CVkView::OnSize(nType, cx, cy);
}


void CVkCtrlView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CVkView::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CVkCtrlView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return CVkView::OnSetCursor(pWnd, nHitTest, message);
}


void CVkCtrlView::OnTimer(UINT_PTR nIDEvent)
{
	CVkView::OnTimer(nIDEvent);
}

