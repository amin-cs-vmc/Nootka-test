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

#include "tscoreobject.h"
#include "tstaffobject.h"
#include "tmeasureobject.h"
#include "tnoteobject.h"
#include "tbeamobject.h"
#include "tnotepair.h"
#include "music/tmeter.h"
#include "music/tmelody.h"
#include "music/tchunk.h"

#include <QtGui/qguiapplication.h>
#include <QtGui/qpalette.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include "checktime.h"


#define WIDTH_CHANGE_DELAY (50) // when score width changes, give 50 ms before staves will be resized


TscoreObject::TscoreObject(QObject* parent) :
  QObject(parent),
  m_keySignEnabled(false),
  m_showExtraAccids(false),
  m_remindAccids(false),
  m_enableDoubleAccids(false),
  m_showNoteNames(false),
  m_clefOffset(TclefOffset(3, 2)),
  m_width(0.0), m_adjustInProgress(false),
  m_nameStyle(static_cast<int>(Tnote::defaultStyle)),
  m_workRhythm(new Trhythm()), // quarter by default
  m_allowAdding(false)
{
  m_qmlEngine = new QQmlEngine;
  m_qmlComponent = new QQmlComponent(m_qmlEngine, this);
  m_meter = new Tmeter();
  setMeter(4); // Tmeter::Meter_4_4
  m_measures << new TmeasureObject(0, this);

  m_widthTimer = new QTimer(this);
  m_widthTimer->setSingleShot(true);
  connect(m_widthTimer, &QTimer::timeout, this, &TscoreObject::adjustScoreWidth);

  for (int i = 0; i < 7; i++) // reset accidentals array
    m_accidInKeyArray[i] = 0;

  m_touchHideTimer = new QTimer(this);
  connect(m_touchHideTimer, &QTimer::timeout, [=]{
      changeActiveNote(nullptr);
      setPressedNote(nullptr);
      m_touchHideTimer->stop();
  });
  m_enterTimer = new QTimer(this);
  m_enterTimer->setSingleShot(true);
  connect(m_enterTimer, &QTimer::timeout, this, &TscoreObject::enterTimeElapsed);
  m_leaveTimer = new QTimer(this);
  m_leaveTimer->setSingleShot(true);
  connect(m_leaveTimer, &QTimer::timeout, this, &TscoreObject::leaveTimeElapsed);
  m_bgColor = qApp->palette().base().color();
}


TscoreObject::~TscoreObject()
{
  delete m_meter;
  delete m_qmlComponent;
  delete m_qmlEngine;
  delete m_workRhythm;
  qDebug() << "[TscoreObject] deleting," << m_segments.count() << "segments to flush";
  qDeleteAll(m_segments);
  qDeleteAll(m_spareSegments);
}

//#################################################################################################
//###################          Musical parameters      ############################################
//#################################################################################################

void TscoreObject::setClefType(Tclef::EclefType ct) {
  if (m_clefType != ct) {
    auto oldClef = m_clefType;
    m_clefType = ct;
    updateClefOffset();
    emit clefTypeChanged();

    if (notesCount() > 0) {
        bool pianoChanged = (oldClef == Tclef::PianoStaffClefs && m_clefType != Tclef::PianoStaffClefs)
                            || (oldClef != Tclef::PianoStaffClefs && m_clefType == Tclef::PianoStaffClefs);
        for (int n = 0; n < notesCount(); ++n) {
          auto noteSeg = m_segments[n];
          if (pianoChanged)
            noteSeg->item()->setHeight(m_clefType == Tclef::PianoStaffClefs ? 44.0 : 38.0);
          if (m_clefType == Tclef::NoClef) {
              Tnote newNote(Tnote(), noteSeg->note()->rtm);
              newNote.rtm.setStemDown(false);
              noteSeg->item()->setStemHeight(STEM_HEIGHT);
              noteSeg->setNote(newNote);
          } else {
              Tnote newNote(*noteSeg->note());
              if (oldClef == Tclef::NoClef) {
                  int globalNr = m_clefOffset.octave * 7 - (7 - m_clefOffset.note);
                  newNote.note = static_cast<char>(56 + globalNr) % 7 + 1;
                  newNote.octave = static_cast<char>(56 + globalNr) / 7 - 8;
              } else
                  fitToRange(newNote);
              noteSeg->setNote(newNote);
          }
        }
        for (int m = 0; m < m_measures.count(); ++m)
          m_measures[m]->refresh();
        if (!pianoChanged) // otherwise adjustScoreWidth() will be called due to score scale changes
          adjustScoreWidth();
    }
  }
}


/**
 * When meter is changed and there are notes on the score, all segments are flushed and stored
 * then measures and staves are deleted,
 * then all notes are added from scratch, but stored segments are reused to improve single segment creation time
 */
