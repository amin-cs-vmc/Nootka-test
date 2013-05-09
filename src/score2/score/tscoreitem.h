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

#ifndef TSCOREITEM_H
#define TSCOREITEM_H

#include <QGraphicsObject>

class TscoreScene;

/** TscoreItem is base class for all items on the score:
* staff, clef, key signature, notes, scordature etc..
* It automaticaly adds created item to the TscoreScene 
* given as constructor parameter.
* Also this class manages status tips. 
* If the status is set, it emits statusTip(QString) signal.
*/
class TscoreItem : public QGraphicsObject
{
    Q_OBJECT

public:
    TscoreItem(TscoreScene *scene);
    
    QString statusTip() { return m_statusTip; }
    void setStatusTip(QString status);
    TscoreScene* scoreScene() { return m_scene; }
    
signals:
    void statusTip(QString);

protected:
      /** If status tip is set it sends signal.
       * Notice!! 
       * Any subclass has to call this when hoverEnterEvent is overriden
       * and statusTip funcionality is expected. */
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

private:
    QString         m_statusTip;
    TscoreScene     *m_scene;

};

#endif // TSCOREITEM_H
