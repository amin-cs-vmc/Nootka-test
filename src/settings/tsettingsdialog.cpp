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

#include "tsettingsdialog.h"
#include "texamsettings.h"
#include "tguitarsettings.h"
#include "tscoresettings.h"
#include "tnotenamesettings.h"
#include "tglobalsettings.h"
#include "audioinsettings.h"
#include "audiooutsettings.h"
#include <taudioparams.h>
#include <trtaudio.h>
#include <tmidiout.h>
#include <tglobals.h>
#include <tfirstrunwizzard.h>
#include <QtWidgets>


extern Tglobals *gl;
extern bool resetConfig;


TsettingsDialog::TsettingsDialog(QWidget *parent) :
        TsettingsDialogBase(parent),
        m_globalSett(0), m_scoreSett(0),
        m_nameSett(0), m_guitarSett(0),
        m_examSett(0), m_sndOutSett(0),
        m_sndInSett(0), m_audioSettingsPage(0),
        m_7thNoteToDefaults(false)
{
    setWindowTitle("Nootka - " + tr("application's settings"));

//     navList->setFixedWidth(110);
    navList->addItem(tr("Common"));
    navList->item(0)->setIcon(QIcon(gl->path+"picts/global.png"));
    navList->item(0)->setTextAlignment(Qt::AlignCenter);
    navList->addItem(tr("Score"));
    navList->item(1)->setIcon(QIcon(gl->path+"picts/scoreSettings.png"));
    navList->item(1)->setTextAlignment(Qt::AlignCenter);
    navList->addItem(tr("Names","name-calling"));
    navList->item(2)->setIcon(QIcon(gl->path+"picts/nameSettings.png"));
    navList->item(2)->setTextAlignment(Qt::AlignCenter);
    navList->addItem(tr("Instrument"));
    navList->item(3)->setIcon(QIcon(gl->path+"picts/guitarSettings.png"));
    navList->item(3)->setTextAlignment(Qt::AlignCenter);
    navList->addItem(tr("Sound"));
    navList->item(4)->setIcon(QIcon(gl->path+"picts/soundSettings.png"));
    navList->item(4)->setTextAlignment(Qt::AlignCenter);
		navList->addItem(tr("Exercises") + "\n& " + tr("Exam"));
    navList->item(5)->setIcon(QIcon(gl->path+"picts/questionsSettings.png"));
    navList->item(5)->setTextAlignment(Qt::AlignCenter);
    
		defaultBut = buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
			defaultBut->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
			defaultBut->setStatusTip(tr("Restore default settings for above parameters."));
		okBut = buttonBox->addButton(QDialogButtonBox::Apply);
			okBut->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
		cancelBut = buttonBox->addButton(QDialogButtonBox::Cancel);
			cancelBut->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    
		connect(okBut, SIGNAL(clicked()), this, SLOT(accept()));
    connect(navList, SIGNAL(currentRowChanged(int)), this, SLOT(changeSettingsWidget(int)));
    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
		connect(defaultBut, SIGNAL(pressed()), this, SLOT(restoreDefaults()));

		
    navList->setCurrentRow(0);
		changeSettingsWidget(1); // score settings appears first - it is the biggest
		changeSettingsWidget(0);
}


void TsettingsDialog::saveSettings() {
  if (m_scoreSett)
			m_scoreSett->saveSettings();
  if (m_globalSett)
			m_globalSett->saveSettings();
  if (m_nameSett)
			m_nameSett->saveSettings();
  if (m_guitarSett) {
			m_guitarSett->saveSettings();
			if (!m_audioSettingsPage) { // when no audio settings set pitch range according to clef for tune
				if (m_guitarSett->lowestNote().getChromaticNrOfNote() < Tnote(6, -2, 0).getChromaticNrOfNote())
						gl->A->range = TaudioParams::e_low;
				else /** In fact, even treble clef requires middle scale, so high scale is ignored here*/
						gl->A->range = TaudioParams::e_middle;
				gl->A->audioInstrNr = qBound(1, m_guitarSett->currentInstrument(), 3);
			}
	}
  if (m_examSett)
			m_examSett->saveSettings();
  if (m_sndOutSett) // if audio out settings was created
			m_sndOutSett->saveSettings(); // respect user settings
  if (m_sndInSett)
				m_sndInSett->saveSettings();
	if (m_7thNoteToDefaults) {
		if ((Tpage_3::note7txt().toLower() == "b") != (gl->seventhIs_B)) {
			/** NOTE As long as TscoreSettings is created at first and always exist 
			 * only adjustment of global note names is required. 
			 * How: When user opens Name settings and changes 7-th note TscoreSettings changes automatically 
			 * This condition is called in opposite situation: 
			 * TscoreSettings wants defaults and already has been adjusted. 
			 * Theoretically - if TscoreSettings would not exist it is more difficult to restore its defaults here. */
			if (Tpage_3::note7txt().toLower() == "b")
					gl->seventhIs_B = true;
			else
					gl->seventhIs_B = false;
		}
	}
}


void TsettingsDialog::restoreDefaults() {
		if (stackLayout->currentWidget() == m_globalSett)
			m_globalSett->restoreDefaults();
		if (stackLayout->currentWidget() == m_scoreSett) {
			m_scoreSett->restoreDefaults();
			m_7thNoteToDefaults = true;
		}
		if (m_nameSett)
			m_nameSett->restoreDefaults();
		if (stackLayout->currentWidget() == m_guitarSett)
			m_guitarSett->restoreDefaults();
		if (stackLayout->currentWidget() == m_examSett)
			m_examSett->restoreDefaults();
		if (m_audioSettingsPage) {
			if (m_audioTab->currentWidget() == m_sndInSett)
				m_sndInSett->restoreDefaults();
			else if (m_audioTab->currentWidget() == m_sndOutSett)
				m_sndOutSett->restoreDefaults();
		}
}


