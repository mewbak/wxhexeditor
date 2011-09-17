/***********************************(GPL)********************************
*   wxHexEditor is a hex edit tool for editing massive files in Linux   *
*   Copyright (C) 2010  Erdem U. Altinyurt                              *
*                                                                       *
*   This program is free software; you can redistribute it and/or       *
*   modify it under the terms of the GNU General Public License         *
*   as published by the Free Software Foundation; either version 2      *
*   of the License.                                                     *
*                                                                       *
*   This program is distributed in the hope that it will be useful,     *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
*   GNU General Public License for more details.                        *
*                                                                       *
*   You should have received a copy of the GNU General Public License   *
*   along with this program;                                            *
*   if not, write to the Free Software Foundation, Inc.,                *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA        *
*                                                                       *
*               home  : wxhexeditor.sourceforge.net                     *
*               email : spamjunkeater at gmail dot com                  *
*************************************************************************/

#define NANINT 0xFFFFFFFFFFFFFFFFLL
#include "HexDialogs.h"
#include <wx/progdlg.h>
#include "../hashlibpp/src/hashlibpp.h"

GotoDialog::GotoDialog( wxWindow* parent, uint64_t& _offset, uint64_t _cursor_offset, uint64_t _filesize, DialogVector *_myDialogVector=NULL ):GotoDialogGui(parent, wxID_ANY){
	offset = &_offset;
	cursor_offset = _cursor_offset;
	is_olddec =true;
	filesize = _filesize;
	m_textCtrlOffset->SetFocus();
	myDialogVector = _myDialogVector;
	if(myDialogVector != NULL ){
		m_hex->SetValue( myDialogVector->goto_hex );
		if( myDialogVector->goto_hex )
			m_textCtrlOffset->ChangeValue( wxString::Format(wxT("%"wxLongLongFmtSpec"X"),myDialogVector->goto_input) );
		else
			m_textCtrlOffset->ChangeValue( wxString::Format(wxT("%"wxLongLongFmtSpec"d"),myDialogVector->goto_input) );
		m_branch->Select( myDialogVector->goto_branch );
		}
	}

wxString GotoDialog::Filter( wxString text ){
	for( unsigned i = 0 ; i < text.Length() ; i++ ){
		if( m_dec->GetValue() ? isdigit( text.GetChar( i ) ) : isxdigit( text.GetChar( i ) ))
			continue;
		else
			text.Remove( i, 1);
		}
	return text;
	}

void GotoDialog::OnInput( wxCommandEvent& event ){
	if(!m_textCtrlOffset->IsModified())
        return;
    long insertionPoint = m_textCtrlOffset->GetInsertionPoint();
// TODO (death#1#): Copy/Paste/Drop & illegal key walking?
    m_textCtrlOffset->ChangeValue( Filter( m_textCtrlOffset->GetValue() ) );
    m_textCtrlOffset->SetInsertionPoint(insertionPoint);
	event.Skip(false);
	}
void GotoDialog::OnGo( wxCommandEvent& event ){
	wxULongLong_t wxul = 0;
	if( not m_textCtrlOffset->GetValue().ToULongLong( &wxul, m_dec->GetValue() ? 10 : 16 ))//Mingw32/64 workaround
		wxul = strtoull( m_textCtrlOffset->GetValue().ToAscii(), '\0', m_dec->GetValue() ? 10 : 16 );
	*offset = wxul;

	if(myDialogVector != NULL ){
		myDialogVector->goto_hex = m_hex->GetValue();
		myDialogVector->goto_branch = m_branch->GetSelection();
		myDialogVector->goto_input = wxul;
		}

	if( m_branch->GetSelection() == 1)
		*offset += cursor_offset;
	else if( m_branch->GetSelection() == 2)
		*offset = filesize - *offset;
	if( *offset < 0 )
		*offset = 0;

	EndModal( wxID_OK );
	}

void GotoDialog::OnConvert( wxCommandEvent& event ){
	wxULongLong_t wxul = 0;
	if( event.GetId() == m_dec->GetId() && !is_olddec ){	//old value is hex, new value is dec
		is_olddec = true;
		if( not m_textCtrlOffset->GetValue().ToULongLong( &wxul, 16 ) )//Mingw32/64 workaround
			wxul = strtoull( m_textCtrlOffset->GetValue().ToAscii(), '\0', 16 );
		*offset = wxul;
		m_textCtrlOffset->SetValue( wxString::Format( wxT("%"wxLongLongFmtSpec"u" ), wxul) );
		}
	else if( event.GetId() == m_hex->GetId() && is_olddec ){	//old value is dec, new value is hex
		is_olddec = false;
		if( not m_textCtrlOffset->GetValue().ToULongLong( &wxul, 10 ) )//Mingw32/64 workaround
			wxul = strtoull( m_textCtrlOffset->GetValue().ToAscii(), '\0', 10 );
		*offset = wxul;
		m_textCtrlOffset->SetValue( wxString::Format( wxT("%"wxLongLongFmtSpec"X") , wxul) );
		}
// TODO (death#1#): myDialogVector->goto_hex = 0;

	event.Skip(false);
	}

// TODO (death#1#):Paint 4 Find All
// TODO (death#1#):Make ComboBox remember old values
// TODO (death#1#):Remember options last state
FindDialog::FindDialog( wxWindow* _parent, FAL *_findfile, wxString title ):FindDialogGui( _parent, wxID_ANY, title){
	parent = static_cast< HexEditor* >(_parent);
	findfile = _findfile;
	m_comboBoxSearch->SetFocus();
	}

void FindDialog::EventHandler( wxCommandEvent& event ){
	parent->HighlightArray.Clear();
	if( event.GetId() == btnFind->GetId())
		OnFind();
	else if(event.GetId() == m_comboBoxSearch->GetId()){
		if( event.GetEventType() == 10041)
			OnFind();
		else
			chkUTF8->SetValue( not m_comboBoxSearch->GetValue().IsAscii() );
		}
	else if( event.GetId() == m_searchtype->GetId()){
		m_searchtype->GetSelection() == 1 ? chkMatchCase->Enable(false) : chkMatchCase->Enable(true) ;
		}
	else if( event.GetId() == btnFindAll->GetId() )
		OnFindAll();

	//Disables chkUTF8 setting by user.
	else if( event.GetId() == chkUTF8->GetId() ){
		chkUTF8->SetValue( not chkUTF8->GetValue( ));
		wxBell();
		}
	else
		wxBell();


	}

