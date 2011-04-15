/***************************************************************************
 *   Copyright (C) 2011 by Tomasz Bojczuk  				   *
 *   tomaszbojczuk@gmail.com   						   *
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
 *  You should have received a copy of the GNU General Public License	   *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/


#ifndef QUESTIONSSETTINGS_H
#define QUESTIONSSETTINGS_H

#include <QtGui>
#include "tquestionaswdg.h"
#include "tkeysigncombobox.h"

class questionsSettings : public QWidget
{
    Q_OBJECT
public:
    explicit questionsSettings(QWidget *parent = 0);



signals:

public slots:

private:
    QToolBox *questAsToolBox;


};



class TasNoteWdg : public QWidget
{
    Q_OBJECT
public:
    explicit TasNoteWdg(QWidget *parent = 0);

private:
    TquestionAsWdg *asNoteGr;
    QGroupBox *accidGr, *keySignGr;
    QRadioButton *singleKeyRadio, *rangeKeysRadio;
    QRadioButton *majorOnlyRadio, *minorOnlyRadio, *bothKeysRadio;
    QButtonGroup *rangeButGr, *modeButGr;
    QCheckBox *sharpsChB, *flatsChB, *doubleAccChB;
    TkeySignComboBox *fromKeyCombo, *toKeyCombo;
};



class TasNameWdg : public QWidget
{
    Q_OBJECT
public:
    explicit TasNameWdg(QWidget *parent = 0);

private:
    TquestionAsWdg *asNameGr;
};

#endif // QUESTIONSSETTINGS_H