void TscoreObject::setMeter(int m) {
  Tmeter::Emeter newMeter = static_cast<Tmeter::Emeter>(m);
  Tmeter::Emeter prevMeter = m_meter->meter();
  if (m_meter->meter() != newMeter) {
    // set notes grouping
    m_meter->setMeter(newMeter);
    m_meter->fillMeterGroups(m_meterGroups);
    if (measuresCount())
      firstMeasure()->meterChanged();
    emit meterChanged();

    if (!m_singleNote && measuresCount() && firstMeasure()->noteCount() > 0) {
    CHECKTIME (
      clearScorePrivate();
      QList<Tnote> oldList = m_notes;
      m_notes.clear();
      for (int n = 0; n < oldList.size(); ++n) {
        // TODO fold (merge) all ties
        if (m_meter->meter() == Tmeter::NoMeter) // remove rhythm then
          oldList[n].setRhythm(Trhythm(Trhythm::NoRhythm));
        if (prevMeter == Tmeter::NoMeter) // initialize empty rhythm
          oldList[n].setRhythm(Trhythm()); // quarter by default
        addNote(oldList[n]);
      }
      m_activeBarNr = 0;
      adjustScoreWidth();
    )
    }
    emitLastNote();
  }
}


int TscoreObject::meterToInt() const { return static_cast<int>(m_meter->meter()); }


void TscoreObject::setKeySignature(int k) {
  qint8 key = static_cast<qint8>(k);
  if (m_keySignEnabled && key != m_keySignature) {
    if (key != m_keySignature) {
      m_keySignature = key;
      for (int i = 1; i < 8; i++) {
        qint8 sign = k < 0 ? -1 : 1;
        int startVal = k < 0 ? 38 : 48;
        if (i <= qAbs(k))
          m_accidInKeyArray[(startVal + sign * (i * 4)) % 7] = sign;
        else
          m_accidInKeyArray[(startVal + sign * (i * 4)) % 7] = 0;
      }
      m_keyChanged = true;
      for (TmeasureObject* m : qAsConst(m_measures))
        m->keySignatureChanged();
      if (notesCount() > 0)
        adjustScoreWidth();
      emit keySignatureChanged();
    }
  }
}


/** @p static
 * This method creates @p outList of notes that have pitch of @p n note
 * but splits @p dur duration into possible rhythmic values.
 */
void solveList(const Tnote& n, int dur, QList<Tnote>& outList) {
  Trhythm rtm(dur); // try to obtain rhythm value
  if (!rtm.isValid()) { // if it is not possible to express the duration in single rhythm - resolve it in multiple values
      TrhythmList solvList;
      Trhythm::resolve(dur, solvList);
      for (int r = 0; r < solvList.count(); ++r) {
        outList << Tnote(n, Trhythm(solvList[r].rhythm(), n.isRest(), solvList[r].hasDot(), solvList[r].isTriplet()));
      }
  } else { // just single note in the list
      rtm.setRest(n.isRest());
      outList << Tnote(n, rtm);
  }
}

void TscoreObject::addNote(const Tnote& newNote, bool fromQML) {
CHECKTIME (

  if (m_singleNote) {
    qDebug() << "[TscoreObject] FIXME! Trying to add note in single mode";
    return;
  }

  auto lastMeasure = m_measures.last();
  if (lastMeasure->free() == 0) { // new measure is needed
    lastMeasure = new TmeasureObject(m_measures.count(), this);
    m_measures << lastMeasure;
    lastStaff()->appendMeasure(lastMeasure);
  }

  Tnote n = newNote;
  fitToRange(n);
  int noteDur = n.rhythm() == Trhythm::NoRhythm ? 1 : n.duration();
  if (noteDur > lastMeasure->free()) { // split note that is adding
      int leftDuration = noteDur - lastMeasure->free();
      int lastNoteId = m_segments.count();

      QList<Tnote> notesToCurrent;
      solveList(n, lastMeasure->free(), notesToCurrent); // solve free duration in current measure
      if (notesToCurrent.isEmpty())
          qDebug() << "[TscoreObject] can't resolve duration of" << lastMeasure->free();
      else {
          if (!n.isRest()) {
            notesToCurrent.first().rtm.setTie(newNote.rtm.tie() > Trhythm::e_tieStart ? Trhythm::e_tieCont : Trhythm::e_tieStart);
            if (notesToCurrent.count() == 2)
              notesToCurrent.last().rtm.setTie(Trhythm::e_tieCont);
          }
          appendToNoteList(notesToCurrent);
          lastMeasure->appendNewNotes(lastNoteId, notesToCurrent.count());
      }

      QList<Tnote> notesToNext;
      solveList(n, leftDuration, notesToNext); // solve remaining duration for the next measure
      lastNoteId = m_segments.count(); // update id of the last note segment
      if (notesToNext.isEmpty())
          qDebug() << "[TscoreObject] can't resolve duration" << leftDuration;
      else {
          if (!n.isRest()) {
            if (notesToNext.count() == 1)
                notesToNext.first().rtm.setTie(Trhythm::e_tieEnd);
            else {
                notesToNext.first().rtm.setTie(Trhythm::e_tieCont);
                notesToNext.last().rtm.setTie(Trhythm::e_tieEnd);
            }
          }
          appendToNoteList(notesToNext);
          auto newLastMeasure = new TmeasureObject(m_measures.count(), this); // add a new measure
          m_measures << newLastMeasure;
          lastStaff()->appendMeasure(newLastMeasure);
          newLastMeasure->appendNewNotes(lastNoteId, notesToNext.count());
      }
  } else { // just add new note to the last measure
      m_notes << n;
      int lastNoteId = m_segments.count();
      m_segments << getSegment(lastNoteId, &m_notes.last());
      lastMeasure->appendNewNotes(lastNoteId, 1);
  }
  emitLastNote();
  if (fromQML) {
    if (!m_recordMode)
      setSelectedItem(lastNote());
    emit noteWasAdded();
  }
)
}


