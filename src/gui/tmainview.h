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

#ifndef TMAINVIEW_H
#define TMAINVIEW_H

#include <QGraphicsView>
#include <QPointer>

class Tmenu;
class TtoolBar;
class TnoteName;
class TnameTip;
class TpitchView;
class TlayoutParams;
class TcombinedAnim;
class QVBoxLayout;
class QBoxLayout;
class QGraphicsProxyWidget;

class QAction;

/** 
 * This widget manages of a layout of main Nootka window
 * All widgets are wrapped with QGraphicsProxyWidget 
 * and their layout is managed QGraphicsLayout
 */
class TmainView : public QGraphicsView
{

Q_OBJECT

public:
	TmainView(TlayoutParams* layParams, TtoolBar* toolW, QWidget* statLabW, TpitchView* pitchW,
            QGraphicsView* scoreW, QGraphicsView* guitarW, TnoteName* name, QWidget* parent = 0);
	
	void addNoteName(); /** Adds note name widget over a score (for single note mode) */
	void takeNoteName(); /** Takes note name from view. */
	
	void addExamViews(QWidget* resultsW, QWidget* progressW); /** Adds bar with those widgets */
	void takeExamViews(); /** Removes exam widgets, WIDGETS ARE DELETED! */
	
	void moveExamToName(); /** Moves 'exam view' above note view. Changes its direction to vertical. */
	
	void setBarAutoHide(bool autoHide); /** Makes tool bar permanently visible or displayed on demand (mouse action) */
	bool isAutoHide() { return m_isAutoHide; }

signals:
	void statusTip(const QString&);
  void sizeChanged(const QSize&);
	
protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual bool eventFilter(QObject* ob, QEvent* event);
  virtual bool viewportEvent(QEvent *event);
	virtual void mouseMoveEvent(QMouseEvent* event);
  void updateBarLine();

	void startHideAnim();

  void mainMenuExec();
  void scoreMenuExec();

protected slots:
	void showToolBar();
  void updateLayout(); /** Method called by timer from resizeEvent() */
	void menuSlot(Tmenu* m);
	
private:
	QWidget													*m_status;
  QGraphicsView                   *m_score, *m_guitar;
	QWidget													*m_results, *m_progress, *m_container, *m_touchedWidget;
  TpitchView                      *m_pitch;
	TnoteName												*m_name;
  TtoolBar                        *m_tool;
	QGraphicsWidget									*m_proxy;
	QPointer<QBoxLayout>				 		 m_mainLay, m_statAndPitchLay, m_scoreAndNameLay, m_nameLay, m_resultLay;
	QGraphicsLineItem			 					*m_barLine;
	QPointer<QGraphicsProxyWidget>	 m_proxyBar;
	QPointer<TcombinedAnim>					 m_animBar;
	bool												 		 m_isAutoHide;
	TlayoutParams										*m_layParams;
	QTimer													*m_timerBar;
	TnameTip												*m_nameTip;
  bool                             m_mainMenuTap, m_scoreMenuTap, m_playBarTap;

};

#endif // TMAINVIEW_H
