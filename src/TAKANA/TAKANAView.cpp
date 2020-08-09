// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// TAKANAView.cpp : implementation of the CTAKANAView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "TAKANA.h"
#endif

#include "TAKANADoc.h"
#include "TAKANAView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTAKANAView

IMPLEMENT_DYNCREATE(CTAKANAView, CTAKANAViewBase)

BEGIN_MESSAGE_MAP(CTAKANAView, CTAKANAViewBase)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CTAKANAViewBase::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CTAKANAViewBase::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CTAKANAView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CTAKANAView construction/destruction

CTAKANAView::CTAKANAView() noexcept
{
	// TODO: add construction code here

}

CTAKANAView::~CTAKANAView()
{
}

BOOL CTAKANAView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTAKANAViewBase::PreCreateWindow(cs);
}

// CTAKANAView drawing

void CTAKANAView::OnDraw(CDC* /*pDC*/)
{
	CTAKANADoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CTAKANAView printing


void CTAKANAView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CTAKANAView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTAKANAView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTAKANAView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CTAKANAView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CTAKANAView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CTAKANAView diagnostics

#ifdef _DEBUG
void CTAKANAView::AssertValid() const
{
	CTAKANAViewBase::AssertValid();
}

void CTAKANAView::Dump(CDumpContext& dc) const
{
	CTAKANAViewBase::Dump(dc);
}

CTAKANADoc* CTAKANAView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTAKANADoc)));
	return (CTAKANADoc*)m_pDocument;
}
#endif //_DEBUG


// CTAKANAView message handlers