bool FindDialog::OnFind( bool internal ) {
	uint64_t found = NANINT;
	uint64_t search_size = 0;
	//prepare Operator
	unsigned options = 0;
	options |= m_searchtype->GetSelection() == 0 ? SEARCH_TEXT : SEARCH_HEX;
	options |= chkWrapAround->GetValue() ? SEARCH_WRAPAROUND : 0;
	options |= chkSearchBackwards->GetValue() ? SEARCH_BACKWARDS : 0;
	if(options & SEARCH_TEXT) {
		options |= chkUTF8->GetValue() ? SEARCH_UTF8 : 0;
		options |= chkMatchCase->GetValue() ? SEARCH_MATCHCASE : 0;
		if( m_comboBoxSearch->GetValue().IsAscii() )
			search_size = m_comboBoxSearch->GetValue().Len();
		else{
			search_size=0;
			while( m_comboBoxSearch->GetValue().ToUTF8()[search_size++]);
			search_size--;
			}
		found = FindText( m_comboBoxSearch->GetValue(), parent->CursorOffset()+1, options );
		}
	else { //SEARCH_HEX
		wxString hexval = m_comboBoxSearch->GetValue();

		for( unsigned i = 0 ; i < hexval.Len() ; i++ )
			if( !isxdigit( hexval[i] ) or hexval == ' ' ) { //Not hexadecimal!
				wxMessageBox(_("Search value is not hexadecimal!"), _("Format Error!"), wxOK, this );
				wxBell();
				return false;
				}
		//Remove all space chars and update the Search value
		while( hexval.find(' ') != -1 )
			hexval.Remove( hexval.find(' '),1);
		if( hexval.Len() % 2 )//there is odd hex value, must be even for byte search!
			hexval = wxChar('0')+hexval;
		m_comboBoxSearch->SetValue(hexval.Upper());

		wxMemoryBuffer search_binary = wxHexCtrl::HexToBin( m_comboBoxSearch->GetValue());
		search_size = search_binary.GetDataLen();
		found = FindBinary( search_binary, parent->CursorOffset()+1, options );
		}

	if( found != NANINT ) {
		parent->Goto( found );
		parent->Select( found,  found+search_size-1 );
		return true;
		}

	else if( !internal )
		wxMessageBox(_("Search value not found"), _("Nothing found!"), wxOK, this );
	return false;
	}

uint64_t FindDialog::FindText( wxString target, uint64_t start_from, unsigned options ){
	wxMemoryBuffer textsrc;
	if(not (options & SEARCH_MATCHCASE))
			target = target.Lower();

	if( target.IsAscii() ){
		textsrc.AppendData( target.ToAscii() , target.Length() );
		return FindBinary( textsrc, start_from, options );
		}
	else{//Search as UTF string.
		wxCharBuffer a = target.ToUTF8().data(); //Convert to UTF8 Binary
		int i=0;
		char *b=a.data();							//Silences errors
		while(b[i++] not_eq 0);					//Find stream size
		textsrc.AppendData( a , i-1 );//-1 for discard null termination char
		return FindBinary( textsrc, start_from, options|SEARCH_UTF8 );
		}
	}

// TODO (death#1#): New Find as "bool FindText/Bin( &uint64_t )
// TODO (death#1#): Implement Search_Backwards
uint64_t FindDialog::FindBinary( wxMemoryBuffer target, uint64_t from, unsigned options ){
	if( target.GetDataLen() == 0 )
		return NANINT;
	wxString msg= _("Finding matches... ");
	wxString emsg;
	wxProgressDialog progress_gauge(_("wxHexEditor Searching") , msg, 1000,  this, wxPD_SMOOTH|wxPD_REMAINING_TIME|wxPD_CAN_ABORT|wxPD_AUTO_HIDE );
	progress_gauge.SetWindowStyleFlag( progress_gauge.GetWindowStyleFlag()|wxSTAY_ON_TOP|wxMINIMIZE_BOX );
// TODO (death#1#): Search icon	//wxIcon search_ICON (?_xpm);
	//progress_gauge.SetIcon(search_ICON);

	uint64_t current_offset = from;
	int search_step = parent->FileLength() < MB ? parent->FileLength() : MB ;
	findfile->Seek( current_offset, wxFromStart );
	char* buffer = new char [search_step];
	if(buffer == NULL) return NANINT;
	// TODO (death#6#): insert error check message here
	int found = -1;
	int readed = 0;

	time_t ts,te;
	time (&ts);
	ts=te;
	uint64_t readspeed=0;
	//Search step 1: From cursor to file end.
	do{
		findfile->Seek( current_offset, wxFromStart );

		readed = findfile->Read( buffer , search_step );
		found = SearchAtBuffer( buffer, readed, static_cast<char*>(target.GetData()),target.GetDataLen(), options );//Makes raw search here
		if(found >= 0){
			if( options & SEARCH_FINDALL ){
				TagElement *mytag=new TagElement(current_offset+found, current_offset+found+target.GetDataLen()-1,wxEmptyString,*wxBLACK, wxColour(255,255,0,0) );
				parent->HighlightArray.Add(mytag);
				current_offset += found+target.GetDataLen(); //Unprocessed bytes
				readed=search_step; //to stay in loop
				}
			else{
				delete [] buffer;
				return current_offset+found;
				}
			}
		else{
			current_offset +=readed - target.GetDataLen() - 1; //Unprocessed bytes
			}

		time(&te);
		if(ts != te ){
				ts=te;
				emsg = msg + wxString::Format(_("\nSearch Speed : %d MB/s"), (current_offset-readspeed)/MB);
				readspeed=current_offset;
				}
		if( ! progress_gauge.Update(current_offset*1000/parent->FileLength(), emsg))		// update progress and break on abort
			break;

		}while(readed >= search_step); //indicate also file end.

	//Search step 2: From start to file end.
	if( options & SEARCH_WRAPAROUND ){
		current_offset = 0;
		do{
			findfile->Seek(current_offset, wxFromStart );
			readed = findfile->Read( buffer , search_step );
			if( readed + current_offset > from )
				search_step = readed + current_offset - from - 1;
			found = SearchAtBuffer( buffer, readed, static_cast<char*>(target.GetData()),target.GetDataLen(), options );//Makes raw search here
			if(found >= 0){
				delete [] buffer;
				return current_offset+found;
				}
			else
				current_offset +=readed - target.GetDataLen() - 1; //Unprocessed bytes
			}while(current_offset + readed < from); //Search until cursor
		}


	delete [] buffer;
	return NANINT;
	}

void FindDialog::OnFindAll( bool internal ) {
	parent->HighlightArray.Clear();

	unsigned options = SEARCH_FINDALL; //fill continue search until file and with this option.
	options |= m_searchtype->GetSelection() == 0 ? SEARCH_TEXT : SEARCH_HEX;
	options |= chkWrapAround->GetValue() ? SEARCH_WRAPAROUND : 0;
	options |= chkSearchBackwards->GetValue() ? SEARCH_BACKWARDS : 0;

	int mode = 0;
	if(options & SEARCH_TEXT) {
		mode = SEARCH_TEXT;
		options |= chkUTF8->GetValue() ? SEARCH_UTF8 : 0;
		options |= chkMatchCase->GetValue() ? SEARCH_MATCHCASE : 0;
		FindText( m_comboBoxSearch->GetValue(), 0, options );
		}

	else {
		mode = SEARCH_HEX;
		wxString hexval = m_comboBoxSearch->GetValue();
		//parent->Goto(0);
		for( unsigned i = 0 ; i < hexval.Len() ; i++ )
			if( !isxdigit( hexval[i] ) or hexval == ' ' ) { //Not hexadecimal!
				wxMessageBox(_("Search value is not hexadecimal!"), _("Format Error!"), wxOK, this );
				wxBell();
				}
		//Remove all space chars and update the Search value
		while( hexval.find(' ') != -1 )
			hexval.Remove( hexval.find(' '),1);
		if( hexval.Len() % 2 )//there is odd hex value, must be even for byte search!
			hexval = wxChar('0')+hexval;
		m_comboBoxSearch->SetValue(hexval.Upper());
		FindBinary( wxHexCtrl::HexToBin( m_comboBoxSearch->GetValue()), 0 ,options );
		}

	if(parent->HighlightArray.GetCount()==0 and not internal) {
		wxMessageBox(_("Search value not found"), _("Nothing found!"), wxOK, this );
		}
	else {
		//Is selection needed to show first tag?
		parent->Reload(); //To highlighting current screen
		parent->UpdateCursorLocation( parent->HighlightArray.Item(0)->start );

		wxUpdateUIEvent eventx( SEARCH_CHANGE_EVENT );
		parent->GetEventHandler()->ProcessEvent( eventx );
		if( not internal )
			wxMessageBox(wxString::Format(_("Found %d matches."),parent->HighlightArray.GetCount()), _("Find All Done!"), wxOK, this );
		}
	}


