#pragma once

#include "VulkanView.h"

class AFX_EXT_CLASS CVulkanCtrlView : public CVulkanView
{
	DECLARE_DYNCREATE(CVulkanCtrlView)

public:
	CVulkanCtrlView();
	virtual ~CVulkanCtrlView();



	// Generated message map functions
protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);


	DECLARE_MESSAGE_MAP()



private:

};
