// FEManDoc.cpp : implementation of the CFEManDoc class
//

#include "stdafx.h"
#include "FEMan.h"
#include "assert.h"

#include "Screen.h"
#include "FEManDoc.h"
#include "MainFrm.h"

#include "hwexport.h"

#include "FlowTreeView.h"
#include "ScreenEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFEManDoc

IMPLEMENT_DYNCREATE(CFEManDoc, CDocument)

BEGIN_MESSAGE_MAP(CFEManDoc, CDocument)
    //{{AFX_MSG_MAP(CFEManDoc)
    ON_COMMAND(ID_FILE_NEW, OnFileNew)
    ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFEManDoc construction/destruction

CFEManDoc::CFEManDoc()
{
    // TODO: add one-time construction code here
    gridActive = gridVisible = 0;
    gridSpacing = 10;
}

CFEManDoc::~CFEManDoc()
{
}

BOOL CFEManDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFEManDoc serialization

void CFEManDoc::Serialize(CArchive& ar)
{
    CFile *pFile = ar.GetFile();

    assert(pFile);

    if (ar.IsStoring())
    {
        SaveEverything(&ar);
    }
    else
    {
        LoadEverything(&ar);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CFEManDoc diagnostics

#ifdef _DEBUG
void CFEManDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CFEManDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFEManDoc commands

Screen *CFEManDoc::CreateScreen(CDC *pDC, CPoint *pPoint)
{
    Screen *pScreen;
    
    pScreen = new Screen(pDC, pPoint->x, pPoint->y);

    screenList.AddNode(pScreen);

    return(pScreen);
}

Screen *CFEManDoc::CreateLoadedScreen(void)
{
    Screen *pScreen;
    
    pScreen = new Screen();

    screenList.AddNode(pScreen);

    return(pScreen);
}

void CFEManDoc::DeleteScreen(Screen *pScreen)
{
    screenList.PurgeNode(pScreen);
}

void CFEManDoc::DeleteSelectedScreens(void)
{
    Screen *pWorkScreen, *pKillScreen;

    pWorkScreen = GetFirstSelectedScreen();

    while(pWorkScreen)
    {
        pKillScreen = pWorkScreen;

        pWorkScreen = (Screen *)pWorkScreen->GetNext();

        if(pKillScreen->GetScreenStatus() == Screen::SS_SELECTED)
        {
            DeleteScreen(pKillScreen);
        }
    }
}

Screen *CFEManDoc::GetFirstSelectedScreen(void)
{
    Screen *pWorkScreen;
    
    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        if(pWorkScreen->GetScreenStatus() == Screen::SS_SELECTED)
        {
            return(pWorkScreen);
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    return(0);
}

void CFEManDoc::SelectScreenIcon(Screen *s)     
{
    if(!IsSelectedScreenIcon(s))
    {
        s->SetScreenStatus(Screen::SS_SELECTED);

        UpdateScreenEditView();
    }
}

void CFEManDoc::DeSelectScreenIcon(Screen *s)   
{
    if(IsSelectedScreenIcon(s))
    {
        s->SetScreenStatus(Screen::SS_NORMAL);

        UpdateScreenEditView();
        
        ScreenLink *pWorkLink;

        pWorkLink = s->GetFirstScreenLink();

        while(pWorkLink)
        {
            s->DeSelectScreenLink(pWorkLink);

            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }
    }
}

unsigned long CFEManDoc::GetNumSelectedScreenIcons(void)
{
    Screen *pWorkScreen;
    unsigned long numSelected = 0;

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        if(pWorkScreen->GetScreenStatus() == Screen::SS_SELECTED)
        {
            numSelected ++;
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    return(numSelected);
}

void CFEManDoc::ClearAllSelectedScreenIcons(void)
{
    Screen *pWorkScreen;

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        DeSelectScreenIcon(pWorkScreen);
        
        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }
}

Screen *CFEManDoc::PointOnScreenIcon(CPoint *pPoint)
{
    Screen *pWorkScreen;

    pWorkScreen = GetFirstScreen();

    CRect *pRect;

    while(pWorkScreen)
    {
        pRect = pWorkScreen->GetIconBounds();
        
        if(pRect->PtInRect(*pPoint))
        {
            return(pWorkScreen);
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    return(0);
}

ScreenLink *CFEManDoc::PointOnScreenLink(CPoint *pPoint)
{
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;

    pWorkScreen = GetFirstScreen();

    CRect *pRect;

    while(pWorkScreen)
    {
        pWorkLink = pWorkScreen->GetFirstScreenLink();

        while(pWorkLink)
        {
            pRect = pWorkLink->GetBounds();

            if(pRect->PtInRect(*pPoint))
            {
                return(pWorkLink);
            }
            
            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }
    
    return(0);
}

void CFEManDoc::SelectScreenIconsInRect(CRect *pRect)
{
    Screen *pWorkScreen;
    CRect *pWorkRect;
    
    CRect workRect = *pRect, dstRect;

    workRect.NormalizeRect();

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        pWorkRect = pWorkScreen->GetIconBounds();

        if(dstRect.IntersectRect(pWorkRect, workRect))
            SelectScreenIcon(pWorkScreen);
        else
            DeSelectScreenIcon(pWorkScreen);

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }
}

void CFEManDoc::RelocateAllScreens(void)
{
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;
    ScreenLinkContainer *pWorkContainer;
    CPoint workPoint, *pWorkPoint;
    ccList *pWorkList;

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        pWorkPoint = pWorkScreen->GetIconPos();

        pWorkScreen->SetScreenPos(pWorkPoint->x, pWorkPoint->y);

        // Now relocate each of this screens screenlinks.
        pWorkLink = pWorkScreen->GetFirstScreenLink();

        while(pWorkLink)
        {
            pWorkLink->SetLinkPos();

            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }

        // Now relocate each of this screen's inbound screen links.
        pWorkList = pWorkScreen->GetInboundLinkList();

        pWorkContainer = (ScreenLinkContainer *)pWorkList->GetHead();

        while(pWorkContainer)
        {
            pWorkLink = pWorkContainer->GetMyLink();

            pWorkLink->SetLinkDst(&pWorkScreen->GetIconBounds()->CenterPoint());

            pWorkContainer = (ScreenLinkContainer *)pWorkContainer->GetNext();
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }
}

void CFEManDoc::RelocateSelectedScreens(CPoint *pPressedAt, CPoint *pPoint)
{
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;
    ScreenLinkContainer *pWorkContainer;
    CPoint workPoint, *pWorkPoint;
    ccList *pWorkList;

    workPoint.x = pPoint->x - pPressedAt->x;
    workPoint.y = pPoint->y - pPressedAt->y;

    pWorkScreen = GetFirstSelectedScreen();

    while(pWorkScreen)
    {
        if(pWorkScreen->GetScreenStatus() == Screen::SS_SELECTED)
        {
            pWorkPoint = pWorkScreen->GetIconPos();

            pWorkScreen->SetScreenPos(pWorkPoint->x + workPoint.x, pWorkPoint->y + workPoint.y);

            // Now relocate each of this screens screenlinks.
            pWorkLink = pWorkScreen->GetFirstScreenLink();

            while(pWorkLink)
            {
                pWorkLink->SetLinkPos();

                pWorkLink = (ScreenLink *)pWorkLink->GetNext();
            }

            // Now relocate each of this screen's inbound screen links.
            pWorkList = pWorkScreen->GetInboundLinkList();

            pWorkContainer = (ScreenLinkContainer *)pWorkList->GetHead();

            while(pWorkContainer)
            {
                pWorkLink = pWorkContainer->GetMyLink();

                pWorkLink->SetLinkDst(&pWorkScreen->GetIconBounds()->CenterPoint());

                pWorkContainer = (ScreenLinkContainer *)pWorkContainer->GetNext();
            }
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }
}

void CFEManDoc::RelocateSelectedScreenLink(CPoint *pPressedAt, CPoint *pPoint)
{
    Screen *pWorkScreen;
    ScreenLink *pWorkLink;
    CPoint workPoint, *pWorkPoint;

    workPoint.x = pPoint->x - pPressedAt->x;
    workPoint.y = pPoint->y - pPressedAt->y;

    pWorkScreen = GetFirstSelectedScreen();

    pWorkLink = pWorkScreen->GetSelectedLink();

    pWorkPoint = pWorkLink->GetLinkDst();

    pWorkLink->SetLinkDst(pWorkPoint->x + workPoint.x, pWorkPoint->y + workPoint.y);
}

void CFEManDoc::UpdateScreenEditView(void)
{
    UpdateAllViews(NULL);
}

void CFEManDoc::BringSelectedScreenObjectsToFront(void)
{
    // Take all selected screen objects and move them to the tail of the list.
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pScreenObject;
    ccList *srcList;

    srcList = pWorkScreen->GetScreenObjectList();

    // Remove each node from the list and add it to the tail.
    pScreenObject = pWorkScreen->GetFirstSelectedScreenObject();

    while(pScreenObject)
    {
        if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
        {
            srcList->FlushNode(pScreenObject);
            srcList->AddTail(pScreenObject);
        }

        pScreenObject = (ScreenObject *)pScreenObject->GetNext();
    }
}

void CFEManDoc::SendSelectedScreenObjectsToBack(void)
{
    // Take all selected screen objects and move them to the head of the list.
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pScreenObject;
    ccList *srcList;

    srcList = pWorkScreen->GetScreenObjectList();

    // Remove each node from the list and add it to the tail.
    pScreenObject = pWorkScreen->GetFirstSelectedScreenObject();

    while(pScreenObject)
    {
        if(pScreenObject->GetStatus() == ScreenObject::SOS_SELECTED)
        {
            srcList->FlushNode(pScreenObject);
            srcList->AddHead(pScreenObject);
        }

        pScreenObject = (ScreenObject *)pScreenObject->GetNext();
    }
}

void CFEManDoc::OnFileNew()
{
    // Clean out everything.
    DeleteAbsolutelyEverything();

    SetTitle("Untitled");
    SetPathName("Untitled", FALSE);
}

void CFEManDoc::DeleteAbsolutelyEverything(void)
{
    screenList.PurgeList();

    gridActive = gridVisible = 0;
    gridSpacing = 10;

    UpdateAllViews(NULL);
}

#define FEM_FILE_TAG                "{FEMANAGER_TAG}\n"

#define FEM_GRIDACTIVE_FILE_TAG     "{FEMANAGER_GRIDACTIVE}\t"
#define FEM_GRIDVISIBLE_FILE_TAG    "{FEMANAGER_GRIDVISIBLE}\t"
#define FEM_GRIDSPACING_FILE_TAG    "{FEMANAGER_GRIDSPACING}\t"
#define FEM_DEFAULTFONT_FILE_TAG    "{FEMANAGER_DEFAULTFONT}\n"

#define FEM_END_FILE_TAG            "\n{FEMANAGER_END}"

void CFEManDoc::SaveEverything(CArchive *ar)
{   
    char tempBuffer[500];

    // Don't do anything if there are no screens.
    if(GetNumScreens() == 0)
        return;
    
    // Write the serial tag for the file.
    ar->WriteString(FEM_FILE_TAG);

    // Write the local CFEManDoc info to the file.
    ar->WriteString(FEM_GRIDACTIVE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", gridActive);
    ar->WriteString(tempBuffer);

    ar->WriteString(FEM_GRIDVISIBLE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", gridVisible);
    ar->WriteString(tempBuffer);

    ar->WriteString(FEM_GRIDSPACING_FILE_TAG);
    sprintf(tempBuffer, "%d\n", gridSpacing);
    ar->WriteString(tempBuffer);

    ar->WriteString(FEM_DEFAULTFONT_FILE_TAG);
    ar->Write(&gDefaultFont, sizeof(LOGFONT));
    ar->WriteString("\n");
    
    Screen *pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        SaveScreen(pWorkScreen, ar);

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    ar->WriteString(FEM_END_FILE_TAG);
}

#define SCREEN_FILE_TAG                 "\n\t{SCREEN_TAG}\n"

#define SCREEN_ICONPOS_FILE_TAG         "\t\t{SCREEN_ICONPOS}\t"
#define SCREEN_ICONBOUNDS_FILE_TAG      "\t\t{SCREEN_ICONBOUNDS}\t"
#define SCREEN_NAMESIZE_FILE_TAG        "\t\t{SCREEN_NAMESIZE}\t"
#define SCREEN_SCREENNAME_FILE_TAG      "\t\t{SCREEN_SCREENNAME}\t"
#define SCREEN_SCREENSTATUS_FILE_TAG    "\t\t{SCREEN_SCREENSTATUS}\t"
#define SCREEN_SCREENID_FILE_TAG        "\t\t{SCREEN_SCREENID}\t"
#define SCREEN_USERINFO_FILE_TAG        "\t\t{SCREEN_USERINFO}\t"

#define SCREEN_END_FILE_TAG             "\t{SCREEN_END}\n"

void CFEManDoc::SaveScreen(Screen *s, CArchive *ar)
{
    char tempBuffer[500];

    // Write the serial tag for this screen.
    ar->WriteString(SCREEN_FILE_TAG);

    // Save the information for this screen's icon representation.
    ar->WriteString(SCREEN_ICONPOS_FILE_TAG);
    sprintf(tempBuffer, "%d, %d\n", s->GetIconX(), s->GetIconY());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREEN_ICONBOUNDS_FILE_TAG);
    CRect *tempRect = s->GetIconBounds();
    sprintf(tempBuffer, "%d, %d, %d, %d\n", tempRect->left, tempRect->top, tempRect->right, tempRect->bottom);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREEN_NAMESIZE_FILE_TAG);
    CSize *tempSize = s->GetNameSize();
    sprintf(tempBuffer, "%d, %d\n", tempSize->cx, tempSize->cy);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREEN_SCREENNAME_FILE_TAG);
    sprintf(tempBuffer, "%s\n", s->GetName());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREEN_SCREENSTATUS_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetScreenStatus());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREEN_SCREENID_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetScreenID());
    ar->WriteString(tempBuffer);

    // Save the user info.
    if(s->GetMyUserInfo())
    {
        ar->WriteString(SCREEN_USERINFO_FILE_TAG);
        sprintf(tempBuffer, "%d\n", strlen(s->GetMyUserInfo()) + 1);
        ar->WriteString(tempBuffer);
        ar->Write(s->GetMyUserInfo(), strlen(s->GetMyUserInfo()) + 1);
        ar->WriteString("\n");
    }
    
    // Save all this Screen's ScreenLinks.
    ScreenLink *pWorkLink = s->GetFirstScreenLink();

    while(pWorkLink)
    {
        SaveScreenLink(pWorkLink, ar);

        pWorkLink = (ScreenLink *)pWorkLink->GetNext();
    }

    // Save all of this Screen's ScreenObejcts.
    ScreenObject *pWorkObject = s->GetFirstScreenObject();

    while(pWorkObject)
    {
        switch(pWorkObject->GetType())
        {
        case ScreenObject::SOT_SCREENOBJECTRECT:
            SaveScreenObjectRect((ScreenObjectRect *)pWorkObject, ar);
            break;
        
        case ScreenObject::SOT_SCREENOBJECTBITMAP:
            SaveScreenObjectBitmap((ScreenObjectBitmap *)pWorkObject, ar);
            break;

        case ScreenObject::SOT_SCREENOBJECTSTRING:
            SaveScreenObjectString((ScreenObjectString *)pWorkObject, ar);
            break;
        }

        pWorkObject = (ScreenObject *)pWorkObject->GetNext();
    }
    
    ar->WriteString(SCREEN_END_FILE_TAG);
}

#define SCREENLINK_FILE_TAG                 "\n\t\t{SCREENLINK_TAG}\n"

#define SCREENLINK_STATUS_FILE_TAG          "\t\t\t{SCREENLINK_STATUS}\t"
#define SCREENLINK_TYPE_FILE_TAG            "\t\t\t{SCREENLINK_TYPE}\t"
#define SCREENLINK_BOUNDS_FILE_TAG          "\t\t\t{SCREENLINK_BOUNDS}\t"
#define SCREENLINK_SRC_FILE_TAG             "\t\t\t{SCREENLINK_SRC}\t"
#define SCREENLINK_DST_FILE_TAG             "\t\t\t{SCREENLINK_DST}\t"
#define SCREENLINK_NAME_FILE_TAG            "\t\t\t{SCREENLINK_NAME}\t"

#define SCREENLINK_LINKEDTOID_FILE_TAG      "\t\t\t{SCREENLINK_LINKEDTOID}\t"

#define SCREENLINK_END_FILE_TAG             "\t\t{SCREENLINK_END}\n"

void CFEManDoc::SaveScreenLink(ScreenLink *s, CArchive *ar)
{
    char tempBuffer[500];

    // Write the serial tag for this ScreenLink.
    ar->WriteString(SCREENLINK_FILE_TAG);

    // Save the information for this screen's links.
    ar->WriteString(SCREENLINK_STATUS_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetStatus());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_TYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_BOUNDS_FILE_TAG);
    CRect *tempRect = s->GetBounds();
    sprintf(tempBuffer, "%d, %d, %d, %d\n", tempRect->left, tempRect->top, tempRect->right, tempRect->bottom);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_SRC_FILE_TAG);
    CPoint *tempPoint = s->GetLinkSrc();
    sprintf(tempBuffer, "%d, %d\n", tempPoint->x, tempPoint->y);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_DST_FILE_TAG);
    tempPoint = s->GetLinkDst();
    sprintf(tempBuffer, "%d, %d\n", tempPoint->x, tempPoint->y);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_NAME_FILE_TAG);
    sprintf(tempBuffer, "%s\n", s->GetName());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_LINKEDTOID_FILE_TAG);
    Screen *tempScreen = s->LinkedTo();
    signed long ID;

    if(tempScreen)
    {
        ID = tempScreen->GetScreenID();
    }
    else
    {
        ID = -1;
    }

    sprintf(tempBuffer, "%d\n", ID);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENLINK_END_FILE_TAG);
}

