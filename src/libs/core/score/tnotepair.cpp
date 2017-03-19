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

#include "tnotepair.h"
#include "music/tnote.h"
#include "tnoteobject.h"
#include "tstaffobject.h"


TnotePair::TnotePair(int index, Tnote* n, TnoteObject* ob) :
  m_note(n),
  m_noteItem(ob),
  m_index(index)
{

}


void TnotePair::setNoteObject(TnoteObject* ob) {
  m_noteItem = ob;
}


void TnotePair::approve() {
  if (m_changes) {
    if (m_changes & e_beamChanged || m_changes & e_stemDirChanged)
      m_noteItem->setNote(*m_note);
    m_changes = 0;
  }
}

//#################################################################################################
//###################              PROTECTED           ############################################
//#################################################################################################

void TnotePair::setBeam(TbeamObject* b) {
  m_beam = b;
}


void TnotePair::disconnectTie(Euntie untie) {
  Trhythm::Etie t;
  if (untie == e_untieNext)
    t = m_note->rtm.tie() == Trhythm::e_tieCont ? Trhythm::e_tieStart : Trhythm::e_noTie;
  else // e_untiePrev
    t = m_note->rtm.tie() == Trhythm::e_tieCont ? Trhythm::e_tieEnd : Trhythm::e_noTie;
  m_note->rtm.setTie(t);
  m_noteItem->note()->rtm.setTie(t);
  m_noteItem->checkTie();
  if (this == m_noteItem->staff()->firstNote() && (t == Trhythm::e_noTie || t == Trhythm::e_tieStart))
    m_noteItem->staff()->deleteExtraTie();
}
