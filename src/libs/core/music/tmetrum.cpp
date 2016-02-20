/***************************************************************************
 *   Copyright (C) 2016 by Tomasz Bojczuk                                  *
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

#include "tmetrum.h"
#include <tnoofont.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpalette.h>
#include <QtWidgets/qapplication.h>

// qDebug() << "Metrum" << QString("%1/%2").arg(m.upper()).arg(m.lower());

int Tmetrum::upper() const {
  switch (m_meter) {
    case e_2_4: return 2;
    case e_3_4:
    case e_3_8: return 3;
    case e_4_4: return 4;
    case e_5_4:
    case e_5_8: return 5;
    case e_6_4:
    case e_6_8: return 6;
    case e_7_4:
    case e_7_8: return 7;
    case e_9_8: return 9;
    case e_12_8: return 12;
    default: return 0;
  }
}


int Tmetrum::lower() const {
  int v = (int)m_meter;
  if (v > 0) {
    if (v <= 32)
      return 4;
    else
      return 8;
  } else
      return 0;
}


qreal Tmetrum::value() const {
  switch (m_meter) {
    case e_2_4: return 0.5;
    case e_6_8:
    case e_3_4: return 0.75;
    case e_4_4: return 1.0;
    case e_5_4: return 1.25;
    case e_12_8:
    case e_6_4: return 1.5;
    case e_7_4: return 1.75;
    case e_3_8: return 0.375;
    case e_5_8: return 0.625;
    case e_7_8: return 0.875;
    case e_9_8: return 1.125;
    default: return 0.0;
  }
}


QPixmap Tmetrum::pixmap(int fontSize, const QColor& c) {
  if (meter() == e_none)
    return QPixmap();

  TnooFont font(fontSize);
  QFontMetrics fm(font);
  QString upperDigit = TnooFont::digit(upper());
  QPixmap pix(QSize(fm.boundingRect(upperDigit).width() + 4, fontSize + 12)); // upper digit is usually wider
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing, true);
  p.setFont(font);
  p.setPen(c == -1 ? qApp->palette().text().color() : c);
  p.setBrush(Qt::NoBrush);
  p.drawText(QRect(0, 0, pix.width(), fontSize / 2 + 8), Qt::AlignCenter, upperDigit);
  p.drawText(QRect(0, fontSize / 2 - 1, pix.width(), fontSize / 2 + 8), Qt::AlignCenter, TnooFont::digit(lower()));
  return pix;
}


void Tmetrum::toXml(QXmlStreamWriter& xml) const
{

}


bool Tmetrum::fromXml(QXmlStreamReader& xml)
{
  return false;
}





