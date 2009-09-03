/***********************************(GPL)********************************
*   wxHexEditor is a hex edit tool for editing massive files in Linux   *
*   Copyright (C) 2006  Erdem U. Altinyurt                              *
*                                                                       *
*   This program is free software; you can redistribute it and/or       *
*   modify it under the terms of the GNU General Public License         *
*   as published by the Free Software Foundation; either version 2      *
*   of the License, or any later version.                               *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program;                                            *
*   if not, write to the Free Software	Foundation, Inc.,                *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : wxhexeditor.sourceforge.net                     *
*               email : death_knight at gamebox.net                     *
*************************************************************************/
#ifndef __wxHexEditorCtrl__
#define __wxHexEditorCtrl__
#include <wx/xml/xml.h>
#include <wx/filename.h>

#include "HexEditorCtrlGui.h"
class HexEditorCtrl: public HexEditorCtrlGui{
	public:
		HexEditorCtrl(wxWindow* parent, int id,
						const wxPoint& pos=wxDefaultPosition,
						const wxSize& size=wxDefaultSize,
						long style=0);
		~HexEditorCtrl( void );
		enum IDS{ idTagSelect=1001,idTagEdit };
		void ReadFromBuffer( int64_t position, unsigned lenght, char *buffer, bool cursor_reset = true, bool paint = true );
		int64_t CursorOffset( void );
	protected:
		ArrayOfTAG MainTagArray;
		void Dynamic_Connector();
		void Dynamic_Disconnector();
		void Clear( bool RePaint=false, bool cursor_reset=true );
//		DECLARE_EVENT_TABLE()
		virtual void OnResize( wxSizeEvent& event);
		void ClearPaint( void );
		void PaintSelection( void );
		void PreparePaintTAGs( void );
//		void TagPaint( void );
		bool TAGMutex;
		void LoadTAGS( wxFileName );
		void SaveTAGS( wxFileName );

		bool Selector( void );
	public:
		bool Select( uint64_t start_offset, uint64_t end_offset );
		struct xselect{		//select structure
			enum states{ S_FALSE, S_TRUE, S_END } state;
			uint64_t start_offset;	//real start position
			uint64_t end_offset;		//real end position, included to select
			uint64_t size( void ){ return abs(end_offset - start_offset)+1;};	//for select byte 13 start=13, end=13
			} select;
	protected:
		void HexCharReplace( long char_location, const wxChar chr);
		void TextCharReplace( long char_location, const wxChar chr);
		void MyFreeze();
		void MyThaw();
		void ShowContextMenu(const wxMouseEvent& event);
		//------EVENTS---------//
		void OnMouseLeft(wxMouseEvent& event);
		void OnMouseMove( wxMouseEvent& event );
		void OnMouseSelectionEnd( wxMouseEvent& event );
		void OnMouseRight( wxMouseEvent& event );
		void OnTagSelection( wxCommandEvent& event );
		void OnTagEdit( wxCommandEvent& event );
		void TagHideAll( void );
		uint64_t LastRightClickAt;
		//----ADAPTERS----//
	public:
		int HexPerLine( void )  { return hex_ctrl->CharacterPerLine(); }
		int BytePerLine( void )	{ return hex_ctrl->BytePerLine(); }
		unsigned ByteCapacity( void ){ return hex_ctrl->ByteCapacity(); }
		int LineCount( void )	{ return hex_ctrl->LineCount(); }
		int ActiveLine( void )	{ return hex_ctrl->ActiveLine(); }
		int GetByteCount( void ){ return hex_ctrl->GetByteCount(); }
		int GetLastPosition( void ){ return hex_ctrl->GetLastPosition(); }
		int GetLocalHexInsertionPoint( void );
		int GetLocalInsertionPoint( void );
virtual void SetLocalHexInsertionPoint( int hex_location );
		void OnOffsetMouseFocus( wxMouseEvent& event );
	protected:
		int64_t page_offset;	//holds current start offset of file
/*
virtual int ToExactPosition( int InternalPosition );
virtual int ToInternalPosition( int ExactPosition );
virtual int PixelCoordToInternalPosition( wxPoint mouse );
		wxPoint InternalPositionToExactCoord( int position );
		wxPoint PixelCoordToInternalCoord( wxPoint mouse );
*/
//		virtual void OnKeyboardChar( wxKeyEvent& event );
//		virtual void OnKeyboardInput( wxKeyEvent& event );
	};

#endif
//