// Treat this all differently because it's a sub-object of one of the derived types.
#define SCREENOBJECT_STATUS_FILE_TAG            "\t\t\t{SCREENOBJECT_STATUS}\t"
#define SCREENOBJECT_TYPE_FILE_TAG              "\t\t\t{SCREENOBJECT_TYPE}\t"
#define SCREENOBJECT_BOUNDS_FILE_TAG            "\t\t\t{SCREENOBJECT_BOUNDS}\t"
#define SCREENOBJECT_POS_FILE_TAG               "\t\t\t{SCREENOBJECT_SRC}\t"
#define SCREENOBJECT_NAME_FILE_TAG              "\t\t\t{SCREENOBJECT_NAME}\t"
#define SCREENOBJECT_USERINFO_FILE_TAG          "\t\t\t{SCREENOBJECT_USERINFO}\t"

#define SCREENOBJECT_END_FILE_TAG               "\t\t\t{SCREENOBJECT_END}\n"

void CFEManDoc::SaveScreenObject(ScreenObject *s, CArchive *ar)
{
    char tempBuffer[500];
    
    // Save basic ScreenObject stuff.
    ar->WriteString(SCREENOBJECT_STATUS_FILE_TAG);  
    sprintf(tempBuffer, "%d\n", s->GetStatus());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECT_TYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECT_BOUNDS_FILE_TAG);
    CRect *tempRect = s->GetBounds();
    sprintf(tempBuffer, "%d, %d, %d, %d\n", tempRect->left, tempRect->top, tempRect->right, tempRect->bottom);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECT_POS_FILE_TAG);
    CPoint *tempPoint = s->GetPos();
    sprintf(tempBuffer, "%d, %d\n", tempPoint->x, tempPoint->y);
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECT_NAME_FILE_TAG);
    sprintf(tempBuffer, "%s\n", s->GetName());
    ar->WriteString(tempBuffer);

    // Save the user info.
    if(s->GetMyUserInfo())
    {
        ar->WriteString(SCREENOBJECT_USERINFO_FILE_TAG);
        sprintf(tempBuffer, "%d\n", strlen(s->GetMyUserInfo()) + 1);
        ar->WriteString(tempBuffer);
        ar->Write(s->GetMyUserInfo(), strlen(s->GetMyUserInfo()) + 1);
        ar->WriteString("\n");
    }

    // Write end tag anyway.
    ar->WriteString(SCREENOBJECT_END_FILE_TAG);
}

