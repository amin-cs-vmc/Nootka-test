/***************************************************************************
 *   Copyright (C) 2006-2016 by Tomasz Bojczuk                             *
 *   seelook@gmail.com                                                      *
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

#ifndef TTUNE_H
#define TTUNE_H

#include "tnote.h"
#include <QtCore/qmetatype.h>
#include <QtCore/qxmlstream.h>
#include <nootkacoreglobal.h>


/**
 * It stores six notes of Tnote objects, which can be set
 * in constructor and get by overloaded [] operator.
*/
class NOOTKACORE_EXPORT Ttune
{

public:
        /** @p tuneName is the name, @p S(1-6) are notes.
        * Empty notes (Tnote(0,0,0)) can control strings number
        * when empty - it is moved to the end of a array and stringNr() is less.
        * This way only a number of string [from 1 to 6] is supported. */
    Ttune(const QString& tuneName = QString(), const Tnote& S1 = Tnote(0,0,0) , const Tnote& S2 = Tnote(0,0,0),
                    const Tnote& S3 = Tnote(0,0,0), const Tnote& S4 = Tnote(0,0,0),
                    const Tnote& S5 = Tnote(0,0,0), const Tnote& S6 = Tnote(0,0,0));

    QString name; /**< It is a name of the tune*/
    quint8 stringNr() { return m_strNumber; } /** Number of strings for current tune/guitar */

        /** When tune has less than 3 strings and "scale" as a name it represents a scale of an instrument
         * and it is not guitar. */
    bool isGuitar() { return m_isGuitar; }

        /** Substitute of [] operator - returns note of given string. */
    Tnote  str(quint8 stringNr) { return stringsArray[stringNr - 1]; }

    static Ttune stdTune; // standard tune template
    static Ttune tunes[4]; // templates for guitar tunes
    static Ttune bassTunes[4]; // templates for bass guitar tunes

    void copy(Ttune& t); /**< Copies given tuning to this one. */

    static void prepareDefinedTunes(); /**< Makes translations in defined tunes. */

    friend QDataStream &operator<< (QDataStream &out, const Ttune &t);
    friend QDataStream &operator>> (QDataStream &in, Ttune &t);

        /** Method responses for converting tuning to XML structure..
         * When @p isExam is @p TRUE it is wrapped in <tuning id="0"> tag.
         * Attribute @p id determining kind of tune.
         * Only when tuning is different than all defined tuning suitable XML elements are written.
         * When @p isExam is @p FALSE <staff-details> is a main key and all elements are saved. */
    void toXml(QXmlStreamWriter& xml, bool isExam = true);
    bool fromXml(QXmlStreamReader& xml, bool isExam = true);

        /** Overloaded operator [] allows to use statement
        * @li Ttune @p your_variable[number_of_a_string]
        * @p stringNr is real string number (1 to 6) */
    Tnote &operator[] (quint8 stringNr) { return stringsArray[stringNr - 1]; }
    bool operator==(Ttune& T2) {
        return ( stringsArray[0]==T2[1] && stringsArray[1]==T2[2] && stringsArray[2]==T2[3] &&
                 stringsArray[3]==T2[4] && stringsArray[4]==T2[5] && stringsArray[5]==T2[6] );
    }
    bool operator!=(Ttune& T2) {
        return ( stringsArray[0]!=T2[1] || stringsArray[1]!=T2[2] || stringsArray[2]!=T2[3] ||
                stringsArray[3]!=T2[4] || stringsArray[4]!=T2[5] || stringsArray[5]!=T2[6] );
    }

protected:
    Tnote stringsArray[6]; /**< Array of Tnote that represent six strings */

        /** This method is called by constructor and operator.
         * It calculates number of strings by selecting string with defined notes
         * and moving empty ones to the end of stringsArray.
         * THIS IS ONLY WAY TO MANAGE STRINGS NUMBER. */
    void determineStringsNumber();

private:
    quint8   m_strNumber;
    bool    m_isGuitar;

};

Q_DECLARE_METATYPE(Ttune)


NOOTKACORE_EXPORT QDataStream &operator<< (QDataStream &out, const Ttune &t);
NOOTKACORE_EXPORT QDataStream &operator>> (QDataStream &in, Ttune &t);

#endif
