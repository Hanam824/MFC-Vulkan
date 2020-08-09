#include "pch.h"
#include "VkView.h"

#include <cstdlib>
#include <fstream>

IMPLEMENT_DYNCREATE(CVkView, CView)

BEGIN_MESSAGE_MAP(CVkView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CVkView::CVkView()
{
}

CVkView::~CVkView()
{
}


int CVkView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;


	return 0;
}

void CVkView::OnDestroy()
{
	CView::OnDestroy();
	
}


// CGLView drawing

void CVkView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// CGLView diagnostics

#ifdef _DEBUG
void CVkView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CVkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG