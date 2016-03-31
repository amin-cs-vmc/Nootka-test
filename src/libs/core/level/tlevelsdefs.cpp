/***************************************************************************
 *   Copyright (C) 2014-2015 by Tomasz Bojczuk                             *
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

#include "tlevelsdefs.h"
#include <exam/tlevel.h>
#include <music/ttune.h>
#include <tinitcorelib.h>



void getExampleLevels(QList<Tlevel>& llist) {
	Tlevel l = Tlevel();
		int octaveOffset = 0; // depends on guitar type and for bass drops range octave down
		if (Tcore::gl()->instrument == e_bassGuitar)
			octaveOffset = -1;
		else if (Tcore::gl()->instrument == e_noInstrument)
			octaveOffset = 1;
//----------------------------------------------------------------------------
    l.name = QObject::tr("open strings");
    l.desc = QObject::tr("The simplest. No key signatures, no double accidentals and no sound.<br>Automatically adjusted to current tune.");
    l.questionAs.setAsSound(false);
    l.answersAs[0] = TQAtype(false, true, true, false); // q: score -- a: name, guitar
    l.answersAs[1] = TQAtype(true, false, true, false); // q: name -- a: score, guitar
		l.answersAs[2] = TQAtype(true, true, false, false); // q: guitar -- a: score, name
    l.answersAs[3] = TQAtype(false, false, false, false);  
    l.withSharps = false;
    l.withFlats = false;
    l.withDblAcc = false;
    l.useKeySign = false;
    l.loKey = 0;
    l.hiKey = 0;
    l.forceAccids = false;
    l.requireOctave = false;
    l.requireStyle = false;
    l.showStrNr = false;
		l.intonation = 0; // do not check
		//clef default, user/tune defined
		//instrument default - selected by user
    l.hiNote = Tnote(Tcore::gl()->hiString().chromatic()); 
		//loNote is lowest by constructor
    l.hiFret = 0;// loFret is 0 by constuctor
    for (int i = 1; i < 7; i++) { //accids will be used if current tune requires it
        if (Tcore::gl()->Gtune()->str(i).alter == 1)
            l.withSharps = true;
        if (Tcore::gl()->Gtune()->str(i).alter == -1)
            l.withFlats = true;
		}
		if (l.instrument == e_noInstrument) // force instrument when not defined
			l.instrument = e_classicalGuitar;
		l.melodyLen = 1;
// 		l.requireInTempo = false;
// 		l.endsOnTonic = fasle;
    llist << l;
//----------------------------------------------------------------------------
    l.name = QObject::tr("C-major scale");
    l.desc = QObject::tr("In first position. No accidentals, no sound.<br>Octave required.");
    l.questionAs.setAsSound(false);
		bool isGuitar = true;
		if (Tcore::gl()->instrument == e_noInstrument) {
			isGuitar = false;
			l.desc = QObject::tr("Give note name in C-major scale or show note on the staff knowing its name.");
		}
		l.instrument = Tcore::gl()->instrument;
		l.questionAs.setAsFret(isGuitar);
    l.answersAs[0] = TQAtype(false, true, isGuitar, false);
    l.answersAs[1] = TQAtype(true, false, isGuitar, false);
		l.answersAs[2] = TQAtype(isGuitar, isGuitar, false, false);
    l.answersAs[3] = TQAtype(false, false, false, false);
    l.withSharps = false;
    l.withFlats = false;
    l.withDblAcc = false;
    l.useKeySign = false;
    l.manualKey = false;
    l.loKey = 0;
    l.hiKey = 0;
    l.forceAccids = false;
    l.requireOctave = true;
    l.requireStyle = false;
    l.showStrNr = false;
    l.loNote = Tnote(1, 0 + octaveOffset, 0);
    l.hiNote = Tnote(1, 1 + octaveOffset, 0);
    l.hiFret = 3; // loFret is 0 by constructor
    l.intonation = 0; // do not check
    llist << l;
//----------------------------------------------------------------------------
    l.name = QObject::tr("All to V fret");
    l.desc = QObject::tr("All chromatic notes till V-th fret, no sound.<br>Without double accidentals. Key signatures: C, G, D, A");
    l.questionAs.setAsSound(false);
    l.answersAs[0] = TQAtype(false, true, true, false);
    l.answersAs[1] = TQAtype(true, false, true, false);
    l.answersAs[3] = TQAtype(false, false, false, false);
    l.withSharps = true;
    l.withFlats = true;
    l.withDblAcc = false;
    l.useKeySign = true;
    l.manualKey = true;
    l.loKey = 0;
    l.hiKey = 3;
    l.forceAccids = true;
    l.requireOctave = true;
    l.requireStyle = false;
    l.showStrNr = true;
    l.loNote = Tcore::gl()->loString();
    l.hiNote = Tnote(Tcore::gl()->hiString().chromatic() + 5);
    l.hiFret = 5;// loFret is 0 by constructor
    l.intonation = 0; // do not check
    if (Tcore::gl()->instrument == e_noInstrument) // force instrument when not defined
			l.instrument = e_classicalGuitar;
		l.melodyLen = 1;
    llist << l;
//----------------------------------------------------------------------------
    l = Tlevel();
    l.name = QObject::tr("Ear training");
    l.desc = QObject::tr("Listen to a sound and show it on the staff.<br>Guitar, note names and key signatures are not used.<br>Scale a - a<sup>2</sup>.");
    l.questionAs.setAsFret(false); // no guitar
    l.questionAs.setAsName(false); // no names
    l.questionAs.setAsNote(false); // score only as an question
    l.answersAs[0] = TQAtype(false, false, false, false);
    l.answersAs[1] = TQAtype(false, false, false, false);
    l.answersAs[2] = TQAtype(false, false, false, false);
    l.answersAs[3] = TQAtype(true, false, false, false); // score only
    l.withSharps = true;
    l.withFlats = true;
    l.withDblAcc = false;
    l.useKeySign = false;
    l.manualKey = false;
    l.loKey = 0;
    l.hiKey = 0;
    l.forceAccids = false;
    l.requireOctave = true;
    l.requireStyle = false;
    l.showStrNr = false;
		l.clef = Tclef(Tclef::e_treble_G);
		l.instrument = e_noInstrument;
		l.intonation = 0; // do not check
    l.loNote = Tnote(6, 0); // a
    l.hiNote = Tnote(6, 2); // a2
    l.hiFret = 19;// loFret is 0 by constructor
    l.melodyLen = 10;
    llist << l;
//----------------------------------------------------------------------------
	if (Tcore::gl()->instrument == e_classicalGuitar || Tcore::gl()->instrument == e_electricGuitar) {
		l = Tlevel();
    l.name = QObject::tr("Melody on trebles");
    l.desc = QObject::tr("Play short melody on treble strings with notes just on first three frets. Sharps keys only.");
    l.questionAs.setAsFret(false); // no guitar
    l.questionAs.setAsName(false); // no names
    l.questionAs.setAsSound(false); // don't play
    l.answersAs[0] = TQAtype(false, false, false, true); // score only
    l.answersAs[1] = TQAtype(false, false, false,false);
    l.answersAs[2] = TQAtype(false, false, false,false);
    l.answersAs[3] = TQAtype(false, false, false,false);
    l.withSharps = true;
    l.withFlats = false;
    l.withDblAcc = false;
    l.useKeySign = true;
    l.manualKey = false;
    l.loKey = 0;
    l.hiKey = 3;
    l.forceAccids = false;
    l.requireOctave = true;
    l.requireStyle = false;
    l.showStrNr = false;
		//clef default, user/tune defined
		//instrument default - selected by user
		l.loNote = Tcore::gl()->Gtune()->str(3);
		l.hiNote = Tnote(Tcore::gl()->hiString().chromatic() + 3);
		l.hiFret = 3;
// 		l.intonation = Tcore::gl()->A->intonation; // user preferences (in constructor)
    l.onlyLowPos = true;
// 		if (Tcore::gl()->instrument == e_noInstrument) // force instrument when not defined
// 			l.instrument = Tcore::gl()->instrument;
		l.melodyLen = 5;
		l.endsOnTonic = true;
		l.onlyCurrKey = true;
    llist << l;
	}
//----------------------------------------------------------------------------
    l = Tlevel();
    l.name = QObject::tr("Play scores");
    l.desc = QObject::tr("Take your instrument and just play...<br>No note names, no fretboard. Keys till 4 accids and double accids enabled! Scale of whole guitar without positions.");
    l.questionAs.setAsFret(false); // no guitar
    l.questionAs.setAsName(false); // no names
    l.questionAs.setAsSound(false); // don't play
    l.answersAs[0] = TQAtype(false, false, false, true); // score only
    l.answersAs[1] = TQAtype(false, false, false,false);
    l.answersAs[2] = TQAtype(false, false, false,false);
    l.answersAs[3] = TQAtype(false, false, false,false);
    l.withSharps = true;
    l.withFlats = true;
    l.withDblAcc = true;
    l.useKeySign = true;
    l.manualKey = false;
    l.loKey = -4;
    l.hiKey = 4;
    l.forceAccids = false;
    l.requireOctave = true;
    l.requireStyle = false;
    l.showStrNr = false;
		//clef default, user/tune defined
		//instrument default - selected by user
//     l.loNote & l.hiNote from constructor
//     l.hiFret by constructor
// 		l.intonation = Tcore::gl()->A->intonation; // user preferences (in constructor)
    l.onlyLowPos = true;
// 		if (Tcore::gl()->instrument == e_noInstrument) // force instrument when not defined
// 			l.instrument = Tcore::gl()->instrument;
		l.melodyLen = 10;
    llist << l;
//----------------------------------------------------------------------------
    l = Tlevel();
    l.name = QObject::tr("Sing scores");
    l.desc = QObject::tr("Just sing a score...<br>No note names, no fretboard, no keys and double accids. Scale doesn't matter because octaves are ignored - you can transpose.");
    l.questionAs.setAsFret(false); // no guitar
    l.questionAs.setAsName(false); // no names
    l.questionAs.setAsSound(false); // don't play
    l.answersAs[0] = TQAtype(false, false, false, true); // score only
    l.answersAs[1] = TQAtype(false, false, false,false);
    l.answersAs[2] = TQAtype(false, false, false,false);
    l.answersAs[3] = TQAtype(false, false, false,false);
    l.withSharps = true;
    l.withFlats = true;
    l.withDblAcc = false;
    l.useKeySign = false;
    l.manualKey = false;
		l.clef = Tclef(Tclef::e_treble_G);
		l.instrument = Tcore::gl()->instrument;
    l.loKey = 0;
    l.hiKey = 0;
    l.forceAccids = false;
    l.requireOctave = false;
    l.requireStyle = false;
    l.showStrNr = false;
    l.loNote = Tnote(5, 0); // G contra
    l.hiNote = Tnote(6, 2); // a one-line
//     l.hiFret by constructor
// 		l.intonation = Tcore::gl()->A->intonation; // user preferences (in constructor)
		l.instrument = e_noInstrument;
    l.onlyLowPos = true;
		l.melodyLen = 5;
    llist << l;
}