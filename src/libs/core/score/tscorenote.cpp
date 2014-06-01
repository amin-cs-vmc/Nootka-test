/***************************************************************************
 *   Copyright (C) 2013-2014 by Tomasz Bojczuk                             *
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


#include "tscorenote.h"
#include "tscorescene.h"
#include "tscorestaff.h"
#include "tnotecontrol.h"
#include <graphics/tdropshadoweffect.h>
#include <animations/tcrossfadetextanim.h>
#include <animations/tcombinedanim.h>
#include <music/tnote.h>
#include <tnoofont.h>
#include <QGraphicsEffect>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QApplication>
#include <QPalette>
#include <QTimer>
#include <QTouchEvent>

#include <QDebug>

const int accCharTable[6] = { 0xe123, 0xe11a, 0x20, 0xe10e, 0xe125, 0xe116 };

/*static*/
QString TscoreNote::getAccid(int accNr) {
//     const int accCharTable[6] = { 0xe123, 0xe11a, 0x20, 0xe10e, 0xe125, 0xe116 };
    QString str = QString(QChar(accCharTable[accNr + 2]));
    return str;
}


qreal TscoreNote::m_accidYoffset = 0.0;
qreal TscoreNote::m_accidScale = -1.0;
int TscoreNote::m_curentAccid = 0;
int TscoreNote::m_workPosY = 0;
QGraphicsEllipseItem* TscoreNote::m_workNote = 0;
QGraphicsSimpleTextItem* TscoreNote::m_workAccid = 0;
QList<QGraphicsLineItem*> TscoreNote::m_upLines;
QList<QGraphicsLineItem*> TscoreNote::m_downLines;
QList<QGraphicsLineItem*> TscoreNote::m_midLines;
QColor TscoreNote::m_workColor = -1;

TnoteControl* TscoreNote::m_rightBox = 0;
TnoteControl* TscoreNote::m_leftBox = 0;

/** To avoid creating many tips - one for every instance and waste RAM
 * this text exist as static variable 
 * and TscoreNote manages itself when status tip is necessary to be displayed. */
QString TscoreNote::m_staticTip = "";


void TscoreNote::adjustCursor() {
	if (m_rightBox && !m_rightBox->scene()->views().isEmpty()) {
		setPointedColor(m_workColor);
		m_rightBox->adjustSize();
		m_leftBox->adjustSize();
	}
}


void TscoreNote::setPointedColor(QColor color) {
    m_workColor = color;
    m_workNote->setPen(QPen(m_workColor, 0.2));
    m_workNote->setBrush(QBrush(m_workColor, Qt::SolidPattern));
    m_workAccid->setBrush(QBrush(m_workColor));
    for (int i = 0; i < m_upLines.size(); i++)
        m_upLines[i]->setPen(QPen(color, 0.2));
		for (int i = 0; i < m_midLines.size(); i++)
        m_midLines[i]->setPen(QPen(color, 0.2));
    for (int i = 0; i < m_downLines.size(); i++)
      m_downLines[i]->setPen(QPen(color, 0.2));
}

/*------------------------*/

