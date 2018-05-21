#include "stdafx.h"
#include "ChildFrm.h"
#include "ViewLeft.h"
#include "ViewRightTop.h"
#include "ViewRightBottom.h"

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	CRect rect { };
	::GetClientRect(AfxGetMainWnd()->m_hWnd, &rect);

	m_MainSplitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE);
	m_MainSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewLeft), CSize(rect.Width() / 5, 0), pContext);
	m_RightSplitter.CreateStatic(&m_MainSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_MainSplitter.IdFromRowCol(0, 1));
	m_RightSplitter.CreateView(0, 0, RUNTIME_CLASS(CViewRightTop), CSize(0, rect.Height() / 2), pContext);
	m_RightSplitter.CreateView(1, 0, RUNTIME_CLASS(CViewRightBottom), CSize(0, rect.Height() / 2), pContext);

	m_fSpliterCreated = true;

	return TRUE;
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC)
{
	return CMDIChildWndEx::OnEraseBkgnd(pDC);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	if (m_fSpliterCreated)
	{
		CRect rect { };
		::GetClientRect(AfxGetMainWnd()->m_hWnd, &rect);

		if (m_cx > 0)
		{
			int _cxCur, _min;
			m_MainSplitter.GetColumnInfo(0, _cxCur, _min);
			UINT _nCurrentWidth = ((double)_cxCur * ((double)cx / (double)m_cx)) + 0.5;
			m_MainSplitter.SetColumnInfo(0, _nCurrentWidth, _min);

			m_RightSplitter.GetRowInfo(0, _cxCur, _min);
			_nCurrentWidth = ((double)_cxCur * ((double)cy / (double)m_cy)) + 0.5;
			m_RightSplitter.SetRowInfo(0, _nCurrentWidth, 0);

			m_MainSplitter.RecalcLayout();
			m_RightSplitter.RecalcLayout();
		}
		else {
			m_MainSplitter.SetColumnInfo(0, rect.Width() / 5, 0);
			m_RightSplitter.SetRowInfo(0, rect.Height() / 2, 0);
		}

		m_cx = cx; m_cy = cy;
	}
	CMDIChildWndEx::OnSize(nType, cx, cy);
}
