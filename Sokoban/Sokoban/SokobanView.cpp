
// SokobanView.cpp : implementation of the CSokobanView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Sokoban.h"
#endif

#include "SokobanDoc.h"
#include "SokobanView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CSokobanView

IMPLEMENT_DYNCREATE(CSokobanView, CView)

BEGIN_MESSAGE_MAP(CSokobanView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CSokobanView construction/destruction

CSokobanView::CSokobanView() noexcept
{
	// TODO: add construction code here
	//m_p = &ep;
	//m_p = &Endpoint(TCP | CLIENT, "192.168.1.114", "5000");
}

CSokobanView::~CSokobanView()
{
}

BOOL CSokobanView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSokobanView drawing

void CSokobanView::OnDraw(CDC* pDC)
{
	CSokobanDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CClientDC aDC(this);
	CDC memDC;
	memDC.CreateCompatibleDC(&aDC);

	int  BlockSize = 30;
//*
	//CRect rect;
	//GetClientRect(&rect);
	//for (int i = 0; i < rect.Height() / BlockSize; i++) {
	//	for (int j = 0; j < rect.Width() / BlockSize; j++) {
	//		memDC.SelectObject(&pDoc->Blank);
	//		aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
	//	}
	//}

	pDoc->m_Arrival = 0;
	for (int i = 0; i < pDoc->m_MapSize.y; i++) {
	for (int j = 0; j < pDoc->m_MapSize.x; j++) {
		switch (pDoc->m_Map[i][j]) {
		case 'H': // Wall
			memDC.SelectObject(&pDoc->Wall);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			break;
		case '0': // Blank
			memDC.SelectObject(&pDoc->Blank);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			break;
		case ' ': // Blank
			memDC.SelectObject(&pDoc->Blank);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			break;
		case 'B': // Box
			memDC.SelectObject(&pDoc->Box);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			break;
		case 'D': // Destination
			memDC.SelectObject(&pDoc->Destination);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			break;
		case 'C': // Arrival
			memDC.SelectObject(&pDoc->Arrival);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			pDoc->m_Arrival++;
			break;
		case 'W': // Worker
			memDC.SelectObject(&pDoc->Worker);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			pDoc->m_Worker.x = j;
			pDoc->m_Worker.y = i;
			break;
		case 'E': // Worker on Destination
			memDC.SelectObject(&pDoc->Worker);
			aDC.BitBlt(j * BlockSize, i * BlockSize, BlockSize, BlockSize, &memDC, 0, 0, SRCCOPY);
			pDoc->m_Worker.x = j;
			pDoc->m_Worker.y = i;
			break;
		}
	}
	}

	level.Format(L"Level : %d", pDoc->m_Level);
	pDC->TextOut(pDoc->m_MapSize.x * BlockSize + 50, 20, level);

	destination.Format(L"Destination : %d", pDoc->m_Destination);
	pDC->TextOut(pDoc->m_MapSize.x * BlockSize + 50, 40, destination);

	arrival.Format(L"Arrival : %d", pDoc->m_Arrival);
	pDC->TextOut(pDoc->m_MapSize.x * BlockSize + 50, 60, arrival);

	step.Format(L"Step : %d", pDoc->m_Step);
	pDC->TextOut(pDoc->m_MapSize.x * BlockSize + 50, 80, step);
	
	if (pDoc->m_Arrival == pDoc->m_Destination) {
		MessageBox(L"Pass!");
		pDoc->m_p->Write("pass");
		pDoc->ChangeLevel(UP);
		Invalidate();
	}
//*/
}


// CSokobanView printing

BOOL CSokobanView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSokobanView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSokobanView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CSokobanView diagnostics

#ifdef _DEBUG
void CSokobanView::AssertValid() const
{
	CView::AssertValid();
}

void CSokobanView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSokobanDoc* CSokobanView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSokobanDoc)));
	return (CSokobanDoc*)m_pDocument;
}
#endif //_DEBUG


// CSokobanView message handlers


void CSokobanView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CSokobanDoc* pDoc = GetDocument();

	if (nChar == VK_UP || nChar == 'W' || nChar == 'K')         pDoc->MoveWorker(UP);
	else if (nChar == VK_DOWN || nChar == 'S' || nChar == 'J')  pDoc->MoveWorker(DOWN);
	else if (nChar == VK_LEFT || nChar == 'A' || nChar == 'H')  pDoc->MoveWorker(LEFT);
	else if (nChar == VK_RIGHT || nChar == 'D' || nChar == 'L') pDoc->MoveWorker(RIGHT);
	else if (nChar == VK_NEXT)  pDoc->ChangeLevel(UP);
	else if (nChar == VK_PRIOR) pDoc->ChangeLevel(DOWN);
	else if (nChar == 'B')      pDoc->MoveBack();

	//pDoc->UpdateAllViews(NULL);
	Invalidate();

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