//################################## CONSTRUCTOR ###################################
TscoreNote::TscoreNote(TscoreScene* scene, TscoreStaff* staff, int index) :
  TscoreItem(scene),
  m_mainPosY(0.0),
  m_accidental(0),
  m_index(index),
  m_stringText(0), m_stringNr(0),
  m_readOnly(false),
  m_nameText(0),
  m_ottava(0),
  m_bgColor(-1),
  m_noteAnim(0), m_accidToKeyAnim(false),
  m_selected(false),
  m_touchedToMove(false)
{
  setStaff(staff);
	setParentItem(staff);
#if defined (Q_OS_ANDROID)
	enableTouchToMouse(false); // Touch events are re-implemented here
#endif
  m_height = staff->height();
  m_mainColor = qApp->palette().text().color();
	m_note = new Tnote(0, 0, 0);

	createLines(m_mainDownLines, m_mainUpLines, m_mainMidLines);  
  m_mainNote = createNoteHead();
	
  m_mainAccid = new QGraphicsSimpleTextItem();
	m_mainAccid->setParentItem(m_mainNote);
	
  m_mainAccid->setFont(TnooFont(5));
	bool prepareScale = false;
	if (m_accidScale == -1.0) { // only when first TscoreNote is constructed
			m_staticTip = tr("Click to select a note, use mouse wheel to change accidentals.");
			m_mainAccid->setText(getAccid(1));
			m_accidScale = 6.0 / m_mainAccid->boundingRect().height();
			prepareScale = true;
	}
  m_mainAccid->setScale(m_accidScale);
	if (prepareScale) {
			m_accidYoffset = m_mainAccid->boundingRect().height() * m_accidScale * 0.34;
			m_mainAccid->setText("");
	}
	m_mainAccid->setPos(-3.0, - m_accidYoffset);
	
	if (!scene->views().isEmpty() && m_rightBox == 0)
			initNoteCursor();
  
  setColor(m_mainColor);
  m_mainNote->setZValue(34); // under
  m_mainAccid->setZValue(m_mainNote->zValue());
  if (staff->isPianoStaff())
		setAmbitus(40, 1);
	else
		setAmbitus(34, 1);
	connect(this, SIGNAL(statusTip(QString)), scene, SLOT(statusTipChanged(QString)));
}


TscoreNote::~TscoreNote() { // release work note and controls from destructing parent
	if (m_rightBox && (m_workNote->parentItem() == this || m_rightBox->parentItem() == parentItem())) {
			m_rightBox->setScoreNote(0);
			m_leftBox->setScoreNote(0);
			setCursorParent(0);
	}
	delete m_note;
}

//##############################################################################
//#################### PUBLIC METHODS    #######################################
//##############################################################################

void TscoreNote::adjustSize() {
	m_height = staff()->height();
	createLines(m_mainDownLines, m_mainUpLines, m_mainMidLines);
	createLines(m_downLines, m_upLines, m_midLines);
	setColor(m_mainColor);
}


void TscoreNote::setColor(QColor color) {
    m_mainColor = color;
    m_mainNote->setPen(QPen(m_mainColor, 0.2));
    m_mainNote->setBrush(QBrush(m_mainColor, Qt::SolidPattern));
    m_mainAccid->setBrush(QBrush(m_mainColor));
    for (int i = 0; i < m_mainUpLines.size(); i++)
        m_mainUpLines[i]->setPen(QPen(color, 0.2));
    for (int i = 0; i < m_mainDownLines.size(); i++)
      m_mainDownLines[i]->setPen(QPen(color, 0.2));
		for (int i = 0; i < m_mainMidLines.size(); i++)
      m_mainMidLines[i]->setPen(QPen(color, 0.2));
    if (m_stringText)
        m_stringText->setBrush(QBrush(m_mainColor));
}


void TscoreNote::selectNote(bool sel) {
	m_selected = sel;
}


void TscoreNote::setWorkAccid(int accNr) {
		m_curentAccid = accNr;
		m_workAccid->setText(getAccid(accNr));
}


