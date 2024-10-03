
// SokobanView.h : interface of the CSokobanView class
//

#pragma once

class CSokobanView : public CView
{
protected: // create from serialization only
	CSokobanView() noexcept;
	DECLARE_DYNCREATE(CSokobanView)

// Attributes
public:
	CSokobanDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CSokobanView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	CString level;
	CString destination;
	CString arrival;
	CString step;
};

#ifndef _DEBUG  // debug version in SokobanView.cpp
inline CSokobanDoc* CSokobanView::GetDocument() const
   { return reinterpret_cast<CSokobanDoc*>(m_pDocument); }
#endif

