/***************************************************************************
 *   Copyright (C) 2011-2014 by Tomasz Bojczuk                             *
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


#ifndef TLEVELSELECTOR_H
#define TLEVELSELECTOR_H

#include <QtQuick/qquickitem.h>
#include <exam/tlevel.h>


class QFile;
class TlevelPreviewItem;


/**
 * @p TlevelSelector is C++ logic of LevelSelector.qml
 */
class TlevelSelector : public QQuickItem
{

  Q_OBJECT

  Q_PROPERTY(QStringList levelsModel READ levelsModel NOTIFY levelsModelChanged)
  Q_PROPERTY(TlevelPreviewItem* levelPreview READ levelPreview WRITE setLevelPreview)

public:
  explicit TlevelSelector(QQuickItem* parent = nullptr);
  virtual ~TlevelSelector() override;

  QStringList levelsModel() { return m_levelsModel; }

  TlevelPreviewItem* levelPreview() { return m_levelPreview; }
  void setLevelPreview(TlevelPreviewItem* lpi);

      /**
       * It's looking for levels:
       * 1. in Tlevel constructor
       * 2. In default install directory
       * 3. In latest used files
       */
  void findLevels();

      /**
       * Levels
       */
  static QString levelFilterTxt() { return tr("Levels"); }

      /**
       * Range of notes in the level is beyond the scale of your instrument
       */
  static QString rangeBeyondScaleTxt() { return tr("Range of notes in the level is beyond the scale of your instrument"); }

  struct SlevelContener {
      Tlevel             level;
      QString            file; /**< file name of a level */
      int                itemId; /**< id number in the list */
      bool               suitable; /**< true when level is possible to perform by current instrument */
  };

      /**
       * Adds level @param lev to list. 
       * Also corresponding file name.
       * when @param check is true it checks list for duplicates
       */
  void addLevel(const Tlevel &lev, QString levelFile = QString(), bool check = false);

  Tlevel* currentLevel();

      /**
       * Shows the level with given @p id in the level preview
       */
  Q_INVOKABLE void showLevel(int id);

      /**
       * Description text of a given @p id level of the level list
       */
  Q_INVOKABLE QString desc(int id);

  Q_INVOKABLE bool isMelody(int id);

  Q_INVOKABLE QString levelName(int id) const;
  Q_INVOKABLE QString levelFile(int id) const;

      /**
       * Returns current selected level
       */
  Tlevel& getSelectedLevel();

  SlevelContener& levelContener(int id) { return m_levels[id]; }

      /**
       * Updates config file with new levels list.
       * Returns true when given level file was added to config.
       */
  void updateRecentLevels();

      /**
       * Static method to check a level with current global instrument settings
       */
  static QString checkLevel(Tlevel &l);

      /**
       * Checks is given level in range of current tune and frets number.
       * If not, it disables the latest entry in the list - BE SURE to call this
       * only after addLevel() method which puts the last level on the list.
       */
  bool isSuitable(Tlevel &l);

      /**
       * Checks currently selected level
       */
  bool isSuitable() const;
  bool isSuitable(int id) const { return m_levels[id].suitable; }

      /**
       * Returns @p TRUE when level has its file and can be removed (from the list and from a storage)
       */
  Q_INVOKABLE bool isRemovable(int id) const;

      /**
       * Disables all levels which not match to instrument settings.
       */
  void disableNotSuitable();

      /**
       *
       */
  Q_INVOKABLE void loadFromFile(QString levelFile = QString());

  Q_INVOKABLE bool removeLevel(int id, bool removeFile);


signals:
  void levelsModelChanged();
  void levelChanged();

private:
  QList<SlevelContener>           m_levels;
  QStringList                     m_levelsModel;
  TlevelPreviewItem              *m_levelPreview;
  Tlevel                          m_fakeLevel; /**< Default @p Tlevel with empty name. Can be used for references */
  int                             m_currentLevelId = -1;

  Tlevel getLevelFromFile(QFile &file);

};

#endif // TLEVELSELECTOR_H





