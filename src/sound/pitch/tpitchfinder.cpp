/***************************************************************************
 *   Copyright (C) 2011 by Tomasz Bojczuk                                  *
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

#include "tpitchfinder.h"
#include "tartini/channel.h"
// #include "tartini/mytransforms.h"
#include "tartini/filters/Filter.h"
#include "tartini/filters/IIR_Filter.h"
#include "tartini/analysisdata.h"

#include <QDebug>
#include <stdio.h>


TpitchFinder::audioSetts *glAsett;

float *filteredChunk = 0;

TpitchFinder::TpitchFinder() :
  m_chunkNum(0)
{
	m_aGl.chanells = 1;
	m_aGl.rate = 44100;
	m_aGl.windowSize = 2048;
	m_aGl.framesPerChunk = 1024;
	m_aGl.dBFloor = -150.0;
	m_aGl.equalLoudness = true;
	m_aGl.doingFreqAnalysis = true;
	m_aGl.doingAutoNoiseFloor = true;
	m_aGl.doingHarmonicAnalysis = true;
	m_aGl.firstTimeThrough = true;
	m_aGl.doingDetailedPitch = true;
	m_aGl.threshold = 93;
	m_aGl.analysisType = e_MPM_MODIFIED_CEPSTRUM;
	m_aGl.topPitch = 150.0;
	m_aGl.ampThresholds[AMPLITUDE_RMS][0]           = -85.0; m_aGl.ampThresholds[AMPLITUDE_RMS][1]           = -0.0;
	m_aGl.ampThresholds[AMPLITUDE_MAX_INTENSITY][0] = -30.0; m_aGl.ampThresholds[AMPLITUDE_MAX_INTENSITY][1] = -20.0;
	m_aGl.ampThresholds[AMPLITUDE_CORRELATION][0]   =  0.40; m_aGl.ampThresholds[AMPLITUDE_CORRELATION][1]   =  1.00;
	m_aGl.ampThresholds[FREQ_CHANGENESS][0]         =  0.50; m_aGl.ampThresholds[FREQ_CHANGENESS][1]         =  0.02;
	m_aGl.ampThresholds[DELTA_FREQ_CENTROID][0]     =  0.00; m_aGl.ampThresholds[DELTA_FREQ_CENTROID][1]     =  0.10;
	m_aGl.ampThresholds[NOTE_SCORE][0]              =  0.03; m_aGl.ampThresholds[NOTE_SCORE][1]              =  0.20;
	m_aGl.ampThresholds[NOTE_CHANGE_SCORE][0]       =  0.12; m_aGl.ampThresholds[NOTE_CHANGE_SCORE][1]       =  0.30;
	
	
	glAsett = &m_aGl;
	
	m_channel = new Channel(this, aGl().windowSize);
	myTransforms.init(aGl().windowSize, 0, aGl().rate, aGl().equalLoudness);
	if (aGl().equalLoudness)
	  filteredChunk = new float[aGl().framesPerChunk+16] + 16;
}

TpitchFinder::~TpitchFinder()
{
	

}


void TpitchFinder::searchIn(float* chunk) {
	// copy chunk to channel
	if (m_channel->locked())
		qDebug() << "channel still locked";
	if (aGl().equalLoudness) {
	  m_channel->highPassFilter->filter(chunk, filteredChunk, aGl().framesPerChunk);
	  for(int i = 0; i < aGl().framesPerChunk-1; i++)
		  filteredChunk[i] = qBound(filteredChunk[i], -1.0f, 1.0f);
	}
	m_channel->shift_left(aGl().framesPerChunk);
	std::copy(chunk, chunk+aGl().framesPerChunk - 1, m_channel->end() - aGl().framesPerChunk);
	if (aGl().equalLoudness)
	  std::copy(filteredChunk, filteredChunk+aGl().framesPerChunk-1, m_channel->filteredInput.end() - aGl().framesPerChunk);
	
// 	for(int i = 0; i<aGl().framesPerChunk-1; i++) {
// 		std::cout << *(m_channel->end()-1023+i) <<  "\t";
// 		std::cout << *(filteredChunk+i) <<  "\t";
// 	}
	start();
// 	qDebug() << "started";
// 	if (filteredChunk)
// 	  delete[] filteredChunk;

}

void TpitchFinder::start() {
	FilterState filterState;
	qDebug() << currentChunk();
	if (currentChunk()) {
		m_channel->processNewChunk(&filterState);
  // 	incrementChunk();
	  m_channel->lock();
	  AnalysisData *data = m_channel->dataAtCurrentChunk();
	  if (data) {
            qDebug() << "data chunk" << data->noteIndex;
		
		if (m_channel->isVisibleNote(data->noteIndex) && m_channel->isLabelNote(data->noteIndex))
		  qDebug() << data->pitch;
	  }
  // 	qDebug() << "data chunk";
	  m_channel->unlock();
	}
	incrementChunk();
	
}	

/*
Channel *active = gdata->getActiveChannel();
  if(active) {
    active->lock();
    AnalysisData *data = active->dataAtCurrentChunk();
    if(data && active->isVisibleNote(data->noteIndex) && active->isLabelNote(data->noteIndex)) {
        sprintf(temp, "Note: %d", data->noteIndex);
        if (shown && data->pitch > 35) {
            qDebug() << data->pitch;
            shown = false;
        }
    } else {
        sprintf(temp, "Note:    ");
        shown = true;
    }
    */
