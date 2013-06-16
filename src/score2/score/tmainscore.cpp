/***************************************************************************
 *   Copyright (C) 2013 by Tomasz Bojczuk                                  *
 *   tomaszbojczuk@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "tmainscore.h"
#include "tscorestaff.h"
#include "tscorenote.h"
#include "tscorekeysignature.h"
#include "tscorecontrol.h"
#include "ttune.h"

// TODO: change this to gl when merged
Ttune glTune;
bool showEnharmNotes = true;
bool doubleAccidentalsEnabled = true;
bool SkeySignatureEnabled = true;
QColor SpointerColor = -1;
QColor enharmNotesColor = -1;

TmainScore::TmainScore(QWidget* parent) :
	TsimpleScore(3, parent),
	m_questMark(0),
	m_questKey(0)
{
	glTune = Ttune::tunes[3];
	
	if (enharmNotesColor == -1)
        enharmNotesColor = palette().highlight().color();
    if (SpointerColor == -1) {
        SpointerColor = palette().highlightedText().color(); // TODO gl->invertColor(palette().highlight().color());
        SpointerColor.setAlpha(200);
    }
    staff()->noteSegment(0)->setPointedColor(SpointerColor);
	  staff()->noteSegment(1)->setReadOnly(true);
		staff()->noteSegment(1)->setColor(enharmNotesColor);
    staff()->noteSegment(2)->setReadOnly(true);
		staff()->noteSegment(2)->setColor(enharmNotesColor);
    
    setScordature();
    setEnabledDblAccid(doubleAccidentalsEnabled);
    setEnableKeySign(SkeySignatureEnabled);
    
//     setAmbitus(Tnote(gl->loString().getChromaticNrOfNote()-1),
//                Tnote(gl->hiString().getChromaticNrOfNote()+gl->GfretsNumber+1));

    isExamExecuting(false);
	
	
	connect(this, SIGNAL(pianoStaffSwitched()), this, SLOT(onPianoSwitch()));
}

TmainScore::~TmainScore()
{
}


//####################################################################################################
//########################################## PUBLIC ##################################################
//####################################################################################################

void TmainScore::setEnableEnharmNotes(bool isEnabled) {
	if (!isEnabled) {
		clearNote(1);
		clearNote(2);
	}
}


void TmainScore::acceptSettings() {
	setEnabledDblAccid(doubleAccidentalsEnabled);
	setScordature();
	setEnableKeySign(SkeySignatureEnabled);
	if (!doubleAccidentalsEnabled)
		clearNote(2);
	staff()->noteSegment(0)->setPointedColor(SpointerColor);
  staff()->noteSegment(1)->setColor(enharmNotesColor);
	staff()->noteSegment(2)->setColor(enharmNotesColor);
	setEnableEnharmNotes(showEnharmNotes);
	if (SkeySignatureEnabled) // refreshKeySignNameStyle();
		if (staff()->scoreKey())
			staff()->scoreKey()->showKeyName(true);
//     setAmbitus(Tnote(gl->loString().getChromaticNrOfNote()-1),
//                Tnote(gl->hiString().getChromaticNrOfNote()+gl->GfretsNumber+1));
}



void TmainScore::setScordature() {
	staff()->setScordature(glTune);
	resizeEvent(0);
}



void TmainScore::unLockScore() {
	setScoreDisabled(false);
	staff()->noteSegment(1)->setReadOnly(true);
	staff()->noteSegment(2)->setReadOnly(true);
//     if (m_questMark) {
//       setBGcolor(gl->mergeColors(gl->EanswerColor, palette().window().color()));
//       noteViews[0]->setStyleSheet(gl->getBGcolorText(gl->EanswerColor) + "border-radius: 10px;");
//     }
}


//####################################################################################################
//############################## METHODS RELATED TO EXAMS ############################################
//####################################################################################################

void TmainScore::isExamExecuting(bool isIt)
{

}


void TmainScore::clearScore() {
	clearNote(0);
	clearNote(1);
	staff()->noteSegment(1)->removeString(); // so far string number to remove occur only on this view
  // TODO also hide question mark when will be implemented
	if (staff()->scoreKey()) {
			setKeySignature(TkeySignature());
			staff()->scoreKey()->setBackgroundColor(-1);
			if (m_questKey) {
				delete m_questKey;
				m_questKey = 0;
			}
    }
    if (scoreControler())
			scoreControler()->setAccidental(0); // reset buttons with accidentals
//     m_questMark->hide();
		staff()->noteSegment(0)->setBackgroundColor(-1);
    setBGcolor(palette().base().color());
}


void TmainScore::askQuestion(Tnote note, char realStr) 
{

}

void TmainScore::askQuestion(Tnote note, TkeySignature key, char realStr)
{

}

void TmainScore::expertNoteChanged()
{

}

void TmainScore::forceAccidental(Tnote::Eacidentals accid)
{

}

void TmainScore::markAnswered(QColor blurColor)
{

}

void TmainScore::markQuestion(QColor blurColor)
{

}

void TmainScore::prepareKeyToAnswer(TkeySignature fakeKey, QString expectKeyName)
{

}

void TmainScore::setKeyViewBg(QColor C)
{

}

void TmainScore::setNoteViewBg(int id, QColor C)
{

}

//####################################################################################################
//########################################## PUBLIC SLOTS ############################################
//####################################################################################################

void TmainScore::whenNoteWasChanged(int index, Tnote note) {
	//We are sure that index is 0, cause others are disabled :-)
    if (showEnharmNotes) {
        TnotesList enharmList = note.getTheSameNotes(doubleAccidentalsEnabled);
        TnotesList::iterator it = enharmList.begin();
        ++it;
        if (it != enharmList.end())
            setNote(1,*(it));
        else
            clearNote(1);
        if (doubleAccidentalsEnabled) {
            ++it;
            if (it != enharmList.end())
                setNote(2,*(it));
            else
                clearNote(2);
        }
    }
    emit noteChanged(index, note);
}


//####################################################################################################
//########################################## PROTECTED ###############################################
//####################################################################################################

void TmainScore::onPianoSwitch() {
	if (glTune != Ttune::stdTune)
			setScordature();
}







