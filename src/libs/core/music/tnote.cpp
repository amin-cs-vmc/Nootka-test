/***************************************************************************
 *   Copyright (C) 2006-2016 by Tomasz Bojczuk                             *
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
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "tnote.h"
#include <music/tnamestylefilter.h>

#include <sstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <QtCore/qvariant.h>


std::string IntToString(int num) {
  std::ostringstream myStream;
  myStream << num << std::flush;
  return(myStream.str());
}


std::string CharToString(char chr) {
  std::ostringstream myStream;
  myStream << (int) chr << std::flush;
  return(myStream.str());
}


QString accidInSpan(char accid) {
  QString accTxt;
  switch (accid) {
    case -2: accTxt = QStringLiteral("B"); break;
    case -1: accTxt = QStringLiteral("b"); break;
    case 1: accTxt = QStringLiteral("#"); break;
    case 2: accTxt = QStringLiteral("x"); break;
    default: break;
  }
  return  QString("<span style=\"font-family: nootka;\">%1</span>").arg(accTxt);
}

/*static*/
std::string Tnote::m_solmization[7] = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si"};
std::string Tnote::m_solmizationRu[7] = {"До", "Ре", "Ми", "Фа", "Соль", "Ля", "Си"};
// QString Tnote::m_KodalySharps[6] = {"Di", "Ri", "", "Fi", "Si", "Li"};
// QString Tnote::m_KodalyFlats[7] = {"", "Ra", "Me", "", "Se", "Le", "Te" };

Tnote::EnameStyle Tnote::defaultStyle = Tnote::e_norsk_Hb;


//#############################################################################################
//################################ PUBLIC #####################################################
//#############################################################################################
Tnote::Tnote ()
{
  note = 0;
  octave = 0;
  alter = Tnote::e_Natural;
}


Tnote::Tnote( char diatonNote, char oct, char accid)
{
  note = diatonNote;
  octave = oct;
  alter = accid;
}


Tnote::Tnote (short chromaticNrOfNote)
{
   setChromatic(chromaticNrOfNote);
}


Tnote::~Tnote ()
{}


void Tnote::setChromatic(short int noteNr) {
  switch ((noteNr + 143) % 12 + 1)  {
      case 1: note = 1; alter = 0; break;
      case 2: note = 1; alter = 1; break;
      case 3: note = 2; alter = 0; break;
      case 4: note = 2; alter = 1; break;
      case 5: note = 3; alter = 0; break;
      case 6: note = 4; alter = 0; break;
      case 7: note = 4; alter = 1; break;
      case 8: note = 5; alter = 0; break;
      case 9: note = 5; alter = 1; break;
      case 10: note = 6; alter = 0; break;
      case 11: note = 6; alter = 1; break;
      case 12: note = 7; alter = 0; break;
   }
   octave = ((noteNr + 143) / 12 - 12) ;
}


short Tnote::chromatic() const {
  char a;
  switch  (Tnote::note)  {
    case 1: a = 1; break;  // note C
    case 2: a = 3; break;  // D
    case 3: a = 5; break;  // E
    case 4: a = 6; break;  // F
    case 5: a = 8; break;  // G
    case 6: a = 10; break;  // A
    case 7: a = 12; break;  // H
  }
    return a + (Tnote::octave) * 12 + Tnote::alter;
}


Tnote Tnote::showAsNatural() const {
  return Tnote(Tnote(note, octave, alter).chromatic());
}


Tnote Tnote::showWithFlat() const {
  if (alter != Tnote::e_Flat)  {
    Tnote outputNote = Tnote::showAsNatural();
// Tnote::showAsNatural() allways returns notes with sharps or neutral, and never returns H# or E#, but
// C or F,
    if (outputNote.alter == Tnote::e_Sharp)  {
        outputNote.note++;
      outputNote.alter = Tnote::e_Flat;
    }
    else //so only for H changed to Cb we have to increment octave,
    if (outputNote.note == 7 || outputNote.note == 3)  {
      if (outputNote.note == 7)  {outputNote.octave++; outputNote.note = 1;}
      if (outputNote.note == 3) outputNote.note = 4;//and for E simply convert to Fb
      outputNote.alter = Tnote::e_Flat;
    }
    return outputNote;
  }
  else  return Tnote(note, octave, alter);
}


Tnote Tnote::showWithSharp() const {
  if (alter != Tnote::e_Sharp)  {
    Tnote outputNote = Tnote::showAsNatural();
    if (outputNote.alter == Tnote::e_Natural)  {
      if (outputNote.note == 4) { outputNote.note = 3; outputNote.alter = Tnote::e_Sharp;}
      else
        if (outputNote.note == 1)
        {outputNote.octave--; outputNote.note = 7; outputNote.alter = Tnote::e_Sharp;}
    }
    return outputNote;
  }
  else return Tnote(note,octave,alter);
}


