
// SokobanDoc.cpp : implementation of the CSokobanDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Sokoban.h"
#endif

#include "SokobanDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
Endpoint ep(TCP | CLIENT, "192.168.1.114", "5000");
// CSokobanDoc

IMPLEMENT_DYNCREATE(CSokobanDoc, CDocument)

BEGIN_MESSAGE_MAP(CSokobanDoc, CDocument)
END_MESSAGE_MAP()


// CSokobanDoc construction/destruction

CSokobanDoc::CSokobanDoc() noexcept
{
	// TODO: add one-time construction code here

}

CSokobanDoc::~CSokobanDoc()
{
}

BOOL CSokobanDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	Worker.LoadBitmap(IDB_BITMAPWORKER);
	Blank.LoadBitmap(IDB_BITMAPBLANK);;
	Destination.LoadBitmap(IDB_BITMAPDESTINATION);;
	Box.LoadBitmap(IDB_BITMAPBOX);;
	Arrival.LoadBitmap(IDB_BITMAPARRIVAL);;
	Wall.LoadBitmap(IDB_BITMAPWALL);;

	for (int i = 0; i < MAX_ROW; i++) {
		for (int j = 0; j < MAX_COLUMN; j++) {
			m_Map[i][j] = ' ';
		}
	}
	m_Level = 1;
	m_Destination = 0;
	m_Step = 0;
	m_Arrival = 0;
	m_MapSize.x = m_MapSize.y = 0;

	m_p = &ep;

	// read level 1 map
	ReadMap(1);
	//EpReadMap(1);

	return TRUE;
}




// CSokobanDoc serialization

void CSokobanDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CSokobanDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CSokobanDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CSokobanDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CSokobanDoc diagnostics

#ifdef _DEBUG
void CSokobanDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSokobanDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSokobanDoc commands


bool CSokobanDoc::ReadMap(int n)
{
	
	// create the file name
	string s(to_string(n));
	while (s.length() < 3) s = "0" + s;
	string map_name = "map" + s + ".txt";

	// read the file
	ifstream fin(map_name);
	if (fin.fail()) {
		AfxMessageBox(L"File can not be opened!");
		m_Level = m_PreLevel;
		return 0;
	}

	// initialize data
	memset(m_Map, ' ', sizeof(m_Map));
	m_Step = m_Arrival = m_Destination = 0;
	m_MapSize.x = m_MapSize.y = 0;
	m_backup = stack<Backup>();

	// load the map
	int y = 0, x;
	string line;
	while (getline(fin, line)) {
		for (x = 0; x < line.length(); x++) {
			if (line[x] == 'D' || line[x] == 'C') m_Destination++;
			m_Map[y][x] = line[x];
		}
		m_MapSize.x = max(m_MapSize.x, x);
		y++;
	}
	m_MapSize.y = y;

	return 1; // sucess
}

char CSokobanDoc::NextToWorker(Direction d) {
	if      (d == LEFT)  return m_Map[m_Worker.y][m_Worker.x-1];
	else if (d == DOWN)  return m_Map[m_Worker.y+1][m_Worker.x];
	else if (d == UP)    return m_Map[m_Worker.y-1][m_Worker.x];
	else if (d == RIGHT) return m_Map[m_Worker.y][m_Worker.x+1];
}

char CSokobanDoc::NextNextToWorker(Direction d) {
	if      (d == LEFT)  return m_Map[m_Worker.y][m_Worker.x-2];
	else if (d == DOWN)  return m_Map[m_Worker.y+2][m_Worker.x];
	else if (d == UP)    return m_Map[m_Worker.y-2][m_Worker.x];
	else if (d == RIGHT) return m_Map[m_Worker.y][m_Worker.x+2];
}