// TODO (death#9#): Implement better search algorithm. (Like one using OpenCL and one using OpenMP) :)
//WARNING! THIS FUNCTION WILL CHANGE BFR and/or SEARCH strings if SEARCH_MATCHCASE not selected as an option!
int FindDialog::SearchAtBuffer( char *bfr, int bfr_size, char* search, int search_size, unsigned options ){	// Dummy search algorithm\ Yes yes I know there are better ones but I think this enought for now.
	if( bfr_size < search_size )
		return -1;

	//UTF with no matched case handled here !!!SLOW!!!
	if(options & SEARCH_UTF8 and options & SEARCH_TEXT and not (options & SEARCH_MATCHCASE) ){
			wxString ucode;
			wxCharBuffer ubuf;
			for(int i=0 ; i < bfr_size - search_size + 1 ; i++ ){
				ucode = wxString::FromUTF8(bfr+i, search_size);
				ubuf = ucode.Lower().ToUTF8();
				if(! memcmp( ubuf, search, search_size ))	//if match found
					return i;
				}

		return -1;
		}

	//Make buffer lower if required.
	else if(options & SEARCH_TEXT and not (options & SEARCH_MATCHCASE) ){
		///Search text already lowered at FindText()
		//for( int i = 0 ; i < search_size; i++)
		//	search[i]=tolower(search[i]);

		///Make buffer low to match
		for( int i = 0 ; i < bfr_size; i++)
			bfr[i]=tolower(bfr[i]);

		//Disabled speedy code.
		if(0){
			//Search at no match case ASCII handled here
			char topSearch[search_size];
			for( int i = 0 ; i < search_size; i++)
				topSearch[i]=tolower(search[i]);

			for(int i=0 ; i < bfr_size - search_size + 1 ; i++ ){
				if( bfr[i] == search[0] or bfr[i] == topSearch[0] ){
					//partial lowering code
					for( int j = i ; i < bfr_size; i++)
						bfr[j]=tolower(bfr[j]);

					if(! memcmp( bfr+i, search, search_size ))	//if match found
						return i;
						}
					}
			return -1;
			}
		}

	//Search at buffer
	for(int i=0 ; i < bfr_size - search_size + 1 ; i++ ){
		if(! memcmp( bfr+i, search, search_size ))	//if match found
			return i;
		}
	return -1;
	}

ReplaceDialog::ReplaceDialog( wxWindow* parent, FAL *find_file, wxString title ):FindDialog( parent, find_file, title ){
	m_comboBoxReplace->Show();
	m_static_replace->Show();
	btnReplace->Show();
	btnReplaceAll->Show();
	Fit();
	}

int ReplaceDialog::OnReplace( bool internal ){
	if( parent->select->IsState( parent->select->SELECT_FALSE ) ) {
		if( OnFind( internal ) == false )
			return 0;
		else
			return -1;
		}

	else{
		if( m_searchtype->GetSelection() == 0 ){//text search
			if( parent->select->GetSize() == m_comboBoxReplace->GetValue().Len() ){
				parent->FileAddDiff( parent->CursorOffset(), m_comboBoxReplace->GetValue().ToAscii(), m_comboBoxReplace->GetValue().Len());
				parent->select->SetState( parent->select->SELECT_FALSE );
				parent->Reload();
				return 1;
				}
			else{
				wxMessageBox(_("Search and Replace sizes are not equal!\nReplacing with differnet sizez are avoided."), _("Error!"), wxOK, this);
				return 0;
				}
			}
		else{ //hex search
			wxMemoryBuffer search_binary = wxHexCtrl::HexToBin( m_comboBoxReplace->GetValue());
			if( parent->select->GetSize() == search_binary.GetDataLen() ){
				parent->FileAddDiff( parent->CursorOffset(), static_cast<char*>(search_binary.GetData()) ,search_binary.GetDataLen() );
				parent->select->IsState( parent->select->SELECT_FALSE );
				parent->Reload();
				return 1;
				}
			else{
				wxMessageBox(_("Search and Replace sizes are not equal!\nReplacing with differnet sizez are avoided."), _("Error!"), wxOK, this);
				return 0;
				}
			}
		}
	}

void ReplaceDialog::OnReplaceAll( void ){
	//First search all file with find all to detect locations.
	OnFindAll( true );
	//Now Highlight array has matches. We could replace them with replace string.
	for( uint32_t i=0 ; i < parent->HighlightArray.Count() ; i++ ){
		if( m_searchtype->GetSelection() == 0 ) //text search
				parent->FileAddDiff( parent->HighlightArray.Item(i)->start,
											m_comboBoxReplace->GetValue().ToAscii(),
											m_comboBoxReplace->GetValue().Len());
		else{ //hex search
			wxMemoryBuffer search_binary = wxHexCtrl::HexToBin( m_comboBoxReplace->GetValue());
			parent->FileAddDiff( parent->HighlightArray.Item(i)->start,
										static_cast<char*>(search_binary.GetData()),
										search_binary.GetDataLen() );
			}

		if( parent->HighlightArray.Count() < 20 )						 //if there is too much matches,
			parent->Goto( parent->HighlightArray.Item(i)->start ); // this make program unresponsive and slow.
		}

	if( parent->HighlightArray.Count() > 0){
		parent->Goto( parent->HighlightArray.Item(0)->start );
		parent->Refresh();
		wxMessageBox(wxString::Format(_("%d records changed."), parent->HighlightArray.Count() ), _("Info!"), wxOK, this);
		}
	}

void ReplaceDialog::EventHandler( wxCommandEvent& event ){
	int id = event.GetId();
	if( id == btnFind->GetId() )
		OnFind();
	else if( id == btnFindAll->GetId() )
		OnFindAll();
	else if( id == btnReplace->GetId() )
		OnReplace();
	else if( id == btnReplaceAll->GetId() )
		OnReplaceAll();
	else
		wxBell();
	}