#define SCREENOBJECTRECT_FILE_TAG                   "\n\t\t{SCREENOBJECTRECT_TAG}\n"

#define SCREENOBJECTRECT_BORDERTYPE_FILE_TAG        "\t\t\t{SCREENOBJECTRECT_BORDERTYPE}\t"
#define SCREENOBJECTRECT_BORDERCOLOR_FILE_TAG       "\t\t\t{SCREENOBJECTRECT_BORDERCOLOR}\t"
#define SCREENOBJECTRECT_BORDERTHICKNESS_FILE_TAG   "\t\t\t{SCREENOBJECTRECT_BORDERTHICKNESS}\t"
#define SCREENOBJECTRECT_CONTENTTYPE_FILE_TAG       "\t\t\t{SCREENOBJECTRECT_CONTENTTYPE}\t"
#define SCREENOBJECTRECT_CONTENTCOLOR_FILE_TAG      "\t\t\t{SCREENOBJECTRECT_CONTENTCOLOR}\t"

#define SCREENOBJECTRECT_END_FILE_TAG               "\t\t{SCREENOBJECTRECT_END}\n"

void CFEManDoc::SaveScreenObjectRect(ScreenObjectRect *s, CArchive *ar)
{
    char tempBuffer[500];

    // Write the serial tag for this ScreenObjectRect.
    ar->WriteString(SCREENOBJECTRECT_FILE_TAG);

    // Save basic ScreenObject Stuff.
    SaveScreenObject((ScreenObject *)s, ar);

    // Save specific ScreenObjectRect stuff.
    ar->WriteString(SCREENOBJECTRECT_BORDERTYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetBorderType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTRECT_BORDERCOLOR_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetBorderColor());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTRECT_BORDERTHICKNESS_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetBorderWidth());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTRECT_CONTENTTYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetContentType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTRECT_CONTENTCOLOR_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetContentColor());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTRECT_END_FILE_TAG);
}

