/***************************************************************************
 *   Copyright (C) 2011-2016 by Tomasz Bojczuk                             *
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


#ifndef TAUDIOPARAMS_H
#define TAUDIOPARAMS_H

#include <nootkacoreglobal.h>
#include <QtCore/qstring.h>


/**
 * Class describes audio input and output parameters.
 */
class NOOTKACORE_EXPORT TaudioParams
{

public:

  bool    JACKorASIO; /**< forces to use JACK under Linux or ASIO under Windows (if they are available) */

  bool    INenabled; /**< is audio input enabled */
  QString INdevName; /**< input device name */
  float   a440diff; /**< difference between standard a1 440Hz and user preferred base pitch */
  float   minimalVol; /**< only above this value detected note is sending to Nootka */
  qreal   minDuration; /**< minimal duration of a sound above which it is detected */
  int     detectMethod; /**< pitch detection method (0 - MPM, 1 - auto-correlation, 2 - MPM modified cepstrum) */
  quint8  intonation; /**< accuracy of intonation in detected note - corresponds with Eaccuracy */
  qreal   minSplitVol; /**< minimum volume change to split the same note (0.0 - no split) */

      /** multiplexer of sound volume (aka %)
       * that determines minimum volume of next note to be pitch-detected.
       * i.e. - value of 0.8 determines that note has to have at least 80% volume of average volume */
  qreal   skipStillerVal;
  bool    equalLoudness; /**< if TRUE - noise filters are performed - FALSE by default */
// duplex mode
  bool    forwardInput; /**< if true, all captured audio data is pushed to output */
  bool    playDetected; /**< pitch-detected notes will be played */

// audio output settings
  bool    OUTenabled; /**< audio output enabled */
  QString OUTdevName; /**< output device name */
  bool    midiEnabled; /**< default false */
  QString midiPortName; /**< default empty to find system default */
  unsigned char midiInstrNr; /**< default 0 - grand piano */
  int     audioInstrNr; /**< corresponds with Einstrument enum numbers, default 1 classical guitar */

  QString dumpPath; /**< Path when captured PCM audio data will be dumped, if empty (default) - no dump performed */
};


#endif // TAUDIOPARAMS_H
