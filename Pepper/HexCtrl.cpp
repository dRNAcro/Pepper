/************************************************************************************
* Copyright (C) 2018, Jovibor: https://github.com/jovibor/							*
* This is a HEX control for MFC, implemented as CWnd derived class.					*
* The usage is quite simple:														*
* 1. Construct CHexCtrl object:	(CHexCtrl myHex;).									*
* 2. Call CHexCtrl::Create member function to create an instance.					*
* 3. Call CHexCtrl::SetData method to set the data and its size to display as hex.	*
************************************************************************************/
#include "stdafx.h"
#include "HexCtrl.h"
#include "strsafe.h"

/************************************************************************
* CHexCtrl implementation.												*
************************************************************************/
IMPLEMENT_DYNAMIC(CHexCtrl, CWnd)

BEGIN_MESSAGE_MAP(CHexCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CHexCtrl::Create(CWnd * pParent, const RECT & rect, UINT nID, const LOGFONT* pLogFont)
{
	m_pLogFontHexView = pLogFont;

	return CWnd::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID);
}

int CHexCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRuntimeClass* pNewViewClass = RUNTIME_CLASS(CHexCtrl::CHexView);
	CCreateContext context;
	context.m_pNewViewClass = pNewViewClass;

	m_pHexView = (CHexView*)pNewViewClass->CreateObject();

	CRect rect;
	GetClientRect(rect);
	m_pHexView->Create(this, rect, 0x01, &context, m_pLogFontHexView);
	m_pHexView->ShowWindow(SW_SHOW);

	return 0;
}

void CHexCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (m_pHexView)
		m_pHexView->SetWindowPos(this, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);

	CWnd::OnSize(nType, cx, cy);
}

BOOL CHexCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CHexCtrl::SetData(const PBYTE pData, DWORD_PTR dwCount) const
{
	if (GetActiveView())
		GetActiveView()->SetData(pData, dwCount);
}

void CHexCtrl::ClearData()
{
	if (GetActiveView())
		GetActiveView()->ClearData();
}

void CHexCtrl::SetFont(const LOGFONT* pLogFontNew) const
{
	if (GetActiveView())
		return GetActiveView()->SetFont(pLogFontNew);
}

void CHexCtrl::SetFontSize(UINT nSize) const
{
	if (GetActiveView())
		return GetActiveView()->SetFontSize(nSize);
}

void CHexCtrl::SetColor(COLORREF clrText, COLORREF clrTextOffset,
	COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected) const
{
	if (GetActiveView())
		GetActiveView()->SetColor(clrText, clrTextOffset, clrTextSelected,
			clrBk, clrBkSelected);
}



/************************************************************************
* CHexView implementation.												*
************************************************************************/

/********************************************************************
* Below is the custom implementation of MFC IMPLEMENT_DYNCREATE()	*
* macro, which doesn't work with nested classes by default.			*
********************************************************************/
CObject* PASCAL CHexCtrl::CHexView::CreateObject()
{
	return new CHexCtrl::CHexView;
}

CRuntimeClass* PASCAL CHexCtrl::CHexView::_GetBaseClass()
{
	return RUNTIME_CLASS(CScrollView);
}

AFX_COMDAT const CRuntimeClass CHexCtrl::CHexView::classCHexView {
	"CHexView", sizeof(class CHexCtrl::CHexView), 0xFFFF, CHexCtrl::CHexView::CreateObject,
	&CHexCtrl::CHexView::_GetBaseClass, NULL, NULL };

CRuntimeClass* PASCAL CHexCtrl::CHexView::GetThisClass()
{
	return (CRuntimeClass*)(&CHexCtrl::CHexView::classCHexView);
}

CRuntimeClass* CHexCtrl::CHexView::GetRuntimeClass() const
{
	return (CRuntimeClass*)(&CHexCtrl::CHexView::classCHexView);
}
/********************************************************************
* End of custom IMPLEMENT_DYNCREATE()								*
********************************************************************/

BEGIN_MESSAGE_MAP(CHexCtrl::CHexView, CScrollView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND_RANGE(IDC_MENU_POPUP_COPY_AS_HEX, IDC_MENU_POPUP_SEARCH, &CHexCtrl::CHexView::OnMenuRange)
	ON_MESSAGE(WM_HEXCTRL_SEARCH, &CHexCtrl::CHexView::OnSearchRequest)
END_MESSAGE_MAP()

BOOL CHexCtrl::CHexView::Create(CWnd * pParent, const RECT & rect, UINT nID, CCreateContext* pContext, const LOGFONT* pLogFont)
{
	BOOL ret = CScrollView::Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, pParent, nID, pContext);

	NONCLIENTMETRICSW ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

	LOGFONT lf { };
	StringCchCopyW(lf.lfFaceName, 9, L"Consolas");
	lf.lfHeight = 18;

	//In case of inability to create font from LOGFONT*
	//creating default windows font.
	if (pLogFont)
	{
		if (!m_fontHexView.CreateFontIndirectW(pLogFont))
			if (!m_fontHexView.CreateFontIndirectW(&lf))
				m_fontHexView.CreateFontIndirectW(&ncm.lfMessageFont);
	}
	else
		if (!m_fontHexView.CreateFontIndirectW(&lf))
			m_fontHexView.CreateFontIndirectW(&ncm.lfMessageFont);

	lf.lfHeight = 16;
	if (!m_fontRectBottom.CreateFontIndirectW(&lf))
		m_fontRectBottom.CreateFontIndirectW(&ncm.lfMessageFont);

	m_menuPopup.CreatePopupMenu();
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_HEX, L"Copy as Hex...		Ctrl+C");
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED, L"Copy as Formatted Hex...");
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_COPY_AS_ASCII, L"Copy as Ascii...");
	m_menuPopup.AppendMenuW(MF_STRING, IDC_MENU_POPUP_SEARCH, L"Search...		Ctrl+F");

	m_dlgSearch.Create(IDD_DIALOG_SEARCH, this);

	Recalc();

	return ret;
}