void TscoreObject::setNote(TnoteObject* no, const Tnote& n) {
  if (no) {
    if (no == lastNote() && no->note()->rtm != n.rtm) {
      deleteLastNote();
      addNote(n);
      emitLastNote();
      return;
    }

    auto oldNote = *no->wrapper()->note();
    auto newNote = n;
    newNote.rtm.setBeam(oldNote.rtm.beam()); // TODO as long as we don't change rhythm
    newNote.rtm.setTie(oldNote.rtm.tie()); // TODO as long as we don't change rhythm
    fitToRange(newNote);
    bool fitStaff = false;
    // disconnect tie (if any) if note pitch changed
    QPoint notesForAlterCheck;// x is first note and y is the last note to check
    if (oldNote.rtm.tie() && newNote.chromatic() != oldNote.chromatic()) {
      // alters of notes has to be checked due to note changed
      // and all measures contained note with the tie are affected. Find them then
      notesForAlterCheck = tieRange(no);
      notesForAlterCheck.setX(m_segments[notesForAlterCheck.x()]->item()->measure()->firstNoteId());
      notesForAlterCheck.setY(m_segments[notesForAlterCheck.y()]->item()->measure()->lastNoteId());
      if (oldNote.rtm.tie() == Trhythm::e_tieStart) {
        m_segments[no->index() + 1]->disconnectTie(TnotePair::e_untieNext);
      } else { // tie continue or end
        if (oldNote.rtm.tie() == Trhythm::e_tieCont)
          m_segments[no->index() + 1]->disconnectTie(TnotePair::e_untieNext);
        m_segments[no->index() - 1]->disconnectTie(TnotePair::e_untiePrev);
      }
      newNote.rtm.setTie(Trhythm::e_noTie);
      if (no->wrapper() == no->staff()->firstNote())
        no->staff()->deleteExtraTie();
      fitStaff = true;
    }
    no->wrapper()->setNote(newNote);
    // When note or alter are different - check accidentals in whole measure and fit staff if necessary
    if (!notesForAlterCheck.isNull() || oldNote.note != newNote.note || oldNote.alter != newNote.alter) {
      if (notesForAlterCheck.isNull())
        notesForAlterCheck = QPoint(no->measure()->firstNoteId(), no->measure()->lastNoteId());
      auto measureToRefresh = m_segments[notesForAlterCheck.x()]->item()->measure();
      for (int n = notesForAlterCheck.x(); n <= notesForAlterCheck.y(); ++n) {
        if (m_segments[n]->note()->note == oldNote.note || m_segments[n]->note()->note == newNote.note) {
          fitStaff = true;
          m_segments[n]->item()->updateAlter();
        }
        // Update measure sums (notes width)
        if (m_segments[n]->item()->measure() != measureToRefresh) {
          measureToRefresh->refresh();
          measureToRefresh = m_segments[n]->item()->measure();
        }
      }
      measureToRefresh->refresh();
    }
    // update note range on current staff
    if (oldNote.note != newNote.note || oldNote.octave != newNote.octave)
      no->staff()->checkNotesRange();
    // If there is a beam - prepare it again then draw
    if (no->wrapper()->beam()) {
      no->wrapper()->beam()->prepareBeam();
      if (!fitStaff)
        no->wrapper()->beam()->drawBeam();
    }
    if (fitStaff) {
      m_segments[notesForAlterCheck.x()]->item()->staff()->fit();
      if (m_segments[notesForAlterCheck.y()]->item()->staff() != m_segments[notesForAlterCheck.x()]->item()->staff())
        m_segments[notesForAlterCheck.y()]->item()->staff()->fit();
    }
    if (no == m_selectedItem)
      emit selectedNoteChanged();
    if (m_singleNote && m_enharmNotesEnabled && n.isValid()) {
      TnotesList enharmList = newNote.getTheSameNotes(m_enableDoubleAccids);
      TnotesList::iterator it = enharmList.begin();
      ++it;
      if (it != enharmList.end()) {
          note(1)->setVisible(true);
          m_segments[1]->setNote(*(it));
      } else
          note(1)->setVisible(false);
      ++it;
      if (it != enharmList.end()) {
          note(2)->setVisible(true);
          m_segments[2]->setNote(*(it));
      } else
          note(2)->setVisible(false);
    }
  }
  if (m_recordMode)
    setSelectedItem(no);
}


