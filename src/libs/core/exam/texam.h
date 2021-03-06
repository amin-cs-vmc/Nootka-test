/***************************************************************************
 *   Copyright (C) 2011-2015 by Tomasz Bojczuk                             *
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

#ifndef TEXAM_H
#define TEXAM_H


#include <nootkacoreglobal.h>
#include <QString>
#include <QList>
#include <QXmlStreamWriter>
#include <QPointer>
#include <QApplication>
#include "tqaunit.h"
#include <music/ttune.h>

class Tlevel;


/**
* This class describes instance of an exam.
*/
class NOOTKACORE_EXPORT Texam
{

public:
      /** An exam constructor.
       * \a Tlevel has to be pointer to existing exam level
       */
    explicit Texam(Tlevel *l, QString userName);
    virtual ~Texam();

      /** Magic numbers in exam file to identify it.*/
    static const qint32 examVersion;
    static const qint32 examVersion2;
    static const qint32 currentVersion;

    static bool isExamVersion(qint32 ver); /** Returns true when given value match to exam versions. */

        /** Returns number of exam version like 1, 2 or so or -1 when given version is not valid. */
    static int examVersionNr(qint32 ver);

        /** Returns true when given value 'could be' or 'it is' some version of exam file.
         * This way exams created with newer Nootka version can be detected. */
    static bool couldBeExam(qint32 ver);

        /** Generates exam version identifier number from given simple version number (1, 2 etc.)
         * It doesn't verify does given number make sense! */
    static qint32 getVersionId(quint8 verNr) { return examVersion + (verNr - 1) * 2; }

        /** Returns level version number used in given exam version
         * It doesn't check is given exam version valid!. */
    static qint32 examVersionToLevel(qint32 examVer);

        /** returns nice formatted time (1:05:15.3). Time is in seconds multiplied by 10.
        * When @p withUnit is true adds s (seconds) unit*/
    static QString formatReactTime(quint16 timeX10, bool withUnit = false);

        /** Maximal time of an answer = 65500. Values over are equal to it.
      * 65501 & 65502 are for counting probes in blackList */
  static const quint16 maxAnswerTime;
  static bool areQuestTheSame(TQAunit* q1, TQAunit* q2);     /** Compares given questions are they the same. */

    enum EerrorType { e_file_OK = 0,
                    e_file_not_valid, // occurs when examVersion is different
                    e_file_corrupted, // when data in file is corrupted
                    e_cant_open, // problems with reading file
                    e_noFileName,
                    e_newerVersion // when file version is newer than this Nootka version can support
          }; /** \enum EerrorType are possible errors during opening and saving exam file.*/

  Tlevel* level() { return m_level; }
  void setLevel(Tlevel *l);
  Ttune tune() { return m_tune; }
  void setTune(Ttune &tune) { m_tune = tune; }

    /** @p TRUE when level for melodies, otherwise it is single note.
     * It is determined through @p setLevel() */
  bool melodies() { return m_melody; }
  quint32 attempts() { return m_attempts; } /** Total number of attempts when melodies. Updated in @p sumarizeAnswer() */
  bool isExercise() { return m_isExercise; }

      /** Makes exam an exercise (without penalties). Also sets default file name for exercises.
       * It possible to set an exercise only for empty exam (without questions in the list yet). */
  void setExercise();

  quint32 totalTime() { return m_totalTime; }
  void setTotalTime(quint32 total) { m_totalTime = total; }

      /** Adds \a TQAunit object at the end of the questions list. */
  void addQuestion(TQAunit &question) { m_answList << new TQAunit(question); }

      /** Checks the last TQAunit on the list, updates its effectiveness
       * actualizes the exam effectiveness.
       * Remember to add penalties to black list before invoke this !!! */
  void sumarizeAnswer();

      /** Check was answer correct and adds penalty(s) if necessary.
       * For 'single note' answer it is invoked with @p sumarizeAnswer()
       * but for melodies user has to decide about new attempt or new question first,
       * so it has to be invoked only when new question is prepared.
       * BE SURE IT IS CALLED ONCE PER WHOLE QUESTION! */
  void addPenalties();

  TQAunit* curQ() { return m_answList.last(); } /** Last q/a unit */
  QList<TQAunit*>* answList() { return &m_answList; }