#define SCREENOBJECTBITMAP_FILE_TAG                 "\n\t\t{SCREENOBJECTBITMAP_TAG}\n"

#define SCREENOBJECTBITMAP_FILENAME_FILE_TAG        "\t\t\t{SCREENOBJECTBITMAP_FILENAME}\t"
#define SCREENOBJECTBITMAP_CONTENTTYPE_FILE_TAG     "\t\t\t{SCREENOBJECTBITMAP_CONTENTTYPE}\t"

#define SCREENOBJECTBITMAP_END_FILE_TAG             "\t\t{SCREENOBJECTBITMAP_END}\n"

void CFEManDoc::SaveScreenObjectBitmap(ScreenObjectBitmap *s, CArchive *ar)
{
    char tempBuffer[500];

    // Write the serial tag for this ScreenObjectBitmap.
    ar->WriteString(SCREENOBJECTBITMAP_FILE_TAG);

    // Save basic ScreenObject Stuff.
    SaveScreenObject((ScreenObject *)s, ar);

    // Save specific ScreenObjectBitmap stuff.
    ar->WriteString(SCREENOBJECTBITMAP_FILENAME_FILE_TAG);
    sprintf(tempBuffer, "%s\n", s->GetFileName());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTBITMAP_CONTENTTYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetContentType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTBITMAP_END_FILE_TAG);
}