void CHexCtrl::CHexView::SetData(const unsigned char* pData, DWORD_PTR dwCount)
{
	ClearData();

	m_pRawData = pData;
	m_dwRawDataCount = dwCount;

	UpdateBottomBarText();

	Recalc();
}

void CHexCtrl::CHexView::ClearData()
{
	m_dwRawDataCount = 0;
	m_pRawData = nullptr;
	m_fSelected = false;
	m_stScrollVert.nPos = 0;
	m_strBytesDisplayed.clear();
	SetScrollInfo(SB_VERT, &m_stScrollVert);
	SetScrollInfo(SB_HORZ, &m_stScrollVert);
}

void CHexCtrl::CHexView::SetFont(const LOGFONT* pLogFontNew)
{
	if (!pLogFontNew)
		return;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(pLogFontNew);

	Recalc();
}

void CHexCtrl::CHexView::SetFontSize(UINT nSize)
{
	//Prevent font size from being too small or too big.
	if (nSize < 9 || nSize > 75)
		return;

	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);
	lf.lfHeight = nSize;

	m_fontHexView.DeleteObject();
	m_fontHexView.CreateFontIndirectW(&lf);

	Recalc();
}

void CHexCtrl::CHexView::SetColor(COLORREF clrText, COLORREF clrTextOffset,
	COLORREF clrTextSelected, COLORREF clrBk, COLORREF clrBkSelected)
{
	m_clrTextHexAndAscii = clrText;
	m_clrTextOffset = clrTextOffset;
	m_clrTextSelected = clrTextSelected;
	m_clrBk = clrBk;
	m_clrBkSelected;

	Invalidate();
	UpdateWindow();
}

void CHexCtrl::CHexView::SetCapacity(DWORD dwCapacity)
{
	if (dwCapacity < 1 || dwCapacity > 64)
		return;

	m_dwGridCapacity = dwCapacity;
	Recalc();
}

UINT CHexCtrl::CHexView::GetFontSize()
{
	LOGFONT lf;
	m_fontHexView.GetLogFont(&lf);

	return lf.lfHeight;
}

void CHexCtrl::CHexView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

void CHexCtrl::CHexView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//If scrollbar doesn't exist on the screen - do nothing.
	GetScrollBarInfo(OBJID_VSCROLL, &m_stSBI);
	if (m_stSBI.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return;

	GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);

	int pos = m_stScrollVert.nPos;
	switch (nSBCode)
	{
	case SB_TOP: pos = m_stScrollVert.nMin; break;
	case SB_BOTTOM: pos = m_stScrollVert.nMax; break;
	case SB_LINEUP: pos -= m_sizeLetter.cy; break;
	case SB_LINEDOWN: pos += m_sizeLetter.cy;  break;
	case SB_PAGEUP: pos -= m_sizeLetter.cy * 16; break;
	case SB_PAGEDOWN: pos += m_sizeLetter.cy * 16; break;
	case SB_THUMBPOSITION: pos = m_stScrollVert.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollVert.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (pos < m_stScrollVert.nMin)
		pos = m_stScrollVert.nMin;
	int max = m_stScrollVert.nMax - m_stScrollVert.nPage + 1;
	if (pos > max)
		pos = max;

	m_stScrollVert.nPos = pos;
	SetScrollInfo(SB_VERT, &m_stScrollVert);

	Invalidate();
}

void CHexCtrl::CHexView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//If scrollbar doesn't exist on the screen - do nothing.
	GetScrollBarInfo(OBJID_HSCROLL, &m_stSBI);
	if (m_stSBI.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return;

	GetScrollInfo(SB_HORZ, &m_stScrollVert, SIF_ALL);

	int pos = m_stScrollVert.nPos;
	switch (nSBCode)
	{
	case SB_LEFT: pos = m_stScrollVert.nMin; break;
	case SB_RIGHT: pos = m_stScrollVert.nMax; break;
	case SB_LINELEFT: pos -= m_sizeLetter.cx; break;
	case SB_LINERIGHT: pos += m_sizeLetter.cx;  break;
	case SB_PAGELEFT: pos -= m_stScrollVert.nPage; break;
	case SB_PAGERIGHT: pos += m_stScrollVert.nPage; break;
	case SB_THUMBPOSITION: pos = m_stScrollVert.nTrackPos; break;
	case SB_THUMBTRACK: pos = m_stScrollVert.nTrackPos; break;
	}

	//Make sure the new position is within range.
	if (pos < m_stScrollVert.nMin)
		pos = m_stScrollVert.nMin;
	int max = m_stScrollVert.nMax - m_stScrollVert.nPage + 1;
	if (pos > max)
		pos = max;

	m_stScrollVert.nPos = pos;
	SetScrollInfo(SB_HORZ, &m_stScrollVert);

	Invalidate();
}