void TscoreObject::setNote(int noteNr, const Tnote& n) {
  if (noteNr >= 0 && noteNr < notesCount())
    setNote(note(noteNr), n);
  else
    qDebug() << "[TscoreObject FIXME] Trying to set note of item that doesn't exist!" << noteNr;
}



TnoteObject* TscoreObject::note(int noteId) {
  return noteId > -1 && noteId < notesCount() ? m_segments[noteId]->item() : nullptr;
}


Tnote TscoreObject::noteOfItem(TnoteObject* item) const {
  return item ? *item->note() : Tnote();
}


Tnote TscoreObject::noteAt(int index) const {
  return index >= 0 && index < m_notes.count() ? m_notes[index] : Tnote();
}


void TscoreObject::noteClicked(qreal yPos) {
  if (!activeNote())
    return;

  Trhythm newRhythm = *m_workRhythm;
  if (activeNote()->note()->rhythm() != m_workRhythm->rhythm() || activeNote()->note()->hasDot() != m_workRhythm->hasDot()) {
    if (activeNote() != lastNote()) {
      newRhythm = activeNote()->note()->rtm; // TODO so far it forces old rhythm until note rhythm change will be implemented
      newRhythm.setRest(m_workRhythm->isRest()); // only changes note to rest if set by user
    }
  }

  int globalNr = globalNoteNr(yPos);
  Tnote newNote(static_cast<char>(56 + globalNr) % 7 + 1, static_cast<char>(56 + globalNr) / 7 - 8,
          static_cast<char>(m_cursorAlter), newRhythm);
  if (m_workRhythm->isRest() || m_clefType == Tclef::NoClef)
    newNote.note = 0;

  setNote(m_activeNote, newNote);

  emit clicked();
}


void TscoreObject::setCursorAlter(int curAlt) {
  if (curAlt != m_cursorAlter) {
    m_cursorAlter = curAlt;
    emit cursorAlterChanged();
  }
}


void TscoreObject::openMusicXml(const QString& musicFile) {
  if (!musicFile.isEmpty()) {
    auto melody = new Tmelody();
    if (melody->grabFromMusicXml(musicFile))
      setMelody(melody);
    delete melody;
  }
}


void TscoreObject::saveMusicXml(const QString& musicFile) {
  if (!musicFile.isEmpty()) {
    QString fileName = musicFile;
    if (fileName.right(4) != QLatin1String(".xml"))
      fileName += QLatin1String(".xml");
    auto melody = new Tmelody(QStringLiteral("Nootka melody"), TkeySignature(static_cast<char>(keySignature())));
    melody->setClef(clefType());
    melody->setMeter(m_meter->meter());
    if (m_keySignEnabled)
      melody->setKey(TkeySignature(static_cast<char>(m_keySignature)));
    for (int n = 0; n < notesCount(); ++n)
      melody->addNote(Tchunk(m_notes[n]));
    melody->saveToMusicXml(fileName);
    delete melody;
  }
}


void TscoreObject::setMelody(Tmelody* melody) {
CHECKTIME (
  clearScorePrivate();
  m_notes.clear();
  setMeter(melody->meter()->meter());
  setClefType(melody->clef());
  int newKey = static_cast<int>(melody->key().value());
  if (newKey != keySignature()) {
    if (!m_keySignEnabled && qAbs(newKey) != 0)
      setKeySignatureEnabled(true);
    setKeySignature(newKey);
  }
  for (int n = 0; n < melody->length(); ++n)
    addNote(melody->note(n)->p());
  adjustScoreWidth();
  emitLastNote();
)
}


//#################################################################################################
//###################         Score switches           ############################################
//#################################################################################################

void TscoreObject::setKeySignatureEnabled(bool enKey) {
  if (enKey != m_keySignEnabled) {
    if (!enKey)
      m_keySignature = 0;
    m_keySignEnabled = enKey;
    emit keySignatureEnabledChanged();
    if (notesCount() > 0)
      adjustScoreWidth();
  }
}


void TscoreObject::setKeyReadOnly(bool ro) {
  if (m_keySignEnabled) {
    if (ro != m_keyReadOnly) {
      m_keyReadOnly = ro;
      emit keyReadOnlyChanged();
    }
  }
}


void TscoreObject::setEnableDoubleAccids(bool dblEnabled) {
  if (m_enableDoubleAccids != dblEnabled) {
    m_enableDoubleAccids = dblEnabled;
    if (!m_enableDoubleAccids) {
      // TODO: convert notes with double accidentals into single-ones
    }
  }
}


