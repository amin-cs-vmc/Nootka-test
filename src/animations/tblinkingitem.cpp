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

#include "tblinkingitem.h"


TblinkingItem::TblinkingItem(QGraphicsItem* item, QObject* parent ):
	TabstractAnim(item, parent)
{

}


void TblinkingItem::startBlinking(int count) {
	installTimer();
	m_maxCount = count * 2;
	m_blinkPhase = 0;
	timer()->start(150);
	animationRoutine();
}


void TblinkingItem::animationRoutine() {
		m_blinkPhase++;
		if (m_blinkPhase <= m_maxCount) {
			if (m_blinkPhase % 2) { // phase 1, 3, ...
					item()->hide();
			} else { // phase 2, 4, ...
					item()->show();
			}
		} else {
				timer()->stop();
				emit finished();
		}
}