void TscoreNote::moveNote(int posY) {
// 		if (posY == 0 || !(posY >= m_ambitMax - 1 && posY <= m_ambitMin)) {
		if (posY == 0 || !(posY >= 1 && posY <= m_height - 3)) {
				hideNote();
				m_mainAccid->setText(" ");
				m_accidental = 0;
				return;
    }
		if (!m_mainNote->isVisible()) {
        m_mainNote->show();
        m_mainAccid->show();
    }
		if (m_noteAnim) { // initialize animation
				m_noteAnim->setMoving(m_mainNote->pos(), QPointF(3.0, posY));
				m_noteAnim->startAnimations();
		} else { // just move a note
			m_mainNote->setPos(3.0, posY);
		}
		m_mainPosY = posY;
    int noteNr = (56 + staff()->notePosRelatedToClef(staff()->fixNotePos(posY))) % 7;
		QString newAccid = getAccid(m_accidental);
		if (staff()->accidInKeyArray[noteNr]) {
      if (m_accidental == 0) {
					newAccid = getAccid(3); // neutral
					m_mainAccid->hide();
					if (m_accidToKeyAnim)
							emit fromKeyAnim(newAccid, m_mainAccid->scenePos(), m_mainPosY);
			} else {
					if (staff()->accidInKeyArray[noteNr] == m_accidental) {
						if (m_accidToKeyAnim)
								emit toKeyAnim(newAccid, m_mainAccid->scenePos(), m_mainPosY);
						if (staff()->extraAccids()) // accidental from key signature in brackets
							newAccid = QString(QChar(accCharTable[m_accidental + 2] + 1));
						else
							newAccid = " "; // hide accidental
						
					}
      }
    }
    if (m_noteAnim) {
				m_accidAnim->startCrossFading(newAccid, m_mainColor);
    } else {
				m_mainAccid->setText(newAccid);
				m_mainAccid->show();
    }

    setStringPos();
		checkLines(posY, m_mainDownLines, m_mainUpLines, m_mainMidLines);
}


void TscoreNote::setNote(int notePos, int accNr, const Tnote& n) {
	m_accidental = accNr;
	*m_note = n;
	moveNote(notePos);
	if (m_mainPosY == 0)
			*m_note = Tnote(); // set note to null if beyond the score possibilities
	if (m_nameText)
			showNoteName();
	update();
}


void TscoreNote::hideNote() {
    m_mainNote->hide();
    m_mainAccid->hide();
    hideLines(m_mainUpLines);
    hideLines(m_mainDownLines);
		if (staff()->isPianoStaff())
			hideLines(m_mainMidLines);
    m_mainPosY = 0;
    m_accidental = 0;
		m_mainNote->setPos(3.0, 0);
}


void TscoreNote::moveWorkNote(const QPointF& newPos) {
	QGraphicsSceneHoverEvent moveEvent(QEvent::GraphicsSceneHoverMove);
	moveEvent.setPos(newPos);
	hoverMoveEvent(&moveEvent);
}


void TscoreNote::hideWorkNote() {
	if (m_workNote->isVisible()) {
    m_workNote->hide();
    m_workAccid->hide();
    hideLines(m_upLines);
    hideLines(m_downLines);
		if (staff()->isPianoStaff())
			hideLines(m_midLines);
    m_workPosY = 0.0;
	}
}


void TscoreNote::markNote(QColor blurColor) {
    if (blurColor == -1) {
      m_mainNote->setPen(Qt::NoPen);
      m_mainNote->setGraphicsEffect(0);
    } else {
      m_mainNote->setPen(QPen(blurColor, 0.2));
      m_mainNote->setGraphicsEffect(new TdropShadowEffect(blurColor));
    }
}


void TscoreNote::setString(int realNr) {
	if (!m_stringText) {
        m_stringText = new QGraphicsSimpleTextItem();
				m_stringText->setFont(TnooFont(5));
        m_stringText->setBrush(QBrush(m_mainColor));
        m_stringText->setParentItem(this);
        m_stringText->setZValue(-1);
    }
    m_stringText->setText(QString("%1").arg(realNr));
		m_stringText->setScale(5.0 / m_stringText->boundingRect().width());
		m_stringNr = realNr;
    setStringPos();
}


void TscoreNote::removeString() {
		if (m_stringText) {
        delete m_stringText;
        m_stringText = 0;
    }
    m_stringNr = 0;
}


void TscoreNote::setReadOnly(bool ro) {
		setAcceptHoverEvents(!ro);
		m_readOnly = ro;
}