/**
 * When @p m_showNoteNames is set to @p TRUE:
 * @p TmeasureObject during adding a note item calls @p TnoteObject::setNoteNameVisible() to create note name
 * This method iterates all notes and switches its state of displaying note name
 */
void TscoreObject::setShowNoteNames(bool showNames) {
  if (m_showNoteNames != showNames) {
    m_showNoteNames = showNames;
    if (notesCount()) {
    CHECKTIME (
      for (int n = 0; n < notesCount(); ++n)
        m_segments[n]->item()->setNoteNameVisible(m_showNoteNames && m_clefType != Tclef::NoClef);
    )
    }
  }
}


void TscoreObject::setNameColor(const QColor& nameC) {
  if (m_nameColor != nameC) {
    m_nameColor = nameC;
    if (m_showNoteNames) {
      if (notesCount()) {
      CHECKTIME(
        for (int n = 0; n < notesCount(); ++n) // with hope that all items have name item created
          m_segments[n]->item()->nameItem()->setProperty("styleColor", m_nameColor);
      )
      }
    }
  }
}


void TscoreObject::setNameStyle(int nameS) {
  if (m_nameStyle != nameS) {
    m_nameStyle = nameS;
    if (m_showNoteNames) {
      if (notesCount()) {
      CHECKTIME(
        for (int n = 0; n < notesCount(); ++n) // with hope that all items have name item created
          m_segments[n]->item()->nameItem()->setProperty("text", m_notes[n].styledName());
      )
      }
    }
  }
}


void TscoreObject::setReadOnly(bool ro) {
  if (m_readOnly != ro) {
    m_readOnly = ro;
    emit readOnlyChanged();
    setKeyReadOnly(ro);
  }
}


/**
 * MainScore.qml also handles single note mode change,
 * but this method is (AND HAS TO BE) invoked first
 */
void TscoreObject::setSingleNote(bool singleN) {
  if (singleN != m_singleNote) {
    clearScore(); // In single note mode this call is ignored
    if (singleN) {
        setShowNoteNames(false);
        addNote(Tnote()); // it is avoided in single note mode
        addNote(Tnote());
        addNote(Tnote());
        m_singleNote = true;
        setNote(0, Tnote()); // reset it (hide) because addNote was performed above in multi notes mode
        setNote(1, Tnote());
        setNote(2, Tnote());
        note(0)->shiftHead(1.5);
        note(1)->shiftHead(1.5);
        note(2)->shiftHead(1.5);
        note(1)->setEnabled(false);
        note(2)->setEnabled(false);
        m_selectedItem = note(0);
    } else {
        m_singleNote = false;
        resetNoteItem(note(0));
        resetNoteItem(note(1));
        resetNoteItem(note(2));
        clearScore(); // call it again when transitioning from single note mode
    }
    emit singleNoteChanged();
  }
}


void TscoreObject::setRecordMode(bool r) {
  if (r != m_recordMode) {
    m_recordMode = r;
    emit recordModeChanged();
  }
}


void TscoreObject::setScaleFactor(qreal factor) {
  if (factor != m_scaleFactor) {
    m_scaleFactor = factor;
    emit scaleFactorChanged();
  }
}


qreal TscoreObject::stavesHeight() {
  if (m_staves.isEmpty())
    return 0.0;
  auto last = lastStaff();
  return last->staffItem()->y() + last->staffItem()->height() * last->scale();
}


void TscoreObject::changeActiveNote(TnoteObject* aNote) {
  if (aNote != m_activeNote) {
    auto prevActive = m_activeNote;
    m_activeNote = aNote;
    if (aNote == nullptr) {
        m_leaveTimer->start(300);
    } else {
        if (prevActive == nullptr)
          m_enterTimer->start(300);
        else {
          enterTimeElapsed();
          emit activeYposChanged();
        }
    }
  }
}


void TscoreObject::setActiveNotePos(qreal yPos) {
  if (!m_enterTimer->isActive() && yPos != m_activeYpos) {
    if (m_workRhythm->isRest())
      yPos = upperLine() + 4.0; // TODO it works because there is no rhythm change
    m_activeYpos = yPos;
    emit activeYposChanged();
  }
}


/**
 * Returns X coordinate of the first note in active measure (if any).
 *  or (if score is empty yet) - returns staff indent (position after meter)
 * So far it is used for positioning accidental pane on the active measure left.
 */
qreal TscoreObject::xFirstInActivBar() {
  if (m_activeBarNr < 0)
    return (firstStaff()->notesIndent() - 2.0) * firstStaff()->scale();
  else
    return (m_measures[m_activeBarNr]->first()->item()->x() - m_measures[m_activeBarNr]->first()->item()->alterWidth() - 1.0) * firstStaff()->scale();
}


/**
 * Returns X coordinate of the last note in active measure (if any),
 * but if that note is too far on the right (near a staff end), coordinate of first note is returned,
 * subtracted with 11.2 units of score scale which is width of the rhythm pane - to keep it visible.
 * So far it is used for positioning rhythm pane on the active measure right
 */
