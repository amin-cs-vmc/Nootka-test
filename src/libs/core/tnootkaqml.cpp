/***************************************************************************
 *   Copyright (C) 2017 by Tomasz Bojczuk                                  *
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

#include "tnootkaqml.h"
#include "qtr.h"
#include "nootkaconfig.h"
#include "tglobals.h"
#include "tpath.h"
#include "music/tkeysignature.h"
#include "music/tnamestylefilter.h"
#include "score/tscoreobject.h"
#include "score/tstaffobject.h"
#include "score/tnoteobject.h"
#include "score/tstafflines.h"
#include "score/taddobject.h"
#include "instruments/tguitarbg.h"
#include "instruments/tpianobg.h"
#include "instruments/tbandoneonbg.h"
#include "instruments/tsaxbg.h"
#include "taction.h"
#include "music/ttuneobject.h"
#include "tmtr.h"

#include <QtQml/qqmlengine.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatetime.h>
#if defined (Q_OS_ANDROID)
  #include "Android/tfiledialog.h"
  #include "Android/tmobilemenu.h"
#else
  #include <QtWidgets/qfiledialog.h>
#endif
#include <QtCore/qdebug.h>


TnootkaQML* TnootkaQML::m_instance = nullptr;


TnootkaQML::TnootkaQML(QObject* parent) :
  QObject(parent)
{
  if (m_instance) {
    qDebug() << "TnootkaQML instance already exists!";
    return;
  }
  m_instance = this;

  qsrand(QDateTime::currentDateTime().toTime_t());

  qRegisterMetaType<Tclef>();
  qmlRegisterUncreatableType<Tclef>("Score", 1, 0, "Tclef", QStringLiteral("You cannot create an instance of the Tclef."));
  qRegisterMetaType<Tmeter>();
  qmlRegisterUncreatableType<Tmeter>("Score", 1, 0, "Tmeter", QStringLiteral("You cannot create an instance of the Tmeter."));
  qmlRegisterUncreatableType<Trhythm>("Score", 1, 0, "Trhythm", QStringLiteral("You cannot create an instance of the Trhythm."));
  qRegisterMetaType<Trhythm>();

  qmlRegisterType<TscoreObject>("Score", 1, 0, "TscoreObject");
  qmlRegisterType<TstaffObject>("Score", 1, 0, "TstaffObject");
  qmlRegisterType<TnoteObject>("Score", 1, 0, "TnoteItem");
  qmlRegisterType<TstaffLines>("Score", 1, 0, "TstaffLines");
  qmlRegisterType<TaddObject>("Score", 1, 0, "TaddObject");

  qmlRegisterUncreatableType<TcommonInstrument>("Nootka", 1, 0, "TcommonInstrument", QStringLiteral("You cannot create an instance of the TcommonInstrument."));
  qmlRegisterType<TguitarBg>("Nootka", 1, 0, "TguitarBg");
  qmlRegisterType<TpianoBg>("Nootka", 1, 0, "TpianoBg");
  qmlRegisterType<TbandoneonBg>("Nootka", 1, 0, "TbandoneonBg");
  qmlRegisterType<TsaxBg>("Nootka", 1, 0, "TsaxBg");
  qmlRegisterType<Taction>("Nootka", 1, 0, "Taction");
#if defined (Q_OS_ANDROID)
  qmlRegisterType<TmobileMenu>("Nootka", 1, 0, "TmobileMenu");
#endif

  qmlRegisterUncreatableType<TnootkaQML>("Nootka", 1, 0, "Nootka", QStringLiteral("You cannot create an instance of the TnootkaQML."));
  qRegisterMetaType<Tinstrument>();
  qmlRegisterUncreatableType<Tinstrument>("Nootka", 1, 0, "Tinstrument", QStringLiteral("You cannot create an instance of the Tinstrument."));
  qRegisterMetaType<Ttune>();
  qmlRegisterUncreatableType<Ttune>("Nootka", 1, 0, "Ttune", QStringLiteral("You cannot create an instance of the Ttune."));
  qmlRegisterType<TtuneObject>("Nootka", 1, 0, "TtuneObject");
}


TnootkaQML::~TnootkaQML()
{
  m_instance = nullptr;
}

//#################################################################################################
//###################       INVOKABLE METHODS          ############################################
//#################################################################################################

QString TnootkaQML::version() { return NOOTKA_VERSION; }


Tclef TnootkaQML::clef(int type) {
  return Tclef(static_cast<Tclef::EclefType>(type));
}


Tmeter TnootkaQML::meter(int m) {
  return Tmeter(static_cast<Tmeter::Emeter>(m));
}


Tnote TnootkaQML::note(int pitch, int octave, int alter, int rhythm, bool rest, bool dot) {
  return Tnote(static_cast<char>(pitch), static_cast<char>(octave), static_cast<char>(alter),
               Trhythm(static_cast<Trhythm::Erhythm>(rhythm), rest, dot, false));
}


Tnote TnootkaQML::note(const Tnote& n, int rhythm, bool rest, bool dot) {
  return Tnote(n, Trhythm(static_cast<Trhythm::Erhythm>(rhythm), rest, dot, false));
}


Tnote TnootkaQML::note(const Tnote& n, const Trhythm& r) {
  return Tnote(n, r);
}


Tnote TnootkaQML::note(int chroma,  bool sharp) {
  Tnote n(static_cast<short>(chroma));
  if (!sharp && (n.alter != 0 || (n.alter == 0 && n.note != 3 && n.note != 7))) // but skip e and b - otherwise they become fb or cb
    n = n.showWithFlat();
  return n;
}


Trhythm TnootkaQML::rhythm(int rtm, bool rest, bool dot, bool triplet) {
  return Trhythm(static_cast<Trhythm::Erhythm>(rtm), rest, dot, triplet);
}


/**
 * It returns glyphs of 'Nootka' font in contrary to TnoteObject::getHeadText()
 */
