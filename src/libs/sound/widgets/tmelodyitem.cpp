/***************************************************************************
 *   Copyright (C) 2015 by Tomasz Bojczuk                                  *
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

#include "tmelodyitem.h"
#include "tqtaudioin.h"
#include "tqtaudioout.h"
#include <tmtr.h>
#include <tpath.h>
#include <graphics/tnotepixmap.h>
#include <graphics/tdropshadoweffect.h>
#include <graphics/tgraphicstexttip.h>
#include <QtWidgets/qaction.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicssceneevent.h>
#include <QtGui/qpen.h>
#include <QtGui/qpainter.h>
#include <QtCore/qtimer.h>
#include <QtCore/qmath.h>

#include <QtCore/qdebug.h>


#define SHORT_TAP_TIME (150)


/**
 * NOTICE!!!!!!!!!!!
 * It works only with tqtaudioin - means only under Android
 */


TmelodyItem* TmelodyItem::m_instance = 0;


TmelodyItem::TmelodyItem() :
  QGraphicsObject(0),
  m_touched(false),
  m_selectedAction(0)
{
  m_instance = this;
  setAcceptTouchEvents(true);
  m_playDot = createDot(1);
  m_recDot = createDot(2);
  m_snifDot = createDot(3);
  m_timer = new QTimer(this);
  m_timer->setSingleShot(true);
  m_timer->setTimerType(Qt::PreciseTimer);
  m_timer->setInterval(SHORT_TAP_TIME);
  connect(m_timer, &QTimer::timeout, this, &TmelodyItem::createFlyActions);
}


TmelodyItem::~TmelodyItem()
{
  m_instance = 0;
}


bool TmelodyItem::audioInEnabled() {
  return (bool)TaudioIN::instance();
}


bool TmelodyItem::audioOutEnabled() {
  return (bool)TaudioOUT::instance();
}



void TmelodyItem::setPlaying(bool isPlay) {
  if (isPlay)
    setDotColor(m_playDot, Qt::green);
  else
    setDotColor(m_playDot, qApp->palette().text().color());
}


void TmelodyItem::setRecording(bool isRecord) {
  if (isRecord)
    setDotColor(m_recDot, Qt::red);
  else
    setDotColor(m_recDot, qApp->palette().text().color());
}


void TmelodyItem::setListening(bool isListen) {
  if (isListen)
    setDotColor(m_snifDot, qApp->palette().highlight().color());
  else
    setDotColor(m_snifDot, qApp->palette().text().color());
}


QRectF TmelodyItem::boundingRect() const {
  return QRectF(0, 0, Tmtr::fingerPixels(), Tmtr::fingerPixels());
}

//#################################################################################################
//###################              PROTECTED           ############################################
//#################################################################################################

void TmelodyItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)
  QColor bg(m_touched ? qApp->palette().highlight().color() : qApp->palette().base().color());
  bg.setAlpha(200);
  painter->setBrush(bg);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(boundingRect().adjusted(0, 0, -Tmtr::fingerPixels() / 2, 0), 30, 30, Qt::RelativeSize);
}


QGraphicsEllipseItem* TmelodyItem::createDot(int dotNr) {
  QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(0, 0, boundingRect().width() / 6, boundingRect().width() / 6, this);
  setDotColor(dot, qApp->palette().text().color());
  dot->setPos((boundingRect().width() / 2 - (dot->boundingRect().width())) / 2,
              dotNr * ((boundingRect().height() - 3 * dot->boundingRect().height()) / 4) + (dotNr - 1) * dot->boundingRect().height());
  return dot;
}


void TmelodyItem::setDotColor(QGraphicsEllipseItem* dot, const QColor& c) {
  QColor dotColor = c;
  dotColor.setAlpha(225);
  dot->setPen(QPen(dotColor, 1.0));
  dotColor.setAlpha(150);
  dot->setBrush(QBrush(dotColor));
}


void TmelodyItem::createFlyActions() {
  m_selectedAction = 0;
  qreal angle = qDegreesToRadians(88.0) / (m_actions.size() - 1);
  qreal off = qDegreesToRadians(89.0);
  qreal r = Tmtr::fingerPixels() * 4;
  int iconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
  for (int i = 0; i < m_actions.size(); ++i) {
    auto it = new TgraphicsTextTip(pixToHtml(m_actions[i]->icon().pixmap(iconSize, iconSize)), qApp->palette().highlight().color());
    scene()->addItem(it);
    it->setData(0, i);
    m_flyList << it;
    it->setPos(qSin(off - i * angle) * r, qCos(off - i * angle) * r);
  }
}


//#################################################################################################
//###################              EVENTS              ############################################
//#################################################################################################

void TmelodyItem::mousePressEvent(QGraphicsSceneMouseEvent*) {
  m_touched = true;
  update();
  emit touched();
  m_timer->start();
}


void TmelodyItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  if (!m_timer->isActive() && m_touched && // actions were created
    event->pos().x() > boundingRect().width() || event->pos().y() > boundingRect().height()) { // enough as long as this item is at (0, 0)
      auto *it = static_cast<TgraphicsTextTip*>(scene()->itemAt(mapToScene(event->pos()), transform()));
      if (m_flyList.contains(it)) {
        m_touched = false; // ignore menu
        m_selectedAction = m_actions.at(it->data(0).toInt());
        it->setBgColor(Qt::green);
      }
  }
  QGraphicsItem::mouseMoveEvent(event);
}



void TmelodyItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  m_timer->stop();
  for (int i = 0 ; i < m_flyList.size(); ++i)
    delete m_flyList[i];
  m_flyList.clear();
  if (!m_touched && m_selectedAction) {
    update();
    m_selectedAction->trigger();
    m_selectedAction = 0;
  } else {
    m_touched = false;
    update();
    emit menuSignal();
  }
  QGraphicsItem::mouseReleaseEvent(event);
}