#define SCREENOBJECTSTRING_FILE_TAG                     "\n\t\t{SCREENOBJECTSTRING_TAG}\n"

#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG0           "\t\t\t{SCREENOBJECTSTRING_MYSTRING}\t"
#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG1           "\t\t\t{SCREENOBJECTSTRING_MYSTRING1}\t"
#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG2           "\t\t\t{SCREENOBJECTSTRING_MYSTRING2}\t"
#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG3           "\t\t\t{SCREENOBJECTSTRING_MYSTRING3}\t"
#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG4           "\t\t\t{SCREENOBJECTSTRING_MYSTRING4}\t"
#define SCREENOBJECTSTRING_MYSTRING_FILE_TAG5           "\t\t\t{SCREENOBJECTSTRING_MYSTRING5}\t"
#define SCREENOBJECTSTRING_CONTENTTYPE_FILE_TAG         "\t\t\t{SCREENOBJECTSTRING_CONTENTTYPE}\t"
#define SCREENOBJECTSTRING_TEXTCOLOR_FILE_TAG           "\t\t\t{SCREENOBJECTSTRING_TEXTCOLOR}\t"
#define SCREENOBJECTSTRING_TEXTBACKGROUND_FILE_TAG      "\t\t\t{SCREENOBJECTSTRING_TEXTBACKGROUND}\t"
#define SCREENOBJECTSTRING_AUTOSIZETOCONTENT_FILE_TAG   "\t\t\t{SCREENOBJECTSTRING_AUTOSIZETOCONTENT}\t"
#define SCREENOBJECTSTRING_JUSTIFICATION                "\t\t\t{SCREENOBJECTSTRING_JUSTIFICATION}\t"
#define SCREENOBJECTSTRING_MYFONT_FILE_TAG              "\t\t\t{SCREENOBJECTSTRING_MYFONT}\n"

#define SCREENOBJECTSTRING_END_FILE_TAG                 "\t\t{SCREENOBJECTSTRING_END}\n"

char *tagByLanguageIndex[NUMBER_LANGUAGES] =
{
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG0,
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG1,
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG2,
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG3,
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG4,
    SCREENOBJECTSTRING_MYSTRING_FILE_TAG5
};
void CFEManDoc::SaveScreenObjectString(ScreenObjectString *s, CArchive *ar)
{
    char tempBuffer[500];
    unsigned long index;

    // Write the serial tag for this ScreenObjectString.
    ar->WriteString(SCREENOBJECTSTRING_FILE_TAG);

    // Save basic ScreenObject Stuff.
    SaveScreenObject((ScreenObject *)s, ar);

    // Save specific ScreenObjectString stuff.
    for (index = 0; index < NUMBER_LANGUAGES; index++)
    {
        if (s->GetMyString(index) != NULL)
        {
            ar->WriteString(tagByLanguageIndex[index]);
            sprintf(tempBuffer, "%d\n", strlen(s->GetMyString(index)) + 1);
            ar->WriteString(tempBuffer);
            ar->Write(s->GetMyString(index), strlen(s->GetMyString(index)) + 1);
            ar->WriteString("\n");
        }
    }

    ar->WriteString(SCREENOBJECTSTRING_CONTENTTYPE_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetContentType());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTSTRING_TEXTCOLOR_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetTextColor());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTSTRING_TEXTBACKGROUND_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetTextBackground());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTSTRING_AUTOSIZETOCONTENT_FILE_TAG);
    sprintf(tempBuffer, "%d\n", s->GetAutoSizeToContent());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTSTRING_JUSTIFICATION);
    sprintf(tempBuffer, "%d\n", s->GetJustification());
    ar->WriteString(tempBuffer);

    ar->WriteString(SCREENOBJECTSTRING_MYFONT_FILE_TAG);
    ar->Write(s->GetMyFont(), sizeof(LOGFONT));
    ar->WriteString("\n");

    ar->WriteString(SCREENOBJECTSTRING_END_FILE_TAG);
}

char *CFEManDoc::FindTag(char *buffer, char *tag, CArchive *ar)
{
    char *pTempBuffer, *pTagBuffer;
    char delimiter[100] = {' ', '\t', '\n'};

    ASSERT(buffer);
    ASSERT(tag);

    pTagBuffer = strtok(tag, delimiter);

    pTempBuffer = strtok(buffer, delimiter);

    while(pTempBuffer)
    {
        if(!strcmp(pTempBuffer, pTagBuffer))
            return(pTempBuffer);

        pTempBuffer = strtok(NULL, delimiter);
    }
    
    return(0);
}

char *CFEManDoc::GetInt(char *buffer, signed long *num)
{
    char *pRet;
    char delimiter[200] = {' ', ',', '\t', 'n'};

    ASSERT(buffer);
    ASSERT(num);

    pRet = strtok(buffer, delimiter);

    *num = atoi(pRet);

    return(pRet);
}

char *CFEManDoc::SkipTag(char *buffer)
{
    char *pRet = buffer;

    while(*pRet)
        pRet ++;

    while(!*pRet)
        pRet ++;

    return(pRet);
}