CopyAsDialog::CopyAsDialog( wxWindow* _parent, FAL *file, Select *select_ , ArrayOfTAG* MainTagArray_):CopyAsDialogGui( _parent , wxID_ANY){
	parent = static_cast< HexEditor* >( _parent );
	spnBytePerLine->SetValue( parent->BytePerLine() );
	select = select_;
	copy_file = file;
	MainTagArray=MainTagArray_;
	bool IsBigEndian;
	//parent->interpreter->chkBigEndian->GetValue(); Does this approach better than remembering old value?
	wxConfigBase::Get()->Read( _T("CopyAsBigEndian"), &IsBigEndian );
	chkBigEndian->SetValue( IsBigEndian );

	int CopyAsFunction;
	if (wxConfigBase::Get()->Read(_T("CopyAsSelectedFunction"), &CopyAsFunction))
		chcCopyAs->SetSelection( CopyAsFunction );
	PrepareOptions( CopyAsFunction );
	}

void CopyAsDialog::PrepareOptions( int SelectedFunction ){
//wxT("Full Text"), wxT("Special Hex"), wxT("HTML"), wxT("C/C++"), wxT("Assembler")
	chcOption->Enable(SelectedFunction > 0);
	chcOption->Clear();

	if( SelectedFunction == 0){
		chcOption->Insert(_("Not Available"),0);
		chkOffset->Enable(true);
		chkHex->Enable(true);
		chkText->Enable(true);
		}
	else if( SelectedFunction == 1){ // Hex
		chcOption->Insert(_("Raw Hex"),0);
		chcOption->Insert(_("With Space"),1);
		chcOption->Insert(_("Quad Hex"),2);
		chcOption->Insert(_("with 0x"),3);
		chcOption->Insert(_("with 0x and period"),4);
		chkOffset->Enable(false);
		chkHex->Enable(false);
		chkText->Enable(false);

		}
	else if( SelectedFunction == 2){ // HTML
		chcOption->Insert(_("HTML format"),0);
		chcOption->Insert(_("HTML with TAGs"),1);
		chcOption->Insert(_("phpBB forum style"),2);
		chcOption->Insert(_("WiKi format"),3);
		chcOption->Insert(_("WiKi with TAGs"),4);
		chkOffset->Enable(true);
		chkHex->Enable(true);
		chkText->Enable(true);
		}
	else if( SelectedFunction >= 3){ // C/C++/ASM Sources
		chcOption->Insert(_("8bit Byte "),0);
		chcOption->Insert(_("16bit Words"),1);
		chcOption->Insert(_("32bit Dwords"),2);
		chcOption->Insert(_("64bit Qwords"),3);
		chkOffset->Enable(false);
		chkHex->Enable(false);
		chkText->Enable(false);
		}
	int option=0;
	wxConfigBase::Get()->Read(_T("CopyAsSelectedOptionOfFunction") + wxString::Format(wxT("%d"),SelectedFunction), &option);
	chcOption->SetSelection( option );

	//Enable big endian checkbox if multi byte representation selected for C/C++/ASM sources.
	chkBigEndian->Enable( chcCopyAs->GetSelection() >=3 and chcOption->GetSelection() > 0 );

	wxYield();
	this->GetSizer()->Fit(this);
	this->GetSizer()->Layout();
	}

void CopyAsDialog::EventHandler( wxCommandEvent& event ){
	int id = event.GetId();
	if( id == wxID_CANCEL )
		Destroy();
	else if( id == wxID_OK ){
		Copy();
		Destroy();
		}
	else if( id == chcCopyAs->GetId() ){
		int SelectedFunction =  chcCopyAs->GetSelection();
		PrepareOptions(SelectedFunction);

		int option;
		//Adjustinf selection part
		if (wxConfigBase::Get()->Read(_T("CopyAsSelectedOptionOfFunction") + wxString::Format(wxT("%d"),SelectedFunction), &option))
			chcOption->SetSelection( option );
		else
			chcOption->SetSelection(0);

		wxConfigBase::Get()->Write( _T("CopyAsSelectedFunction"), SelectedFunction );
		}
	else if( id == chcOption->GetId() ){
		wxConfigBase::Get()->Write( _T("CopyAsSelectedOptionOfFunction") + wxString::Format(wxT("%d"), chcCopyAs->GetSelection()), chcOption->GetSelection() );

		//Enable big endian checkbox if multi byte representation selected for C/C++/ASM sources.
		chkBigEndian->Enable( chcCopyAs->GetSelection() >=3 and chcOption->GetSelection() > 0 );
		}
	else if( id == chkBigEndian->GetId() ){
		wxConfigBase::Get()->Write( _T("CopyAsBigEndian"), chkBigEndian->GetValue() );
		}

	}

wxString CopyAsDialog::GetDigitFormat( void ){
	int digit_count=0;
	int base=parent->GetIsHexOffset() ? 16 : 10;
	while(select->GetEnd() > pow(base,++digit_count));
	if( digit_count < 6)
		digit_count=6;
	if( base == 10 )
		return wxT("%")+wxString::Format( wxT("%02d"),digit_count) + wxLongLongFmtSpec + wxT("u   ");
	else
		return wxT("0x%")+wxString::Format( wxT("%02d"),digit_count) + wxLongLongFmtSpec + wxT("X   ");
	}

void CopyAsDialog::PrepareFullText( wxString& cb, wxMemoryBuffer& buff ){
	unsigned BytePerLine = spnBytePerLine->GetValue();
	for(unsigned current_offset = 0; current_offset < select->GetSize() ; current_offset += BytePerLine){
		if(chkOffset->GetValue()){
			cb += wxString::Format(GetDigitFormat() , select->GetStart() + current_offset );
			}

		//Add 16 hex val
		if(chkHex->GetValue()){
			for(unsigned i = 0 ; i < BytePerLine ; i++){
				if( i + current_offset < select->GetSize())
					cb+= wxString::Format( wxT("%02X "), (unsigned char)buff[ current_offset + i] );
				else
					cb+= wxT("   "); //fill with zero to make text area at proper location
				}
			cb += wxT("  ");
			}

		if(chkText->GetValue()){
		//Add 16 Ascii rep
			char chr;
			for(unsigned i = 0 ; i < BytePerLine ; i++){
				if( i + current_offset < select->GetSize()){
					//Char filter for ascii
					chr = buff[ current_offset + i];
					if( (chr !=173) && ( (chr>31 && chr<127) || chr>159) )
						cb+= wxString::FromAscii( buff[ current_offset + i] );
					else
						cb+= wxString::FromAscii( '.' );
					}
				}
			}
		cb += wxT("\n");
		}
	}