void TscoreNote::showNoteName() {
	if (!m_nameText) {
		m_nameText = new QGraphicsTextItem();
		m_nameText->setDefaultTextColor(m_mainColor);
		m_nameText->setParentItem(this);
		m_nameText->setZValue(10);
		m_nameText->setAcceptHoverEvents(false);
		QGraphicsDropShadowEffect *dropEff = new QGraphicsDropShadowEffect();
			dropEff->setColor(Qt::darkCyan);
			dropEff->setOffset(0.7, 0.7);
			dropEff->setBlurRadius(5.0);
		m_nameText->setGraphicsEffect(dropEff);
	}
	if (m_note->note) {
			m_nameText->setHtml(m_note->toRichText());
			m_nameText->setScale(8.0 / m_nameText->boundingRect().height());
			if (m_nameText->boundingRect().width() * m_nameText->scale() > boundingRect().width())
					m_nameText->setScale(boundingRect().width() / (m_nameText->boundingRect().width()));
// 			qreal yy;
// 			if (notePos() < staff()->upperLinePos() + 4.0)
// 				yy = staff()->upperLinePos() + 7.5; // under staff
// 			else if (staff()->isPianoStaff() && notePos() > staff()->lowerLinePos()) // above lower staff
// 				yy = staff()->lowerLinePos() - m_nameText->boundingRect().height() * m_nameText->scale();
// 			else // above upper staff
// 				yy = staff()->upperLinePos() - m_nameText->boundingRect().height() * m_nameText->scale() + 1.0;
			m_nameText->setPos((8.0 - m_nameText->boundingRect().width() * m_nameText->scale()) * 0.75, /*yy);*/
							notePos() > staff()->upperLinePos() ? 
										notePos() - (m_nameText->boundingRect().height() + 2.0) * m_nameText->scale(): // above note
										notePos() + m_mainNote->boundingRect().height()); // below note
			m_nameText->show();
	} else
			m_nameText->hide();
}


void TscoreNote::removeNoteName() {
	delete m_nameText;
	m_nameText = 0;
}


void TscoreNote::enableNoteAnim(bool enable, int duration) {
	if (enable) {
			if (!m_noteAnim) {
					m_noteAnim = new TcombinedAnim(m_mainNote, this);
					m_noteAnim->setDuration(duration);
					m_noteAnim->setMoving(mainNote()->pos(), mainNote()->pos());
					m_noteAnim->moving()->setEasingCurveType(QEasingCurve::InExpo);
					m_noteAnim->setScaling(1.0, 1.5);
					m_noteAnim->scaling()->setEasingCurveType(QEasingCurve::OutQuint);
					m_accidAnim = new TcrossFadeTextAnim(m_mainAccid, this);
			}
			m_accidAnim->setDuration(duration);
	} else {
			if (m_noteAnim) {
				delete m_noteAnim;
				m_noteAnim = 0;
				delete m_accidAnim;
				m_accidAnim = 0;
			}
	}
}


void TscoreNote::setAmbitus(int lo, int hi) { 
	m_ambitMin = qBound(1, lo, (int)m_height - 1); 
	m_ambitMax = qBound(1, hi, (int)m_height - 1);
}


QRectF TscoreNote::boundingRect() const{
    return QRectF(0, 0, 7.0, m_height);
}


void TscoreNote::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	if (m_bgColor != -1) {
// 			paintBackground(painter, m_bgColor);
		QPointF center(3.5, m_mainPosY + 1.0);
		if (m_mainPosY == 0)
			center.setY(staff()->upperLinePos() + 4.0);
		QRadialGradient gr(center, 10.0);
		QColor c1 = m_bgColor;
		c1.setAlpha(40);
		QColor c2 = m_bgColor;
		c2.setAlpha(80);
		gr.setColorAt(0.0, c1);
		gr.setColorAt(0.9, c1);
		gr.setColorAt(0.95, c2);
		gr.setColorAt(1.0, Qt::transparent);
		painter->setBrush(gr);
		painter->setPen(Qt::NoPen);
		painter->drawRect(0.0, qMax(center.y() - 10.0, 0.0), 7.0, qMin(center.y() + 10.0, m_height));
	}
}