Tnote Tnote::showWithDoubleSharp() const {
  if (alter != Tnote::e_DoubleSharp)  {
    Tnote outputNote = Tnote::showAsNatural();
    if (outputNote.alter == Tnote::e_Natural)  {
      if (outputNote.note == 4) {outputNote.alter = Tnote::e_Sharp; outputNote.note = 3;}
      else
      if (outputNote.note == 1)
        {outputNote.alter = Tnote::e_Sharp; outputNote.note = 7; outputNote.octave--;}
      else
        {outputNote.alter = Tnote::e_DoubleSharp; outputNote.note--;}
    }
                return outputNote;
  }
  else return Tnote(note, octave, alter);
}


Tnote Tnote::showWithDoubleFlat() const {
  if (alter != Tnote::e_DoubleFlat)  {
    Tnote outputNote = Tnote(note, octave, alter);
    if (outputNote.alter == Tnote::e_Flat && ( (outputNote.note == 3) || (outputNote.note == 7)) )  {
      if (outputNote.note == 3) {outputNote.alter = Tnote::e_DoubleFlat; outputNote.note = 4;}
      else
        {outputNote.alter = Tnote::e_DoubleFlat; outputNote.note = 1; outputNote.octave++;}
    }
    else  {
      outputNote = outputNote.showAsNatural();
      //Tnote outputNote = Tnote::showAsNatural();
      if (outputNote.alter == Tnote::e_Natural)  {
          if (outputNote.note == 3) {outputNote.alter = Tnote::e_Flat; outputNote.note = 4;}
        else
        if (outputNote.note == 7)
          {outputNote.alter = Tnote::e_Flat; outputNote.note = 1; outputNote.octave++;}
        else
            {outputNote.alter = Tnote::e_DoubleFlat; outputNote.note++;}
      }
      else
        if (outputNote.alter == Tnote::e_Sharp)  {
          if (outputNote.note == 2)
            {outputNote.alter = Tnote::e_DoubleFlat; outputNote.note = 4;}
          else
            if (outputNote.note == 6)
            {outputNote.alter = Tnote::e_DoubleFlat; outputNote.note = 1; outputNote.octave++;}
            else
            outputNote = outputNote.showWithFlat();
        }
    }
                return outputNote;
  }
  else return Tnote(note, octave, alter);
}


short Tnote::compareNotes(const Tnote& otherNote, short int ignoreOctave) const {
  if (!ignoreOctave) {
      if (note == otherNote.note && alter == otherNote.alter && octave == otherNote.octave)
        return 1;
      else
        return 0;
  } else {
      if (note == otherNote.note && alter == otherNote.alter)
        return 1;
      else
        return 0;
  }
}


TnotesList Tnote::getTheSameNotes(bool enableDbAccids) const {
  TnotesList notesL;
  short cnt;//counter of notes. With double accids is 5 (4) without 3 (2)
  notesL.push_back(Tnote(note,octave,alter));
  if (notesL[0].alter != Tnote::e_Natural) notesL.push_back(notesL[0].showAsNatural());
  if (notesL[0].alter != Tnote::e_Sharp) notesL.push_back(notesL[0].showWithSharp());
  if (notesL[0].alter != Tnote::e_Flat) notesL.push_back(notesL[0].showWithFlat());
  if (enableDbAccids)  {
    if (notesL[0].alter != Tnote::e_DoubleSharp) notesL.push_back(notesL[0].showWithDoubleSharp());
    if (notesL[0].alter != Tnote::e_DoubleFlat) notesL.push_back(notesL[0].showWithDoubleFlat());
    cnt = 4;
  }
  else cnt = 2;
  for( int m=0; m < cnt; m++)  {
    for( int n=cnt; n > m; n--){
      if ((notesL[m].note != 0) && (notesL[n].note != 0) )
        if(notesL[m].compareNotes(notesL[n]) )  notesL[n].note = 0;
    }
  }
  TnotesList::iterator m = notesL.end();
  while( m != notesL.begin() ){
    --m;
    if (!(*m).note) notesL.erase(m);
  }
  //notesL.erase(notesL.begin());
  return notesL;
}


