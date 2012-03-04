/***************************************************************************
 *   Copyright (C) 2011 by Tomasz Bojczuk  				   *
 *   tomaszbojczuk@gmail.com   						   *
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
 *  You should have received a copy of the GNU General Public License	   *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "tnotename.h"
#include "tnoteview.h"
#include "tglobals.h"
#include "tpushbutton.h"
#include <QtGui>


extern Tglobals *gl;
QString styleTxt = "border: 1px solid palette(Text); border-radius: 10px";

/**static*/
const char * const TnoteName::octaves[6] = { QT_TR_NOOP("Contra"), QT_TR_NOOP("Great"), QT_TR_NOOP("Small"),
                                 QT_TR_NOOP("1-line"), QT_TR_NOOP("2-line"), QT_TR_NOOP("3-line") };
const char * const TnoteName::octavesFull[6] = { QT_TR_NOOP("Contra octave"),
                    QT_TR_NOOP("Great octave"), QT_TR_NOOP("Small octave"),
                    QT_TR_NOOP("One-line octave"), QT_TR_NOOP("Two-line octave"),
                    QT_TR_NOOP("Three-line octave") };





TnoteName::TnoteName(QWidget *parent) :
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

// NAME LABEL
    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->setAlignment(Qt::AlignCenter);

    nameLabel = new QLabel("<b><span style=\"font-size: 24px; color: green;\">Nootka " +
                           gl->version + "</span></b>",this);
    nameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("background-color: palette(Base); " + styleTxt);
    resize();

    mainLay->addStretch(1);
    mainLay->addWidget(nameLabel, 0, Qt::AlignCenter);
    mainLay->addStretch(1);
// BUTTONS WITH NOTES TOOLBAR
    QHBoxLayout *noteLay = new QHBoxLayout();
    noteLay->addStretch(1);
    noteGroup =new QButtonGroup(this);
    for (int i=0; i<7; i++) {
        noteButtons[i] = new TpushButton("", this);
        noteLay->addWidget(noteButtons[i]);
        noteGroup->addButton(noteButtons[i],i);
    }
    noteLay->addStretch(1);
    mainLay->addLayout(noteLay);
    connect(noteGroup, SIGNAL(buttonClicked(int)), this, SLOT(noteWasChanged(int)));
// ACCID BUTTONS TOOOLBAR
    QHBoxLayout *accLay = new QHBoxLayout;
    accLay->addStretch(1);
    dblFlatButt = new TpushButton("B", this);
    dblFlatButt->setFont(QFont("nootka", 10, QFont::Normal));
    accLay->addWidget(dblFlatButt);
    connect(dblFlatButt, SIGNAL(clicked()), this, SLOT(accidWasChanged()));
    flatButt = new TpushButton("b", this);
    flatButt->setFont(QFont("nootka", 10, QFont::Normal));
    accLay->addWidget(flatButt);
    connect(flatButt, SIGNAL(clicked()), this, SLOT(accidWasChanged()));
    sharpButt = new TpushButton("#", this);
    sharpButt->setFont(QFont("nootka", 10, QFont::Normal));
    accLay->addWidget(sharpButt);
    connect(sharpButt, SIGNAL(clicked()), this, SLOT(accidWasChanged()));
    dblSharpButt = new TpushButton("x", this);
    dblSharpButt->setFont(QFont("nootka", 10, QFont::Normal));
    accLay->addWidget(dblSharpButt);
    connect(dblSharpButt, SIGNAL(clicked()), this, SLOT(accidWasChanged()));
    accLay->addStretch(1);
    mainLay->addLayout(accLay);
// OCTAVE BUTTONS TOOLBAR
    QHBoxLayout * octLay = new QHBoxLayout;
    octLay->addStretch(1);
    octaveGroup = new QButtonGroup(this);
    for (int i=0; i<6; i++) {
        octaveButtons[i] = new TpushButton(tr(octaves[i]), this);
        octaveButtons[i]->setToolTip(tr(octavesFull[i]));
        octLay->addWidget(octaveButtons[i]);
        octaveGroup->addButton(octaveButtons[i],i);
    }
    octLay->addStretch(1);
    mainLay->addLayout(octLay);