void TsettingsDialog::allDefaultsRequired() {
// 		resetConfig = true;
		close();
}


/** Settings pages are created on demand, also 
     * to avoid generating audio devices list every opening Nootka preferences
     * witch is slow for pulseaudio, the list is generated on demand.
     * When user first time opens Sound settings widget.*/
void TsettingsDialog::changeSettingsWidget(int index) {
  QWidget* currentWidget = 0;
	if (m_audioSettingsPage)
		m_sndInSett->stopSoundTest();
  switch (index) {
    case 0: {
      if (!m_globalSett) {
        m_globalSett = new TglobalSettings();
        stackLayout->addWidget(m_globalSett);
				connect(m_globalSett, SIGNAL(restoreAllDefaults()), this, SLOT(allDefaultsRequired()));
      }
      currentWidget = m_globalSett;
      break;
    }
    case 1: {
      if (!m_scoreSett) {
        m_scoreSett = new TscoreSettings();
        stackLayout->addWidget(m_scoreSett);
      }
      if (m_nameSett) { // update current state of 7-th note, user could change it already
					m_scoreSett->seventhIsBChanged(m_nameSett->is7th_b()); 
          connect(m_nameSett, SIGNAL(seventhIsBChanged(bool)), m_scoreSett, SLOT(seventhIsBChanged(bool)));
			}
			if (m_guitarSett) {
					m_scoreSett->setDefaultClef(m_guitarSett->currentClef());
					connect(m_guitarSett, SIGNAL(clefChanged(Tclef)), m_scoreSett, SLOT(defaultClefChanged(Tclef)));
			}
      currentWidget = m_scoreSett;
      break;
    }
    case 2: {
      if (!m_nameSett) {
        m_nameSett = new TnoteNameSettings();
        stackLayout->addWidget(m_nameSett);        
      }
      if (m_scoreSett) {
          connect(m_nameSett, SIGNAL(seventhIsBChanged(bool)), m_scoreSett, SLOT(seventhIsBChanged(bool)));
			}
      currentWidget = m_nameSett;
      break;
    }
    case 3: {
      if (!m_guitarSett) {
        m_guitarSett = new TguitarSettings();
        stackLayout->addWidget(m_guitarSett);
      }
      if (m_scoreSett)
					connect(m_guitarSett, SIGNAL(clefChanged(Tclef)), m_scoreSett, SLOT(defaultClefChanged(Tclef)));
			if (m_sndOutSett)
					connect(m_guitarSett, SIGNAL(instrumentChanged(int)), m_sndOutSett, SLOT(whenInstrumentChanged(int)));
			if (m_sndInSett) {
					connect(m_guitarSett, SIGNAL(tuneChanged(Ttune*)), m_sndInSett, SLOT(tuneWasChanged(Ttune*)));
					connect(m_guitarSett, SIGNAL(lowestNoteChanged(Tnote)), m_sndInSett, SLOT(whenLowestNoteChanges(Tnote)));
			}
      currentWidget = m_guitarSett;
      break;
    }
    case 5: {
      if (!m_examSett) {
        m_examSett = new TexamSettings();
        stackLayout->addWidget(m_examSett);
      }
      currentWidget = m_examSett;
      break;
    }
    case 4: {
      if (!m_audioSettingsPage) {
        createAudioPage();
        stackLayout->addWidget(m_audioSettingsPage);
        m_sndInSett->generateDevicesList();
        m_sndOutSett->generateDevicesList();
				if (m_guitarSett) { // update pitches range according to guitar settings state
					connect(m_guitarSett, SIGNAL(lowestNoteChanged(Tnote)), m_sndInSett, SLOT(whenLowestNoteChanges(Tnote)));
					m_sndInSett->whenLowestNoteChanges(m_guitarSett->lowestNote());
					m_sndOutSett->whenInstrumentChanged(m_guitarSett->currentInstrument());
					m_sndInSett->tuneWasChanged(m_guitarSett->currentTune());
					connect(m_guitarSett, SIGNAL(instrumentChanged(int)), m_sndOutSett, SLOT(whenInstrumentChanged(int)));
					connect(m_guitarSett, SIGNAL(tuneChanged(Ttune*)), m_sndInSett, SLOT(tuneWasChanged(Ttune*)));
				}
      }
      currentWidget = m_audioSettingsPage;
      break;
    }
  }
  stackLayout->setCurrentWidget(currentWidget);
}


void TsettingsDialog::createAudioPage() {
    m_sndInSett = new AudioInSettings(gl->A, gl->path, gl->Gtune());
    m_sndOutSett = new AudioOutSettings(gl->A, m_sndInSett); // m_sndInSett is bool - true when exist
    m_audioSettingsPage = new QWidget();
    m_audioTab = new QTabWidget(m_audioSettingsPage);
    QVBoxLayout *audioLay = new QVBoxLayout;
    audioLay->addWidget(m_audioTab);
    m_audioTab->addTab(m_sndInSett, tr("listening"));
    m_audioTab->addTab(m_sndOutSett, tr("playing"));
    m_audioSettingsPage->setLayout(audioLay);
		connect(m_audioTab, SIGNAL(currentChanged(int)), m_sndInSett, SLOT(stopSoundTest()));
}