qreal TscoreObject::xLastInActivBar() {
  if (m_activeBarNr > -1) {
    qreal xx = m_measures[m_activeBarNr]->last()->item()->rightX();
    if (xx > m_width - 12.0)
      return xFirstInActivBar() - 11.2 * firstStaff()->scale();
    else
      return (xx + 7.0) * firstStaff()->scale();
  }
  return (firstStaff()->notesIndent() + 7.0) * firstStaff()->scale();
}

Trhythm TscoreObject::workRhythm() const {
  return *m_workRhythm;
}


void TscoreObject::setWorkRhythm(const Trhythm& r) {
  if (r != *m_workRhythm) {
    m_workRhythm->setRhythm(r);
    emit workRtmTextChanged();
  }
}


QString TscoreObject::workRtmText() const {
  return TnoteObject::getHeadText(workRhythm());
}


QString TscoreObject::activeRtmText() {
  if (m_activeNote)
    return TnoteObject::getHeadText(Trhythm(m_activeNote == lastSegment()->item() ? m_workRhythm->rhythm() : m_activeNote->note()->rhythm(), m_workRhythm->isRest()));
  return QString();
}


Tnote TscoreObject::posToNote(qreal yPos) {
  int globalNr = globalNoteNr(yPos);
  return Tnote(m_workRhythm->isRest() || m_clefType == Tclef::NoClef ? 0 : static_cast<char>(56 + globalNr) % 7 + 1,
               static_cast<char>(56 + globalNr) / 7 - 8, static_cast<char>(m_cursorAlter), workRhythm());
}


void TscoreObject::setAllowAdding(bool allow) {
  if (allow != m_allowAdding) {
    m_allowAdding = allow;
    adjustScoreWidth();
    emit allowAddingChanged();
  }
}


TnoteObject* TscoreObject::lastNote() {
  return m_segments.isEmpty() ? nullptr : lastSegment()->item();
}


qreal TscoreObject::midLine(TnoteObject* actNote) {
  if (stavesCount() == 0)
    return 0.0;
  if (actNote && m_activeNote)
    return activeNote()->staffItem()->y() + (upperLine() + 4.0) * lastStaff()->scale();
  else
    return lastStaff()->staffItem()->y() + (upperLine() + 4.0) * lastStaff()->scale();
}


void TscoreObject::deleteLastNote() {
  if (notesCount()) {
    if (lastNote()->note()->rtm.tie() && notesCount() > 1)
      noteSegment(notesCount() - 2)->disconnectTie(TnotePair::e_untiePrev);
    bool adjust = false;
    auto lastBar = lastMeasure();
    int tempActiveBar = m_activeBarNr;
    if (lastBar->noteCount() == 1 && lastBar != firstMeasure()) {
        delete m_measures.takeLast();
        auto lastSt = lastStaff();
        lastSt->setLastMeasureId(lastSt->lastMeasureId() - 1);
        if (lastSt->measuresCount() == 0) {
          deleteStaff(lastSt);
          adjust = true;
        }
        m_activeBarNr--;
    } else
        lastBar->removeLastNote();
    auto segToRemove = m_segments.takeLast();
    segToRemove->flush();
    m_spareSegments << segToRemove;
    m_notes.removeLast();
    m_activeNote = nullptr;
    if (notesCount() == 0)
      m_activeBarNr = -1;
    if (adjust)
      adjustScoreWidth();

    emit activeNoteChanged();
    emitLastNote();
    if (tempActiveBar != m_activeBarNr)
      emitActiveBarChanged();
    setSelectedItem(nullptr);
  }
}


void TscoreObject::clearScore() {
  if (notesCount() == 0)
    return;

  if (m_singleNote) {
      setNote(note(0), Tnote());
      setNote(note(1), Tnote());
      setNote(note(2), Tnote());
  } else {
      clearScorePrivate();
      m_notes.clear();
      m_activeBarNr = -1;
      m_activeNote = nullptr;
      adjustScoreWidth();
      emitLastNote();
      setSelectedItem(nullptr);
      emit scoreWasCleared();
  }
}


qreal TscoreObject::upperLine() { return m_staves.empty() ? 16.0 : firstStaff()->upperLine(); }


void TscoreObject::setWidth(qreal w) {
  if (w != m_width) {
    m_width = w;
    if (m_widthTimer->isActive())
      m_widthTimer->stop();
    m_widthTimer->start(WIDTH_CHANGE_DELAY);
  }
}


Tnote TscoreObject::highestNote() {
  switch (m_clefType) {
    case Tclef::Treble_G: return Tnote(4, 4);
    case Tclef::Treble_G_8down: return Tnote(4, 3);
    case Tclef::Bass_F: return Tnote(6, 2);
    case Tclef::Alto_C: return Tnote(5, 3);
    case Tclef::Tenor_C: return Tnote(3, 4);
    case Tclef::PianoStaffClefs: return Tnote(1, 4);
    default: return Tnote(); // percussion clef (no clef) and unsupported dropped bass
  }
}


