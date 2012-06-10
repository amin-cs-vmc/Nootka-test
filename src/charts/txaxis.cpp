/***************************************************************************
 *   Copyright (C) 2012 by Tomasz Bojczuk                                  *
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
 *  You should have received a copy of the GNU General Public License	     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "txaxis.h"
#include "tqaunit.h"
#include "texam.h"
#include "tnotename.h"
#include <texamlevel.h>
#include "tgraphicstexttip.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>



TXaxis::TXaxis(QList< TQAunit >* answers, TexamLevel* level) :
  m_qWidth(70)
{
  if (answers && level)
      setAnswersList(answers, level);
  else {
      setLength(100);
      m_answers = 0;
      m_level = 0;
  }
  axisScale = m_qWidth;
}

TXaxis::~TXaxis()
{}


void TXaxis::setAnswersList(QList< TQAunit >* answers, TexamLevel* level) {
  m_answers = answers;
  m_level = level;
  setLength(m_qWidth * (m_answers->size() + 1));
  update(boundingRect());
  m_ticTips.clear();
  for (int i = 0; i < m_answers->size(); i++) {
      QString txt = QString("%1.<br><b>%2</b>").arg(i+1).
      arg(TnoteName::noteToRichText(m_answers->operator[](i).qa.note));
      if (m_level->useKeySign && 
        (m_answers->operator[](i).questionAs == TQAtype::e_asNote || m_answers->operator[](i).answerAs == TQAtype::e_asNote)) {
          if (m_answers->operator[](i).key.isMinor())
            txt += "<br><i>" + m_answers->operator[](i).key.getMinorName() + "</i>";
          else
            txt += "<br><i>" + m_answers->operator[](i).key.getMajorName() + "</i>";
      }
      QGraphicsTextItem *ticTip = new QGraphicsTextItem();
      ticTip->setHtml(txt);
      TgraphicsTextTip::alignCenter(ticTip);
      scene()->addItem(ticTip);
      ticTip->setPos(pos().x() + mapValue(i+1) - ticTip->boundingRect().width() / 2 , pos().y() + 15);
      m_ticTips << ticTip;
  }
}

void TXaxis::setAnswersLists(QList< QList< TQAunit* > >& listOfLists, TexamLevel* level) {
  int ln = 0, cnt = 0;
  m_level = level;
  for (int i = 0; i < listOfLists.size(); i++) {
    ln += listOfLists[i].size();
  }
  setLength(m_qWidth * (ln + 1));
  update(boundingRect());
  m_ticTips.clear();
  for (int i = 0; i < listOfLists.size(); i++) {
    for (int j = 0; j < listOfLists[i].size(); j++) {
      cnt++;
      QString txt = QString("<b>%1</b>").arg(TnoteName::noteToRichText(listOfLists[i].operator[](j)->qa.note));
      if (m_level->useKeySign && 
        (listOfLists[i].operator[](j)->questionAs == TQAtype::e_asNote || 
        listOfLists[i].operator[](j)->answerAs == TQAtype::e_asNote)) {
          if (listOfLists[i].operator[](j)->key.isMinor())
            txt += "<br><i>" + listOfLists[i].operator[](j)->key.getMinorName() + "</i>";
          else
            txt += "<br><i>" + listOfLists[i].operator[](j)->key.getMajorName() + "</i>";
      }
      QGraphicsTextItem *ticTip = new QGraphicsTextItem();
      ticTip->setHtml(txt);
      TgraphicsTextTip::alignCenter(ticTip);
      scene()->addItem(ticTip);
      ticTip->setPos(pos().x() + mapValue(cnt) - ticTip->boundingRect().width() / 2 , pos().y() + 15);
      m_ticTips << ticTip;
    }
  }      
}



void TXaxis::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  
    Q_UNUSED(option)
    Q_UNUSED(widget)
      
  qreal half = axisWidth /  2.0;
  painter->drawLine(0, half, length(), half);
  drawArrow(painter, QPointF(length(), half), true);
  
  int b = length() / m_qWidth -1;
  for (int i=1; i <= b; i++) {
    painter->drawLine(i*m_qWidth, half, i*m_qWidth, half + tickSize);
  }
  
}

QRectF TXaxis::boundingRect()
{
  QRectF rect(0 ,0, length(), axisWidth);
//  rect.translate(1, axisWidth / 2.0);
  return rect;
}