void CopyAsDialog::PrepareFullTextWithTAGs( wxString& cb, wxMemoryBuffer& buff, wxString startup=wxEmptyString ){
	unsigned BytePerLine = spnBytePerLine->GetValue();
	wxString last_color_hex,last_color_text;
	cb += startup+wxT("TAG List:\n");
	for( unsigned i =0 ; i < MainTagArray->Count() ; i++ ){
		TagElement *tag = MainTagArray->Item(i);
		if(( tag->start <  select->GetStart() and tag->end   >= select->GetStart() ) or
			( tag->start >= select->GetStart() and tag->start <= select->GetEnd() ) or
			( tag->end   >= select->GetStart() and tag->end   <= select->GetEnd() ) ){

			cb += startup+wxT("<span style=\"background-color:") + tag->SoftColour( tag->NoteClrData.GetColour() ).GetAsString(wxC2S_HTML_SYNTAX) +
					wxT(";color:") + tag->FontClrData.GetColour().GetAsString(wxC2S_HTML_SYNTAX) +  wxT(";\">") + tag->tag +wxT("</span>\n");
			}
		}
	cb += startup+wxT("\n");

	for(unsigned current_offset = 0; current_offset < select->GetSize() ; current_offset += BytePerLine){
		if(chkOffset->GetValue()){
			cb += startup + wxString::Format(GetDigitFormat()  , select->GetStart() + current_offset );
			}

		if(chkHex->GetValue()){
			//Add 16 hex val
			//Check for middle TAG selection starts
			if( current_offset == 0 )
				for( unsigned j = 0 ; j< MainTagArray->Count() ; j++ ){
					TagElement *tg = MainTagArray->Item(j);
					if( tg->isCover( select->GetStart() ) )
						last_color_hex = last_color_hex = tg->SoftColour(tg->NoteClrData.GetColour()).GetAsString(wxC2S_HTML_SYNTAX);
				}

			if( last_color_hex.Len() )
				cb += wxT("<span style=\"background-color:") + last_color_hex + wxT(";\">");

			for(unsigned i = 0 ; i < BytePerLine ; i++){

				//TAG Paint Loop
				for( unsigned j = 0 ; j< MainTagArray->Count() ; j++ ){
					TagElement *tg = MainTagArray->Item(j);
					if( MainTagArray->Item(j)->start == i + current_offset + select->GetStart()){
						last_color_hex = tg->SoftColour(tg->NoteClrData.GetColour()).GetAsString(wxC2S_HTML_SYNTAX);
						cb += wxT("<span style=\"background-color:") + last_color_hex + wxT(";\">");
						}
					if( MainTagArray->Item(j)->end +1== i + current_offset + select->GetStart() ){
						cb += wxT("</span>");
						last_color_hex = wxEmptyString;
						}
					}

				if( i + current_offset < select->GetSize())
					cb+= wxString::Format( wxT("%02X "), (unsigned char)buff[ current_offset + i] );
				else{
					if(last_color_hex.Len() )
						cb += wxT("</span>");
					last_color_hex = wxEmptyString;
					cb+= wxT("   "); //fill with zero to make text area at proper location
					}
				//This avoid to paint text section.
				if(last_color_hex.Len() and i==BytePerLine-1)
					cb += wxT("</span>");
				}
		cb += wxT("  ");
		}

		if(chkText->GetValue()){
			//Add 16 Ascii rep
			char chr;

			if( current_offset == 0 )
				for( unsigned j = 0 ; j< MainTagArray->Count() ; j++ ){
					TagElement *tg = MainTagArray->Item(j);
					if( tg->isCover( select->GetStart() ) )
						last_color_text = tg->SoftColour(tg->NoteClrData.GetColour()).GetAsString(wxC2S_HTML_SYNTAX);
				}

			if( last_color_text.Len() )
				cb += wxT("<span style=\"background-color:") + last_color_text + wxT(";\">");
			for(unsigned i = 0 ; i < BytePerLine ; i++){
				if( i + current_offset < select->GetSize()){

								//TAG Paint Loop
				for( unsigned j = 0 ; j< MainTagArray->Count() ; j++ ){
					TagElement *tg = MainTagArray->Item(j);
					if( MainTagArray->Item(j)->start == i + current_offset + select->GetStart()){
						last_color_text = tg->SoftColour( tg->NoteClrData.GetColour()).GetAsString(wxC2S_HTML_SYNTAX);
						cb += wxT("<span style=\"background-color:") + last_color_text + wxT(";\">");
						}
					if( MainTagArray->Item(j)->end +1== i + current_offset + select->GetStart()){
						cb += wxT("</span>");
						last_color_text = wxEmptyString;
						}
					}

					//Char filter for ascii
					chr = buff[ current_offset + i];
					if( (chr !=173) && ( (chr>31 && chr<127) || chr>159) )
						cb+= wxString::FromAscii( buff[ current_offset + i] );
					else
						cb+= wxString::FromAscii( '.' );
					}
				if(last_color_text.Len() and i==BytePerLine-1)
					cb += wxT("</span>");
				}
			}
		cb += wxT("\n");
		}
	cb += startup + wxT("\n");
}

