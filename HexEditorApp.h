/***************************************************************
 * Name:      wxHexEditorApp.h
 * Purpose:   Defines Application Class
 * Author:    Death Knight (death_knight@gamebox.net)
 * Created:   2008-05-12
 * Copyright: Death Knight (wxhexeditor.sourceforge.net)
 * License:
 **************************************************************/

#ifndef WXHEXEDITORAPP_H
#define WXHEXEDITORAPP_H

#define _VERSION_ "v0.07-svn Alpha"

#include <wx/app.h>

class wxHexEditorApp : public wxApp
{
    public:
        virtual bool OnInit();
};

#endif // WXHEXEDITORAPP_H