void CHexCtrl::CHexView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fLMousePressed)
	{
		//If LMouse is pressed but cursor is outside client area.
		//SetCapture() behaviour.
		if (point.x < m_rcClient.left)
		{
			GetScrollInfo(SB_HORZ, &m_stScrollHorz, SIF_ALL);
			m_stScrollHorz.nPos -= m_sizeLetter.cx;
			SetScrollInfo(SB_HORZ, &m_stScrollHorz);
			point.x = m_iIndentFirstHexChunk;
		}
		else if (point.x >= m_rcClient.right)
		{
			GetScrollInfo(SB_HORZ, &m_stScrollHorz, SIF_ALL);
			m_stScrollHorz.nPos += m_sizeLetter.cx;
			SetScrollInfo(SB_HORZ, &m_stScrollHorz);
			point.x = m_iFourthVertLine - 1;
		}
		if (point.y < m_iHeightRectHeader)
		{
			GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
			m_stScrollVert.nPos -= m_sizeLetter.cy;
			SetScrollInfo(SB_VERT, &m_stScrollVert);
			point.y = m_iHeightRectHeader;
		}
		else if (point.y >= m_iHeightWorkArea)
		{
			GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
			m_stScrollVert.nPos += m_sizeLetter.cy;
			SetScrollInfo(SB_VERT, &m_stScrollVert);
			point.y = m_iHeightWorkArea - 1;
		}

		const int iHit = HitTest(&point);
		if (iHit != -1)
		{
			if (iHit <= (int)m_dwSelectionClick)
			{
				m_dwSelectionStart = iHit;
				m_dwSelectionEnd = m_dwSelectionClick;
			}
			else
			{
				m_dwSelectionStart = m_dwSelectionClick;
				m_dwSelectionEnd = iHit;
			}

			m_dwBytesSelected = m_dwSelectionEnd - m_dwSelectionStart + 1;

			UpdateBottomBarText();
		}

		Invalidate();
	}
}

BOOL CHexCtrl::CHexView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (nFlags == MK_CONTROL)
	{
		SetFontSize(GetFontSize() + zDelta / WHEEL_DELTA * 2);
		return TRUE;
	}
	else if (nFlags & (MK_CONTROL | MK_SHIFT))
	{
		SetCapacity(m_dwGridCapacity + zDelta / WHEEL_DELTA);
		return TRUE;
	}
	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexCtrl::CHexView::OnLButtonDown(UINT nFlags, CPoint point)
{
	const int iHit = HitTest(&point);
	if (iHit != -1)
	{
		SetCapture();
		if (m_fSelected && (nFlags & MK_SHIFT))
		{
			if (iHit <= (int)m_dwSelectionClick)
			{
				m_dwSelectionStart = iHit;
				m_dwSelectionEnd = m_dwSelectionClick;
			}
			else
			{
				m_dwSelectionStart = m_dwSelectionClick;
				m_dwSelectionEnd = iHit;
			}

			m_dwBytesSelected = m_dwSelectionEnd - m_dwSelectionStart + 1;
		}
		else
			m_dwSelectionClick = m_dwSelectionStart = m_dwSelectionEnd = iHit;

		m_fLMousePressed = true;
		m_fSelected = true;
		UpdateBottomBarText();
		Invalidate();
	}
}

void CHexCtrl::CHexView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_fLMousePressed = false;
	ReleaseCapture();

	CScrollView::OnLButtonUp(nFlags, point);
}

void CHexCtrl::CHexView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	m_menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this);

	CScrollView::OnRButtonUp(nFlags, point);
}

void CHexCtrl::CHexView::OnMButtonDown(UINT nFlags, CPoint point)
{
}

void CHexCtrl::CHexView::OnSize(UINT nType, int cx, int cy)
{
	Recalc();

	CScrollView::OnSize(nType, cx, cy);
}

void CHexCtrl::CHexView::OnMenuRange(UINT nID)
{
	switch (nID)
	{
	case IDC_MENU_POPUP_COPY_AS_HEX:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX);
		break;
	case IDC_MENU_POPUP_COPY_AS_HEX_FORMATTED:
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX_FORMATTED);
		break;
	case IDC_MENU_POPUP_COPY_AS_ASCII:
		CopyToClipboard(CLIPBOARD_COPY_AS_ASCII);
		break;
	case IDC_MENU_POPUP_SEARCH:
		m_dlgSearch.ShowWindow(SW_SHOW);
		break;
	}
}

