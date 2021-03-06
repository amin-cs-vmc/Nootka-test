/***************************************************************************
 *   Copyright (C) 2012 by Tomasz Bojczuk                                  *
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
 *  You should have received a copy of the GNU General Public License	     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/


#ifndef TABSTRACTAXIS_H
#define TABSTRACTAXIS_H

#include <QGraphicsItem>
#include <QFont>


/**
 * Base class for X and Y axis
 */ 
class TabstractAxis : public QGraphicsItem
{
public:
    TabstractAxis();
    virtual ~TabstractAxis() {}

    void setLength(qreal len);
    qreal length() const { return m_length; } /** Returns a length of a axis*/

    QFont font() { return m_font; }

    void setFont(QFont f);

    QRectF rectBoundText(QString txt) const;
        /** Returns value mapped to axis scale. */
    virtual double mapValue(double val) { return axisScale * val; }
        /** Paints arrow at the end of axis. */
    static void drawArrow(QPainter *painter, QPointF endPoint, bool isHorizontal = true);
    double axisFactor() { return axisScale; } // factor of the axis

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) {}
        /** Default QRectF - is valid for horizontal layout. */
    virtual QRectF boundingRect() const { return QRectF(0 ,0, m_length, axisWidth); }
    static const int axisWidth, arrowSize, tickSize;
    double axisScale;


private:
    qreal m_length;
    QFont m_font;

};

#endif // TABSTRACTAXIS_H
