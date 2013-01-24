/***************************************************************************
 *   Copyright (C) 2012-2013 by Tomasz Bojczuk                             *
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

#ifndef TYAXIS_H
#define TYAXIS_H


#include "tabstractaxis.h"


class TYaxis : public TabstractAxis
{

public:
  
    /** Unit of an axis values. */
  enum Eunit {
    e_timeInSec, // time in seconds (default)
    e_questionNr // number of questions
  };
  
  TYaxis();
  virtual ~TYaxis() {}
  

      /** Maximum value of a data on Y axis. Needs update(). */
  void setMaxValue(qreal val);
  qreal maxValue() { return m_maxVal; }
  double mapValue(double val) { return length() - TabstractAxis::mapValue(val); }
  
  virtual QRectF boundingRect();
  
  void setUnit(Eunit unit); // sets an unit
  Eunit unit();
  
  void getYforGrid(QList<double> &yList); // Puts list of Y to yList coordinates to paint grid lines
  
protected:
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
  
private:
  qreal m_maxVal, m_multi, m_multi2;
  int m_textPosOffset; // half of text height
  int m_loop, m_top;
  bool m_halfTick;
  Eunit m_unit;
  QString m_unitDesc; // unit description string f.e: time [s]

};

#endif // TYAXIS_H
