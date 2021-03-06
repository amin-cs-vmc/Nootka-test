/***************************************************************************
 *   Copyright (C) 2011-2016 by Tomasz Bojczuk                             *
 *   seelook@gmail.com                                                     *
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


#include "texamview.h"
#include <exam/tqaunit.h>
#include <exam/texam.h>
#include <exam/textrans.h>
#include <exam/tattempt.h>
#include <QtWidgets/QtWidgets>
#if defined (Q_OS_ANDROID)
  #include <tmtr.h>
#endif


#define SPACE_GAP (7)


static QString borderStyleTxt = QStringLiteral("border: 1px solid palette(shadow); border-radius: 2px;");
static QString space = QStringLiteral(" ");


TexamView* TexamView::m_instance = nullptr;

TexamView::TexamView(QWidget *parent) :
  QWidget(parent),
  m_exam(0)
{
  if (m_instance) {
    qDebug() << "TexamView instance already exists";
    return;
  }

  m_instance = this;
  auto mainLay = new QHBoxLayout;
#if defined (Q_OS_ANDROID)
  mainLay->setContentsMargins(0, 1, 2, 1);
#else
  setStatusTip(tr("Exam results"));
#endif
  mainLay->addStretch();
  QHBoxLayout *okMistLay = new QHBoxLayout;
  m_corrLab = new QLabel(this);
  okMistLay->addWidget(m_corrLab, 0, Qt::AlignRight);
  okMistLay->addSpacing(SPACE_GAP);
  m_halfLab = new QLabel(this);
  okMistLay->addWidget(m_halfLab, 0, Qt::AlignRight);
  m_halfLab->hide();
  okMistLay->addSpacing(SPACE_GAP);
  m_mistLab = new QLabel(this);
  okMistLay->addWidget(m_mistLab, 0, Qt::AlignRight);
  mainLay->addLayout(okMistLay);
  mainLay->addStretch();

  m_effLab = new QLabel(this);
  m_effLab->setStyleSheet(borderStyleTxt);
  mainLay->addWidget(m_effLab);
  mainLay->addStretch();

  m_averTimeLab = new QLabel(this);
  m_averTimeLab->setStyleSheet(borderStyleTxt);
  mainLay->addWidget(m_averTimeLab);
  mainLay->addStretch();

  m_reactTimeLab = new QLabel(this);
  m_reactTimeLab->setStyleSheet(borderStyleTxt);
  mainLay->addWidget(m_reactTimeLab);

  mainLay->addStretch();

  m_totalTimeLab = new QLabel(this);
  m_totalTimeLab->setStyleSheet(borderStyleTxt);
  mainLay->addWidget(m_totalTimeLab);
  mainLay->addStretch();

  setLayout(mainLay);

  clearResults();

  m_corrLab->setAlignment(Qt::AlignCenter);
  m_corrLab->setStatusTip(TexTrans::corrAnswersNrTxt());
  m_halfLab->setAlignment(Qt::AlignCenter);
  m_mistLab->setStatusTip(TexTrans::mistakesNrTxt());
  m_mistLab->setAlignment(Qt::AlignCenter);
  m_averTimeLab->setStatusTip(TexTrans::averAnsverTimeTxt() + space + TexTrans::inSecondsTxt());
  m_averTimeLab->setAlignment(Qt::AlignCenter);
  m_reactTimeLab->setStatusTip(TexTrans::reactTimeTxt() + space + TexTrans::inSecondsTxt());
  m_reactTimeLab->setAlignment(Qt::AlignCenter);
  m_totalTimeLab->setStatusTip(TexTrans::totalTimetxt());
  m_totalTimeLab->setAlignment(Qt::AlignCenter);

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(countTime()));
}


TexamView::~TexamView()
{
  m_instance = nullptr;
}


void TexamView::updateExam() {
	if (m_exam) {
		m_exam->setTotalTime(totalTime());
	}
}


void TexamView::setStyleBg(const QString& okBg, const QString& wrongBg, const QString& notBadBg) {
    m_corrLab->setStyleSheet(okBg + borderStyleTxt);
    m_mistLab->setStyleSheet(wrongBg + borderStyleTxt);
    m_halfLab->setStyleSheet(notBadBg + borderStyleTxt);
}


void TexamView::questionStart() {
	m_questionTime.start();
	m_showReact = true;
	countTime();
}


quint16 TexamView::questionTime() {
  return quint16(qRound(m_questionTime.elapsed() / 100.0));
}


void TexamView::questionStop() {
	m_showReact = false;
  quint16 t = quint16(qRound(m_questionTime.elapsed() / 100.0));
	if (m_exam->melodies()) {
		m_exam->curQ()->time += t; // total time of all attempts
		m_exam->curQ()->lastAttempt()->setTotalTime(t);
	} else
		m_exam->curQ()->time = t; // just elapsed time of single answer
	if (isVisible())
		m_reactTimeLab->setText(space + Texam::formatReactTime(m_exam->curQ()->time) + space);
}


void TexamView::pause() {
	m_pausedAt = m_questionTime.elapsed();
  doNotdisplayTime();
}


void TexamView::go() {
	m_questionTime.start();
  m_questionTime = m_questionTime.addMSecs(-m_pausedAt);
  displayTime();
}


void TexamView::startExam(Texam* exam) {
	m_exam = exam;
	m_totalTime = QTime(0, 0);
  m_startExamTime = int(m_exam->totalTime());
	m_showReact = false;
	m_totalTime.start();
	countTime();
	answered();
	m_averTimeLab->setText(space + Texam::formatReactTime(m_exam->averageReactonTime()) + space);
	if (m_exam->melodies()) {
		m_effLab->setStatusTip(tr("Effectiveness of whole exam (and effectiveness of current question)."));
		m_halfLab->setStatusTip(TexTrans::halfMistakenTxt());
	} else {
		m_effLab->setStatusTip(TexTrans::effectTxt());
		m_halfLab->setStatusTip(TexTrans::halfMistakenTxt() + QLatin1String("<br>") + TexTrans::halfMistakenAddTxt());
	}
}


void TexamView::answered() {
	questionCountUpdate();
	effectUpdate();
	reactTimesUpdate();
}


void TexamView::setFontSize(int s) {
  QFont f = font();
  f.setPointSize(s);
#if defined (Q_OS_ANDROID)
  QFontMetrics fm(f);
  int zeroW = fm.width(QStringLiteral("0")) * 35;
  if (zeroW + layout()->spacing() * 8 > qRound(Tmtr::longScreenSide() * 0.48)) {
      s = qRound(qreal(s) * (Tmtr::longScreenSide() * 0.48) / qreal(zeroW + layout()->spacing() * 8)) - 1;
      f.setPointSize(s);
  }
#endif
  setFont(f);
  m_reactTimeLab->setFont(f);
  m_averTimeLab->setFont(f);
  m_totalTimeLab->setFont(f);
  m_mistLab->setFont(f);
  m_corrLab->setFont(f);
  m_halfLab->setFont(f);
  m_effLab->setFont(f);
  m_sizeHint.setWidth(m_effLab->fontMetrics().width(QStringLiteral("0")) * 35 + layout()->spacing() * 8);
  m_sizeHint.setHeight(m_effLab->fontMetrics().height() + m_effLab->contentsMargins().top() * 2);
}


//######################################################################
//#######################     PUBLIC SLOTS   ###########################
//######################################################################
void TexamView::reactTimesUpdate() {
	if (m_exam && isVisible())
		m_averTimeLab->setText(space + Texam::formatReactTime(m_exam->averageReactonTime()) + space);
}


void TexamView::effectUpdate() {
	if (m_exam && isVisible()) {
		QString effText = QString("<b>%1 %</b>").arg(qRound(m_exam->effectiveness()));
		if (m_exam->count() && m_exam->melodies() && !m_exam->curQ()->answered() && 
				m_exam->curQ()->attemptsCount() && m_exam->curQ()->lastAttempt()->mistakes.size())
			effText += QString(" <small>(%1 %)</small>").arg(qRound(m_exam->curQ()->effectiveness()));
			// It will display current melody question effectiveness only if it makes a sense
		m_effLab->setText(effText);
	}
}


void TexamView::questionCountUpdate() {
	if (m_exam && isVisible()) {
		m_mistLab->setText(QString("%1").arg(m_exam->mistakes()));
		if (m_exam->halfMistaken()) {
				m_halfLab->show();
				m_halfLab->setText(QString("%1").arg(m_exam->halfMistaken()));
		}
		m_corrLab->setText(QString("%1").arg(m_exam->corrects()));
	}
}



void TexamView::countTime() {
	if (isVisible()) {
		if (m_showReact)
				m_reactTimeLab->setText(QString(" %1 ").arg(Texam::formatReactTime(m_questionTime.elapsed() / 100 + m_exam->curQ()->time)));
		m_totalTimeLab->setText(space + formatedTotalTime(m_startExamTime * 1000 + m_totalTime.elapsed()) + space);
	}
}


void TexamView::clearResults() {
  QString zero = QStringLiteral("0");
  m_corrLab->setText(zero);
  m_mistLab->setText(zero);
  m_halfLab->setText(zero);
  m_effLab->setText(QStringLiteral("<b>100%</b>"));
  m_averTimeLab->setText(QLatin1String(" 0.0 "));
  m_reactTimeLab->setText(QLatin1String(" 0.0 "));
  m_totalTimeLab->setText(QStringLiteral(" 0:00:00 "));
}

