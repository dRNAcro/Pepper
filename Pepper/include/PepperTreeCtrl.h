#pragma once

class CPepperTreeCtrl : public CTreeCtrl
{
public:
	DECLARE_DYNAMIC(CPepperTreeCtrl)
	CPepperTreeCtrl() {};
	virtual ~CPepperTreeCtrl() {};
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
protected:
	DECLARE_MESSAGE_MAP()
};