//     mainLay->addStretch(1);
	m_prevOctButton = -1;
    connect(octaveGroup, SIGNAL(buttonClicked(int)), this, SLOT(octaveWasChanged(int)));
    
     QVBoxLayout *lay = new QVBoxLayout;
     QGroupBox *mainGr = new QGroupBox(this);
     mainGr->setLayout(mainLay);
     lay->addWidget(mainGr);
     setLayout(lay);
//    setLayout(mainLay);

    setNoteNamesOnButt(gl->NnameStyleInNoteName);
//    octaveButtons[2]->setChecked(true);
    for (int i=0; i<3; i++) m_notes.push_back(Tnote());
    setAmbitus(gl->loString(),
               Tnote(gl->hiString().getChromaticNrOfNote()+gl->GfretsNumber));
    resize();

}

void TnoteName::setNoteNamesOnButt(Tnote::EnameStyle nameStyle) {
    for (int i=0; i<7; i++) {
        noteButtons[i]->setText(QString::fromStdString(Tnote(i+1,0,0).getName(nameStyle,false)));
    }
}


// private setNoteName method
void TnoteName::setNoteName(char noteNr, char octNr, char accNr) {
    m_notes[0] = Tnote(noteNr, octNr, accNr);
    if (noteNr) {
        if (gl->showEnharmNotes) {
            TnotesList enharmList = m_notes[0].getTheSameNotes(gl->doubleAccidentalsEnabled);
            TnotesList::iterator it = enharmList.begin();
            ++it;
            if (it != enharmList.end())
                m_notes[1] = *(it);
            else m_notes[1] = Tnote();
            ++it;
            if (it != enharmList.end())
                m_notes[2] = *(it);
            else m_notes[2] = Tnote();
        }
        setNameText();
        emit noteNameWasChanged(m_notes[0]);
    }
}

void TnoteName::setNameText() {
    if (m_notes[0].note) {
        if (m_notes[0].getChromaticNrOfNote() >= m_ambitMin &&
            m_notes[0].getChromaticNrOfNote() <= m_ambitMax) {
        QString txt = noteToRichText(m_notes[0]);
        if (m_notes[1].note) {
            txt = txt + QString("  <span style=\"font-size: %1px; color: %2\">(").arg(nameLabel->font().pointSize()-2).arg(gl->enharmNotesColor.name()) + noteToRichText(m_notes[1]);
            if (m_notes[2].note)
                txt = txt + "  " + noteToRichText(m_notes[2]);
            txt = txt + ")</span>";
        }
        nameLabel->setText(txt);
      } else
          nameLabel->setText("<span style=\"font-size: 16px; color: #ff0000;\"><b>" +
                             tr("The note is beyond<br>scale of the guitar") + "</b></span>");
    } else nameLabel->setText("");;
}

/** public setNoteName methods */
void TnoteName::setNoteName(Tnote note) {
	if (m_notes[0].note) {
		noteButtons[m_notes[0].note-1]->setChecked(false);
	}
	if (m_prevOctButton != -1)
		octaveButtons[m_prevOctButton]->setChecked(false);
    if (note.note) {
        m_notes[0] = note;
        setButtons(note);
    } else {
        m_notes[0] = Tnote(0,0,0);
        m_notes[1] = Tnote(0,0,0);
        m_notes[2] = Tnote(0,0,0);
		m_prevOctButton = -1;
    }
    setNameText();
}

void TnoteName::setNoteName(TnotesList notes) {
    TnotesList::iterator it = notes.begin();
    ++it;
    if (it != notes.end())
        m_notes[1] = *(it);
    else m_notes[1] = Tnote(0,0,0);
    ++it;
    if (it != notes.end())
        m_notes[2] = *(it);
    else m_notes[2] = Tnote(0,0,0);
    setNoteName(notes[0]);
}

void TnoteName::setEnabledDblAccid(bool isEnabled) {
    if (isEnabled) {
        dblFlatButt->show();
        dblSharpButt->show();
    } else {
        dblFlatButt->hide();
        dblSharpButt->hide();
        m_notes[2] = Tnote(0,0,0);
        setNameText();
    }
}

