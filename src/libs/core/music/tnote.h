/***************************************************************************
 *   Copyright (C) 2006-2015 by Tomasz Bojczuk                             *
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
#ifndef TNOTE_H
#define TNOTE_H

#include <string>
#include <vector>
#include <QtCore/qdatastream.h>
#include <QtCore/qxmlstream.h>
#include <nootkacoreglobal.h>

class Tnote;

// typedef QList<Tnote> TnotesList;
typedef std::vector<Tnote> TnotesList;


const std::string Letters [7] = {"C","D","E","F","G","A","H"};
const std::string signsAcid[5] = {"bb","b","","#","x",};


/**
 * This class provides the descriptions of classical notation for different variables in apps.
 * It helps to convert the letter notation (C D E ...) for Do Re Mi.
 * It supports accidentals (sharps, flats, double sharps and double flats.
 * General format for each note is its number.
*/
class NOOTKACORE_EXPORT Tnote
{

public:

        /** Enumeration type describes the styles of notation */
  enum EnameStyle {
    e_norsk_Hb = 0, // for letters with signs f.e. C# Cx or Cb    !! THIS IS DEFAULT !!
    e_deutsch_His, // for letters with names f.e. Cis Cisis or Ces H and B (H with flat)
    e_italiano_Si, // for classical Do Re Mi Fa Sol La Si
    e_english_Bb, // like @p e_norsk_Hb but with B and Bb (B flat)
    e_nederl_Bis, // like @p e_deutsch_His but with B ens Bes
    e_russian_Ci // classical but in Russian: До Ре Ми Фа Соль Ля Си
  };

        /** Ealter enumeration type describes all signs which can be before note in score.
        * It can be: @li e_None = 3 @li e_Sharp = 1 @li e_DoubleSharp=2
        * @li e_Flat= -1 @li e_DoubleFlat= -2 @li e_Natural=0  */
  enum Ealter { e_Natural = 0, e_Sharp = 1, e_DoubleSharp = 2, e_Flat = -1, e_DoubleFlat = -2, e_None = 3 };

  char note; /** note variable is a number in "diatonic notation" (see constructor).*/
  char octave; /** Octave number is @p 0 for "small octave",  @p -1 for "Great" @p 1 for "one-line". */

        /** @param accidental means raising or dropping a note, so it ca be:
        * @li 2 for double sharp (x)
        * @li 1 for sharp (#)
        * @li 0 for natural
        * @li -1 for flat (b)
        * @li -2 for double flat (bb) */
  char alter;

        /** Construct object of Tnote from number of note, number of octave and optional
        * accidental. The note number is:
        * @li "1" for C
        * @li "2" for D1
        * @li .......
        * @li 7 for B (H in Deutsh)
        * If accidental is not defined, the note is natural.  */
  Tnote(char diatonNote, char oct, char accid = 0);

        /** The simple constructor, creates the note instance with 0 note -
        * It makes no sense in musical notation. It's needed for vectors. */
  Tnote();

        /** Construct object of Tnote from number, that represents:
        * @li "1" for C1 in first octave
        * @li "2" for Cis
        * @li .......
        * @li 13 for C2 in next octave
        * @li -12 for C in little octave etc....
        * The sharp accidental is default. If You need others, you can use @p showWithFlat(). */
  Tnote(short chromaticNrOfNote);
  ~Tnote();

      /** Returns @p TRUE when note is valid. There are used 'undefined' notes with 0 - they are invalid. */
  bool isValid() const { return (note > 0 && note < 8); }

      /** Static value determines default name style for a note */
  static EnameStyle defaultStyle;

  bool operator==(const Tnote N2) const {
    return (note == N2.note && octave == N2.octave && alter == N2.alter);
  }

  bool operator!=(const Tnote N2) const {
    return ( note != N2.note || octave != N2.octave || alter != N2.alter);
  }

        /** @return List of Tnote objects, which are the same (in sound sense),
        * like note represents by class. (C# = Db, or Cx = D = Ebb)
        * @param enableDbAccids if @p TRUE - checks substitutes with double accidentals  */
  TnotesList getTheSameNotes (bool enableDbAccids) const;

      /** Return this note converted into double-sharp or the same note if not possible. */
  Tnote showWithDoubleSharp() const;

      /** Return this note converted into sharp or the same note if not possible. */
  Tnote showWithSharp() const;

      /** It returns Tnote object with no accidentals, if it's possible, or with sharps if not.  */
  Tnote showAsNatural() const;

      /** Return this note converted into flat or the same note if not possible. */
  Tnote showWithFlat() const;

      /** Return this note converted into double-flat or the same note if not possible. */
  Tnote showWithDoubleFlat() const;

      /** This method compares actual note, with otherNote.
        * @param otherNote
        * @param ignoreOctave If 1 (TRUE) the octave values are ignored,
        * and method compares only number of note and accidental.
        * @return 1 for TRUE or 0 if notes are different */
  short compareNotes(const Tnote& otherNote, short ignoreOctave = 0) const;

  std::string getName(EnameStyle notation = e_norsk_Hb, bool showOctave = 1) const;

        /** Returns note name converted to QString */
  QString toText (EnameStyle notation, bool showOctave = true) const {
    return QString::fromUtf8(getName(notation, showOctave).data());
  }

      /** Returns note name converted to QString */
  QString toText (bool showOctave = true) const { return toText(defaultStyle, showOctave); }

        /** Returns note name formatted to HTML*/
  QString toRichText(EnameStyle notation, bool showOctave = true) const;

        /** Returns note name formatted to HTML in default name style sets by @p defaultStyle. */
  QString toRichText(bool showOctave = true) const { return toRichText(defaultStyle, showOctave); }

  short chromatic() const; /** Returns chromatic number of note f.e. C1 is 60 */
  void setChromatic(short noteNr); /** Determines note, octave and accidental from chromatic value. */

      /** Adds given @p tag or 'pitch' key to XML stream compatible with MusicXML format with current note
       * Following elements can be prefixed with @p prefix (it is used i.e. to tuning in MusicXML)
       * If @p attr and its @p val is set the attribute is added
       * <pitch attr="val">
       *     <prefix-step>G</prefix-step>
       *     <prefix-octave>2</prefix-octave>
       *     <prefix-alter>-1</prefix-alter>
       * </pitch>  */
  void toXml(QXmlStreamWriter& xml, const QString& tag = QStringLiteral("pitch"), const QString& prefix = QString(),
             const QString& attr = QString(), const QString& val = QString()) const;

      /** Reads this note from XML stream.
       * It looks every note element prefixed with @p prefix. */
  void fromXml(QXmlStreamReader& xml, const QString& prefix = QString());


  unsigned int toMidi() const { return chromatic() + 47; } /** Returns this note as midi number. */
  void fromMidi(unsigned int midiNote) { setChromatic(midiNote - 47); } /** Sets this note from midi note number. */

private:

  static std::string m_solmization[7];
  static std::string m_solmizationRu[7];

};
    /** This function is substitute of >> operator for @class Tnote.
    * It checks is Tnote valid, and return Boolean about it. */
NOOTKACORE_EXPORT bool getNoteFromStream(QDataStream &in, Tnote &n);

NOOTKACORE_EXPORT QDataStream &operator<< (QDataStream &out, const Tnote &n);
NOOTKACORE_EXPORT QDataStream &operator>> (QDataStream &in, Tnote &n);


#endif