void CHexCtrl::CHexView::OnDraw(CDC* pDC)
{
	GetScrollInfo(SB_HORZ, &m_stScrollHorz, SIF_ALL);
	int nScrollHorz = m_stScrollHorz.nPos;
	GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
	int nScrollVert = m_stScrollVert.nPos;

	CRect rcVisible = m_rcClient;
	rcVisible.top += nScrollVert;
	rcVisible.bottom += nScrollVert;
	rcVisible.left += nScrollHorz;
	rcVisible.right += nScrollHorz;

	//Drawing through CMemDC, to avoid any flickering.
	CMemDC memDC(*pDC, rcVisible);
	CDC& rDC = memDC.GetDC();

	CRect clip;
	rDC.GetClipBox(&clip);
	rDC.FillSolidRect(clip, m_clrBk);
	rDC.SelectObject(&m_penLines);
	rDC.SelectObject(&m_fontHexView);

	//Find the nLineStart and nLineEnd position, draw the visible portion.
	const UINT nLineStart = nScrollVert / m_sizeLetter.cy;
	UINT nLineEnd = m_dwRawDataCount ?
		(nLineStart + (m_rcClient.Height() - m_iHeightRectHeader - m_iHeightBottomRect) / m_sizeLetter.cy) : 0;
	//If m_dwRawDataCount is really small
	//we adjust nLineEnd to not to be bigger than maximum allowed.
	if (nLineEnd > (m_dwRawDataCount / m_dwGridCapacity))
		nLineEnd = m_dwRawDataCount % m_dwGridCapacity ? m_dwRawDataCount / m_dwGridCapacity + 1 : m_dwRawDataCount / m_dwGridCapacity;

	//Horizontal lines coords depending on scroll position.
	m_iFirstHorizLine = nScrollVert;
	m_iSecondHorizLine = m_iHeightRectHeader - 1 + nScrollVert;
	m_iThirdHorizLine = m_rcClient.Height() + nScrollVert - m_iHeightBottomRect;
	m_iFourthHorizLine = m_rcClient.Height() + nScrollVert - m_iBottomLineIndent;

	//First horizontal line.
	rDC.MoveTo(0, m_iFirstHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iFirstHorizLine);

	//Second horizontal line.
	rDC.MoveTo(0, m_iSecondHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iSecondHorizLine);

	//Third horizontal line.
	rDC.MoveTo(0, m_iThirdHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iThirdHorizLine);

	//Fourth horizontal line.
	rDC.MoveTo(0, m_iFourthHorizLine);
	rDC.LineTo(m_iFourthVertLine, m_iFourthHorizLine);

	//"Offset" text.
	RECT rect;
	rect.left = m_iFirstVertLine; rect.top = m_iFirstHorizLine;
	rect.right = m_iSecondVertLine; rect.bottom = m_iSecondHorizLine;
	rDC.SetTextColor(m_clrTextOffset);
	DrawTextW(rDC.m_hDC, L"Offset", 6, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//"Bytes displayed:" text.
	rect.left = m_iFirstVertLine; rect.top = m_iThirdHorizLine + 1;
	rect.right = m_rcClient.right > m_iFourthVertLine ? m_rcClient.right : m_iFourthVertLine;
	rect.bottom = m_iFourthHorizLine;
	rDC.FillSolidRect(&rect, m_clrBkRectBottom);
	rDC.SetTextColor(m_clrTextBytesSelected);
	rDC.SelectObject(&m_fontRectBottom);
	rect.left = m_iFirstVertLine + 5;
	DrawTextW(rDC.m_hDC, m_strBytesDisplayed.data(), m_strBytesDisplayed.size(), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	rDC.SelectObject(&m_fontHexView);
	rDC.SetTextColor(m_clrTextOffset);
	rDC.SetBkColor(m_clrBk);

	for (unsigned i = 0; i < m_dwGridCapacity; i++)
	{
		WCHAR str[9];
		swprintf_s(&str[0], 3, L"%X", i);
		int x, c;

		if (i <= m_dwGridCapacity / 2 - 1) //Top offset numbers (0 1 2 3 4 5 6 7...)
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iIndentBetweenHexChunks*i);
		else //Top offset numbers, part after 7 (8 9 A B C D E F...)
			x = m_iIndentFirstHexChunk + m_sizeLetter.cx + (m_iIndentBetweenHexChunks*i) + m_iIndentBetweenBlocks;

		//If i>=16 we need two chars (10, 11..., 1F) to print as capacity.
		if (i < 16)
			c = 1;
		else
		{
			c = 2;
			x -= m_sizeLetter.cx;
		}

		ExtTextOutW(rDC.m_hDC, x, nScrollVert + (m_sizeLetter.cy / 6), NULL, nullptr, &str[0], c, nullptr);
	}

	//"Ascii" text.
	rect.left = m_iThirdVertLine; rect.top = m_iFirstHorizLine;
	rect.right = m_iFourthVertLine; rect.bottom = m_iSecondHorizLine;
	DrawTextW(rDC.m_hDC, L"Ascii", 5, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	//First Vertical line.
	rDC.MoveTo(m_iFirstVertLine, nScrollVert);
	rDC.LineTo(m_iFirstVertLine, m_iFourthHorizLine);

	//Second Vertical line.
	rDC.MoveTo(m_iSecondVertLine, nScrollVert);
	rDC.LineTo(m_iSecondVertLine, m_iThirdHorizLine);

	//Third Vertical line.
	rDC.MoveTo(m_iThirdVertLine, nScrollVert);
	rDC.LineTo(m_iThirdVertLine, m_iThirdHorizLine);

	//Fourth Vertical line.
	rDC.MoveTo(m_iFourthVertLine, nScrollVert);
	rDC.LineTo(m_iFourthVertLine, m_iFourthHorizLine);

	int nLine { };

	for (unsigned iterLines = nLineStart; iterLines < nLineEnd; iterLines++)
	{
		swprintf_s(m_strOffset, 9, L"%08X", iterLines * m_dwGridCapacity);
		rDC.SetTextColor(m_clrTextOffset);

		//Drawing m_strOffset with bk color depending on selection range.
		if (m_fSelected && (iterLines * m_dwGridCapacity + m_dwGridCapacity) > min(m_dwSelectionStart, m_dwSelectionEnd) &&
			(iterLines * m_dwGridCapacity) <= m_dwSelectionEnd)
			rDC.SetBkColor(m_clrBkSelected);
		else
			rDC.SetBkColor(m_clrBk);

		//Left column offset print (00000001...0000FFFF...).
		ExtTextOutW(rDC.m_hDC, m_sizeLetter.cx, m_iHeightRectHeader + (m_sizeLetter.cy * nLine + nScrollVert),
			NULL, nullptr, m_strOffset, 8, nullptr);
		rDC.SetTextColor(m_clrTextHexAndAscii);

		int nIndentHexX { };
		int nIndentAsciiX { };
		int nIndentBetweenBlocks { };

		//Main loop for printing Hex chunks and Ascii chars (right column).
		for (int iterChunks = 0; iterChunks < (int)m_dwGridCapacity; iterChunks++)
		{
			if (iterChunks > (int)m_dwGridCapacity / 2 - 1)
				nIndentBetweenBlocks = m_iIndentBetweenBlocks;

			const UINT nFirstHexPosToPrintX = m_iIndentFirstHexChunk + nIndentHexX + nIndentBetweenBlocks;
			const UINT nFirstHexPosToPrintY = m_iHeightRectHeader + m_sizeLetter.cy * nLine + nScrollVert;
			const UINT nAsciiPosToPrintX = m_iIndentAscii + nIndentAsciiX;
			const UINT nAsciiPosToPrintY = m_iHeightRectHeader + m_sizeLetter.cy * nLine + nScrollVert;

			//Index of the next char (in m_pRawData) to draw.
			const size_t nIndexDataToPrint = iterLines * m_dwGridCapacity + iterChunks;

			//Rect of the space between HEX chunks, for proper selection drawing.
			m_rcSpaceBetweenHex.left = nFirstHexPosToPrintX + m_sizeLetter.cx * 2;
			m_rcSpaceBetweenHex.top = nFirstHexPosToPrintY;
			if (iterChunks == m_dwGridCapacity / 2 - 1)
				m_rcSpaceBetweenHex.right = nFirstHexPosToPrintX + m_sizeLetter.cx * 5;
			else
				m_rcSpaceBetweenHex.right = nFirstHexPosToPrintX + m_sizeLetter.cx * 3;

			m_rcSpaceBetweenHex.bottom = nFirstHexPosToPrintY + m_sizeLetter.cy;

			if (nIndexDataToPrint < m_dwRawDataCount) //Draw until reaching the end of m_dwRawDataCount.
			{
				//HEX chunk to print.
				wchar_t strHexToPrint[2];

				strHexToPrint[0] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0xF0) >> 4];
				strHexToPrint[1] = m_strHexMap[((const unsigned char)m_pRawData[nIndexDataToPrint] & 0x0F)];

				//Selection draw with different BK color.
				if (m_fSelected && nIndexDataToPrint >= m_dwSelectionStart && nIndexDataToPrint <= m_dwSelectionEnd)
				{
					rDC.SetBkColor(m_clrBkSelected);

					//To prevent change bk color after last selected HEX in a row, and very last HEX.
					if (nIndexDataToPrint != m_dwSelectionEnd && (nIndexDataToPrint + 1) % m_dwGridCapacity)
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_stBrushBkSelected.m_hObject);
					else
						FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_stBrushBk.m_hObject);
				}
				else
				{
					rDC.SetBkColor(m_clrBk);
					FillRect(rDC.m_hDC, &m_rcSpaceBetweenHex, (HBRUSH)m_stBrushBk.m_hObject);
				}

				//HEX chunk print.
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, &strHexToPrint[0], 2, nullptr);

				//Ascii to print.
				char chAsciiToPrint = m_pRawData[nIndexDataToPrint];
				//For non printable ASCII, just print a dot.
				if (chAsciiToPrint < 32 || chAsciiToPrint == 127)
					chAsciiToPrint = '.';

				//Ascii print..
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, &chAsciiToPrint, 1, nullptr);
			}
			else
			{	//Fill remaining chunks with blank spaces.
				rDC.SetBkColor(m_clrBk);
				ExtTextOutW(rDC.m_hDC, nFirstHexPosToPrintX, nFirstHexPosToPrintY, 0, nullptr, L" ", 2, nullptr);
				ExtTextOutA(rDC.m_hDC, nAsciiPosToPrintX, nAsciiPosToPrintY, 0, nullptr, "", 1, nullptr);
			}
			//Increasing indents for next print, for both - Hex and ASCII
			nIndentHexX += m_iIndentBetweenHexChunks;
			nIndentAsciiX += m_iIndentBetweenAscii;
		}
		nLine++;
	}
}