void TscoreNote::keyAnimFinished() {
	if (!m_readOnly)
			m_mainAccid->show();
}


//#################################################################################################
//########################################## PROTECTED   ##########################################
//#################################################################################################

void TscoreNote::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
// 	qDebug() << "hoverEnterEvent";
	if (staff()->controlledNotes()) {
		if (right()->isEnabled()) {
			right()->setPos(pos().x() + boundingRect().width(), 0.0);
			right()->setScoreNote(this);
		}
		if (left()->isEnabled()) {
			left()->setPos(pos().x() - m_leftBox->boundingRect().width(), 0.0);
			left()->setScoreNote(this);
		}
	}
	if (m_workNote->parentItem() != this)
			setCursorParent(this);
  if ((event->pos().y() >= m_ambitMax) && (event->pos().y() <= m_ambitMin)) {
			m_workNote->show();
			m_workAccid->show();
  }
  emit statusTip(m_staticTip);
  TscoreItem::hoverEnterEvent(event);
}


void TscoreNote::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
// 	qDebug() << "hoverLeaveEvent";
  hideWorkNote();
	if (staff()->controlledNotes()) {
			if (right()->isEnabled() && !right()->hasCursor())
					right()->hideWithDelay();
			if (left()->isEnabled() && !left()->hasCursor())
					left()->hideWithDelay();
	}
  TscoreItem::hoverLeaveEvent(event);
}


void TscoreNote::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
//   if ((event->pos().y() >= m_ambitMax) && (event->pos().y() <= m_ambitMin)) {
	if ((event->pos().y() >= 1) && (event->pos().y() <= m_height - 3.0)) {
		if (staff()->isPianoStaff() && 
			(event->pos().y() >= staff()->upperLinePos() + 10.6) && (event->pos().y() <= staff()->lowerLinePos() - 2.4)) {
				hideWorkNote();
				return;
		}
    if (event->pos().y() != m_workPosY) {
			if (m_curentAccid != scoreScene()->currentAccid()) { // update accidental symbol
					m_curentAccid = scoreScene()->currentAccid();
					m_workAccid->setText(getAccid(m_curentAccid));
			}
      m_workPosY = event->pos().y() - 0.6;
      m_workNote->setPos(3.0, m_workPosY);
      if (!m_workNote->isVisible()) {
        m_workNote->show();
        m_workAccid->show();
      }
      checkLines(m_workPosY, m_downLines, m_upLines, m_midLines);
    }
  }
}


void TscoreNote::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  if (event->button() == Qt::LeftButton && m_workPosY) {
        m_accidental = m_curentAccid;
        moveNote(m_workPosY);
        emit noteWasClicked(m_index);
				if (m_nameText)
					showNoteName();
				update();
    } else if (event->button() == Qt::RightButton && m_workPosY) {
				if (staff()->selectableNotes() || staff()->controlledNotes()) {
						setBackgroundColor(qApp->palette().highlight().color());
						emit noteWasSelected(m_index);
						update();
				}
    }
}


void TscoreNote::wheelEvent(QGraphicsSceneWheelEvent* event) {
		if (m_readOnly)
			return;
    int prevAcc = m_curentAccid;
    if (event->delta() < 0) {
        m_curentAccid--;
        if (m_curentAccid < (-scoreScene()->doubleAccidsFuse()))
          m_curentAccid = -(scoreScene()->doubleAccidsFuse());
    } else {
        m_curentAccid++;
        if (m_curentAccid > scoreScene()->doubleAccidsFuse())
          m_curentAccid = scoreScene()->doubleAccidsFuse();
    }
    if (prevAcc != m_curentAccid) {
				setWorkAccid(m_curentAccid);
        scoreScene()->setCurrentAccid(m_curentAccid);
				staff()->noteChangedAccid(m_curentAccid);
//         emit accidWasChanged(m_curentAccid);
    }
}


