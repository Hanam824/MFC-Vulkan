#include "pch.h"
#include "VulkanCtrlView.h"




IMPLEMENT_DYNCREATE(CVulkanCtrlView, CVulkanView)

CVulkanCtrlView::CVulkanCtrlView()
{
}

CVulkanCtrlView::~CVulkanCtrlView()
{
}


BEGIN_MESSAGE_MAP(CVulkanCtrlView, CVulkanView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CVulkanCtrlView::OnSize(UINT nType, int cx, int cy)
{
	CVulkanView::OnSize(nType, cx, cy);
}