void CopyAsDialog::Copy( void ){
	chcOption->GetSelection();
	if( not select->IsState( select->SELECT_FALSE ) ) {
		int BytePerLine = spnBytePerLine->GetValue();
		wxString cb;
		uint64_t RAM_limit = 10*MB;
		wxMemoryBuffer buff;
		if(select->GetSize() > RAM_limit) {
			wxMessageBox( _("Selected block bigger than limit." ),_("Error!"), wxOK|wxICON_ERROR , this);
			return;
			}

		copy_file->Seek( select->GetStart(), wxFromStart );
		buff.UngetWriteBuf( copy_file->Read( static_cast<unsigned char*>(buff.GetWriteBuf(select->GetSize())), select->GetSize() ) );

		if( chcCopyAs->GetSelection() == 0 ){ //Full text copy
			PrepareFullText( cb, buff );
			}
		else if( chcCopyAs->GetSelection() == 1){
			wxString HexFormat;
			bool quad=false;
			switch( chcOption->GetSelection()){
				case 0: HexFormat=wxT("%02X");break; //Raw Hex
				case 1: HexFormat=wxT("%02X ");break; //Standard
				case 2: HexFormat=wxT("%02X"); quad=true; break; //Quad
				case 3: HexFormat=wxT("0x%02X ");break; //Ox
				case 4: HexFormat=wxT("0x%02X, ");break; //Ox with period
				}

			for(unsigned current_offset = 0; current_offset < select->GetSize() ; current_offset ++){
				cb+= wxString::Format( HexFormat, (unsigned char)buff[ current_offset ] );
				if( quad and ((current_offset+1)%2)==0)
					cb += wxT(" ");
				if(( (current_offset+1) % BytePerLine)==0 )
					cb += wxT("\n");
				}
			}

		else if( chcCopyAs->GetSelection() == 2){//Internet
			if( chcOption->GetSelection() == 0 ){ //html
//				<html><head><title></title></head><body>
//				<pre>
//				<code style="color:#000000;background-color:#FFFFFF">0000E973   1375 6E5A 8696 553A 01C9 51A2 F244 90BD   .unZ..U:.ÉQ¢òD.½</code>
//				<code style="color:#000000;background-color:zebra?;">0000E983   1375 6E5A 8696 553A 01C9 51A2 F244 90BD   .unZ..U:.ÉQ¢òD.½</code>
//				</pre>
//				<font size="-3">Generated by <a href="http://wxhexeditor.sourceforge.net/">wxHexEditor</a></font>
//				</body></html>
				cb += wxT("<pre><code style=\"color:#000000;\">");
				PrepareFullText( cb, buff );
				cb += wxT("</code></pre><font size=\"-3\">Generated by <a href=\"http://wxhexeditor.sourceforge.net/\">wxHexEditor</a></font>");
				}

			else if( chcOption->GetSelection() == 1 ){ //HTML with Tags
				cb += wxT("<pre><code style=\"color:#000000;\">");
				PrepareFullTextWithTAGs( cb, buff );
				cb += wxT("</code></pre><font size=\"-3\">Generated by <a href=\"http://wxhexeditor.sourceforge.net/\">wxHexEditor</a></font>");
				}

			else if( chcOption->GetSelection() == 2 ){ //phpBB Forum
				cb+= wxT("[code]");
				PrepareFullText( cb, buff );
				cb += wxT("[/code]Generated by [url=http://wxhexeditor.sourceforge.net/]wxHexEditor[/url]\n");
				}

			else if( chcOption->GetSelection() == 3 ){ //WiKi format
				cb+= wxT("<pre>");
				PrepareFullText( cb, buff );
				cb += wxT("</pre> Generated by [http://wxhexeditor.sourceforge.net/ wxHexEditor]\n");
				}
			else if( chcOption->GetSelection() == 4 ){ //WiKi with TAGs
				PrepareFullTextWithTAGs( cb, buff, wxT(" ") );
				cb += wxT(" Generated by [http://wxhexeditor.sourceforge.net/ wxHexEditor]\n");
				}
			}
		else if( chcCopyAs->GetSelection() == 3){//C/C++
			unsigned HexSize = pow( 2, chcOption->GetSelection());
			unsigned count = select->GetSize()/HexSize;
			cb+=wxT("// Generated by wxHexEditor //\n");
			switch( HexSize ){
				case 1: cb += wxT("int8_t"); break;
				case 2: cb += wxT("int16_t"); break;
				case 4: cb += wxT("int32_t"); break;
				case 8: cb += wxT("int64_t"); break;
				}

			cb+=wxString::Format( wxT(" hexData[0x%x] = {\n  "), count );
			bool bigEndianSwapReq = chkBigEndian->GetValue() and chcOption->GetSelection();//No big endian for 8 bit
			int b;
			int limit=(bigEndianSwapReq ? -1 : HexSize);
			int incr = (bigEndianSwapReq ? -1 : +1);
			for(unsigned current_offset = 0; current_offset < count ; current_offset ++){
				b = (bigEndianSwapReq ? HexSize-1 : 0);
				cb+= wxT("0x");
				for(; b not_eq limit ; b+=incr)
					cb+= wxString::Format( wxT("%02X"), *reinterpret_cast<unsigned char*>(	buff.GetData()+current_offset*HexSize+b ));
				cb+= wxT(", ");
//				switch( HexSize ){
//					case 1: cb+= wxString::Format( wxT("0x%02X, "), *reinterpret_cast<unsigned char*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 2: cb+= wxString::Format( wxT("0x%04X, "), *reinterpret_cast<unsigned short*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 4: cb+= wxString::Format( wxT("0x%08X, "), *reinterpret_cast<unsigned int*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 8: cb+= wxString::Format( wxT("0x%016"wxLongLongFmtSpec"X, "), *reinterpret_cast<uint64_t*>( buff.GetData()+current_offset*HexSize )); break;
//					}
				if(( (current_offset+1) % (BytePerLine/HexSize)==0 ) and current_offset not_eq count)
						cb += wxT("\n  ");
				}
			cb=cb.BeforeLast(',')+wxT(" }\n");
		   }
		else if( chcCopyAs->GetSelection() == 4){//ASM
			unsigned HexSize = pow( 2, chcOption->GetSelection());
			unsigned count = select->GetSize()/HexSize;
			cb+=wxT(";Generated by wxHexEditor");

			wxString HexFormat;
			switch( HexSize ){
				case 1: HexFormat += wxT("\ndb "); break;
				case 2: HexFormat += wxT("\ndw "); break;
				case 4: HexFormat += wxT("\ndd "); break;
				case 8: HexFormat += wxT("\ndq "); break;
				}

			bool bigEndianSwapReq = chkBigEndian->GetValue() and chcOption->GetSelection();//No big endian for 8 bit
			int b;
			int limit=(bigEndianSwapReq ? -1 : HexSize);
			int incr = (bigEndianSwapReq ? -1 : +1);
			for(unsigned current_offset = 0; current_offset < count ; current_offset ++){
				if( current_offset % (BytePerLine/HexSize)==0 )
					cb += HexFormat;
				b = (bigEndianSwapReq ? HexSize-1 : 0);
				cb+= wxT("0");
				for(; b not_eq limit ; b+=incr)
					cb+= wxString::Format( wxT("%02X"), *reinterpret_cast<unsigned char*>(	buff.GetData()+current_offset*HexSize+b ));
				cb+= wxT("h ");
//				switch( HexSize ){
//					case 1: cb+= wxString::Format( wxT("0%02Xh "), *reinterpret_cast<unsigned char*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 2: cb+= wxString::Format( wxT("0%04Xh "), *reinterpret_cast<unsigned short*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 4: cb+= wxString::Format( wxT("0%08Xh "), *reinterpret_cast<unsigned int*>(	buff.GetData()+current_offset*HexSize )); break;
//					case 8: cb+= wxString::Format( wxT("0%016"wxLongLongFmtSpec"Xh "), *reinterpret_cast<uint64_t*>( buff.GetData()+current_offset*HexSize )); break;
//					}
				}
			cb+=wxT('\n');
			}

		if(wxTheClipboard->Open()) {
//					if (wxTheClipboard->IsSupported( wxDF_TEXT )){
			wxTheClipboard->Clear();
			int isok = wxTheClipboard->SetData( new wxTextDataObject( cb ));
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
			}
		}
	}

CompareDialog::CompareDialog( wxWindow* parent_ ):CompareDialogGui(parent_, wxID_ANY){
	parent = static_cast< HexEditorFrame* >(parent_);
	}

