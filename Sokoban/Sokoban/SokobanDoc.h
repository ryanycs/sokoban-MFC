
// SokobanDoc.h : interface of the CSokobanDoc class
//
#include <fstream>
#include <string>
#include <stack>
#include "protocols.h"
#include "mysocket.h"
using namespace std;
#pragma once
#define MAX_ROW 25
#define MAX_COLUMN 80
enum Direction { LEFT, UP, DOWN, RIGHT };



class CSokobanDoc : public CDocument
{
protected: // create from serialization only
	CSokobanDoc() noexcept;
	DECLARE_DYNCREATE(CSokobanDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CSokobanDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	char m_Map[MAX_ROW][MAX_COLUMN];
	CBitmap Worker;
	CBitmap Blank;
	CBitmap Destination;
	CBitmap Box;
	CBitmap Arrival;
	CBitmap Wall;
	
	struct Point {
		char c;
		int x;
		int y;
	};
	Point m_Worker;
	Point m_Box;
	Point m_MapSize;

	struct Backup {
		Point a;
		Point b;
		Point c;
	};
	stack<Backup> m_backup;

	int m_Level;
	int m_PreLevel;
	int m_Destination;
	int m_Step;
	int m_Arrival;

	bool ReadMap(int);
	char NextToWorker(Direction);
	char NextNextToWorker(Direction);
	void MoveWorker(Direction);
	void MoveBox(Direction);
	void ChangeLevel(Direction);
	void MemStep(Direction);
	void MoveBack();
	bool EpReadMap(int);

	Endpoint* m_p;
};