void CFEManDoc::LoadEverything(CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;
    char foundFile = 0;

    // Is this a .FEM file?
    while(ar->ReadString(tempBuffer, 500))
    {
        if(FindTag(tempBuffer, FEM_FILE_TAG, ar))
        {
            foundFile = 1;
            break;
        }
    }
    
    // Is this a .FEM file?
    if(!foundFile)
        return;

    // It is, so delete everything in preparation for new stuff.
    DeleteAbsolutelyEverything();
    
    while(ar->ReadString(tempBuffer, 500))
    {
        // Parse the grid info.
        if(pTempBuffer = FindTag(tempBuffer, FEM_GRIDACTIVE_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &gridActive);
        }

        if(pTempBuffer = FindTag(tempBuffer, FEM_GRIDVISIBLE_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &gridVisible);
        }

        if(pTempBuffer = FindTag(tempBuffer, FEM_GRIDSPACING_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &gridSpacing);
        }

        if(pTempBuffer = FindTag(tempBuffer, FEM_DEFAULTFONT_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            ar->Read(&gDefaultFont, sizeof(LOGFONT));
        }

        // Search for the screen nested tag.
        if(FindTag(tempBuffer, SCREEN_FILE_TAG, ar))
            LoadScreen(ar);

        // End of file?
        if(FindTag(tempBuffer, FEM_END_FILE_TAG, ar))
            break;
    }

    RealizeAllScreenLinks();

    // Select the first screen.
    Screen *pWorkScreen = GetFirstScreen();

    SelectScreenIcon(pWorkScreen);
}

void CFEManDoc::LoadScreen(CArchive *ar)
{
    Screen *pWorkScreen;
    char tempBuffer[500], *pTempBuffer;
    CDC cDC;
        
    // This is needed for screen creation.
    cDC.CreateCompatibleDC(NULL);

    // Create the new screen and add it to the list.
    pWorkScreen = CreateLoadedScreen();

    while(ar->ReadString(tempBuffer, 500))
    {
        CPoint tempPoint;
        
        // Screen Icon pos?
        if((pTempBuffer = FindTag(tempBuffer, SCREEN_ICONPOS_FILE_TAG, ar)))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempPoint.x);

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempPoint.y);

            pWorkScreen->SetScreenPos(tempPoint.x, tempPoint.y);

            continue;
        }

        // Screen Icon bounds?
        if(pTempBuffer = FindTag(tempBuffer, SCREEN_ICONBOUNDS_FILE_TAG, ar))
        {
            // Don't need to set this.  It's automatic.
        }

        // Screen Name size?
        if(pTempBuffer = FindTag(tempBuffer, SCREEN_NAMESIZE_FILE_TAG, ar))
        {
            // Don't need to set this.  It's automatic.
        }

        // Screen Name?
        if(pTempBuffer = FindTag(tempBuffer, SCREEN_SCREENNAME_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            pWorkScreen->SetScreenName(&cDC, pTempBuffer);
        }

        // Screen Status?
        if(pTempBuffer = FindTag(tempBuffer, SCREEN_SCREENSTATUS_FILE_TAG, ar))
        {
            // Don't need to set this.  It's automatic.
        }

        // Screen's unique ID?
        if(pTempBuffer = FindTag(tempBuffer, SCREEN_SCREENID_FILE_TAG, ar))
        {
            signed long tempID;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempID);

            pWorkScreen->SetScreenID(tempID);
        }

        // End of internal screen data.  Now parse nested blocks.
        
        // ScrenLink?
        if(FindTag(tempBuffer, SCREENLINK_FILE_TAG, ar))
            LoadScreenLink(pWorkScreen, ar);

        if(FindTag(tempBuffer, SCREENOBJECTRECT_FILE_TAG, ar))
            LoadScreenObjectRect(pWorkScreen, ar);
        
        if(FindTag(tempBuffer, SCREENOBJECTBITMAP_FILE_TAG, ar))
            LoadScreenObjectBitmap(pWorkScreen, ar);

        if(FindTag(tempBuffer, SCREENOBJECTSTRING_FILE_TAG, ar))
            LoadScreenObjectString(pWorkScreen, ar);

        // End of screen block?
        if(FindTag(tempBuffer, SCREEN_END_FILE_TAG, ar))
            break;
    }
}

void CFEManDoc::LoadScreenLink(Screen *s, CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;
    
    ScreenLink *pWorkLink;

    pWorkLink = s->AddScreenLink();

    while(ar->ReadString(tempBuffer, 500))
    {
        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_STATUS_FILE_TAG, ar))
        {
            // This is set automatically.
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_TYPE_FILE_TAG, ar))
        {
            signed long tempType;

            pTempBuffer = SkipTag(pTempBuffer);

            pTempBuffer = GetInt(pTempBuffer, &tempType);

            pWorkLink->SetType(tempType);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_BOUNDS_FILE_TAG, ar))
        {
            // This is set automatically.
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_SRC_FILE_TAG, ar))
        {
            // This is set automatically.
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_DST_FILE_TAG, ar))
        {
            // This is set automatically.
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_NAME_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);

            pWorkLink->SetName(pTempBuffer);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_LINKEDTOID_FILE_TAG, ar))
        {
            signed long tempID;

            pTempBuffer = SkipTag(pTempBuffer);

            pTempBuffer = GetInt(pTempBuffer, &tempID);

            pWorkLink->SetLinkedToID(tempID);
        }
        
        if(pTempBuffer = FindTag(tempBuffer, SCREENLINK_END_FILE_TAG, ar))
            break;
    }
}

void CFEManDoc::LoadScreenObject(ScreenObject *s, CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;

    while(ar->ReadString(tempBuffer, 500))
    {
        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_STATUS_FILE_TAG, ar))
        {
            // Don't need to set this.  It's automatic.
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_TYPE_FILE_TAG, ar))
        {
            signed long tempType;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempType);

            s->SetType(tempType);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_BOUNDS_FILE_TAG, ar))
        {
            CRect tempRect;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempRect.left);

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempRect.top);

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempRect.right);

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempRect.bottom);

            s->SetBounds(&tempRect);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_POS_FILE_TAG, ar))
        {
            CPoint tempPoint;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempPoint.x);

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempPoint.y);

            s->SetPos(tempPoint.x, tempPoint.y);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_NAME_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);

            s->SetName(pTempBuffer);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECT_USERINFO_FILE_TAG, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            s->SetMyUserInfo(tempBuffer);
        }

        if(FindTag(tempBuffer, SCREENOBJECT_END_FILE_TAG, ar))
            break;
    }
}