BOOL CHexCtrl::CHexView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

BOOL CHexCtrl::CHexView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'C' && GetKeyState(VK_CONTROL) < 0)
		CopyToClipboard(CLIPBOARD_COPY_AS_HEX);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'F' && GetKeyState(VK_CONTROL) < 0)
		OnMenuRange(IDC_MENU_POPUP_SEARCH);

	return CScrollView::PreTranslateMessage(pMsg);
}

int CHexCtrl::CHexView::HitTest(LPPOINT pPoint)
{
	DWORD dwHexChunk;
	GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_POS);
	SCROLLINFO stScrollX;
	GetScrollInfo(SB_HORZ, &stScrollX, SIF_POS);
	//To compensate horizontal scroll.
	pPoint->x += stScrollX.nPos;

	//Checking if cursor is within HEX chunks area.
	if ((pPoint->x >= m_iIndentFirstHexChunk) && (pPoint->x < m_iThirdVertLine)
		&& (pPoint->y >= m_iHeightRectHeader) && pPoint->y <= m_iHeightWorkArea)
	{
		int tmpBetweenBlocks;
		if (pPoint->x > m_iIndentFirstHexChunk + (m_iIndentBetweenHexChunks * (int)m_dwGridCapacity / 2))
			tmpBetweenBlocks = m_iIndentBetweenBlocks;
		else
			tmpBetweenBlocks = 0;

		//Calculate iHit HEX chunk, taking into account scroll position and letter sizes.
		dwHexChunk = ((pPoint->x - m_iIndentFirstHexChunk - tmpBetweenBlocks) / (m_sizeLetter.cx * 3)) +
			((pPoint->y - m_iHeightRectHeader) / m_sizeLetter.cy) * m_dwGridCapacity +
			((m_stScrollVert.nPos / m_sizeLetter.cy) * m_dwGridCapacity);
	}
	else if ((pPoint->x >= m_iIndentAscii) && (pPoint->x < (m_iIndentAscii + m_iIndentBetweenAscii * (int)m_dwGridCapacity))
		&& (pPoint->y >= m_iHeightRectHeader) && pPoint->y <= m_iHeightWorkArea)
	{
		//Calculate iHit Ascii symbol.
		dwHexChunk = ((pPoint->x - m_iIndentAscii) / (m_iIndentBetweenAscii)) +
			((pPoint->y - m_iHeightRectHeader) / m_sizeLetter.cy) * m_dwGridCapacity +
			((m_stScrollVert.nPos / m_sizeLetter.cy) * m_dwGridCapacity);
	}
	else
		dwHexChunk = -1;

	//If cursor is out of end-bound of HEX chunks or Ascii chars.
	if (dwHexChunk >= m_dwRawDataCount)
		dwHexChunk = -1;

	return dwHexChunk;
}

