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
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/



#ifndef TSUPPORTNOOTKA_H
#define TSUPPORTNOOTKA_H

#include <QWidget>
#include <QDialog>



class TsupportNootka : public QWidget
{
public:
    explicit TsupportNootka(QWidget *parent = 0);
};



class TsupportStandalone : public QDialog
{
public:
    explicit TsupportStandalone(QString &path, QWidget *parent = 0);
};

#endif // TSUPPORTNOOTKA_H