  int count() { return m_answList.size(); } /** Returns number of questions/answers in en exam. */
  quint16 mistakes() { return m_mistNr; } /** Returns number of committed mistakes in en exam. */
  quint16 halfMistaken() { return m_halfMistNr; } /** Number of 'not bad' answer in the exam. */
  quint16 corrects() { return (count()) - mistakes() - halfMistaken(); } /** Number of correct answers */
  quint16 averageReactonTime() { return m_averReactTime; } /** Average answer time */
  void setAverageReactonTime(quint16 avTime) { m_averReactTime = avTime; }
  void skipLast(bool skip); /** Hides last question in the list. It is allowed only if question is not answered. */

      /** Iterates all questions to calculate average reaction time.
       * When @p skipWrong - time of wrong answers is not calculated into. */
  void updateAverageReactTime(bool skipWrong = false);

      /** Total time spent for answering without breaks between questions */
  quint16 workTime() { return qRound((qreal)m_workTime / 10.0); }
  QString userName() { return m_userName; }
  QString fileName() { return m_fileName; }
  void setFileName(const QString& fileName);

  QList<TQAunit>* blacList() { return &m_blackList; } /** Pointer to list with penalty questions. */

      /** Pointer to list with numbers of question in which mistakes were committed. */
  QList<int>* blackNumbers() { return &m_blackNumbers; }
  int penalty() { return m_penaltysNr; } /** Number of penalties during whole exam */
  int blackCount() { return m_blackCount; } /** Remained questions in black list */
  bool isFinished() { return m_isFinished; }

      /** Sets exam as finished and there is no way back. */
  void setFinished() { m_isFinished = true; }
  void increasePenaltys(int penaltyNr) { m_penaltysNr += penaltyNr; }

  EerrorType loadFromFile(QString &fileName);
  EerrorType saveToFile(QString fileName = "");

  void writeToXml(QXmlStreamWriter& xml);
  bool loadFromXml(QXmlStreamReader& xml);
  bool loadFromBin(QDataStream& in, quint32 ev);

      /** For single note it is 100% - correct, 50% - 'not bad' and 0% - wrong
       * For melody (attempt) it is calculated as average of all notes in the attempt
       * and average of all attempts in single TQAunit.
       * There are Tattempt::updateEffectiveness() and TQAunit::updateEffectiveness()
       * to refresh their effectiveness but it is done automatically when melodies are compared.
       * Current exam effectiveness is updated after sumarizeAnswer()
       * but @p updateEffectiveness() can do it again. */
  qreal effectiveness() { return m_effectivenes; }
  void updateEffectiveness(); /** Iterates all answers to obtain average effectiveness. */

      /** Returns a reference to question/answer unit nr @param index.
      * Beware! Index value is not checked */
  TQAunit* question(unsigned int index) { return m_answList[index]; }

  void newAttempt();

protected:
  void updateBlackCount(); /** Iterates through m_blackList to calculate number */
  void convertToVersion2(); /** Grabs answers with mistakes and creates black list */

    /** Reads TQAunit from given XML and appends it at the end of given list or prints error and return @p FALSE*/
  bool readAnswerFromXml(QList<TQAunit*>& list, QXmlStreamReader& xml);
  bool readPenaltyFromXml(QList<TQAunit>& blackList, QXmlStreamReader& xml);

  void grabFromLastUnit(); /** increases some counter variables to obtain checking values */

    /** Compares number of units with number got from file and prints message if doesn't match. */
  bool checkQuestionNumber(int questNr);
  void clearAnswList(); /** Wipes @p m_answList and all units pointed by it. */

private:
  QString                   m_fileName, m_userName;
  Tlevel                    *m_level;
  QList<TQAunit*>           m_answList;
  QList<TQAunit>             m_blackList;
  QList<int>                m_blackNumbers; /** List of question numbers in which mistakes were committed. */
  Ttune                     m_tune;
  quint32                   m_totalTime, m_attempts;
  quint16                   m_mistNr, m_tmpMist, m_averReactTime, m_workTime, m_halfMistNr, m_tmpHalf;
  bool                       m_isFinished, m_melody, m_isExercise;
  int                       m_penaltysNr;
  int                       m_blackCount;
  int                       m_okTime; // time of correct and notBad answers to calculate average
  qreal                      m_effectivenes;
  TQAunit                  *m_skippedUnit;
};

#endif // TEXAM_H
