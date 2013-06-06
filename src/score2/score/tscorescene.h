/***************************************************************************
 *   Copyright (C) 2013 by Tomasz Bojczuk                                  *
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

#ifndef TSCORESCENE_H
#define TSCORESCENE_H

#include <QGraphicsScene>

// priority (like z value) of score elements
#define P_LINES  (5)
#define P_NOTES  (20)
#define P_CURSOR (30) // pointing note
#define P_KEY    (20)
#define P_CLEF   (50)
#define P_TIP    (100) // tip (like clef selector) over all elements

class TscoreItem;
class QGraphicsItem;

class TscoreScene : public QGraphicsScene
{
  
  Q_OBJECT
  
public:
    TscoreScene(QObject* parent = 0);
        
        
    void setDoubleAccidsEnabled(bool enable);
        /** Returns value 2 when double accidentals are enabled and 1 if not. */
    qint8 doubleAccidsFuse() { return m_dblAccFuse; }
        /** Working accidental in TscoreNote segment.
        * also changed by buttons. */
    void setCurrentAccid(char accid) { m_currentAccid = (char)qBound(-2, (int)accid, 2);}
    char currentAccid() { return m_currentAccid; }
    
        /** This method is simple like QGraphicsItem::setZvalue().
         * Unfortunately parent-children hierarchy of items breaks its simplicity. 
         * Every TscoreItem can call this and then setZvalue will called outside
         * with given priority. Following const are defined: 
         * P_LINES  (5) 
         * P_NOTES  (20) 
         * P_CURSOR (30) - pointing note
         * P_KEY    (20)
         * P_CLEF   (50)
         * P_TIP    (100) // tip (like clef selector) over all elements
         */
    void setPriority(TscoreItem* item, int z);
    
        /** Adds blur graphics effect. In the contrary to QGraphicsItem::setGraphicsEffect() 
         * a radius value in global scale.  */
    void addBlur(QGraphicsItem *item, qreal radius);
signals:
    void statusTip(QString);
    
protected slots:
    void statusTipChanged(QString status) { emit statusTip(status); }
    
private:
        /** It is @p 2 if double accidentals are enabled and @p 1 if not*/
    qint8 m_dblAccFuse;
    char  m_currentAccid;
        

};

#endif // TSCORESCENE_H
