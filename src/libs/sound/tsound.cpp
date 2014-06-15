/***************************************************************************
 *   Copyright (C) 2011-2014 by Tomasz Bojczuk                             *
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
#include "tsound.h"
#include "widgets/tpitchview.h"
#include "tmidiout.h"
#include "trtaudioout.h"
#include "trtaudioin.h"
#include <tglobals.h>
#include <taudioparams.h>
#include <QPushButton>
#include <QTimer>
#include <QDebug>



extern Tglobals *gl;

Tsound::Tsound(QObject* parent) :
  QObject(parent),
  sniffer(0),
  player(0),
  m_examMode(false)
{
  if (gl->A->OUTenabled)
      createPlayer();
  else
      player = 0;
  if (gl->A->INenabled) {
			createSniffer();
  } else {
			sniffer = 0;
  }
}

Tsound::~Tsound()
{ //They have not a parent
  deleteSniffer();
  deletePlayer();
}

//------------------------------------------------------------------------------------
//------------  public  methods     --------------------------------------------------
//------------------------------------------------------------------------------------

void Tsound::play(Tnote note) {
  bool playing = false;
  if (player && note.note)
			playing = player->play(note.getChromaticNrOfNote());
  if (playing && !gl->A->playDetected && player->type() == TabstractPlayer::e_midi) {
    if (sniffer) { // pause sniffer if midi output was started
			if (!m_midiPlays) { // stop listenning just once
				sniffer->stopListening();
				m_pitchView->stopVolume();
				m_midiPlays = true;
			}
    }
  }
}


void Tsound::acceptSettings() {
	bool doParamsUpdated = false;
  // for output
  if (gl->A->OUTenabled) {
    if (!player)
        createPlayer();
    else {
        if (gl->A->midiEnabled) {
          deletePlayer(); // it is safe to delete midi
          createPlayer(); // and create it again
        } else { // avoids deleting TaudioOUT instance and loading ogg file every acceptSettings call
          if (player->type() == TabstractPlayer::e_midi) {
              deletePlayer(); // player was midi so delete
              createPlayer();
          } else { // just set new params to TaudioOUT
              doParamsUpdated = true;
          }
        }
        if (player) {
          if (!player->isPlayable())
            deletePlayer();
				}
    }
  } else {
      deletePlayer();
  }
  // for input
  if (gl->A->INenabled) {
    m_pitchView->setEnabled(true);
    if (!sniffer) {
      createSniffer();
      m_pitchView->setAudioInput(sniffer);
    }
    else {
      setDefaultAmbitus();
      doParamsUpdated = true;
    }
    m_pitchView->setIsVoice(gl->A->isVoice);
		m_pitchView->setMinimalVolume(gl->A->minimalVol);
		m_pitchView->setIntonationAccuracy(gl->A->intonation);
  } else {
    m_pitchView->setDisabled(true);
    if (sniffer)
      deleteSniffer();
  }
  if (doParamsUpdated) {
			if (player && player->type() == TabstractPlayer::e_audio) {
					static_cast<TaudioOUT*>(player)->updateAudioParams();
			} else if (sniffer) {
					sniffer->updateAudioParams();
			}
	}
	if (sniffer && !m_pitchView->isPaused()) {
		sniffer->startListening();
		m_pitchView->startVolume();
	}
}


void Tsound::setPitchView(TpitchView* pView) {
  m_pitchView = pView;
  m_pitchView->setPitchColor(gl->EanswerColor);
  m_pitchView->setIsVoice(gl->A->isVoice);
  m_pitchView->setMinimalVolume(gl->A->minimalVol);
	m_pitchView->setIntonationAccuracy(gl->A->intonation);
  if (sniffer) {
			m_pitchView->setAudioInput(sniffer);
      m_pitchView->startVolume();
			sniffer->startListening();
	} else
			m_pitchView->setDisabled(true);
}


void Tsound::prepareToConf() {
//   stopPlaying();
  if (player) {
		player->stop();
    player->deleteMidi();
	}
  if (sniffer) {
    sniffer->stopListening();
    m_pitchView->stopVolume();
		sniffer->terminate();
  }
}


void Tsound::restoreAfterConf() {
  if (gl->A->midiEnabled) {
    if (player)
      player->setMidiParams();
  }
  if (sniffer) {
    if (!m_pitchView->isPaused()) {
      sniffer->startListening();
      m_pitchView->startVolume();
    }
  }
}


float Tsound::pitch() {
	if (sniffer)
		return sniffer->lastNotePitch();
	else
		return 0.0f;
}



void Tsound::wait() {
//     qDebug("wait");
  if (sniffer) {
    sniffer->stopListening();
    m_pitchView->stopVolume();
  }
}


void Tsound::go() {
//     qDebug("go");
  if (sniffer && !m_pitchView->isPaused()) {
    sniffer->startListening();
    m_pitchView->startVolume();
  }
}


void Tsound::prepareAnswer() {
  m_pitchView->setBgColor(QColor(gl->EanswerColor));
  m_pitchView->setDisabled(false);
}


void Tsound::pauseSinffing() {
    if (sniffer)
        sniffer->pause();
}


void Tsound::unPauseSniffing() {
    if (sniffer)
        sniffer->unPause();
}

bool Tsound::isSnifferPaused() {
    if (sniffer)
        return sniffer->isPaused();
    else
        return false;
}


void Tsound::restoreAfterAnswer() {
  m_pitchView->setBgColor(Qt::transparent);
  m_pitchView->setDisabled(true);
}


void Tsound::prepareToExam(Tnote loNote, Tnote hiNote) {
  m_examMode = true;
  if (sniffer) {
     m_pitchView->setDisabled(true);
		 m_prevLoNote = sniffer->loNote();
		 m_prevHiNote = sniffer->hiNote();
		 sniffer->setAmbitus(loNote, hiNote);
	}
}


void Tsound::restoreAfterExam() {
  m_examMode = false;
  if (sniffer) {
		sniffer->setAmbitus(m_prevLoNote, m_prevHiNote);
		m_pitchView->setDisabled(false);
		unPauseSniffing();
		go();
  }
}


void Tsound::stopPlaying() {
  if (player)
    player->stop();
}


bool Tsound::isPlayable() {
  if (player)
   return true;
  else
    return false;
}


void Tsound::setDefaultAmbitus() {
	if (sniffer)
		sniffer->setAmbitus(Tnote(gl->loString().getChromaticNrOfNote() - 5), // range extended about 4th up and down
									Tnote(gl->hiString().getChromaticNrOfNote() + gl->GfretsNumber + 5));
}

//------------------------------------------------------------------------------------
//------------  private  methods     --------------------------------------------------
//------------------------------------------------------------------------------------

void Tsound::createPlayer() {
  if (gl->A->midiEnabled) {
    player = new TmidiOut(gl->A);
		connect(player, SIGNAL(noteFinished()), this, SLOT(playingFinishedSlot()));
		m_midiPlays = false;
	} else
    player = new TaudioOUT(gl->A, gl->path);
}


void Tsound::createSniffer() {
  sniffer = new TaudioIN(gl->A);
  setDefaultAmbitus();
// 	sniffer->setAmbitus(Tnote(-31), Tnote(82)); // fixed ambitus bounded Tartini capacities
  connect(sniffer, SIGNAL(noteDetected(Tnote)), this, SLOT(noteDetectedSlot(Tnote)));
// 	QTimer::singleShot(500, sniffer, SLOT(startListening())); // Give time for launch whole app
}

void Tsound::deletePlayer() {
  if (player) {
    player->stop();
//   if (player) {
//     player->deleteLater();
		delete player;
    player = 0;
  }
}


void Tsound::deleteSniffer() {
	delete sniffer;
  sniffer = 0;
}


//------------------------------------------------------------------------------------
//------------  slots               --------------------------------------------------
//------------------------------------------------------------------------------------
void Tsound::playingFinishedSlot() {
//   qDebug("playingFinished");
  if (!m_examMode && sniffer) {
    if (!m_pitchView->isPaused()) {
        sniffer->startListening();
        m_pitchView->startVolume();
				m_midiPlays = false;
    }
  }
  emit plaingFinished();
//   go();
}

void Tsound::noteDetectedSlot(Tnote note) {
	//  qDebug() << "Tsound: got note" << note.toText();
	if (player && gl->A->playDetected)
		play(note);
  m_detectedNote = note;
  emit detectedNote(note);
}