int CHexCtrl::CHexView::CopyToClipboard(UINT nType)
{
	if (m_fSelected)
	{
		const char* const strHexMap = "0123456789ABCDEF";
		char chHexToCopy[2];
		std::string strToClipboard { };

		switch (nType)
		{
		case CLIPBOARD_COPY_AS_HEX:
		{
			for (unsigned i = 0; i < m_dwBytesSelected; i++)
			{
				chHexToCopy[0] = strHexMap[((const unsigned char)m_pRawData[m_dwSelectionStart + i] & 0xF0) >> 4];
				chHexToCopy[1] = strHexMap[((const unsigned char)m_pRawData[m_dwSelectionStart + i] & 0x0F)];
				strToClipboard += chHexToCopy[0];
				strToClipboard += chHexToCopy[1];
			}
			break;
		}
		case CLIPBOARD_COPY_AS_HEX_FORMATTED:
		{
			//How many spaces are needed to be inserted at the beginnig.
			DWORD dwModStart = m_dwSelectionStart % m_dwGridCapacity;
			//When to insert first "\r\n".
			DWORD dwTail = m_dwGridCapacity - dwModStart;
			DWORD dwNextBlock = m_dwGridCapacity % 2 ? m_dwGridCapacity / 2 + 2 : m_dwGridCapacity / 2 + 1;

			//If at least two rows are selected.
			if (dwModStart + m_dwBytesSelected > m_dwGridCapacity)
			{
				strToClipboard.insert(0, dwModStart * 3, ' ');
				if (dwTail < m_dwGridCapacity / 2 + 1)
					strToClipboard.insert(0, 2, ' ');
			}

			for (unsigned i = 0; i < m_dwBytesSelected; i++)
			{
				chHexToCopy[0] = strHexMap[((const unsigned char)m_pRawData[m_dwSelectionStart + i] & 0xF0) >> 4];
				chHexToCopy[1] = strHexMap[((const unsigned char)m_pRawData[m_dwSelectionStart + i] & 0x0F)];
				strToClipboard += chHexToCopy[0];
				strToClipboard += chHexToCopy[1];

				if (i < (m_dwBytesSelected - 1) && (dwTail - 1) != 0)
					if (dwTail == dwNextBlock) //Space between blocks.
						strToClipboard += "   ";
					else
						strToClipboard += " ";
				if (--dwTail == 0 && i < (m_dwBytesSelected - 1)) //Next string.
				{
					strToClipboard += "\r\n";
					dwTail = m_dwGridCapacity;
				}
			}
			break;
		}
		case CLIPBOARD_COPY_AS_ASCII:
		{
			char ch;
			for (unsigned i = 0; i < m_dwBytesSelected; i++)
			{
				ch = m_pRawData[m_dwSelectionStart + i];
				//If next byte is zero �> substitute it with space.
				if (ch == 0)
					ch = ' ';
				strToClipboard += ch;
			}
			break;
		}
		}

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strToClipboard.length() + 1);
		if (!hMem)
			return -1;
		LPVOID hMemLock = GlobalLock(hMem);
		if (!hMemLock)
			return -1;

		memcpy(hMemLock, strToClipboard.data(), strToClipboard.length() + 1);
		GlobalUnlock(hMem);
		OpenClipboard();
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	return 1;
}

