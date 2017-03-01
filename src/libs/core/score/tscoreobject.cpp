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
#include "tnotepair.h"
#include "music/tmeter.h"
#include "music/tnote.h"

#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include "checktime.h"


#define WIDTH_CHANGE_DELAY (50) // when score width changes, give 50 ms before staves will be resized


TscoreObject::TscoreObject(QObject* parent) :
  QObject(parent),
  m_keySignEnabled(false),
  m_clefOffset(TclefOffset(3, 1)),
  m_width(0.0)
{
  m_meter = new Tmeter(Tmeter::Meter_4_4);
  setMeter(4); // Tmeter::Meter_4_4
  m_measures << new TmeasureObject(0, this);
  m_widthTimer = new QTimer(this);
  m_widthTimer->setSingleShot(true);
  connect(m_widthTimer, &QTimer::timeout, this, &TscoreObject::adjustScoreWidth);
}


TscoreObject::~TscoreObject()
{
  delete m_meter;
  qDebug() << "[TscoreObject] deleted";
}


// TODO: Is it necessary at all?
void TscoreObject::setParent(QObject* p) {
  QObject::setParent(p);
  qDebug() << "[TscoreObject] parent changed to" << p;
}


void TscoreObject::setWidth(qreal w) {
  if (w != m_width) {
    m_width = w;
    if (m_widthTimer->isActive())
      m_widthTimer->stop();
    m_widthTimer->start(WIDTH_CHANGE_DELAY);
  }
}


/** @p static
 * This method creates @p outList of notes that have pitch of @p n note
 * but splits @p dur duration into possible rhythmic values.
 */
void solveList(const Tnote& n, int dur, QList<Tnote>& outList) {
  // TODO: add ties
  Trhythm rtm(dur); // try to obtain rhythm value
  if (!rtm.isValid()) { // if it is not possible to express the duration in single rhythm - resolve it in multiple values
      TrhythmList solvList;
      Trhythm::resolve(dur, solvList);
      for (int r = 0; r < solvList.count(); ++r)
        outList << Tnote(n, Trhythm(solvList[r].rhythm(), n.isRest(), solvList[r].hasDot(), solvList[r].isTriplet()));
  } else { // just single note in the list
      rtm.setRest(n.isRest());
      outList << Tnote(n, rtm);
  }
}

void TscoreObject::addNote(const Tnote& n) {
// CHECKTIME (

  auto lastMeasure = m_measures.last();
  if (lastMeasure->free() == 0) { // new measure is needed
    lastMeasure = new TmeasureObject(m_measures.count(), this);
    m_measures << lastMeasure;
    lastStaff()->appendMeasure(lastMeasure);
  }
  int noteDur = n.duration();
  if (noteDur > lastMeasure->free()) { // split note that is adding
      int leftDuration = noteDur - lastMeasure->free();
      int lastNoteId = m_segments.count();

      QList<Tnote> notesToCurrent;
      solveList(n, lastMeasure->free(), notesToCurrent); // solve free duration in current measure
      if (notesToCurrent.isEmpty())
          qDebug() << "[TscoreObject] can't resolve duration of" << lastMeasure->free();
      else {
          appendNoteList(notesToCurrent);
          lastMeasure->appendNewNotes(lastNoteId, notesToCurrent.count());
      }

      QList<Tnote> notesToNext;
      solveList(n, leftDuration, notesToNext); // solve remaining duration for the next measure
      lastNoteId = m_segments.count(); // update id of the last note segment
      if (notesToNext.isEmpty())
          qDebug() << "[TscoreObject] can't resolve duration" << leftDuration;
      else {
          appendNoteList(notesToNext);
          auto newLastMeasure = new TmeasureObject(m_measures.count(), this); // add a new measure
          m_measures << newLastMeasure;
          lastStaff()->appendMeasure(newLastMeasure);
          newLastMeasure->appendNewNotes(lastNoteId, notesToNext.count());
      }
  } else { // just add new note to the last measure
      m_notes << n;
      int lastNoteId = m_segments.count();
      m_segments << new TnotePair(lastNoteId, &m_notes.last());
      lastMeasure->appendNewNotes(lastNoteId, 1);
  }

// )
}


void TscoreObject::setNote(int staffNr, int noteNr, const Tnote& n) {
  if (staffNr < 0 || staffNr >= m_staves.count()) {
    qDebug() << "[TscoreObject] There is no staff number" << staffNr;
    return;
  }
//   m_staves[staffNr]->setNote(noteNr, n);
}



void TscoreObject::setKeySignatureEnabled(bool enKey) {
  m_keySignEnabled = enKey;
}