#if defined(Q_OS_ANDROID)
void TscoreNote::touched(const QPointF& cPos) {
	if (m_readOnly)
			return;
	TscoreItem::touched(cPos);			
}


void TscoreNote::untouched(const QPointF& cPos) {
	if (m_readOnly)
			return;
	TscoreItem::untouched(cPos);
	m_touchedToMove = false;
	if (staff()->controlledNotes()) {
			if (right()->isEnabled() && !right()->hasCursor())
					right()->hideWithDelay();
			if (left()->isEnabled() && !left()->hasCursor())
					left()->hideWithDelay();
			if (isCursorVisible()) {
					QTimer::singleShot(950, this, SLOT(hideWorkNote()));
			}
	}		
}


void TscoreNote::touchMove(const QPointF& cPos) {
	if (m_readOnly)
			return;
  if (m_touchedToMove && isCursorVisible())
		moveWorkNote(cPos);
}


void TscoreNote::shortTap(const QPointF& cPos) {
	if (m_readOnly)
			return;
	if (isCursorVisible()) {
			if (cPos.y() >= m_workPosY - 4 && cPos.y() <= m_workPosY + 4) {
				if (cPos.x() >= 0.0 && cPos.x() <= 7.0) { // if finger taken over note - it was selected
						QGraphicsSceneMouseEvent me(QEvent::MouseButtonPress);
						me.setPos(cPos);
						me.setButton(Qt::LeftButton);
						mousePressEvent(&me);
				}
			}
			hideWorkNote();
			right()->hide();
			left()->hide();
	} else 
			emit noteWasSelected(m_index);
}


void TscoreNote::longTap(const QPointF& cPos) {
	if (m_readOnly)
			return;
	if (staff()->controlledNotes()) {
			m_touchedToMove = true;
			if (right()->isEnabled()) {
			right()->setPos(pos().x() + boundingRect().width(), 0.0);
			right()->setScoreNote(this);
		}
		if (left()->isEnabled()) {
			left()->setPos(pos().x() - m_leftBox->boundingRect().width(), 0.0);
			left()->setScoreNote(this);
		}
	}
	if (m_workNote->parentItem() != this)
			setCursorParent(this);
  if ((cPos.y() >= m_ambitMax) && (cPos.y() <= m_ambitMin)) {
			m_workNote->show();
			m_workAccid->show();
  }
}
#endif

//##########################################################################################################
//####################################         PRIVATE     #################################################
//##########################################################################################################

QGraphicsEllipseItem* TscoreNote::createNoteHead() {
  QGraphicsEllipseItem *noteHead = new QGraphicsEllipseItem();
	noteHead->setParentItem(this);
  noteHead->setRect(0, 0, 3.5, 2);
  noteHead->hide();
  return noteHead;
}


QGraphicsLineItem* TscoreNote::createNoteLine(int yPos) {
	QGraphicsLineItem *line = new QGraphicsLineItem();
	line->hide();
	registryItem(line);
	line->setZValue(7);
	line->setLine(2.5, yPos, boundingRect().width(), yPos);
	return line;
}


void TscoreNote::hideLines(TaddLines& linesList) {
	for (int i=0; i < linesList.size(); i++)
		linesList[i]->hide();
}


void TscoreNote::setStringPos() {
	if (m_stringText) {
			qreal yy = staff()->upperLinePos() + 9.0; // below the staff
			if (m_mainPosY > staff()->upperLinePos() + 4.0) 
					yy = staff()->upperLinePos() - 7.0; // above the staff 
			m_stringText->setPos(6.5 - m_stringText->boundingRect().width() * m_stringText->scale(), yy);
			// 6.5 is right position of note head
	}
}