void TnoteName::noteWasChanged(int noteNr) {
	if (m_notes[0].note) {
		if (m_notes[0].note != noteNr+1) //uncheck only if previous was different
			noteButtons[m_notes[0].note-1]->setChecked(false);
	}	
	noteButtons[noteNr]->setChecked(true);
    setNoteName(noteNr+1, m_notes[0].octave, m_notes[0].acidental);
	if (octaveGroup->checkedId() == -1 && m_prevOctButton == -1) {
		octaveButtons[2]->setChecked(true);
    m_prevOctButton = 2;
// 		octaveGroup->setId(2);
	}
	emit noteButtonClicked();
}

void TnoteName::accidWasChanged() {
	if (sender() != flatButt)
		flatButt->setChecked(false);
	if (sender() != sharpButt)
		sharpButt->setChecked(false);
	if (sender() != dblSharpButt)
		dblSharpButt->setChecked(false);
	if (sender() != dblFlatButt)
		dblFlatButt->setChecked(false);
    char ac;
	TpushButton *button = static_cast<TpushButton *>(sender());
	button->setChecked(!button->isChecked());
    if (sender() == dblFlatButt) {
        if (dblFlatButt->isChecked()) ac = -2;
        else ac = 0;
    } else {
        if (sender() == flatButt) {
            if (flatButt->isChecked()) ac = -1;
            else ac = 0;
    } else {
        if (sender() == sharpButt) {
            if (sharpButt->isChecked()) ac = 1;
            else ac = 0;
        } else {
            if (dblSharpButt->isChecked()) ac = 2;
            else ac = 0;
          }
      }
    }
    setNoteName(m_notes[0].note, m_notes[0].octave, ac);
}

void TnoteName::octaveWasChanged(int octNr) { // octNr is button nr in the group
	if (octNr != m_prevOctButton && m_prevOctButton != -1)
		octaveButtons[m_prevOctButton]->setChecked(false);
	m_prevOctButton = octNr;
	octaveButtons[octNr]->setChecked(true);
    setNoteName(m_notes[0].note, octNr-2, m_notes[0].acidental);
}

/*static*/
QString TnoteName::noteToRichText(Tnote note) {
    QString nameTxt = QString::fromStdString(note.getName(gl->NnameStyleInNoteName,false));
    if (gl->NnameStyleInNoteName == Tnote::e_italiano_Si ||
        gl->NnameStyleInNoteName == Tnote::e_english_Bb ||
        gl->NnameStyleInNoteName == Tnote::e_norsk_Hb ) {
        if (note.acidental) {
            int a = 1;
            if (note.acidental == -2) a = 2;
            nameTxt.insert(nameTxt.size()-a,"<sub><i>");
        nameTxt.insert(nameTxt.size(),"</i></sub>");
        }
    }
    nameTxt = nameTxt.toLower();
    if (gl->NoctaveInNoteNameFormat) {
        if (note.octave < 0) { //first letter capitalize
         QString l1 = nameTxt.mid(0,1).toUpper();
         nameTxt.replace(0,1,l1);
         if (note.octave < -1)
             nameTxt = nameTxt + QString("<sub>%1</sub>").arg(int(note.octave*(-1)-1));
        }
        if (note.octave > 0)
            nameTxt = nameTxt + QString("<sup>%1</sup>").arg((int)note.octave);
    }
    return nameTxt;
}

void TnoteName::setButtons(Tnote note) {
    noteButtons[note.note-1]->setChecked(true);

    uncheckAccidButtons();
    switch (note.acidental) {
    case -2 : dblFlatButt->setChecked(true); break;
    case -1 : flatButt->setChecked(true); break;
    case 1 : sharpButt->setChecked(true); break;
    case 2 : dblSharpButt->setChecked(true); break;
    }
    if (note.octave >= -2 && note.octave <= 3) {
        octaveButtons[note.octave+2]->setChecked(true);
		m_prevOctButton = note.octave+2;
	}
}

void TnoteName::setEnabledEnharmNotes(bool isEnabled) {
    if (!isEnabled) {
        m_notes[1] = Tnote(0,0,0);
        m_notes[2] = Tnote(0,0,0);
        setNameText();
    }
}