QString TnootkaQML::rhythmText(const Trhythm& r) {
  if (r.rhythm() == Trhythm::NoRhythm)
    return QStringLiteral("z"); // just black note-head
  QString out;
  if (r.isRest())
    out = QString(QChar(0xe106 + static_cast<int>(r.rhythm())));
  else
    out = QString(QChar(66 + static_cast<int>(r.rhythm())));
  if (r.hasDot())
    out += QStringLiteral(".");
  return out;
}


QString TnootkaQML::noteName(const Tnote& n, int style, bool showOctave) {
  // Tnote::toText() method returns only names in user preferred according to settings
  // To cheat it and force note name in any given style we are resetting pointer of is7th_B 
  // then Tnote skips filtering of a style during name generation.
  auto tmpPtr = TnameStyleFilter::is7th_B();
  TnameStyleFilter::setStyleFilter(nullptr, TnameStyleFilter::solfegeStyle());
  auto name = n.toText(static_cast<Tnote::EnameStyle>(style), showOctave);
  TnameStyleFilter::setStyleFilter(tmpPtr, TnameStyleFilter::solfegeStyle()); // restore is7th_B settings
  return name;
}


QString TnootkaQML::majorKeyName(int key) {
  return TkeySignature(static_cast<char>(key)).getMajorName();
}


QString TnootkaQML::minorKeyName(int key) {
  return TkeySignature(static_cast<char>(key)).getMinorName();
}


QString TnootkaQML::majAndMinKeyName(int key) {
  return TkeySignature(static_cast<char>(key)).getMajorName() + QLatin1String("<br>") + TkeySignature(static_cast<char>(key)).getMinorName();
}


QStringList TnootkaQML::keyComboModel() {
  QStringList model;
  for (int i = -7; i < 8; i++) {
    TkeySignature k(i);
    model << QLatin1String("(") + k.accidNumber() + QLatin1String(") ") + k.getMajorName() + QLatin1String(" / ") + k.getMinorName();
  }
  return model;
}


QString TnootkaQML::getLicense() {
  QFile file(Tpath::main + QLatin1String("gpl"));
  QString license;
  QTextStream in;
  if (!file.exists()) { // Debian based
      QDir d(Tpath::main);
      d.cdUp();
      file.setFileName(d.path() + QLatin1String("/doc/nootka/copyright"));
  }
  if(file.open(QFile::ReadOnly | QFile::Text)) {
      QTextStream in(&file);
      in.setCodec("UTF-8");
      license = in.readAll();
  }
  file.close();
  return license;
}


