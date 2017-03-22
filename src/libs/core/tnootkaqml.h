/***************************************************************************
 *   Copyright (C) 2017 by Tomasz Bojczuk                                  *
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

#ifndef TNOOTKAQML_H
#define TNOOTKAQML_H


#include <nootkacoreglobal.h>
#include "music/tmeter.h"
#include "music/tclef.h"
#include "music/tnote.h"

#include <QtCore/qobject.h>


/**
 * Singleton object to manage (create) custom types from QML
 * In constructor it registers types accessible from QML in Nootka
 * Access it through @p Noo object
 */
class NOOTKACORE_EXPORT TnootkaQML : public QObject
{

  Q_OBJECT

public:
  explicit TnootkaQML(QObject* parent = nullptr);
  ~TnootkaQML();

      /**
       * Dialogues recognized by main QML Dialog instance of main window
       */
  enum Edialogs {
    Settings = 1, About = 2, LevelCreator = 3, ExamStart = 4, ExamSummary = 5,
    Charts = 6, Wizard = 7, QuickAudio = 8
  };
  Q_ENUM(Edialogs)

  Q_INVOKABLE QString version();
  Q_INVOKABLE Tclef clef(int type);
  Q_INVOKABLE Tmeter meter(int m);
  Q_INVOKABLE Tnote note(int pitch, int octave, int alter, int rhythm = 3, bool rest = false, bool dot = false);
  Q_INVOKABLE QString majorKeyName(int key);
  Q_INVOKABLE QString minorKeyName(int key);
  Q_INVOKABLE QString getLicense();
  Q_INVOKABLE QString getChanges();
  Q_INVOKABLE bool isAndroid();

private:
  static TnootkaQML             *m_instance;
};

#endif // TNOOTKAQML_H
