/***************************************************************************
 *   Copyright (C) 2015 by Tomasz Bojczuk                                  *
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

#include "tmtr.h"

int Tmtr::m_fingerPixels = 20;
int Tmtr::m_shortScreenSide = 480;
int Tmtr::m_longScreenSide = 600;


void Tmtr::init(QApplication *a) {
  if (a->screens().size()) {
    m_shortScreenSide = qMin(a->screens()[0]->geometry().height(), a->screens()[0]->geometry().width());
    m_longScreenSide = qMax(a->screens()[0]->geometry().height(), a->screens()[0]->geometry().width());
    m_fingerPixels = a->screens()[0]->geometry().width() / (a->screens()[0]->physicalSize().width() / 7.0);
  }
}