Tnote TscoreObject::lowestNote() {
  switch (m_clefType) {
    case Tclef::Treble_G: return Tnote(7, -1);
    case Tclef::Treble_G_8down: return Tnote(7, -2);
    case Tclef::Bass_F: return Tnote(7, -2);
    case Tclef::Alto_C: return Tnote(1, -1);
    case Tclef::Tenor_C: return Tnote(6, -2);
    case Tclef::PianoStaffClefs: return Tnote(1, -1);
    default: return Tnote(); // percussion clef (no clef) and unsupported dropped bass
  }
}


TnoteObject* TscoreObject::getNext(TnoteObject* someNote) {
  if (someNote) {
    if (someNote->index() < notesCount() - 1)
      return noteSegment(someNote->index() + 1)->item();
  }
  return nullptr;
}


TnoteObject * TscoreObject::getPrev(TnoteObject* someNote) {
  if (someNote) {
    if (someNote->index() > 0)
      return noteSegment(someNote->index() - 1)->item();
  }
  return nullptr;
}


void TscoreObject::setSelectedItem(TnoteObject* item) {
  if (item != m_selectedItem) {
    m_selectedItem = item;
    emit selectedItemChanged();
    emit selectedNoteChanged();
  }
}


void TscoreObject::setBgColor(const QColor& bg) {
  if (bg != m_bgColor) {
    m_bgColor = bg;
    emit bgColorChanged();
  }
}


//#################################################################################################
//###################              PROTECTED           ############################################
//#################################################################################################

void TscoreObject::addStaff(TstaffObject* st) {
  st->setNumber(stavesCount());
  m_staves.append(st);
  if (m_staves.count() == 1) { // initialize first measure of first staff
    st->appendMeasure(m_measures.first());
    connect(st, &TstaffObject::upperLineChanged, this, &TscoreObject::upperLineChanged);
  }

  // next staves position can be set only when staffItem is set, see TstaffObject::setStaffItem() then
  connect(st, &TstaffObject::hiNotePosChanged, [=](int staffNr, qreal offset){
    for (int i = staffNr; i < m_staves.size(); ++i) // move every staff about offset
      m_staves[i]->staffItem()->setY(m_staves[i]->staffItem()->y() + offset);
    emit stavesHeightChanged();
  });
  connect(st, &TstaffObject::loNotePosChanged, [=](int staffNr, qreal offset){
    if (staffNr == 0) // never change Y position of first staff - it is always 0
        staffNr = 1;
    if (m_staves.size() > 1 && staffNr < m_staves.size() - 1) { // ignore change of the last staff
      for (int i = staffNr; i < m_staves.size(); ++i) // move every staff about offset
        m_staves[i]->staffItem()->setY(m_staves[i]->staffItem()->y() + offset);
    }
    emit stavesHeightChanged();
  });
}

/**
 * More-less it means that staff @p sourceStaff has no space for @p count number of measures
 * starting from @p measureNr, but those measures belong to it still
 */
void TscoreObject::startStaffFromMeasure(TstaffObject* sourceStaff, int measureNr, int count) {
  TstaffObject* targetStaff = nullptr;
  if (sourceStaff == lastStaff()) { // create new staff to shift measure(s) there
      emit staffCreate();
      targetStaff = lastStaff();
  } else {
      targetStaff = m_staves[sourceStaff->number() + 1];
      targetStaff->deleteExtraTie();
  }

  for (int m = measureNr; m < measureNr + count; ++m)
    m_measures[m]->setStaff(targetStaff);
  targetStaff->setLastMeasureId(qMax(measureNr + count - 1, targetStaff->lastMeasureId()));
  targetStaff->setFirstMeasureId(measureNr);
}


void TscoreObject::deleteStaff(TstaffObject* st) {
  if (st->measuresCount() < 1) {
      bool fixStaffNumbers = st != lastStaff();
      m_staves.removeAt(st->number());
      st->staffItem()->deleteLater();
      if (fixStaffNumbers) {
        for (int s = 0; s < stavesCount(); ++s)
          m_staves[s]->setNumber(s);
      }
  }
}


void TscoreObject::adjustScoreWidth() {
CHECKTIME (
  m_adjustInProgress = true;
  int refreshStaffNr = 0;
  while (refreshStaffNr < stavesCount()) {
    m_staves[refreshStaffNr]->refresh();
    refreshStaffNr++;
  }
  m_adjustInProgress = false;
  updateStavesPos();
)
}


void TscoreObject::updateStavesPos() {
  if (m_adjustInProgress)
    return;
  TstaffObject* prev = nullptr;
  for (QList<TstaffObject*>::iterator s = m_staves.begin(); s != m_staves.end(); ++s) {
    auto curr = *s;
    if (curr->number() != 0 && curr->number() < stavesCount())
      curr->staffItem()->setY(prev->staffItem()->y() + (prev->loNotePos() - curr->hiNotePos() + 4.0) * prev->scale()); // TODO scordature!
    prev = curr;
  }
  emit stavesHeightChanged();
}