QString TnootkaQML::getChanges() {
  QFile file(Tpath::main + QLatin1String("changes"));
  QString chLog;
  QTextStream in;
  if(file.open(QFile::ReadOnly | QFile::Text)) {
      QTextStream in(&file);
      in.setCodec("UTF-8");
      QStringList htmlText = in.readAll().replace(QLatin1String("  "), QLatin1String("&nbsp;&nbsp;")).split(QLatin1String("\n"));
      for (int i = 0; i < htmlText.size(); i++) {
        if (htmlText[i].startsWith(QLatin1String("0.")) || htmlText[i].startsWith(QLatin1String("1.")))
          htmlText[i] = QLatin1String("<span style=\"font-size: x-large; color: #0000ff\"><b>&nbsp;") + htmlText[i] + QLatin1String("</b></span>");
        else if (htmlText[i].contains(QLatin1String("======")))
          htmlText[i] = QStringLiteral("<br><hr><b><big><center>Nootka ONE</big></b></center><hr>");
        else if (htmlText[i].contains(QLatin1String("BUG")))
          htmlText[i] = QStringLiteral("&nbsp;&nbsp;<u>BUG FIXES</u>");
        else if (htmlText[i].contains(QLatin1String("Under the hood")))
          htmlText[i] = QStringLiteral("&nbsp;&nbsp;<u>Under the hood</u>");
        else if (!htmlText[i].contains(QLatin1String("&nbsp;&nbsp; - ")))
          htmlText[i] = QLatin1String("<b>") + htmlText[i] + QLatin1String("</b>");
        htmlText[i].append(QLatin1String("<br>"));
      }
      chLog = htmlText.join(QString());
  }
  file.close();
  return chLog;
}


bool TnootkaQML::isAndroid()  {
#if defined (Q_OS_ANDROID)
  return true;
#else
  return false;
#endif
}


QStringList TnootkaQML::guitarTunings() {
  QStringList tunList;
  int start = static_cast<int>(Ttune::Standard_EADGBE);
  for (int t = start; t < start + 5; ++t)
    tunList << Ttune::definedName(static_cast<Ttune::Etunings>(t));
  return tunList;
}


QStringList TnootkaQML::bassTunings() {
  QStringList tunList;
  int start = static_cast<int>(Ttune::Bass4_EADG);
  for (int t = start; t < start + 4; ++t)
    tunList << Ttune::definedName(static_cast<Ttune::Etunings>(t));
  return tunList;
}


QString TnootkaQML::stdButtonText(int role) {
  switch (role) {
    case 33554432: return qTR("QPlatformTheme", "Apply");
    case 4194304: return qTR("QPlatformTheme", "Cancel");
    case 134217728: return qTR("QPlatformTheme", "Restore Defaults");
    case 16777216: return qTR("QPlatformTheme", "Help");
    case 2097152: return qTR("QPlatformTheme", "Close");
    case 1024: return qTR("QPlatformTheme", "OK");
    default: return QString();
  }
}


QString TnootkaQML::stdButtonIcon(int role) {
  switch (role) {
    case 33554432: return QStringLiteral("check"); // Apply
    case 4194304: return QStringLiteral("record"); // Cancel
    case 134217728: return QStringLiteral("fingerpoint"); // Restore defaults
    case 16777216: return QStringLiteral("help"); // Help
    case 2097152: return QStringLiteral("exit"); // Close
    case 1024: return QStringLiteral("check"); // OK
    default: return QString();
  }
}


Tinstrument TnootkaQML::instr(int type) {
  return Tinstrument(static_cast<Tinstrument::Etype>(type < 0 || type > INSTR_COUNT - 1 ? 0 : type));
}


QString TnootkaQML::getXmlToOpen() {
  QString openFile;
#if defined (Q_OS_ANDROID)
  openFile = TfileDialog::getOpenFileName(nullptr, GLOB->lastXmlDir(), QStringLiteral("xml"));
#else
  openFile = QFileDialog::getOpenFileName(nullptr, qApp->translate("TmelMan", "Open melody file"), GLOB->lastXmlDir(),
                                      qApp->translate("TmelMan", "MusicXML file") + QLatin1String(" (*.xml)"));
#endif
  if (!openFile.isEmpty())
    GLOB->setLastXmlDir(QFileInfo(openFile).absoluteDir().path());
  return openFile;
}