std::string Tnote::getName(Tnote::EnameStyle notation, bool showOctave) const {
  std::string noteStr;
  if (note < 1 || note > 7) {
      std::cout << "Oops !! getName() with note=0\n";
      return "none";
  }
  switch (TnameStyleFilter::get(notation)) {
      case e_italiano_Si:
          noteStr = m_solmization[note - 1] + signsAcid[alter + 2];
          break;
      case e_russian_Ci:
          noteStr = m_solmizationRu[note - 1] + signsAcid[alter + 2];
          break;
      case e_deutsch_His:
          noteStr = Letters[note - 1];
          switch (alter)  {
            case e_Natural: break;
            case e_DoubleSharp: noteStr = noteStr + "isis"; break;
            case e_Sharp: noteStr = noteStr + "is"; break;
            case e_DoubleFlat: switch (note)  {
              case 3: noteStr = noteStr + "ses"; break;
              case 6: noteStr = noteStr + "sas"; break;
              default: noteStr = noteStr + "eses"; break;
              }
              break;
            case e_Flat: switch (note)  {
              case 3: noteStr = noteStr + "s"; break;
              case 6: noteStr = noteStr + "s"; break;
              case 7: noteStr = "B"; break;
              default: noteStr = noteStr + "es"; break;
                }
                break;
          }
          break;
    case e_nederl_Bis:
        noteStr = Letters[note-1];
        if (note == 7) noteStr = "B";
        switch( alter ){
          case e_Natural: break;
          case e_DoubleSharp: noteStr = noteStr + "isis"; break;
          case e_Sharp: noteStr = noteStr + "is"; break;
          case e_DoubleFlat:
            switch (note)  {
            case 3: noteStr = noteStr + "ses"; break;
            case 6: noteStr = noteStr + "ses"; break;
            default: noteStr = noteStr + "eses"; break;
            }
            break;
          case e_Flat:
            switch (note)  {
            case 3: noteStr = noteStr + "s"; break;
            case 6: noteStr = noteStr + "s"; break;
            default: noteStr = noteStr + "es"; break;
            }
            break;
        }
        break;
    default:
        noteStr = Letters[note - 1];
        if ((notation == e_english_Bb) && (note == 7)) noteStr = "B";
            noteStr = noteStr + signsAcid[alter + 2];
        break;
   }
  if (showOctave)
      noteStr = noteStr + CharToString(octave);
  return noteStr;
}


QString Tnote::toRichText(Tnote::EnameStyle notation, bool showOctave) const {
  QString result = toText(notation, false);
    if (notation == Tnote::e_italiano_Si ||
        notation == Tnote::e_russian_Ci ||
        notation == Tnote::e_english_Bb ||
        notation == Tnote::e_norsk_Hb ) {
        if (alter)
          result.replace(QString::fromStdString(signsAcid[alter + 2]), QString("<sub>%1</sub>").arg(accidInSpan(alter)));
    }
    if (alter == -2)
        result.replace(QLatin1String("B"), QLatin1String("!")); // store capital B otherwise toLower() make it lower
    result = result.toLower(); // it converts double flat (B) to single flat (b)
    if (alter == -2)
        result.replace(QLatin1String("!"), QLatin1String("B")); // bring back capital B
    if (showOctave) {
        if (octave < 0) { //first letter capitalize
          QString l1 = result.mid(0, 1).toUpper();
          result.replace(0, 1, l1);
          if (octave < -1)
              result = result + QString("<sub>%1</sub>").arg(int(octave*(-1)-1));
        }
        if (octave > 0)
            result = result + QString("<sup>%1</sup>").arg((int)octave);
    }
    return result;
}


void Tnote::toXml(QXmlStreamWriter& xml, const QString& tag, const QString& prefix,
                  const QString& attr, const QString& val) const {
  if (!tag.isEmpty()) {
    xml.writeStartElement(tag);
    if (!attr.isEmpty())
      xml.writeAttribute(attr, val);
  }
  if (note !=0) { // write <pitch> context only if note is valid
    Tnote bareNote = Tnote(note, octave, 0);
    xml.writeTextElement(prefix + QLatin1String("step"), bareNote.toText(Tnote::e_english_Bb, false));
    xml.writeTextElement(prefix + QLatin1String("octave"), QVariant((int)octave + 3).toString());
    if (alter)
      xml.writeTextElement(prefix + QLatin1String("alter"), QVariant((int)alter).toString());
  }
  if (!tag.isEmpty())
    xml.writeEndElement(); // pitch
}


void Tnote::fromXml(QXmlStreamReader& xml, const QString& prefix) {
//   if (xml.name() == "pitch") {
    note = 0; octave = 0; alter = 0; // reset this note
    while (xml.readNextStartElement()) {
      if (xml.name() == (prefix + QLatin1String("step"))) {
        QString step = xml.readElementText().toUpper();
        for (char i = 1; i < 8; i++) {
          Tnote n(i, 0, 0);
          if (n.toText(Tnote::e_english_Bb, false) == step) {
            note = i;
            break;
          }
        }
      } else if (xml.name() == (prefix + QLatin1String("octave")))
        octave = char(xml.readElementText().toInt() - 3);
      else if (xml.name() == (prefix + QLatin1String("alter")))
        alter = char(xml.readElementText().toInt());
      else
        xml.skipCurrentElement();
    }
//   }
}


bool getNoteFromStream(QDataStream &in, Tnote &n) {
    bool ok = true;
    qint8 nn, oo, aa;
    in >> nn >> oo >> aa;
    if (nn < 1 || nn > 8 || aa < -2 || aa > 2) {
        nn = 1; aa = 0; oo = 0;
        ok = false;
    }
    n = Tnote(nn, oo, aa);
    return ok;
}


QDataStream &operator << (QDataStream &out, const Tnote &n) {
    out << (qint8)n.note << (qint8)n.octave << (qint8)n.alter;
    return out;
}


QDataStream &operator>> (QDataStream &in, Tnote &n) {
    qint8 nn, oo, aa;
    in >> nn >> oo >> aa;
    n = Tnote(nn, oo, aa);
    return in;
}
