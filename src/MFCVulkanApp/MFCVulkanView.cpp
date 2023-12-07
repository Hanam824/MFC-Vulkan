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

// MFCVulkanView.cpp : implementation of the CMFCVulkanView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCVulkan.h"
#endif

#include "MFCVulkanDoc.h"
#include "MFCVulkanView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCVulkanView

IMPLEMENT_DYNCREATE(CMFCVulkanView, MFCViewBase)

BEGIN_MESSAGE_MAP(CMFCVulkanView, MFCViewBase)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCVulkanView construction/destruction

CMFCVulkanView::CMFCVulkanView() noexcept
{
	// TODO: add construction code here

}

CMFCVulkanView::~CMFCVulkanView()
{
}

BOOL CMFCVulkanView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return MFCViewBase::PreCreateWindow(cs);
}

// CMFCVulkanView drawing

void CMFCVulkanView::OnDraw(CDC* /*pDC*/)
{
	CMFCVulkanDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CMFCVulkanView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCVulkanView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCVulkanView diagnostics

#ifdef _DEBUG
void CMFCVulkanView::AssertValid() const
{
	MFCViewBase::AssertValid();
}

void CMFCVulkanView::Dump(CDumpContext& dc) const
{
	MFCViewBase::Dump(dc);
}

CMFCVulkanDoc* CMFCVulkanView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCVulkanDoc)));
	return (CMFCVulkanDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCVulkanView message handlers