void CSokobanDoc::MoveWorker(Direction d) {
	MemStep(d); // Memory the current map, that helps to move back if you want

	int i = 0, j = 0;
	if      (d == LEFT)  j = -1;
	else if (d == DOWN)  i =  1;
	else if (d == UP)    i = -1;
	else if (d == RIGHT) j =  1;

	int x = m_Worker.x, y = m_Worker.y;
	if (NextToWorker(d) == ' ') {
		if      (m_Map[y][x] == 'W') m_Map[y][x] = ' ';
		else if (m_Map[y][x] == 'E') m_Map[y][x] = 'D';
		m_Map[y+i][x+j] = 'W';
		m_Step++;
	}
	else if (NextToWorker(d) == 'D') {
		if      (m_Map[y][x] == 'W') m_Map[y][x] = ' ';
		else if (m_Map[y][x] == 'E') m_Map[y][x] = 'D';
		m_Map[y+i][x+j] = 'E';
		m_Step++;
	}
	else if ((NextToWorker(d) == 'B' || NextToWorker(d) == 'C') && (NextNextToWorker(d) == ' ' || NextNextToWorker(d) == 'D')) {
		m_Box.x = x + j;
		m_Box.y = y + i;
		MoveBox(d);
		MoveWorker(d);
		m_backup.pop(); // MoveWorker(d) call MemStep(d) again, so needs to pop the backup stack
	}
	else {
		m_backup.pop(); // No doing something, so pop the backup stack
	}
}

void CSokobanDoc::MoveBox(Direction d) {
	int i = 0, j = 0;
	if      (d == LEFT)  j = -1;
	else if (d == DOWN)  i =  1;
	else if (d == UP)    i = -1;
	else if (d == RIGHT) j =  1;

	int x = m_Box.x, y = m_Box.y;
	if (NextNextToWorker(d) == ' ') {
		if      (m_Map[y][x] == 'B') m_Map[y][x] = ' ';
		else if (m_Map[y][x] == 'C') m_Map[y][x] = 'D';
		m_Map[y+i][x+j] = 'B';
	}
	else if (NextNextToWorker(d) == 'D') {
		if      (m_Map[y][x] == 'B') m_Map[y][x] = ' ';
		else if (m_Map[y][x] == 'C') m_Map[y][x] = 'D';
		m_Map[y+i][x+j] = 'C';
	}
}

void CSokobanDoc::ChangeLevel(Direction d) {
	switch (d) {
	case UP:
		m_PreLevel = m_Level;
		ReadMap(++m_Level);
		//EpReadMap(++m_Level);
		break;
	case DOWN:
		m_PreLevel = m_Level;
		ReadMap(--m_Level);
		//EpReadMap(--m_Level);
		break;
	}
}

void CSokobanDoc::MemStep(Direction d) {
	int i = 0, j = 0;
	if      (d == LEFT)  j = -1;
	else if (d == DOWN)  i =  1;
	else if (d == UP)    i = -1;
	else if (d == RIGHT) j =  1;

	Point a({ m_Map[m_Worker.y][m_Worker.x], m_Worker.x, m_Worker.y });
	Point b({ NextToWorker(d), m_Worker.x + j, m_Worker.y + i });
	Point c({ NextNextToWorker(d), m_Worker.x + 2 * j, m_Worker.y + 2 * i });
	Backup buf({a, b, c});
	m_backup.push(buf);
}

void CSokobanDoc::MoveBack(){
	if (!(m_backup.empty())) {
		Backup buf = m_backup.top();
		m_Map[buf.a.y][buf.a.x] = buf.a.c;
		m_Map[buf.b.y][buf.b.x] = buf.b.c;
		m_Map[buf.c.y][buf.c.x] = buf.c.c;
		m_backup.pop();
		m_Step--;
	}
}

bool CSokobanDoc::EpReadMap(int n) {
	m_p->Write(to_string(n));

	// initialize data
	memset(m_Map, ' ', sizeof(m_Map));
	m_Step = m_Arrival = m_Destination = 0;
	m_MapSize.x = m_MapSize.y = 0;
	m_backup = stack<Backup>();

	int y = 0, x;
	while (true) {
		string line("");
		m_p->Read(-80, line);
		if (line == "end") break;
		else if (line == "error") {
			AfxMessageBox(L"File can not be opened!");
			m_Level = m_PreLevel;
			EpReadMap(m_Level);
			return 0;
		}

		for (x = 0; x < line.length(); x++) {
			if (line[x] == 'D' || line[x] == 'C') m_Destination++;
			m_Map[y][x] = line[x];
		}
		m_MapSize.x = max(m_MapSize.x, x);
		y++;
	}
	m_MapSize.y = y;

	return 1;
}