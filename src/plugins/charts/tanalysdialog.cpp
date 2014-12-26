/***************************************************************************
 *   Copyright (C) 2012-2014 by Tomasz Bojczuk                             *
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


#include "tanalysdialog.h"
#include <exam/texam.h>
#include <exam/tlevel.h>
#include <exam/textrans.h>
#include <level//tlevelpreview.h>
#include <graphics/tnotepixmap.h>
#include <tglobals.h>
#include <texamparams.h>
#include <tscoreparams.h>
#include <tnoofont.h>
#include "tchart.h"
#include "tmainchart.h"
#include "tlinearchart.h"
#include "tbarchart.h"
#include "tquestionpoint.h"
#include "ttipchart.h"
#include <QtWidgets>


extern Tglobals *gl;


TanalysDialog::TanalysDialog(Texam* exam, QWidget* parent) :
    QDialog(parent),
    m_exam(0),
    m_level(new Tlevel()),
    m_chart(0),
    m_wasExamCreated(false),
    m_isMaximized(false)
{
#if defined(Q_OS_MAC)
    TquestionPoint::setColors(QColor(gl->EanswerColor.name()), QColor(gl->EquestionColor.name()), 
                              QColor(gl->EnotBadColor.name()), Tcolor::shadow, palette().window().color());
#else
    TquestionPoint::setColors(QColor(gl->EanswerColor.name()), QColor(gl->EquestionColor.name()),
											QColor(gl->EnotBadColor.name()), palette().shadow().color(), palette().base().color());
#endif
;
//   setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint |
// 								 Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
	setWindowIcon(QIcon(gl->path + "/picts/charts.png"));
	if (parent)
			setGeometry(parent->geometry());
	gl->config->beginGroup("General");
    setGeometry(gl->config->value("geometry", QRect(50, 50, 750, 480)).toRect());
  gl->config->endGroup();
  QVBoxLayout *lay = new QVBoxLayout;
// default chart presets
  m_chartSetts.inclWrongAnsw = false;
  m_chartSetts.separateWrong = true;
  m_chartSetts.order = Tchart::e_byNumber;
  m_chartSetts.type = Tchart::e_linear;
  lay = new QVBoxLayout;
  
  m_toolBar = new QToolBar("", this);
  lay->addWidget(m_toolBar);

  QGridLayout *headLay = new QGridLayout;
  headLay->addWidget(new QLabel(tr("ordered by:", "Keep a proper form - whole sentence will be: ordered by: question number, key signature, etc..."), this), 0, 0, Qt::AlignCenter);
  headLay->addWidget(new QLabel(tr("student name:"), this), 0, 1, Qt::AlignCenter);
  headLay->addWidget(new QLabel(tr("level:"), this), 0, 2, Qt::AlignCenter);
  m_chartListCombo = new QComboBox(this);
  m_chartListCombo->addItem(tr("question number", "see comment in 'ordered by:' entry"));
  m_chartListCombo->addItem(tr("note pitch"));
  m_chartListCombo->addItem(tr("fret number"));
  m_chartListCombo->addItem(tr("key signature"));
  m_chartListCombo->addItem(tr("accidentals"));
  m_chartListCombo->addItem(tr("question type"));
  m_chartListCombo->addItem(tr("mistake"));
	m_chartListCombo->addItem(tr("prepare time"));
	m_chartListCombo->addItem(tr("attempts count"));
  headLay->addWidget(m_chartListCombo, 1, 0, Qt::AlignCenter);
  m_userLab = new QLabel(" ", this);
  headLay->addWidget(m_userLab, 1, 1, Qt::AlignCenter);
  m_levelLab = new QLabel(" ", this);
  m_moreButton = new QPushButton(QIcon(gl->path + "picts/levelCreator.png"), "...", this);
  m_moreButton->setIconSize(QSize(24, 24));
  m_moreButton->setDisabled(true);
  m_moreButton->setToolTip(tr("Level summary:"));
	m_tuningLab = new QLabel(tr("Tuning"), this);
	m_tuningLab->hide();
	m_tuningButton = new QPushButton(this);
	m_tuningButton->hide();
	m_tuningButton->setFont(TnooFont());
	
  QHBoxLayout *levLay = new QHBoxLayout;
		levLay->addWidget(m_levelLab);
		levLay->addWidget(m_moreButton);
  headLay->addLayout(levLay, 1, 2, Qt::AlignCenter);
	headLay->addWidget(m_tuningLab, 0, 3, Qt::AlignCenter);
	headLay->addWidget(m_tuningButton, 1, 3, Qt::AlignCenter);
  m_questNrLab = new QLabel(" ", this);
  headLay->addWidget(m_questNrLab, 0, 4, Qt::AlignCenter);
  m_effectLab = new QLabel(" ", this);
  headLay->addWidget(m_effectLab, 1, 4, Qt::AlignCenter);

  lay->addLayout(headLay);

  m_plotLay = new QVBoxLayout;
  lay->addLayout(m_plotLay);
  createChart(m_chartSetts);
  
  setLayout(lay);
  createActions();

  if (exam) {
    m_wasExamCreated = false;
    m_openButton->setDisabled(true); // disable "open exam file" action
    if (exam->isExercise())
      setWindowTitle(analyseExerciseWinTitle());
    else 
       setWindowTitle(analyseExamWinTitle());
    setExam(exam);
  } else { // show help in tip
    QString modKey = "";
#if defined(Q_OS_MAC)
    modKey = "CMD";
#else
    modKey = "CTRL";
#endif
    QString helpTipText = "<br>" + tr("Press %1 button to select an exam from a file.").
            arg("<a href=\"charts\"> " + pixToHtml(gl->path + "picts/nootka-exam.png", 38) + " </a>") + "<br>" +
            tr("Use %1 + mouse wheel or %2 buttons to zoom a chart.").
            arg(modKey).arg(pixToHtml(gl->path + "picts/zoom-in.png", 26) + " " + pixToHtml(gl->path + "picts/zoom-out.png", 26)) + "<br>" +
            tr("Click and Drag the cursor to move the chart.") + "<br>";
					TgraphicsTextTip *helpTip = new TgraphicsTextTip(helpTipText, gl->EanswerColor);
						m_chart->scene->addItem(helpTip);
						helpTip->setFlag(QGraphicsItem::ItemIgnoresTransformations);
						helpTip->setPos(100, 80);
      helpTip->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
      connect(helpTip, &TgraphicsTextTip::linkActivated, this, &TanalysDialog::linkOnTipClicked);
  }
  connect(m_chartListCombo, SIGNAL(activated(int)), this, SLOT(analyseChanged(int)));
	connect(m_tuningButton, SIGNAL(clicked()), SLOT(showTuningPreview()));

}


TanalysDialog::~TanalysDialog()
{
	TtipChart::defaultClef = gl->S->clef;
  if (m_wasExamCreated) // to avoid memory leaks
    delete m_exam;
}



//##########  PUBLIC METHODS #####################

void TanalysDialog::setExam(Texam* exam) {
  if (exam == 0)
      return;
  m_exam = exam;
  m_userLab->setText("<b>" + m_exam->userName() + "</b>");
  m_levelLab->setText("<b>" + m_exam->level()->name + "</b>");
  m_questNrLab->setText(tr("Question number") + QString(": <b>%1</b>").arg(exam->count()) );
  m_effectLab->setText(TexTrans::effectTxt() + QString(": <b>%1%</b>").arg(m_exam->effectiveness(), 0, 'f', 1, '0') );
  m_moreButton->setDisabled(false);
	if (exam->level()->instrument != e_noInstrument) {
		bool showTun = false;
		if (exam->level()->instrument == e_bassGuitar)
			showTun = true; // always display tuning preview button for bass
		else if (exam->tune() != Ttune::stdTune)
			showTun = true;
		if (showTun) {
			m_tuningButton->setText(instrumentToGlyph(exam->level()->instrument));
			m_tuningLab->show();
			m_tuningButton->show();
		} else {
			m_tuningLab->hide();
			m_tuningButton->hide();
		}
	}
  connect(m_moreButton, &QPushButton::clicked, this, &TanalysDialog::moreLevelInfo, Qt::UniqueConnection);
  // sort by note
  if ((m_exam->level()->canBeScore() || m_exam->level()->canBeName() || m_exam->level()->canBeSound()) && !m_exam->melodies())
        enableComboItem(1, true);
    else
        enableComboItem(1, false);
  // sort by fret  
  if (m_exam->level()->canBeGuitar() ||
      m_exam->level()->answersAs[TQAtype::e_asNote].isSound() || // answers as played sound are also important
      m_exam->level()->answersAs[TQAtype::e_asName].isSound() ||
      m_exam->level()->answersAs[TQAtype::e_asSound].isSound() )
        enableComboItem(2, true);
    else
        enableComboItem(2, false);
  // sort by key signature
  if (m_exam->level()->canBeScore() && m_exam->level()->useKeySign)
      enableComboItem(3, true);
  else
      enableComboItem(3, false);
  // sort by accidentals
  if ((m_exam->level()->canBeScore() || m_exam->level()->canBeName() || m_exam->level()->canBeSound()) && !m_exam->melodies())
    enableComboItem(4, true);
  else
    enableComboItem(4, false);
	if (m_exam->melodies()) {
		if (m_exam->level()->answerIsSound())
			enableComboItem(7, true); // 'prepare time' only for played answers
		else 
			enableComboItem(7, false);
		enableComboItem(8, true);
	} else {
		enableComboItem(7, false);
		enableComboItem(8, false);
	}
  TtipChart::defaultClef = m_exam->level()->clef;
	if (m_chartListCombo->model()->index((int)m_chartSetts.order, 0).flags() == 0) {
		m_chartListCombo->setCurrentIndex(0); // set default order when combo item is disabled
		analyseChanged(0);
		return;
	}
  createChart(m_chartSetts);
}


void TanalysDialog::loadExam(QString& examFile) {
    if (m_exam)
        delete m_exam;
    m_exam = new Texam(m_level, "");
    m_wasExamCreated = true; // delete exam in destructor
    if (m_exam->loadFromFile(examFile) == Texam::e_file_OK)
        setExam(m_exam); // only when file was OK
    else {
      delete m_exam;
      m_exam = 0;
      m_userLab->setText("");
      m_levelLab->setText("");
      m_questNrLab->setText("");
              m_effectLab->setText("");
      m_moreButton->setDisabled(true);
      createChart(m_chartSetts);
      TgraphicsTextTip *wrongFileTip = new TgraphicsTextTip("<h3>" +
					tr("File: %1 \n is not valid exam file!").arg(examFile).replace("\n", "<br>") + "</h3>", gl->EquestionColor);
      m_chart->scene->addItem(wrongFileTip);
      wrongFileTip->setPos(100, 180);
    }
}

//##########  PRIVATE METHODS #####################

void TanalysDialog::createActions() {
    QMenu *openMenu = new QMenu("open exam file", this);
    QAction *openAct = new QAction(tr("Open an exam to analyze"), this);
			openAct->setIcon(QIcon(gl->path + "picts/charts.png"));
			openAct->setShortcut(QKeySequence::Open);
    openMenu->addAction(openAct);
			connect(openAct, SIGNAL(triggered()), this, SLOT(loadExamSlot()));
    openMenu->addSeparator();
		QString exerciseFile = QDir::toNativeSeparators(QFileInfo(gl->config->fileName()).absolutePath() + "/exercise.noo");
		if (QFileInfo(exerciseFile).exists()) {
			Tlevel lev;
			Texam ex(&lev, "");
			ex.loadFromFile(exerciseFile);
			QAction *exerciseAct = new QAction(tr("Recent exercise on level") + ": " + lev.name, this);
			exerciseAct->setIcon(QIcon(gl->path + "picts/practice.png"));
			exerciseAct->setShortcut(QKeySequence("CTRL+e"));
			connect(exerciseAct, SIGNAL(triggered()), this, SLOT(openRecentExercise()));
			openMenu->addAction(exerciseAct);
			openMenu->addSeparator();
		}
    openMenu->addAction(tr("recent opened exams:"));
    QStringList recentExams = gl->config->value("recentExams").toStringList();
    for (int i = 0; i < recentExams.size(); i++) {
        QFileInfo fi(recentExams[i]);
        if (fi.exists()) {
            QAction *act = new QAction(recentExams[i], this);
            openMenu->addAction(act);
            connect(act, SIGNAL(triggered()), this, SLOT(openRecentExam()));
        }
    }
    m_openButton = new QToolButton(this);
    m_openButton->setToolTip(openAct->text());
    m_openButton->setIcon(QIcon(gl->path + "picts/nootka-exam.png"));
    m_openButton->setMenu(openMenu);
    m_openButton->setPopupMode(QToolButton::InstantPopup);
    QWidgetAction* openToolButtonAction = new QWidgetAction(this);
    openToolButtonAction->setDefaultWidget(m_openButton);

    m_closeAct = new QAction(QIcon(style()->standardIcon(QStyle::SP_DialogCloseButton)), tr("Close analyzer window"), this);
    connect(m_closeAct, SIGNAL(triggered()), this, SLOT(close()));

    m_zoomInAct = new QAction(QIcon(gl->path+"picts/zoom-in.png"), tr("zoom in"), this);
    connect(m_zoomInAct, SIGNAL(triggered()), this, SLOT(zoomInSlot()));

    m_zoomOutAct = new QAction(QIcon(gl->path+"picts/zoom-out.png"), tr("zoom out"), this);
    connect(m_zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOutSlot()));
// settings menu button
    m_inclWrongAct = new QAction(tr("include time of wrong answers to average"), this);
    m_inclWrongAct->setCheckable(true);
    m_wrongSeparateAct = new QAction(tr("show wrong answers separately"), this);
    m_wrongSeparateAct->setCheckable(true);
    QMenu *menu = new QMenu("chart menu", this);
    menu->addAction(m_wrongSeparateAct);
    menu->addAction(m_inclWrongAct);
    connect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
    connect(m_inclWrongAct, SIGNAL(changed()), this, SLOT(includeWrongSlot()));
    m_wrongSeparateAct->setChecked(m_chartSetts.separateWrong);
    m_inclWrongAct->setChecked(m_chartSetts.inclWrongAnsw);
    
    m_settButt = new QToolButton(this);
    m_settButt->setIcon(QIcon(gl->path + "picts/exam-settings.png"));
    m_settButt->setToolTip(tr("Settings of a chart"));
    m_settButt->setMenu(menu);
    m_settButt->setPopupMode(QToolButton::InstantPopup);
    m_settButt->setDisabled(true); // that options have no sense for default chart
    
    QWidgetAction* toolButtonAction = new QWidgetAction(this);
    toolButtonAction->setDefaultWidget(m_settButt);
    
    m_maximizeAct = new QAction(QIcon(gl->path + "picts/fullscreen.png"), tr("Maximize"), this);
    connect(m_maximizeAct, SIGNAL(triggered()), this, SLOT(maximizeWindow()));

    QActionGroup *chartTypeGroup = new QActionGroup(this);
    m_linearAct = new QAction(QIcon(gl->path + "picts/linearChart.png"), tr("linear chart"), this);
    m_linearAct->setCheckable(true);
    chartTypeGroup->addAction(m_linearAct);
    m_barAct = new QAction(QIcon(gl->path + "picts/barChart.png"), tr("bar chart"), this);
    m_barAct->setCheckable(true);
    chartTypeGroup->addAction(m_barAct);
    if (m_chartSetts.type == Tchart::e_linear)
      m_linearAct->setChecked(true);
    else
      m_barAct->setChecked(true);
    connect(chartTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(chartTypeChanged()));
    
    m_toolBar->addAction(openToolButtonAction);
    m_toolBar->addAction(toolButtonAction);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_zoomOutAct);
    m_toolBar->addAction(m_zoomInAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_linearAct);
    m_toolBar->addAction(m_barAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_maximizeAct);
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_closeAct);
}


void TanalysDialog::createChart(Tchart::Tsettings& chartSett) {
//     qreal scaleFactor = 1; // TODO restore restoring scale factor
    if (m_chart) {
//       scaleFactor = m_chart->transform().m11();
      delete m_chart;
      m_chart = 0;
    }
    if (m_exam) {
      if (chartSett.type == Tchart::e_linear)
        m_chart = new TlinearChart(m_exam, m_chartSetts, this);
      else
        m_chart = new TbarChart(m_exam, m_chartSetts, this);
    }
    else
      m_chart = new Tchart(this); // empty chart by default
//     m_chart->scale(scaleFactor, scaleFactor);
    m_plotLay->addWidget(m_chart);
}


void TanalysDialog::enableComboItem(int index, bool enable) {
    QModelIndex ind = m_chartListCombo->model()->index(index, 0);
    QVariant v;
    if (enable)
        v = QVariant(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    else
        v = QVariant(Qt::NoItemFlags); // disable
    m_chartListCombo->model()->setData(ind, v, Qt::UserRole - 1);
}


//##########  SLOTS #####################

void TanalysDialog::linkOnTipClicked() {
    // loadExamSlot() deletes Tchart and cases crash when is invoked directly by tip signal linkActivated().
    // To avoid this we call it outside by QTimer
    QTimer::singleShot(10, this, SLOT(loadExamSlot()));
}


void TanalysDialog::loadExamSlot() {
  QString fileName = QFileDialog::getOpenFileName(this, TexTrans::loadExamFileTxt(), gl->E->examsDir,
												  TexTrans::examFilterTxt(), 0, QFileDialog::DontUseNativeDialog);
  if (fileName != "") {
      gl->E->examsDir = QFileInfo(fileName).absoluteDir().absolutePath();
      loadExam(fileName);
			setWindowTitle(analyseExamWinTitle());
  }
}


void TanalysDialog::openRecentExercise() {
	setWindowTitle(analyseExerciseWinTitle());
	QString exerciseFile = QDir::toNativeSeparators(QFileInfo(gl->config->fileName()).absolutePath() + "/exercise.noo");
	loadExam(exerciseFile);
}



void TanalysDialog::openRecentExam() {
    QAction *action = qobject_cast<QAction *>(sender());
        if (action) {
            QString file = action->text();
            loadExam(file);
						setWindowTitle(analyseExamWinTitle());
        }
}


void TanalysDialog::analyseChanged(int index) {
  if (!m_exam)
    return;
	if (index)
		m_settButt->setDisabled(false);
	else
		m_settButt->setDisabled(true);
  switch (index) {
    case 0:
      m_chartSetts.order = Tchart::e_byNumber;
      break;
    case 1:
      m_chartSetts.order = Tchart::e_byNote;
      break;
    case 2:
      m_chartSetts.order = Tchart::e_byFret;
      break;
    case 3:
      m_chartSetts.order = Tchart::e_byKey;
      break;
    case 4:
      m_chartSetts.order = Tchart::e_byAccid;
      break;
    case 5:
      m_chartSetts.order = Tchart::e_byQuestAndAnsw;
      break;
    case 6:
      m_chartSetts.order = Tchart::e_byMistake;
      break;
		case 7:
      m_chartSetts.order = Tchart::e_prepareTime;
      break;
		case 8:
      m_chartSetts.order = Tchart::e_attemptsCount;
      break;
  }
  if (m_chartSetts.order == Tchart::e_byQuestAndAnsw || m_chartSetts.order == Tchart::e_byMistake) {
      disconnect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
      m_wrongSeparateAct->setChecked(false); // this sorting types require it!!
      m_wrongSeparateAct->setDisabled(true);
      m_chartSetts.separateWrong = false;
      m_inclWrongAct->setDisabled(false);
  } else {
      m_wrongSeparateAct->setDisabled(false);
      connect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
  }  
  createChart(m_chartSetts);
}


void TanalysDialog::testSlot() {
//   QString testFile = gl->path + "/../../test.noo";
//   loadExam(testFile);
}


void TanalysDialog::zoomInSlot() {
    m_chart->zoom(true);
}


void TanalysDialog::zoomOutSlot() {
    m_chart->zoom(false);
}


void TanalysDialog::maximizeWindow() {
  if (m_isMaximized)
    showNormal();
  else
    showMaximized();
  m_isMaximized = !m_isMaximized;
}


void TanalysDialog::wrongSeparateSlot() {
  m_chartSetts.separateWrong = m_wrongSeparateAct->isChecked();
  if (m_wrongSeparateAct->isChecked()) {
    disconnect(m_inclWrongAct, SIGNAL(changed()), this, SLOT(includeWrongSlot()));
    m_inclWrongAct->setDisabled(true); // wrong separeted - it has no sense - lock it!
    m_chartSetts.inclWrongAnsw = false; // and reset 
    m_inclWrongAct->setChecked(false);
  }
  else {
    disconnect(m_inclWrongAct, SIGNAL(changed()), this, SLOT(includeWrongSlot()));
    m_inclWrongAct->setDisabled(false); // unlock
    connect(m_inclWrongAct, SIGNAL(changed()), this, SLOT(includeWrongSlot()));
  }
  createChart(m_chartSetts);
}


void TanalysDialog::includeWrongSlot() {
  m_chartSetts.inclWrongAnsw = m_inclWrongAct->isChecked();
  createChart(m_chartSetts);
}


void TanalysDialog::moreLevelInfo() {
    QDialog *dialog = new QDialog(this, Qt::CustomizeWindowHint | Qt::Dialog);
    TlevelPreview *levelView = new TlevelPreview(dialog);
    if (m_exam) {
        levelView->setLevel(*m_exam->level());
				levelView->adjustToHeight();
		}
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(levelView);
    QPushButton *okButt = new QPushButton(tr("OK"), dialog);
    lay->addWidget(okButt, 1, Qt::AlignCenter);
    dialog->setLayout(lay);
    connect(okButt, SIGNAL(clicked()), dialog, SLOT(accept()));
    dialog->exec();
    delete dialog;
}


void TanalysDialog::chartTypeChanged() {
    if (m_linearAct->isChecked()) { // linear chart
      if (m_chartSetts.type != Tchart::e_linear) {
        m_chartSetts.type = Tchart::e_linear;
        enableComboItem(0, true);
        disconnect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
        m_wrongSeparateAct->setDisabled(false);
        connect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
        createChart(m_chartSetts);
      }        
    } else { // bar chart
        if (m_chartSetts.type != Tchart::e_bar) {
          m_chartSetts.type = Tchart::e_bar;
          m_settButt->setDisabled(false); // unlock settings
          disconnect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
          m_wrongSeparateAct->setDisabled(true);
          m_wrongSeparateAct->setChecked(false);
          m_chartSetts.separateWrong = false;
          m_inclWrongAct->setDisabled(false);
          connect(m_wrongSeparateAct, SIGNAL(changed()), this, SLOT(wrongSeparateSlot()));
          connect(m_inclWrongAct, SIGNAL(changed()), this, SLOT(includeWrongSlot()));
          if (m_chartSetts.order == Tchart::e_byNumber) { // not suported by barChart (no sense)
            if (m_chartListCombo->model()->index(1, 0).flags() == Qt::NoItemFlags) { // notes don't occur'
                m_chartSetts.order = Tchart::e_byFret;
                m_chartListCombo->setCurrentIndex(2);
            } else {
                m_chartSetts.order = Tchart::e_byNote;
                m_chartListCombo->setCurrentIndex(1);
            }
            enableComboItem(0, false);
          }
          createChart(m_chartSetts);
        }
      }
}


void TanalysDialog::showTuningPreview() {
	if (m_tunTip) {
		delete m_tunTip;
		return;
	}
	QString prevText = "<b>" + m_exam->tune().name + "</b><table style=\"text-align: center;\"><tr>";
	for (int i = m_exam->tune().stringNr() - 1; i >= 0; i--)
		prevText += "<td>" + m_exam->tune().str(i + 1).toRichText() + "<br>" + TnooFont::span(QString("%1").arg(i + 1), 20) + "</td>";
	prevText += "</tr></table>";
	QPointF offP = m_chart->mapToScene(0, 0);
	m_tunTip = new TgraphicsTextTip(prevText, palette().highlight().color());
	m_chart->scene->addItem(m_tunTip);
	m_tunTip->setZValue(250);
// 	m_tunTip->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_tunTip->setScale(2.0);
	m_tunTip->setPos(offP.x() + (m_chart->width() - m_tunTip->boundingRect().width() * 2.0) / 2,
									offP.y() + (m_chart->height() - m_tunTip->boundingRect().height() * 2.0) / 2);
	
}



//##########  EVENTS #####################

void TanalysDialog::resizeEvent(QResizeEvent* event) {
    m_toolBar->setIconSize(QSize(height()/21, height()/21));
    m_openButton->setIconSize(m_toolBar->iconSize());
    m_settButt->setIconSize(m_toolBar->iconSize());
		if (m_chart && event->oldSize().height() > 0) {
				double coef = ((double)event->size().height() / (double)event->oldSize().height());
				m_chart->scale(coef, coef);
		}
}


