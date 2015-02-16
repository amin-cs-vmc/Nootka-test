/***************************************************************************
 *   Copyright (C) 2012-2015 by Tomasz Bojczuk                             *
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

#include "tlevelpreview.h"
#include <exam/tlevel.h>
#include <graphics/tnotepixmap.h>
#include "widgets/tquestionaswdg.h"
#include "tinitcorelib.h"
#include <tnoofont.h>
#include <tscoreparams.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPainter>
#include <QDebug>


/** Returns size of 'A' letter in current widget font. */
int heightOfA(QWidget *w) {
	return w->fontMetrics().boundingRect("A").height();
}


QString tdAlign() {
	return "<td valign=\"middle\" align=\"center\">";
}

TlevelPreview::TlevelPreview(QWidget* parent) :
  QWidget(parent),
  m_instrText(""),
  m_enableFixing(false)
{
		setMouseTracking(true);
    QLabel *headLab = new QLabel(tr("Level summary:"), this);
		m_summaryEdit = new QTextBrowser(this);
		m_summaryEdit->setReadOnly(true);
    m_summaryEdit->setFixedWidth(fontMetrics().boundingRect("W").width() * 28);
    m_summaryEdit->viewport()->setStyleSheet("background-color: transparent;");
		m_summaryEdit->setOpenLinks(false);
		QVBoxLayout *mainLay = new QVBoxLayout;
			mainLay->addWidget(headLab);
			mainLay->addWidget(m_summaryEdit);
    setLayout(mainLay);
		setLevel();
		m_summaryEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
// 		adjustToHeight();
}


TlevelPreview::~TlevelPreview() {}


void TlevelPreview::setLevel() {
		Tlevel empty;
		empty.name = tr("no level selected");
		empty.desc = "";
		empty.loNote = Tnote();
		empty.hiNote = Tnote();
		empty.hiFret = 0;
		empty.questionAs.setAsNote(false);
		empty.questionAs.setAsName(false);
		empty.questionAs.setAsFret(false);
		empty.questionAs.setAsSound(false);
		empty.useKeySign = false;
		empty.withDblAcc = false;
		empty.withFlats = false;
		empty.withSharps = false;
		empty.instrument = e_noInstrument;
		setLevel(empty);
}


