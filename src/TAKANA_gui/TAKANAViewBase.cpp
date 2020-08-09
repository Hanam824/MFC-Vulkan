#include "pch.h"
#include "TAKANAViewBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CTAKANAViewBase, CVkCtrlView)

BEGIN_MESSAGE_MAP(CTAKANAViewBase, CVkCtrlView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

CTAKANAViewBase::CTAKANAViewBase()
{
}

CTAKANAViewBase::~CTAKANAViewBase()
{
}

void CTAKANAViewBase::OnSize(UINT nType, int cx, int cy)
{
	CVkCtrlView::OnSize(nType, cx, cy);
}