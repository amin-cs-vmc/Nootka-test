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
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef TLEVELPREVIEW_H
#define TLEVELPREVIEW_H

#include <QWidget>

class TexamLevel;
class QLabel;

class TlevelPreview : public QWidget
{

  Q_OBJECT
  
public:
  explicit TlevelPreview(QWidget *parent = 0);
  virtual ~TlevelPreview();
  
  static QString notesRangeTxt() { return tr("notes' range:"); }
  static QString fretsRangeTxt() { return tr("frets' range:"); }

  void setLevel(TexamLevel &tl);
			/** Overloaded method with empty level to force empty table. */
	void setLevel();
  
private:
  QLabel *m_summLab, *m_clefLabel;

};

#endif // TLEVELPREVIEW_H