void CFEManDoc::LoadScreenObjectRect(Screen *s, CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;
    
    ScreenObjectRect *pWorkObject;

    pWorkObject = new ScreenObjectRect;

    s->AddScreenObject(pWorkObject);

    LoadScreenObject((ScreenObject *)pWorkObject, ar);

    while(ar->ReadString(tempBuffer, 500))
    {
        // Now load the screenobjectrect specific data.
        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTRECT_BORDERTYPE_FILE_TAG, ar))
        {
            signed long tempBorderType;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempBorderType);

            pWorkObject->SetBorderType(tempBorderType);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTRECT_BORDERCOLOR_FILE_TAG, ar))
        {
            signed long tempBorderColor;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempBorderColor);

            pWorkObject->SetBorderColor(tempBorderColor);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTRECT_BORDERTHICKNESS_FILE_TAG, ar))
        {
            signed long tempBorderThickness;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempBorderThickness);

            pWorkObject->SetBorderWidth(tempBorderThickness);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTRECT_CONTENTTYPE_FILE_TAG, ar))
        {
            signed long tempContentType;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempContentType);

            pWorkObject->SetContentType(tempContentType);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTRECT_CONTENTCOLOR_FILE_TAG, ar))
        {
            signed long tempContentColor;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempContentColor);

            pWorkObject->SetContentColor(tempContentColor);
        }

        if(FindTag(tempBuffer, SCREENOBJECTRECT_END_FILE_TAG, ar))
            break;
    }
}

void CFEManDoc::LoadScreenObjectBitmap(Screen *s, CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;
    
    ScreenObjectBitmap *pWorkObject;

    pWorkObject = new ScreenObjectBitmap;

    s->AddScreenObject(pWorkObject);

    LoadScreenObject((ScreenObject *)pWorkObject, ar);

    // Now load the screenobjectbitmap specific stuff.
    while(ar->ReadString(tempBuffer, 500))
    {
        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTBITMAP_FILENAME_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);

            if(pTempBuffer[0])
            {
                // Load the file.
                POSITION pos = GetFirstViewPosition();
                CView* pFirstView = GetNextView( pos );
                CDC *pDC = pFirstView->GetDC();

                pWorkObject->SetFileName(pTempBuffer);
                pWorkObject->LoadBitmap(pDC, pTempBuffer);
            }
        }
    
        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTBITMAP_CONTENTTYPE_FILE_TAG, ar))
        {
            signed long tempContentType;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempContentType);

            pWorkObject->SetContentType(tempContentType);
        }

        if(FindTag(tempBuffer, SCREENOBJECTBITMAP_END_FILE_TAG, ar))
            break;
    }
}

void CFEManDoc::LoadScreenObjectString(Screen *s, CArchive *ar)
{
    char tempBuffer[500], *pTempBuffer;
    
    ScreenObjectString *pWorkObject;

    pWorkObject = new ScreenObjectString;

    s->AddScreenObject(pWorkObject);

    LoadScreenObject((ScreenObject *)pWorkObject, ar);

    // Now load the screenobjectbitmap specific stuff.
    while(ar->ReadString(tempBuffer, 500))
    {
        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG0, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 0);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG1, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 1);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG2, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 2);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG3, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 3);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG4, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 4);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYSTRING_FILE_TAG5, ar))
        {
            signed long myStringLength;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &myStringLength);

            ar->Read(tempBuffer, myStringLength);

            pWorkObject->SetMyString(tempBuffer, 5);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_CONTENTTYPE_FILE_TAG, ar))
        {
            signed long tempContentType;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempContentType);

            pWorkObject->SetContentType(tempContentType);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_TEXTCOLOR_FILE_TAG, ar))
        {
            signed long tempTextColor;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempTextColor);

            pWorkObject->SetTextColor(tempTextColor);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_TEXTBACKGROUND_FILE_TAG, ar))
        {
            signed long tempTextBackground;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempTextBackground);

            pWorkObject->SetTextBackground(tempTextBackground);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_AUTOSIZETOCONTENT_FILE_TAG, ar))
        {
            signed long tempAutoSizeToContent;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempAutoSizeToContent);

            pWorkObject->SetAutoSizeToContent(tempAutoSizeToContent);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_JUSTIFICATION, ar))
        {
            signed long tempJustification;

            pTempBuffer = SkipTag(pTempBuffer);
            pTempBuffer = GetInt(pTempBuffer, &tempJustification);

            pWorkObject->SetJustification((unsigned long)tempJustification);
        }

        if(pTempBuffer = FindTag(tempBuffer, SCREENOBJECTSTRING_MYFONT_FILE_TAG, ar))
        {
            pTempBuffer = SkipTag(pTempBuffer);
            ar->Read(pWorkObject->GetMyFont(), sizeof(LOGFONT));
        }
    
        if(FindTag(tempBuffer, SCREENOBJECTSTRING_END_FILE_TAG, ar))
            break;
    }
}

Screen *CFEManDoc::FindScreenWithID(signed long ID)
{
    Screen *pWorkScreen;

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        if(pWorkScreen->GetScreenID() == ID)
            return(pWorkScreen);

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    return(0);
}

void CFEManDoc::RealizeAllScreenLinks(void)
{
    Screen *pWorkScreen, *pTempScreen;
    ScreenLink *pWorkLink;

    pWorkScreen = GetFirstScreen();

    while(pWorkScreen)
    {
        if(pWorkScreen->GetScreenID() > globalScreenIDMaster)
            globalScreenIDMaster = pWorkScreen->GetScreenID();

        pWorkLink = pWorkScreen->GetFirstScreenLink();

        while(pWorkLink)
        {
            if(pWorkLink->GetLinkedToID() != -1)
            {
                pTempScreen = FindScreenWithID(pWorkLink->GetLinkedToID());

                //if(pWorkLink->GetLinkedToID() > globalScreenIDMaster)
                //  globalScreenIDMaster = pWorkLink->GetLinkedToID();

                // Should ALWAYS be a valid link.
                ASSERT(pTempScreen);

                pWorkLink->AttachTo(pTempScreen);
            }
            
            pWorkLink = (ScreenLink *)pWorkLink->GetNext();
        }

        pWorkScreen = (Screen *)pWorkScreen->GetNext();
    }

    RelocateAllScreens();

    // Increment the globalScreenIDMaster to ensure we get unique screen id's.
    globalScreenIDMaster ++;
}