bool CompareDialog::Compare( wxFileName fl1, wxFileName fl2, bool SearchForDiff, int StopAfterNMatch, wxFileName flsave ){
	if(not fl1.IsFileReadable()){
		wxMessageBox( _("Error, File #1 is not readable.") );
		return false;
		}
	if(not fl2.IsFileReadable() ){
		wxMessageBox( _("Error, File #2 is not readable.") );
		return false;
		}
//	if( flsave not_eq wxEmptyString ){
//		if(not flsave.IsFileWritable() )
//			wxMessageBox( _("Error, Save File is not writeable.") );
//			return false;
//		}

	wxFFile f1,f2,fs;

	if( not f1.Open( fl1.GetFullPath() ) ){
		wxMessageBox( _("Error, File #1 cannot open." ) );
		return false;
		}
	if( not f2.Open( fl2.GetFullPath() ) ){
		wxMessageBox( _("Error, File #2 cannot open." ) );
		return false;
		}

	if( flsave not_eq wxEmptyString )
		if( not fs.Open( flsave.GetFullPath(), wxT("w") ) ){
			wxMessageBox( _("Error, Save File cannot open." ) );
			return false;
			}
	wxProgressDialog pdlg( _("Compare Progress"), _("Comparison in progress"), 100, this, wxPD_AUTO_HIDE | wxPD_APP_MODAL|wxPD_CAN_ABORT/*|wxPD_CAN_SKIP*/|wxPD_REMAINING_TIME); //SKIP not ready and Buggy
	pdlg.Show();

	wxMemoryBuffer buff1,buff2;
	int diffBuff[1*MB];
	int diffHit = 0;
	bool diff=false;
	uint64_t drange = wxMin( f1.Length() , f2.Length() );
	if(drange == 0)
		drange++; //to avoid Gauge zero div error

	for( uint64_t mb = 0 ; not (f1.Eof() or f2.Eof()) ; mb+=MB){
		buff1.UngetWriteBuf( f1.Read(buff1.GetWriteBuf( MB ),MB) );
		buff2.UngetWriteBuf( f2.Read(buff2.GetWriteBuf( MB ),MB) );
		for( int i = 0 ; i < wxMin( buff1.GetDataLen(), buff2.GetDataLen()); i ++ ){
			if((buff1[i] not_eq buff2[i]) == SearchForDiff){
				if(not diff){
#ifdef _DEBUG_
			std::cout << "Diff Start " << mb+i << " to " ;
#endif
					diff=true;
					diffBuff[diffHit++]=mb+i;
					}

				//this adds latest diff stream to array if one file ends
				if(f1.Eof() or f2.Eof() )
					if(i+1 == wxMin( buff1.GetDataLen(), buff2.GetDataLen()))
						diffBuff[diffHit++]=mb+i;
				}
			else{//bytes are eq.
				if(diff){
#ifdef _DEBUG_
			std::cout << mb+i-1 << std::endl;
#endif
					diff=false;
					diffBuff[diffHit++]=mb+i-1;

					//Some says we do not use goto in a good program.
					//But I don't know better way to break double for loop.
					//Might be better to use f1.Seek() to end with break...
					if( --StopAfterNMatch == 0 )
						goto BreakDoubleFor;
					}
				}
			}
		bool skip=false;
		if( not pdlg.Update( (mb*100)/drange, wxEmptyString, &skip) ){
			f1.Close();
			f2.Close();
			return false;
			}

		if(skip){//Not enabled yet.
			goto BreakDoubleFor;
			}

		}
BreakDoubleFor:

	pdlg.Show( false );

	if( flsave not_eq wxEmptyString ){
		wxString ln = _("File #1 : ") + fl1.GetFullPath() + wxT("\n")+_("File #2 : ") + fl2.GetFullPath() + wxT("\n\n");
		fs.Write( ln );
		wxString line;
		for(int i = 0 ; i < diffHit-1 ; i+=2){
			line = wxString::Format( _("%s found %"wxLongLongFmtSpec"u - %"wxLongLongFmtSpec"u \t Total : %"wxLongLongFmtSpec"u bytes.\n"), ( SearchForDiff ? wxT("Diff"):wxT("Match")), diffBuff[i] , diffBuff[i+1], diffBuff[i+1]-diffBuff[i]+1 );
			fs.Write( line );
			}

		if( f1.Length() not_eq f2.Length() ){
			if( f1.Length() > f2.Length() )
				line =  wxString::Format( _("\nFile #2 ends at offset %"wxLongLongFmtSpec"u. File #1 has extra %"wxLongLongFmtSpec"u bytes.\n"),f2.Length(), f1.Length() - f2.Length() );

			else
				line =  wxString::Format( _("\nFile #1 ends at offset %"wxLongLongFmtSpec"u. File #2 has extra %"wxLongLongFmtSpec"u bytes.\n"),f1.Length(), f2.Length() - f1.Length() );

			fs.Write( line );
			}
		}
	int file1size= f1.Length();
	int file2size= f2.Length();
	f1.Close();
	f2.Close();
	fs.Close();

	HexEditor* hexeditor1 = parent->OpenFile( fl1 );
	HexEditor* hexeditor2 = parent->OpenFile( fl2 );
	if(hexeditor1 != NULL and hexeditor2 != NULL){
		for(int i = 0 ; i < diffHit-1 ; i+=2){
			TagElement *mytag=new TagElement(diffBuff[i], diffBuff[i+1],wxEmptyString,*wxBLACK, *wxRED );
			hexeditor1->CompareArray.Add(mytag);
			hexeditor2->CompareArray.Add(mytag);
			}
		if( file1size not_eq file2size ){
			if( file1size > file2size ){
				TagElement *mytag=new TagElement(file2size, file1size,_("This part doesn't exist at compared file"),*wxBLACK, *wxCYAN );
				hexeditor1->CompareArray.Add(mytag);
				}
			else{
				TagElement *mytag=new TagElement(file1size, file2size,_("This part doesn't exist at compared file"),*wxBLACK, *wxCYAN );
				hexeditor2->CompareArray.Add(mytag);
				}
			}
		//Is selection needed to show first tag?
		hexeditor1->Reload(); //To highlighting current screen
		hexeditor2->Reload(); //To highlighting current screen
		if( hexeditor1->CompareArray.Count() > 0 )
			hexeditor1->UpdateCursorLocation( hexeditor1->CompareArray.Item(0)->start );
		wxUpdateUIEvent eventx( COMPARE_CHANGE_EVENT );
		hexeditor1->GetEventHandler()->ProcessEvent( eventx );
		}

	return true;
	}

void CompareDialog::OnFileChange( wxFileDirPickerEvent& event ){
	if( filePick1->GetPath() not_eq wxEmptyString and filePick2->GetPath() not_eq wxEmptyString)
		btnCompare->Enable(true);
	else
		btnCompare->Enable(false);
	}
// TODO (death#1#): Drag Drop file change event!
void CompareDialog::EventHandler( wxCommandEvent& event ){
#ifdef _DEBUG_
	std::cout << "CompareDialog::EventHandler()" << std::endl;
#endif
	if(event.GetId() == wxID_CANCEL)
		Destroy();
	else if(event.GetId() == btnCompare->GetId()){
		if( not filePick1->GetPath().IsEmpty() and not filePick2->GetPath().IsEmpty()){
			if( checkSaveResults->GetValue() and filePickSave->GetPath().IsEmpty() ){
				wxMessageBox( _("Error, Save File is not selected.") );
				return;
				}
			///Note:Triggers stack Overflow on windows. Use bigger stack...
			if( Compare( filePick1->GetPath(),			//First file.
						filePick2->GetPath(),			//Second file to compare.
						m_radioDifferent->GetValue(), //Compare diffs or same bytes option.
						(checkStopCompare->GetValue() ? spinStopCompare->GetValue() : 0),	//Stop after N Matches. 0 means unlimited.
						(checkSaveResults->GetValue() ? filePickSave->GetPath() : wxT("") ))		//comparison result save path.
																												)
				Destroy();
			}
		else
			wxBell();
		}
	else if( event.GetId() == checkStopCompare->GetId() ){
		spinStopCompare->Enable(event.IsChecked());
		}

	else if( event.GetId() == checkSaveResults->GetId() ){
		filePickSave->Enable(event.IsChecked());
		}
	}