void CHexCtrl::CHexView::UpdateBottomBarText()
{
	WCHAR buff[128];
	if (m_dwBytesSelected)
		swprintf_s(buff, 128, L"Bytes total: 0x%X (%u), Selected: 0x%X (%u), at offset: 0x%X (%u)",
			m_dwRawDataCount, m_dwRawDataCount, m_dwBytesSelected, m_dwBytesSelected, m_dwSelectionStart, m_dwSelectionStart);
	else
		swprintf_s(buff, 128, L"Bytes total: 0x%X (%u)", m_dwRawDataCount, m_dwRawDataCount);

	m_strBytesDisplayed = buff;
}

void CHexCtrl::CHexView::Recalc()
{
	GetClientRect(&m_rcClient);

	UINT nStartLine { };
	if (m_fSecondLaunch)
	{
		GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
		nStartLine = m_stScrollVert.nPos / m_sizeLetter.cy;
	}

	CDC* pDC = GetDC();
	pDC->SelectObject(&m_fontHexView);
	GetTextExtentPoint32W(pDC->m_hDC, L"0", 1, &m_sizeLetter);
	ReleaseDC(pDC);

	m_iFirstVertLine = 0;
	m_iSecondVertLine = m_sizeLetter.cx * 10;
	m_iIndentBetweenHexChunks = m_sizeLetter.cx * 3;
	m_iIndentBetweenBlocks = m_sizeLetter.cx * 2;
	m_iThirdVertLine = m_iSecondVertLine + (m_iIndentBetweenHexChunks * m_dwGridCapacity) + m_iIndentBetweenBlocks + m_sizeLetter.cx;
	m_iIndentAscii = m_iThirdVertLine + m_sizeLetter.cx;
	m_iIndentBetweenAscii = m_sizeLetter.cx + 1;
	m_iFourthVertLine = m_iIndentAscii + (m_iIndentBetweenAscii * m_dwGridCapacity) + m_sizeLetter.cx;
	m_iIndentFirstHexChunk = m_iSecondVertLine + m_sizeLetter.cx;
	m_iHeightRectHeader = int(m_sizeLetter.cy*1.5);
	m_iHeightWorkArea = m_rcClient.Height() - m_iHeightBottomRect - ((m_rcClient.Height() - m_iHeightRectHeader - m_iHeightBottomRect) % m_sizeLetter.cy);

	//Scroll sizes according to current font size.
	SetScrollSizes(MM_TEXT, CSize(m_iFourthVertLine + 1,
		m_iHeightRectHeader + m_iHeightBottomRect + (m_sizeLetter.cy * (m_dwRawDataCount / m_dwGridCapacity + 3))));

	//This flag shows that Recalc() was invoked at least once before,
	//and ScrollSizes have already been set, so we can adjust them.
	if (m_fSecondLaunch)
	{
		GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
		m_stScrollVert.nPos = m_sizeLetter.cy * nStartLine;

		int max = m_stScrollVert.nMax - m_stScrollVert.nPage + 1;
		if (m_stScrollVert.nPos > max)
			m_stScrollVert.nPos = max;

		SetScrollInfo(SB_VERT, &m_stScrollVert);
	}
	else
		m_fSecondLaunch = true;

	Invalidate();
}

