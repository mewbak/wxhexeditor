/***********************************(GPL)********************************
*	wxHexEditor is a hex edit tool for editing massive files in Linux   *
*	Copyright (C) 2007  Erdem U. Altinyurt                              *
*                                                                       *
*	This program is free software; you can redistribute it and/or       *
*	modify it under the terms of the GNU General Public License         *
*	as published by the Free Software Foundation; either version 2      *
*	of the License, or any later version.                               *
*                                                                       *
*	This program is distributed in the hope that it will be useful,     *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*	GNU General Public License for more details.                        *
*                                                                       *
*	You should have received a copy of the GNU General Public License   *
*	along with this program;                                            *
*   if not, write to the Free Software	Foundation, Inc.,               *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : wxhexeditor.sourceforge.net                     *
*               email : death_knight at gamebox.net                     *
*************************************************************************/

#include "HexEditorFrame.h"
#include "wxHexEditor.h"
#include <wx/filename.h>

HexEditorFrame::HexEditorFrame(	wxWindow* parent,int id ):
				HexEditorGui( parent, id )
				{
//				wxFileName myfilename;
//				myfilename.SetFullName( _("/home/death/programing/wxHexEditor/es28.avi") );
//				MyHexEditor->FileOpen( myfilename );
//				MyHexEditor->Select( 1000, 2001 );

				MyAUI = new wxAuiManager( this );
				MyNotebook = new wxAuiNotebook(this,-1);
				MyAUI -> AddPane( MyNotebook, wxAuiPaneInfo().Name(wxT("wxHE")).Caption(wxT("HexEditor")).
						MinSize(wxSize(400,100)).CloseButton(false).
						Center().Layer(1)	);

//				MyNotebook->AddPage( new wxHexEditor(MyNotebook, -1, statusBar, &myfilename), myfilename.GetName(), true);

//	myinterpreter = new DataInterpreter( this, -1 );
//	MyNotebook->SetDropTarget( new DnDFile( MyNotebook, statusbar, myinterpreter) );
//	MyAUI -> AddPane( myinterpreter, wxAuiPaneInfo().
//					Name(wxT("wxHEint")).Caption(wxT("DataInterpreter")).
//					MinSize(wxSize(207,-1)).
//					Left().Layer(1)
//					);
//	MyAUI->GetPane(myinterpreter).Float();


#if defined( _DEBUG_ )
//   wxFileName myname(_("wxHexEditor"));
//   MyNotebook->AddPage( new HexEditor(myname, MyNotebook, -1, statusbar, myinterpreter), myname.GetFullName() );
#endif

	//SetDropTarget( new DnDFile( MyNotebook, statusbar, myinterpreter) );
				MyAUI->Update();
				}

HexEditorFrame::~HexEditorFrame(){}

void HexEditorFrame::OnFileOpen( wxCommandEvent& event ){
	wxFileDialog* filediag = new wxFileDialog(this,
											_("Choose a file for editing"),
											_(""),
											_(""),
											_("*"),
											wxFILE_MUST_EXIST|wxOPEN,
											wxDefaultPosition);
	if(wxID_OK == filediag->ShowModal()){
		wxFileName myname(filediag->GetPath());
		MyNotebook->AddPage( new wxHexEditor(MyNotebook, -1, statusBar, &myname ), myname.GetName(), true);
		filediag->Destroy();
		}
	event.Skip();
	}
void HexEditorFrame::OnFileClose( wxCommandEvent& event ){
	wxHexEditor *MyHexEditor = static_cast<wxHexEditor*>( MyNotebook->GetPage( MyNotebook->GetSelection() ) );
	if( MyHexEditor != NULL )
		if( MyHexEditor->FileClose() ){
			MyNotebook->DeletePage( MyNotebook->GetSelection() );
			//delete MyHexEditor;
			}
	event.Skip();
	}
