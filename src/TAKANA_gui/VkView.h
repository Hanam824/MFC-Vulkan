#pragma once

class AFX_EXT_CLASS CVkView : public CView
{
	DECLARE_DYNCREATE(CVkView)

protected:
	CVkView();
	virtual ~CVkView();


public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

protected:

};