QString TnootkaQML::getXmlToSave() {
  QString saveFile;
#if defined (Q_OS_ANDROID)
  saveFile = TfileDialog::getSaveFileName(nullptr, GLOB->lastXmlDir(), QStringLiteral("xml"));
#else
  saveFile = QFileDialog::getSaveFileName(nullptr, qApp->translate("TmelMan", "Save melody as:"), GLOB->lastXmlDir(),
                                      qTR("TmelMan", "MusicXML file") + QLatin1String(" (*.xml)"));
#endif
  if (!saveFile.isEmpty())
    GLOB->setLastXmlDir(QFileInfo(saveFile).absoluteDir().path());
  return saveFile;
}


QString TnootkaQML::aboutQt() {
  return "<h1 align=\"center\">"
  + qTR("QMessageBox", "<h3>About Qt</h3><p>This program uses Qt version %1.</p>").arg(qVersion())
        .replace(QLatin1String("<p>"), QString()).replace(QLatin1String("</p>"), QString()) + "</h1><br>"
  + qTR("QMessageBox",
      "<p>Qt is a C++ toolkit for cross-platform application "
      "development.</p>"
      "<p>Qt provides single-source portability across all major desktop "
      "operating systems. It is also available for embedded Linux and other "
      "embedded and mobile operating systems.</p>"
      "<p>Qt is available under three different licensing options designed "
      "to accommodate the needs of our various users.</p>"
      "<p>Qt licensed under our commercial license agreement is appropriate "
      "for development of proprietary/commercial software where you do not "
      "want to share any source code with third parties or otherwise cannot "
      "comply with the terms of the GNU LGPL version 3 or GNU LGPL version 2.1.</p>"
      "<p>Qt licensed under the GNU LGPL version 3 is appropriate for the "
      "development of Qt&nbsp;applications provided you can comply with the terms "
      "and conditions of the GNU LGPL version 3.</p>"
      "<p>Qt licensed under the GNU LGPL version 2.1 is appropriate for the "
      "development of Qt&nbsp;applications provided you can comply with the terms "
      "and conditions of the GNU LGPL version 2.1.</p>"
      "<p>Please see <a href=\"http://%2/\">%2</a> "
      "for an overview of Qt licensing.</p>"
      "<p>Copyright (C) %1 The Qt Company Ltd and other "
      "contributors.</p>"
      "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>"
      "<p>Qt is The Qt Company Ltd product developed as an open source "
      "project. See <a href=\"http://%3/\">%3</a> for more information.</p>"
  ).arg(QStringLiteral("2017"),
        QStringLiteral("qt.io/licensing"),
        QStringLiteral("qt.io"));
}


QString TnootkaQML::pix(const QString& imageFileName) {
  return Tpath::pix(imageFileName);
}


QColor TnootkaQML::alpha(const QColor& c, int a) {
  return QColor(c.red(), c.green(), c.blue(), a);
}


QColor TnootkaQML::randomColor(int alpha, int level) {
  return QColor(qrand() % level, qrand() % level, qrand() % level, alpha);
}


int TnootkaQML::fontSize() {
#if defined (Q_OS_ANDROID)
  return (qApp->font().pixelSize() > 0 ? qApp->font().pixelSize() : qApp->font().pointSize()) * 0.8;
#else
  return Tmtr::fingerPixels() * 0.45;
#endif
}


//#################################################################################################
//###################     CONNECTIONS  NODE            ############################################
//#################################################################################################
void TnootkaQML::noteStarted(const Tnote& n) {
  Tnote transNote = n;
  if (transNote.isValid())
    transNote.transpose(-GLOB->transposition());
  if (m_scoreObject->keySignature() < 0 || (m_scoreObject->keySignature() == 0 && GLOB->GpreferFlats))
    transNote = transNote.showWithFlat();
  m_ignoreScore = true;
  if (m_scoreObject->singleNote()) {
      if (!transNote.isRest()) {
        transNote.setRhythm(Trhythm::NoRhythm);
        QMetaObject::invokeMethod(m_mainScore, "setNote", Q_ARG(QVariant, QVariant::fromValue(m_scoreObject->note(0))),
                                  Q_ARG(QVariant, QVariant::fromValue(transNote)));
      }
  } else
      QMetaObject::invokeMethod(m_mainScore, "addNote", Q_ARG(QVariant, QVariant::fromValue(transNote)));
}


