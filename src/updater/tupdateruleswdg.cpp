/***************************************************************************
 *   Copyright (C) 2013 by Tomasz Bojczuk                                  *
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


#include "tupdateruleswdg.h"
#include "updatefunctions.h"
#include <QtGui>

TupdateRulesWdg::TupdateRulesWdg(TupdateRules* updateRules, QWidget* parent) :
  QWidget(parent),
  m_updateRules(updateRules)
{  
  QHBoxLayout *mainLay = new QHBoxLayout;
  QGroupBox *updatesEnableGr = new QGroupBox(tr("check for Nootka updates"), this);
  updatesEnableGr->setCheckable(true);
  updatesEnableGr->setChecked(m_updateRules->enable);
  mainLay->addWidget(updatesEnableGr);
 
  QHBoxLayout *inLay = new QHBoxLayout;
  QVBoxLayout *periodLay = new QVBoxLayout;
  m_dailyRadio = new QRadioButton(tr("daily"), this);
  periodLay->addWidget(m_dailyRadio);
  m_weeklyRadio = new QRadioButton(tr("weekly"), this);
  periodLay->addWidget(m_weeklyRadio);
  m_monthlyRadio = new QRadioButton(tr("monthly"), this);
  periodLay->addWidget(m_monthlyRadio);
  inLay->addLayout(periodLay);
  m_periodGroup = new QButtonGroup(this);
  m_periodGroup->addButton(m_dailyRadio);
  m_periodGroup->addButton(m_weeklyRadio);
  m_periodGroup->addButton(m_monthlyRadio);
  
  if (m_updateRules->period == e_daily)
    m_dailyRadio->setChecked(true);
  else if (m_updateRules->period == e_weekly)
    m_weeklyRadio->setChecked(true);
  else
    m_monthlyRadio->setChecked(true);
  
  QVBoxLayout *stableLay = new QVBoxLayout;
  m_allRadio = new QRadioButton(tr("all new versions"), this);
  stableLay->addWidget(m_allRadio);
  m_stableRadio = new QRadioButton(tr("stable versions only"), this);
  stableLay->addWidget(m_stableRadio);
  inLay->addLayout(stableLay);
  m_stableAllGroup = new QButtonGroup(this);
  m_stableAllGroup->addButton(m_allRadio);
  m_stableAllGroup->addButton(m_stableRadio);
  updatesEnableGr->setLayout(inLay);
  
  if (m_updateRules->checkForAll)
    m_allRadio->setCheckable(true);
  else
    m_stableRadio->setChecked(true);
  
  setLayout(mainLay);
  
}

TupdateRulesWdg::~TupdateRulesWdg() {}

void TupdateRulesWdg::saveSettings() {
  saveUpdateRules(*m_updateRules);
}