void TlevelPreview::setLevel(Tlevel& tl) {
	QString instrName = "";
	if (tl.hasInstrToFix) {
		m_instrText = ""; // don't display background instrument symbol
		if (m_enableFixing) {
				instrName = "<a href=\"fixInstrument\">" + tr("fix an instrument") + "</a>";
				m_summaryEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);
		}
	} else {
		m_instrText = instrumentToGlyph(tl.instrument);
		instrName = instrumentToText(tl.instrument);
		m_summaryEdit->setTextInteractionFlags(Qt::NoTextInteraction);
	}
  QString S;
    S = "<center><b>" + tl.name + "</b>";
    S += "<table border=\"1\" cellpadding=\"3\">";
		S += "<tr><td colspan=\"2\" align=\"center\">" + instrName + "</td>";
    QString clefString = tr("Clef") + ":&nbsp;&nbsp;";
    if (tl.loNote.isValid() && tl.hiNote.isValid())
      clefString += QString("<br><br>%1</td></tr>").
        arg(wrapPixToHtml(Tnote(0, 0, 0), tl.clef.type(), TkeySignature(0), (tl.clef.type() == Tclef::e_pianoStaff) ? 3.0 : 5.0)).
                          replace("<img", "<img width=\"70px\"");
    S += "<td rowspan=\"_ROW_SPAN_\"><br>&nbsp;&nbsp;" + clefString;
    S += "<tr><td>" + notesRangeTxt() + " </td>";
		if (tl.loNote.note && tl.hiNote.note)
			S += tdAlign() + tl.loNote.toRichText() + " - " + tl.hiNote.toRichText() + "</td></tr>";
    if (tl.canBeGuitar()) { // level uses guitar
        S += "<tr><td>" + fretsRangeTxt() + " </td>";
        S += tdAlign() + QString("%1 - %2").arg(int(tl.loFret)).arg(int(tl.hiFret)) + "</td></tr>";
    }
    if (tl.useKeySign) {
        S += "<tr><td>" + tr("key signature:") + " </td>" + tdAlign();
        S += tl.loKey.getMajorName().remove("-" + Tcore::gl()->S->majKeyNameSufix);
        S += " (" + tl.loKey.accidNumber(true) +")";
        if (!tl.isSingleKey) {
            S += " - " + tl.hiKey.getMajorName().remove("-" + Tcore::gl()->S->majKeyNameSufix);
            S += " (" + tl.hiKey.accidNumber(true) + ")";
        }
        S += "</td></tr>";
    }
    S += "<tr><td>" + tr("accidentals:") + " </td>" + tdAlign();
    if (!tl.withSharps && !tl.withFlats && !tl.withDblAcc)
        S += tr("none");
    else {
        if (tl.withSharps) S += TnooFont::span(" #");
        if (tl.withFlats) S += TnooFont::span(" b");
        if (tl.withDblAcc) S += TnooFont::span(" x B");
    }
    S += "</td></tr>";
    S += "<tr><td>" + TquestionAsWdg::questionsTxt() + ": </td>" + tdAlign(); // QUESTIONS
    QString tmp;
    if (tl.questionAs.isNote())
      tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asNote) + " ";
    if (tl.questionAs.isName())
      tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asName) + " ";
    if (tl.questionAs.isFret())
      tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asFretPos) + " ";
    if (tl.questionAs.isSound()) {
			if (tl.canBeMelody())
				tmp += "m";
			else
				tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asSound);
		}
    int fontSize = heightOfA(this) * 1.3;
    S += TnooFont::span(tmp, fontSize);
    S += "</td></tr>";
    tmp   = "";
    S += "<tr><td>" + TquestionAsWdg::answersTxt() + ": </td><td align=\"center\">"; // ANSWERS
    if (tl.answerIsNote())
            tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asNote) + " ";
    if (tl.answerIsName())
            tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asName) + " ";
    if (tl.answerIsGuitar())
            tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asFretPos) + " ";
    if (tl.answerIsSound()) {
			if (tl.canBeMelody())
				tmp += "m";
			else
				tmp += TquestionAsWdg::qaTypeSymbol(TQAtype::e_asSound);
		}
    S += TnooFont::span(tmp, fontSize);
    S += "</td></tr>";
    if (tl.canBeName() || tl.canBeScore() || tl.canBeSound()) {
      S += "<tr><td colspan=\"2\" align=\"center\">";
      if (tl.requireOctave)
          S += tr("proper octave is required");
      else
          S += tr("octave does no matter");
      S += "</td></tr>";
    }
    S += "</table></center>";
    S.replace("_ROW_SPAN_", QString("%1").arg(S.count("<tr>")));
		S += "<br><br>" + tl.desc;
		m_summaryEdit->setHtml(S);
}


void TlevelPreview::adjustToHeight() {
	m_summaryEdit->setFixedHeight((m_summaryEdit->document()->toHtml().count("<tr>") + 3) * (heightOfA(this) + 7));
}

//##########################################################################################################
//########################################## PROTECTED #####################################################
//##########################################################################################################

void TlevelPreview::setFixInstrEnabled(bool enabled) {
	m_enableFixing = enabled;
	if (enabled) {
		connect(m_summaryEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkToFixLevel(QUrl)));
	}
}


void TlevelPreview::paintEvent(QPaintEvent* ) {
	QPainter painter(this);
	painter.setBrush(palette().base());
	painter.setPen(Qt::NoPen);
	painter.drawRect(m_summaryEdit->geometry());
	if (m_instrText != "") {
			TnooFont nFont;
			QFontMetrics fm = QFontMetrics(nFont);
			nFont.setPixelSize(nFont.pixelSize() * ((qreal)m_summaryEdit->height() / (qreal)fm.boundingRect(m_instrText).height()));
			painter.setFont(nFont);
			QColor bg = palette().highlight().color();
			bg.setAlpha(60);
			painter.setPen(QPen(bg));
			painter.drawText(QRect(m_summaryEdit->geometry().x(), 15, m_summaryEdit->width(), m_summaryEdit->height()),
											 Qt::AlignCenter, m_instrText);
	}
}


void TlevelPreview::linkToFixLevel(QUrl url) {
	if (url.toString() == "fixInstrument")
		emit instrumentLevelToFix();
}