void TnootkaQML::noteFinished(const Tnote& n) {
  m_instrument->setNote(n);
  Tnote transNote = n;
  if (transNote.isValid())
    transNote.transpose(-GLOB->transposition());
  if (m_scoreObject->keySignature() < 0 || (m_scoreObject->keySignature() == 0 && GLOB->GpreferFlats))
    transNote = transNote.showWithFlat();
  m_ignoreScore = true;
  if (m_scoreObject->singleNote())
    transNote.setRhythm(Trhythm::NoRhythm);
  if (m_scoreObject->selectedItem() && (!m_scoreObject->singleNote() || (m_scoreObject->singleNote() && !transNote.isRest())))
    QMetaObject::invokeMethod(m_mainScore, "setNote", Q_ARG(QVariant, QVariant::fromValue(m_scoreObject->selectedItem())),
                                                      Q_ARG(QVariant, QVariant::fromValue(transNote)));
//   else // naughty user pressed arrow key
  // TODO remember to treat tied notes as a single one when setNote will be implemented
}


void TnootkaQML::setMainScore(QQuickItem* ms) {
  if (!m_mainScore) {
    m_mainScore = ms;
    m_scoreObject = qobject_cast<TscoreObject*>(qvariant_cast<QObject*>(m_mainScore->property("scoreObj")));
    connect(m_scoreObject, &TscoreObject::selectedNoteChanged, this, &TnootkaQML::scoreChangedNote);
    if (m_scoreObject && !m_nodeConnected)
      connectInstrument();
  }
}


void TnootkaQML::setInstrument(TcommonInstrument* ci) {
  if (m_instrument != ci) {
    if (m_instrument != nullptr)
      m_nodeConnected = false; // reset connection of instrument signal when instrument type changed
      m_instrument = ci;
    if (m_scoreObject && !m_nodeConnected)
      connectInstrument();
  }
}


void TnootkaQML::connectInstrument() {
  m_nodeConnected = true;
  connect(m_instrument, &TcommonInstrument::noteChanged, [=] {
    Tnote rawNote = m_instrument->note();
    qDebug() << "instrument send note" << rawNote.toText();
    m_ignoreScore = true;
    emit playNote(m_instrument->note()); // not yet transposed - to sound properly
    rawNote.transpose(-GLOB->transposition());
    if (m_scoreObject->keySignature() < 0 || (m_scoreObject->keySignature() == 0 && GLOB->GpreferFlats))
      rawNote = rawNote.showWithFlat();

    if (m_scoreObject->singleNote()) {
        QMetaObject::invokeMethod(m_mainScore, "setNote", Q_ARG(QVariant, QVariant::fromValue(m_scoreObject->note(0))),
                                  Q_ARG(QVariant, QVariant::fromValue(rawNote)));
    } else {
        if (m_scoreObject->selectedItem()) {
            rawNote.setRhythm(m_scoreObject->selectedItem()->note()->rtm);
            QMetaObject::invokeMethod(m_mainScore, "setNote", Q_ARG(QVariant, QVariant::fromValue(m_scoreObject->selectedItem())),
                                                              Q_ARG(QVariant, QVariant::fromValue(rawNote)));
        } else {
            rawNote.setRhythm(m_scoreObject->workRhythm());
            QMetaObject::invokeMethod(m_mainScore, "addNote", Q_ARG(QVariant, QVariant::fromValue(rawNote)));
        }
    }
  });
}


void TnootkaQML::scoreChangedNote() {
  if (m_ignoreScore) {
    m_ignoreScore = false;
    return;
  }
  auto n = m_scoreObject->selectedNote();
  if (n.isValid())
    n.transpose(GLOB->transposition());
  m_instrument->setNote(n);
  emit playNote(n);
  qDebug() << "Got note from score" << n.toText() << n.chromatic();
}


QString TnootkaQML::TR(const QString& context, const QString& text, const QString& disambiguation, int n) {
  return qTR(qPrintable(context), qPrintable(text), qPrintable(disambiguation), n);
}


int TnootkaQML::selectedNoteId() const {
  return m_scoreObject->selectedItem() ? m_scoreObject->selectedItem()->index() : -1;
}


void TnootkaQML::selectPlayingNote(int id) {
  m_ignoreScore = true;
  m_scoreObject->setSelectedItem(m_scoreObject->note(id));
  auto n = m_scoreObject->selectedNote();
  if (n.isValid())
    n.transpose(GLOB->transposition());
  m_instrument->setNote(n);
}


int TnootkaQML::scoreNotesCount() const {
  return m_scoreObject->notesCount();
}


QList<Tnote>& TnootkaQML::scoreNoteList() const {
  return m_scoreObject->noteList();
}