ChecksumDialog::ChecksumDialog( wxWindow* parent_ ):ChecksumDialogGui(parent_, wxID_ANY){
	parent = static_cast< HexEditorFrame* >(parent_);
	bool active_hex = parent->GetActiveHexEditor() not_eq NULL;
	chkFile->Enable(active_hex);
	chkFile->SetValue(active_hex);
	filePick->Enable(not active_hex);
	btnCalculate->Enable( active_hex );
	}

void ChecksumDialog::EventHandler( wxCommandEvent& event ){
#ifdef _DEBUG_
	std::cout << "ChecksumDialog::EventHandler()" << std::endl;
#endif
	unsigned options=0;
	options |= (chkMD5->GetValue()    ? MD5    : 0);
	options |= (chkSHA1->GetValue()   ? SHA1   : 0);
	options |= (chkSHA256->GetValue() ? SHA256 : 0);
	options |= (chkSHA384->GetValue() ? SHA384 : 0);
	options |= (chkSHA512->GetValue() ? SHA512 : 0);

	if(event.GetId() == wxID_CANCEL)
		Destroy();

	else if(event.GetId() == btnCalculate->GetId()){
		wxString msg;
		if( chkFile->GetValue() ){
			msg = CalculateChecksum( *parent->GetActiveHexEditor()->myfile, options );
			if(msg not_eq wxEmptyString)
				wxMessageBox( _("For currently active file\n")+msg, _("Checksum Results") );
			}
		else if( filePick->GetPath() not_eq wxEmptyString ){
			wxFileName fl( filePick->GetPath() );
			FAL f( fl );
			msg = CalculateChecksum( f, options );
			f.Close();
			if(msg not_eq wxEmptyString)
				wxMessageBox( wxString(_("File: "))+filePick->GetPath()+wxT("\n\n")+msg, _("Checksum Results") );
			}
		//TODO: Copy to clipboard?
		//wxClipboard << msg
		}
	else if(event.GetId() == chkFile->GetId() ){
		filePick->Enable( not event.IsChecked() );
		}

	if( options == 0 or not ((filePick->GetPath() not_eq wxEmptyString) or chkFile->GetValue()) )
		btnCalculate->Enable(false);
	else
		btnCalculate->Enable(true);
	}

void ChecksumDialog::OnFileChange( wxFileDirPickerEvent& event ){
	wxCommandEvent e;
	EventHandler( e );
	}
wxString ChecksumDialog::CalculateChecksum(FAL& f, unsigned options){
	f.Seek(0);
	wxString msg = _("Please wait while calculating checksum.");
	wxProgressDialog mypd(_("Calculating Checksum"), msg , 1000, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_CAN_ABORT|wxPD_REMAINING_TIME);
	mypd.Show();

	if( 0 ){	//THis is more compact code but it gives seg fault if 4+ hash selected. Why?
//		checksum_options_strings = { "MD5","SHA1","SHA256","SHA384","SHA512" };
//		unsigned NumBits=0;
//		for( unsigned i = 0; i < 16 ; i ++ ){
//			NumBits += (options>>i)&0x1;
//			}
//			hashwrapper **myhash= (hashwrapper**) new char[NumBits];
//			unsigned i=0;
//			if( options & MD5    ) myhash[i++]= new md5wrapper();
//			if( options & SHA1   ) myhash[i++]= new sha1wrapper();
//			if( options & SHA256 ) myhash[i++]= new sha256wrapper();
//			if( options & SHA384 ) myhash[i++]= new sha384wrapper();
//			if( options & SHA512 ) myhash[i++]= new sha512wrapper();
//
//			for(i = 0 ; i < NumBits ; i++)
//				myhash[i]->resetContext();
//
//			int rd=MB;
//			unsigned char buff[MB];
//			while(rd == MB){
//				rd = f.Read( buff, MB );
//				for( i = 0 ; i < NumBits ; i++) myhash[i]->updateContext( buff, rd);
//				}
//			wxString results;
//			i=0;
//			for(unsigned j = 0 ; j < 16 ; j++)
//				if( (options >> j) & 0x1 ){
//					results += wxString::FromAscii(checksum_options_strings[i]);
//					results += wxT(":\t");
//					//TODO: Here is giving seg fault if 4 hash calculated? Why?
//					results += wxString::FromAscii(myhash[i]->hashIt().c_str());
//					results += wxT("\n");
//					delete myhash[i++];
//					}
//			//delete myhash;
//			return results;
		}
	else{
		hashwrapper *MD5Wrapper = new md5wrapper();
		hashwrapper *SHA1Wrapper = new sha1wrapper();
		hashwrapper *SHA256Wrapper = new sha256wrapper();
		hashwrapper *SHA384Wrapper = new sha384wrapper();
		hashwrapper *SHA512Wrapper = new sha512wrapper();

		MD5Wrapper->resetContext();
		SHA1Wrapper->resetContext();
		SHA256Wrapper->resetContext();
		SHA384Wrapper->resetContext();
		SHA512Wrapper->resetContext();

		int buffsize = MB*1;

		unsigned char buff[buffsize];
		uint64_t readfrom=0,readspeed=0, range=f.Length();
		wxString emsg = msg;
		time_t ts,te;
		time (&ts);
		int rd;
		do
		{
			rd = f.Read( buff, buffsize );
			if( options & MD5    ) MD5Wrapper->updateContext( buff, rd);
			if( options & SHA1   ) SHA1Wrapper->updateContext( buff, rd);
			if( options & SHA256 ) SHA256Wrapper->updateContext( buff, rd);
			if( options & SHA384 ) SHA384Wrapper->updateContext( buff, rd);
			if( options & SHA512 ) SHA512Wrapper->updateContext( buff, rd);
			readfrom+=rd;

			time(&te);
			if(ts != te ){
				ts=te;
				emsg = msg + wxString::Format(_("\nHash Speed : %d MB/s"), (readfrom-readspeed)/MB);
				readspeed=readfrom;
				}
			if(not mypd.Update((readfrom*1000)/range, emsg ))
				return wxEmptyString;
			}while(rd == buffsize);

		wxString results;

		if( options & MD5    ) results += wxT("MD5:\t")+ wxString::FromAscii(MD5Wrapper->hashIt().c_str())+wxT("\n");
		if( options & SHA1   ) results += wxT("SHA1:\t")+ wxString::FromAscii(SHA1Wrapper->hashIt().c_str())+wxT("\n");
		if( options & SHA256 ) results += wxT("SHA256:\t")+ wxString::FromAscii(SHA256Wrapper->hashIt().c_str())+wxT("\n");
		if( options & SHA384 ) results += wxT("SHA384:\t")+ wxString::FromAscii(SHA384Wrapper->hashIt().c_str())+wxT("\n");
		if( options & SHA512 ) results += wxT("SHA512:\t")+ wxString::FromAscii(SHA512Wrapper->hashIt().c_str())+wxT("\n");

		delete MD5Wrapper;
		delete SHA1Wrapper;
		delete SHA256Wrapper;
		delete SHA384Wrapper;
		delete SHA512Wrapper;

		return results;
		}
	}