void CHexCtrl::CHexView::Search(search_tup& rTup)
{
	auto& wstrSearch = std::get<0>(rTup);
	auto& dwType = std::get<1>(rTup);
	auto& dwStartAt = std::get<2>(rTup);
	auto& iDirection = std::get<3>(rTup);

	if (wstrSearch.empty() || m_dwRawDataCount == 0 || dwStartAt > (m_dwRawDataCount - 1))
		return;

	int iSizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstrSearch[0], (int)wstrSearch.size(), nullptr, 0, nullptr, nullptr);
	std::string strSearch(iSizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstrSearch[0], (int)wstrSearch.size(), &strSearch[0], iSizeNeeded, nullptr, nullptr);

	DWORD dwIterations;

	switch (dwType)
	{
	case ID_SEARCH_HEX:
	{
		std::string strSearchHex;
		dwIterations = strSearch.size() % 2 ? strSearch.size() / 2 + 1 : strSearch.size() / 2;
		int iNextChar = 0;
		std::string strToUL;

		for (DWORD i = 0; i < dwIterations; i++)
		{
			if (strSearch.size() >= i + 2)
				strToUL = strSearch.substr(iNextChar, 2);
			else
				strToUL = strSearch.substr(iNextChar, 1);

			unsigned long ulNumber = strtoul(strToUL.data(), nullptr, 16);
			strSearchHex += (unsigned char)ulNumber;
			iNextChar += 2;
		}

		dwIterations = m_dwRawDataCount - strSearchHex.size() + 1;
		for (DWORD i = 0; i < dwIterations; i++)
		{
			if (memcmp(m_pRawData + i, strSearchHex.data(), strSearchHex.size()) == 0)
			{
				SetSelectionTo(i, i + strSearchHex.size() - 1);
				break;
			}
		}
		break;
	}
	case ID_SEARCH_ASCII:
	{
		if (strSearch.size() > m_dwRawDataCount)
			MessageBoxW(L"String was not found", L"Not found", MB_ICONEXCLAMATION);

		dwIterations = m_dwRawDataCount - strSearch.size() + 1;
		for (DWORD i = 0; i < dwIterations; i++)
			if (memcmp(m_pRawData + i, strSearch.data(), strSearch.size()) == 0)
			{
				SetSelectionTo(i, i + strSearch.size() - 1);
				break;
			}
		break;
	}
	case ID_SEARCH_UNICODE:
	{
		break;
	}
	}
}

void CHexCtrl::CHexView::SetSelectionTo(DWORD dwStart, DWORD dwEnd)
{
	m_dwSelectionClick = m_dwSelectionStart = dwStart;
	m_dwSelectionEnd = dwEnd;
	m_dwBytesSelected = m_dwSelectionEnd - m_dwSelectionStart + 1;

	GetScrollInfo(SB_VERT, &m_stScrollVert, SIF_ALL);
	m_stScrollVert.nPos = dwStart / m_dwGridCapacity * m_sizeLetter.cy - (m_iHeightWorkArea / 2);
	SetScrollInfo(SB_VERT, &m_stScrollVert);

	UpdateBottomBarText();

	Invalidate();
}

afx_msg LRESULT CHexCtrl::CHexView::OnSearchRequest(WPARAM wParam, LPARAM lParam)
{
	Search(m_dlgSearch.GetSearch());

	return 0;
}


/****************************************************
* CHexDlgSearch class implementation.				*
****************************************************/
BEGIN_MESSAGE_MAP(CHexCtrl::CHexDlgSearch, CDialogEx)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CHexCtrl::CHexDlgSearch::OnBnClickedSearch)
END_MESSAGE_MAP()

BOOL CHexCtrl::CHexDlgSearch::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_stComboBoxSearch.AddString(L"Hex");
	m_stComboBoxSearch.AddString(L"Ascii text");
	m_stComboBoxSearch.AddString(L"Unicode text");
	m_stComboBoxSearch.SetCurSel(0);

	return TRUE;
}

void CHexCtrl::CHexDlgSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_stEditSearch);
	DDX_Control(pDX, IDC_COMBO_SEARCHTYPE, m_stComboBoxSearch);
}

BOOL CHexCtrl::CHexDlgSearch::Create(UINT nIDTemplate, CWnd * pParentWnd)
{
	pParent = pParentWnd;

	return CDialog::Create(nIDTemplate, pParent);
}

CWnd* CHexCtrl::CHexDlgSearch::GetParent()
{
	//For some weird reason original GetParent() function doesn't work as intended.
	//Could not figure out why. Just reimplemented it.
	return pParent ? pParent : CDialog::GetParent();
}

search_tup& CHexCtrl::CHexDlgSearch::GetSearch()
{
	return m_tupSearch;
}

void CHexCtrl::CHexDlgSearch::OnBnClickedSearch()
{
	CString str;
	m_stEditSearch.GetWindowTextW(str);
	std::get<0>(m_tupSearch) = str.GetString();
	std::get<1>(m_tupSearch) = m_stComboBoxSearch.GetCurSel();

	GetParent()->SendMessageW(WM_HEXCTRL_SEARCH);
	SetLayeredWindowAttributes(0, 150, LWA_ALPHA);
	GetParent()->SetFocus();
}

void CHexCtrl::CHexDlgSearch::OnKillFocus(CWnd* pNewWnd)
{
	CDialogEx::OnKillFocus(pNewWnd);
}

void CHexCtrl::CHexDlgSearch::OnSetFocus(CWnd* pOldWnd)
{
	CDialogEx::OnSetFocus(pOldWnd);
}

void CHexCtrl::CHexDlgSearch::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (nState == WA_INACTIVE)
		SetLayeredWindowAttributes(0, 150, LWA_ALPHA);
	else
		SetLayeredWindowAttributes(0, 255, LWA_ALPHA);

	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
}