void TscoreObject::onIndentChanged() {
  if (m_keyChanged) {
    m_keyChanged = false;
    adjustScoreWidth();
  }
}


/** FIXME: if tie starts/stops incorrectly (no start or end) it may return note number out of range */
QPoint TscoreObject::tieRange(TnoteObject* n) {
  QPoint tr;
  if (n->note()->rtm.tie()) {
    tr.setX(n->index());
    while (tr.x() > -1) {
      if (m_notes[tr.x()].rtm.tie() == Trhythm::e_tieStart)
        break;
      --tr.rx();
    }
    tr.setY(n->index());
    while (tr.y() < notesCount()) {
      if (m_notes[tr.y()].rtm.tie() == Trhythm::e_tieEnd)
        break;
      ++tr.ry();
    }
  }
  return tr;
}

void TscoreObject::setTouched(bool t) {
  if (t != m_touched) {
    m_touched = t;
    emit touchedChanged();
  }
}

//#################################################################################################
//###################              PRIVATE             ############################################
//#################################################################################################

void TscoreObject::appendToNoteList(QList<Tnote>& l) {
  for (Tnote& n : l) {
    m_notes << n;
    m_segments << getSegment(m_segments.count(), &m_notes.last());
  }
}


void TscoreObject::updateClefOffset() {
  switch (m_clefType) {
    case Tclef::Treble_G_8down: m_clefOffset.set(3, 1); break;
    case Tclef::Bass_F: m_clefOffset.set(5, 0); break;
    case Tclef::Alto_C: m_clefOffset.set(4, 1); break;
    case Tclef::Tenor_C: m_clefOffset.set(2, 1); break;
    default: m_clefOffset.set(3, 2); break; // Treble, piano staff and no clef (rhythm only)
  }
}


TnotePair* TscoreObject::getSegment(int noteNr, Tnote* n) {
  if (m_spareSegments.isEmpty())
      return new TnotePair(noteNr, n);
  else {
      auto np = m_spareSegments.takeLast();
      np->setNote(n);
      np->setIndex(noteNr);
      return np;
  }
}


int TscoreObject::globalNoteNr(qreal yPos) {
  if (isPianoStaff() && yPos > firstStaff()->upperLine() + 10.0)
    yPos -= 2.0;
  return m_clefOffset.octave * 7 - static_cast<int>(yPos - upperLine() - m_clefOffset.note);
}


void TscoreObject::clearScorePrivate() {
  if (measuresCount() && firstMeasure()->noteCount() > 0) {
    setSelectedItem(nullptr);
    m_activeBarNr = -1;
    changeActiveNote(nullptr);
    for (TnotePair* s : qAsConst(m_segments)) {
      s->flush();
      m_spareSegments << s;
    }
    qDeleteAll(m_measures);
    m_measures.clear();
    m_segments.clear();
    while (m_staves.count() > 1) {
      auto ls = m_staves.takeLast();
      ls->staffItem()->deleteLater();
    }
    m_measures << new TmeasureObject(0, this);
    lastStaff()->appendMeasure(firstMeasure());
    firstStaff()->setFirstMeasureId(0);
  }
}


void TscoreObject::enterTimeElapsed() {
  bool emitBarChange = false;
  if (m_activeNote && m_activeNote->measure()->number() != m_activeBarNr) {
    m_activeBarNr = m_activeNote->measure()->number();
    emitBarChange = true;
  }
  emit activeNoteChanged();
  if (emitBarChange)
    emit activeBarChanged();
  if (m_clefType == Tclef::NoClef)
    setActiveNotePos(upperLine() + 7.0);
}


void TscoreObject::leaveTimeElapsed() {
  emit activeNoteChanged();
}


void TscoreObject::fitToRange(Tnote& n) {
  Tnote loNote = lowestNote();
  Tnote hiNote = highestNote();
  if (!n.isRest()
    && ((n.octave > hiNote.octave || (n.octave == hiNote.octave && n.note > hiNote.note))
    || (n.octave < loNote.octave || (n.octave == loNote.octave && n.note < loNote.note))))
  {
    n.note = 0; n.octave = 0; // invalidate note
    n.setRest(true);
    n.rtm.setTie(Trhythm::e_noTie);
    n.rtm.setBeam(Trhythm::e_noBeam);
  }
}


/**
 * Set @TnoteObject parameters to defaults , usually they are different in single note mode
 */
void TscoreObject::resetNoteItem(TnoteObject* noteItem) {
  noteItem->setVisible(true);
  noteItem->setEnabled(true);
  noteItem->setColor(qApp->palette().text().color());
  noteItem->setNoteNameVisible(m_showNoteNames && m_clefType != Tclef::NoClef);
  noteItem->shiftHead(0.0);
}