void TnoteName::resize(int fontSize) {
    nameLabel->setFixedSize(qRound(width()*0.9), qRound(height() * 0.3));
    nameLabel->setFont(QFont(nameLabel->font().family(), qRound(nameLabel->height() * 0.55), 50));
    nameLabel->setText(nameLabel->text());
    if (fontSize) {
        QFont f = QFont(noteButtons[0]->font().family());
        f.setPixelSize(fontSize);
        for (int i=0; i<7; i++)
            noteButtons[i]->setFont(f);
        for (int i=0; i<6; i++)
            octaveButtons[i]->setFont(f);
        f = QFont(dblFlatButt->font().family());
        f.setPixelSize(fontSize);
        dblFlatButt->setFont(f);
        flatButt->setFont(f);
        sharpButt->setFont(f);
        dblSharpButt->setFont(f);
    }
}

void TnoteName::setAmbitus(Tnote lo, Tnote hi) {
    m_ambitMin = lo.getChromaticNrOfNote();
    m_ambitMax = hi.getChromaticNrOfNote();
}

void TnoteName::askQuestion(Tnote note, char strNr) {
    setNoteName(note);
    QString sN = "";
    if (strNr) sN = QString("  %1").arg((int)strNr);
    nameLabel->setText(nameLabel->text() +
                       QString(" <span style=\"color: %1; font-family: nootka;\">?%2</span>").arg(gl->EquestionColor.name()).arg(sN));
    nameLabel->setStyleSheet(gl->getBGcolorText(gl->EquestionColor) + styleTxt);
    uncheckAllButtons();    
}

void TnoteName::prepAnswer(Tnote backNote) {
    nameLabel->setStyleSheet(gl->getBGcolorText(gl->EanswerColor) + styleTxt);
    if (backNote.acidental) {
        QString accTxt = QString(" <sub><i><span style=\"color: %1;\">(%2)</span></i></sub>").arg(gl->GfingerColor.name()).arg(QString::fromStdString(signsAcid[backNote.acidental + 2]));
        nameLabel->setText(nameLabel->text() + accTxt);
        /** @todo press accccid button */
    }
    setNameDisabled(false);
    m_notes[0] = Tnote(0,0,0); // Reset, otherwise getNoteName() returns it
    /** @todo prepare octave button*/
}

void TnoteName::setNameDisabled(bool isDisabled) {
    if (isDisabled) {
        uncheckAllButtons();
        for (int i=0; i<7; i++)
            noteButtons[i]->setDisabled(true);
        for (int i=0; i<6; i++)
            octaveButtons[i]->setDisabled(true);
        dblFlatButt->setDisabled(true);
        flatButt->setDisabled(true);
        sharpButt->setDisabled(true);
        dblSharpButt->setDisabled(true);
//        nameLabel->setStyleSheet(gl->getBGcolorText(-1) + styleTxt);
    } else {
        for (int i=0; i<7; i++)
            noteButtons[i]->setDisabled(false);
        for (int i=0; i<6; i++)
            octaveButtons[i]->setDisabled(false);
        dblFlatButt->setDisabled(false);
        flatButt->setDisabled(false);
        sharpButt->setDisabled(false);
        dblSharpButt->setDisabled(false);
//        nameLabel->setStyleSheet("background-color: palette(Base); " + styleTxt);
    }
}

void TnoteName::uncheckAccidButtons() {
    dblFlatButt->setChecked(false);
    flatButt->setChecked(false);
    sharpButt->setChecked(false);
    dblSharpButt->setChecked(false);
}

void TnoteName::clearNoteName() {
    setNoteName(Tnote());
    nameLabel->setStyleSheet("background-color: palette(Base); " + styleTxt);
}

void TnoteName::uncheckAllButtons() {
    uncheckAccidButtons();
    noteGroup->setExclusive(false);
    octaveGroup->setExclusive(false);
    for (int i = 0; i < 7; i++)
        noteButtons[i]->setChecked(false);
    for (int i = 0; i < 6; i++)
        octaveButtons[i]->setChecked(false);
	m_prevOctButton = -1;
    noteGroup->setExclusive(true);
    octaveGroup->setExclusive(true);
}
