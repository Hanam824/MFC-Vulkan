#pragma once
#include "VkCtrlView.h"

class AFX_EXT_CLASS CTAKANAViewBase : public CVkCtrlView
{
	DECLARE_DYNCREATE(CTAKANAViewBase)

public:
	CTAKANAViewBase();           // protected constructor used by dynamic creation
	virtual ~CTAKANAViewBase();

protected:

	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