void TscoreObject::setMeter(int m) {
  // set notes grouping
  m_meter->setMeter(static_cast<Tmeter::Emeter>(m));
  m_meterGroups.clear();
  if (m_meter->lower() == 4) { // simple grouping: one group for each quarter
      m_meterGroups << 24 << 48; // 2/4 and above
      if (m_meter->meter() > Tmeter::Meter_2_4)
        m_meterGroups << 72;
      if (m_meter->meter() > Tmeter::Meter_3_4)
        m_meterGroups << 96;
      if (m_meter->meter() > Tmeter::Meter_4_4)
        m_meterGroups << 120;
      if (m_meter->meter() > Tmeter::Meter_5_4)
        m_meterGroups << 144;
      if (m_meter->meter() > Tmeter::Meter_6_4)
        m_meterGroups << 168;
  } else {
      if (m_meter->meter() == Tmeter::Meter_3_8)
        m_meterGroups << 36;
      else if (m_meter->meter() == Tmeter::Meter_5_8)
        m_meterGroups << 36 << 60;
      else if (m_meter->meter() == Tmeter::Meter_6_8)
        m_meterGroups << 36 << 72;
      else if (m_meter->meter() == Tmeter::Meter_7_8)
        m_meterGroups << 36 << 60 << 84;
      else if (m_meter->meter() == Tmeter::Meter_9_8)
        m_meterGroups << 36 << 72 << 108;
      else if (m_meter->meter() == Tmeter::Meter_12_8)
        m_meterGroups << 36 << 72 << 108 << 144;
  }
  qDebug() << "[TscoreObject] Meter changed" << m_meterGroups;
}


int TscoreObject::meterToInt() { return static_cast<int>(m_meter->meter()); }


qreal TscoreObject::stavesHeight() {
  if (m_staves.isEmpty())
    return 0.0;
  auto last = lastStaff();
  return last->staffItem()->y() + last->staffItem()->height() * last->scale();
}


//#################################################################################################
//###################              PROTECTED           ############################################
//#################################################################################################

void TscoreObject::addStaff(TstaffObject* st) {
  m_staves.append(st);
  if (m_staves.count() == 1) // initialize first measure of first staff
    st->appendMeasure(m_measures.first());
  // next staves position ca be set only when staffItem is set, see TstaffObject::setStaffItem() then
  connect(st, &TstaffObject::hiNotePosChanged, [=](int staffNr, qreal offset){
    for (int i = staffNr; i < m_staves.size(); ++i) // move every staff about offset
      m_staves[i]->staffItem()->setY(m_staves[i]->staffItem()->y() + offset);
    emit stavesHeightChanged();
  });
  connect(st, &TstaffObject::loNotePosChanged, [=](int staffNr, qreal offset){
    if (m_staves.size() > 1 && staffNr < m_staves.size() - 1) { // ignore change of the last staff
      for (int i = staffNr; i < m_staves.size(); ++i) // move every staff about offset
        m_staves[i]->staffItem()->setY(m_staves[i]->staffItem()->y() + offset);
    }
    emit stavesHeightChanged();
  });
}


void TscoreObject::shiftMeasures(int firstId, int count) {
  qDebug() << "[TscoreObject] shifting" << count << "measure(s) from" << firstId;
  auto sourceStaff = m_measures[firstId]->staff();
  TstaffObject* targetStaff = nullptr;
  if (sourceStaff == lastStaff()) { // create new staff to shift measure(s) there
      emit staffCreate();
      targetStaff = lastStaff();
  } else
      targetStaff = m_staves[sourceStaff->number() + 1];

  for (int m = firstId; m < firstId + count; ++m) {
    auto meas = m_measures[m];
    for (int n = 0; n < meas->noteCount(); ++n)
      meas->note(n)->object()->setStaff(targetStaff);

    int measNr = meas->number();
    sourceStaff->takeMeasure(measNr - sourceStaff->firstMeasureNr());
    if (targetStaff->measuresCount())
      targetStaff->insertMeasure(measNr - targetStaff->firstMeasureNr(), meas);
    else
      targetStaff->appendMeasure(meas);
  }
  targetStaff->refresh();
}


bool TscoreObject::checkStaffFreeSpace(TstaffObject* st, qreal availWidth) {
  int sourceStaffNr = st->number();
  if (sourceStaffNr < m_staves.count() - 1) { // is there next staff?
    auto nextStaff = m_staves[sourceStaffNr + 1];
    auto nextFirst = nextStaff->firstMeasure();
    qreal estimateW = availWidth - nextFirst->allNotesWidth();
    if (estimateW / (st->gapsSum() + nextFirst->gapsSum()) > 0.8) {
      for (int n = 0; n < nextFirst->noteCount(); ++n)
        nextFirst->note(n)->object()->setStaff(st);
      nextStaff->takeMeasure(0);
      st->appendMeasure(nextFirst);
      if (nextStaff->measuresCount() == 0) { // staff became empty
        if (nextStaff == lastStaff()) { // delete if if it was the last one
            nextStaff->staffItem()->deleteLater();
            m_staves.removeLast();
            emit stavesHeightChanged();
        } else { // or take measures from next-next staff
            qDebug() << "[TscoreObject] filling empty measure";
        }
      }
      return true;
    }
  }
  return false;

}


void TscoreObject::adjustScoreWidth() {
// CHECKTIME (
  for (int s = 0; s < m_staves.count(); ++s) {
    m_staves[s]->fit();
    if (s > 0 && s < m_staves.count()) {
      auto prev = m_staves[s - 1];
      auto curr = m_staves[s];
      auto sc = prev->scale();
      curr->staffItem()->setY(prev->staffItem()->y() + (prev->loNotePos() - curr->hiNotePos() + 4.0) * sc); // TODO scordature!
    }
  }
  emit stavesHeightChanged();
// )
}


//#################################################################################################
//###################              PRIVATE             ############################################
//#################################################################################################

void TscoreObject::appendNoteList(QList<Tnote>& l) {
  for (Tnote n : l) {
    m_notes << n;
    m_segments << new TnotePair(m_segments.count(), &m_notes.last());
  }
}