void CFEManDoc::DoCopyScreenObject(CDC *pDC)
{
    ScreenObjectRect *pWorkObjectRect;
    ScreenObjectBitmap *pWorkObjectBitmap;
    ScreenObjectString *pWorkObjectString;
    
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();
    
    clipboardList.PurgeList();

    while(pWorkObject)
    {
        if(pWorkObject->GetStatus() == ScreenObject::SOS_SELECTED)
        {
            switch(pWorkObject->GetType())
            {
            case ScreenObject::SOT_SCREENOBJECTRECT:
                pWorkObjectRect = (ScreenObjectRect *)pWorkObject->Copy(pDC);

                clipboardList.AddTail(pWorkObjectRect);
                break;

            case ScreenObject::SOT_SCREENOBJECTBITMAP:
                pWorkObjectBitmap = (ScreenObjectBitmap *)pWorkObject->Copy(pDC);

                clipboardList.AddTail(pWorkObjectBitmap);
                break;

            case ScreenObject::SOT_SCREENOBJECTSTRING:
                pWorkObjectString = (ScreenObjectString *)pWorkObject->Copy(pDC);

                clipboardList.AddTail(pWorkObjectString);
                break;
            }
        }

        pWorkObject = (ScreenObject *)pWorkObject->GetNext();
    }
}

void CFEManDoc::DoCutScreenObject(CDC *pDC)
{
    Screen *pWorkScreen = GetFirstSelectedScreen();
    
    DoCopyScreenObject(pDC);

    pWorkScreen->DeleteSelectedScreenObjects();
}

void CFEManDoc::DoPasteScreenObject(CDC *pDC)
{
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pWorkObject = (ScreenObject *)clipboardList.GetHead(), *pTempObject;

    pWorkScreen->ClearAllSelectedScreenObjects();

    while(pWorkObject)
    {
        CPoint *tempPoint = pWorkObject->GetPos();
        pWorkObject->SetPos(tempPoint->x, tempPoint->y);
                
        pTempObject = pWorkObject->Copy(pDC);

        tempPoint = pTempObject->GetPos();
        
        pWorkScreen->AddScreenObject(pTempObject);

        pWorkObject = (ScreenObject *)pWorkObject->GetNext();
    }
}

void CFEManDoc::CopySelectedScreensToDeleteBuffer(CDC *pDC)
{
    ScreenObjectRect *pWorkObjectRect;
    ScreenObjectBitmap *pWorkObjectBitmap;
    ScreenObjectString *pWorkObjectString;
    
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pWorkObject = pWorkScreen->GetFirstSelectedScreenObject();
    
    undoList.PurgeList();

    while(pWorkObject)
    {
        if(pWorkObject->GetStatus() == ScreenObject::SOS_SELECTED)
        {
            switch(pWorkObject->GetType())
            {
            case ScreenObject::SOT_SCREENOBJECTRECT:
                if(pWorkScreen->IsObjectDefaultPage(pWorkObject))
                {
                    pWorkObject = (ScreenObject *)pWorkObject->GetNext();

                    continue;
                }
                
                pWorkObjectRect = (ScreenObjectRect *)pWorkObject->Copy(pDC);

                undoList.AddTail(pWorkObjectRect);
                break;

            case ScreenObject::SOT_SCREENOBJECTBITMAP:
                pWorkObjectBitmap = (ScreenObjectBitmap *)pWorkObject->Copy(pDC);

                undoList.AddTail(pWorkObjectBitmap);
                break;

            case ScreenObject::SOT_SCREENOBJECTSTRING:
                pWorkObjectString = (ScreenObjectString *)pWorkObject->Copy(pDC);

                undoList.AddTail(pWorkObjectString);
                break;
            }
        }

        pWorkObject = (ScreenObject *)pWorkObject->GetNext();
    }
}

void CFEManDoc::DoUndoScreenObject(CDC *pDC)
{
    Screen *pWorkScreen = GetFirstSelectedScreen();
    ScreenObject *pWorkObject = (ScreenObject *)undoList.GetHead(), *pTempObject;

    while(pWorkObject)
    {
        CPoint *tempPoint = pWorkObject->GetPos();
                
        pTempObject = pWorkObject->Copy(pDC);

        tempPoint = pTempObject->GetPos();
        
        pWorkScreen->AddScreenObject(pTempObject);

        pWorkObject = (ScreenObject *)pWorkObject->GetNext();
    }

    // Only do this once.
    undoList.PurgeList();
}

void CFEManDoc::OnFilePrint()
{
}

void CFEManDoc::OnFileSave()
{
    CDocument::OnFileSave();
}

void CFEManDoc::OnFileExport()
{
    static char BASED_CODE szFilter[] = "Homeworld Files (*.fib)|*.FIB|All Files (*.*)|*.*||";
    
    CString pStrDef;
    
    pStrDef = theApp.GetProfileString("Path Information", "Last Export Directory");
    
    // Bring up the dialog to select the file name of the bitmap.
    CFileDialog tempFileDialog(TRUE, ".BMP", pStrDef, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

    if(tempFileDialog.DoModal() == IDOK)
    {
        theApp.WriteProfileString("Path Information", "Last Export Directory", tempFileDialog.GetPathName());
        
        SaveFIBFormat tempSaver;

        tempSaver.ParseFEManData(this);

        pStrDef = tempFileDialog.GetPathName();

        tempSaver.WriteFEManData(pStrDef, this);
    }
}