void TscoreNote::initNoteCursor() {
	m_workColor = qApp->palette().highlight().color();
  m_workColor.setAlpha(200);
	createLines(m_downLines, m_upLines, m_midLines);
	m_workNote = createNoteHead();
  m_workNote->setGraphicsEffect(new TdropShadowEffect(m_workColor));
	m_workAccid = new QGraphicsSimpleTextItem();
  m_workAccid->setBrush(QBrush(m_workColor));
	m_workAccid->setParentItem(m_workNote);
  m_workAccid->hide();
	m_workAccid->setFont(TnooFont(5));
	m_workAccid->hide();
	m_workAccid->setScale(m_accidScale);
	m_workAccid->setPos(-3.0, - m_accidYoffset);
	m_workNote->setZValue(35);
  m_workAccid->setZValue(m_workNote->zValue());
	setPointedColor(m_workColor);
	
	m_rightBox = new TnoteControl(staff(), scoreScene());
	m_leftBox = new TnoteControl(staff(), scoreScene());
	m_leftBox->addAccidentals();
}


void TscoreNote::checkLines(int curPos, TaddLines& low, TaddLines& upp, TaddLines& mid) {
	for (int i = 0; i < upp.size(); i++) {
		if (curPos < upp[i]->line().y1())
			upp[i]->show();
		else 
			upp[i]->hide();
	}
	if (staff()->isPianoStaff()) {
		if (curPos == mid[0]->line().y1() - 1)
			mid[0]->show();
		else
			mid[0]->hide();
		if (curPos == mid[1]->line().y1() - 1)
			mid[1]->show();
		else
			mid[1]->hide();
	}
	for (int i = 0; i < low.size(); i++) {
		if (curPos > low[i]->line().y1() - 2) 
			low[i]->show();
		else 
			low[i]->hide();
	}
}


void TscoreNote::setCursorParent(TscoreItem* item) {
	m_workNote->setParentItem(item);
	for (int i = 0; i < m_downLines.size(); i++)
		m_downLines[i]->setParentItem(item);
	for (int i = 0; i < m_midLines.size(); i++)
		m_midLines[i]->setParentItem(item);
	for (int i = 0; i < m_upLines.size(); i++)
		m_upLines[i]->setParentItem(item);
}


void TscoreNote::createLines(TaddLines& low, TaddLines& upp, TaddLines& mid) {
	deleteLines(upp);
	deleteLines(mid);
	deleteLines(low);
  int i = staff()->upperLinePos() - 2;
  while (i > 0) { // upper lines
		upp << createNoteLine(i);
    i -= 2;
  }
  i = staff()->upperLinePos() + 10.0; // distance between upper and lower (or lower grand staff) staff line
  if (staff()->isPianoStaff()) {
		i = staff()->lowerLinePos() + 10.0;
		mid << createNoteLine(staff()->upperLinePos() + 10);
		mid << createNoteLine(staff()->lowerLinePos() - 2);
	}
  while (i < m_height - 2) {
		low << createNoteLine(i);
    i += 2;
  }
}


void TscoreNote::deleteLines(TaddLines& linesList) {
	for (int i = 0; i < linesList.size(); i++)
		delete linesList[i];
	linesList.clear();
}


/*
void TscoreNote::checkOctavation() {
	bool notPossible = false;
			if (pos < m_ambitMax) {
				m_ottava = -1;
				pos += 7;
				if (pos < m_ambitMax) {
					m_ottava = -2;
					pos += 7;
					if (pos < m_ambitMax)
						notPossible = true;
				}
			} else {
				m_ottava = 1;
				pos -= 7;
				if (pos > m_ambitMin) {
					m_ottava = 2;
					pos -= 7;
					if (pos > m_ambitMin)
						notPossible = true;
				}
			}
			if (notPossible) {
				m_ottava = 0;
				m_mainPosY = 0;
				hideNote();
				return;
			}
			qDebug() << "octavation required" << (int)m_ottava;
} */




