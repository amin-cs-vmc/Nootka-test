#ifndef TNOTENAME_H
#define TNOTENAME_H

#include <QWidget>
#include "tnote.h"

class QLabel;
class QPushButton;
class QButtonGroup;

class TnoteName : public QWidget
{
    Q_OBJECT
public:
    explicit TnoteName(QWidget *parent = 0);

    static const QString octaves[6];
    static const QString octavesFull[6];

    void setNoteNamesOnButt(Tnote::Enotation nameStyle);
    void setEnabledDblAccid(bool isEnabled);
    void setEnabledEnharmNotes(bool isEnabled);
    void setNoteName(Tnote note);
    void setNoteName(TnotesList notes);
    Tnote getNoteName() {return m_notes[0]; }
    Tnote getNoteName(int index) {return m_notes[index]; }
    void resize();
//    void setButtonsWithOctaves

signals:
    void noteNameWasChanged(Tnote note);

public slots:

private:
    QLabel *nameLabel;
    QPushButton *noteButtons[7];
    QPushButton *octaveButtons[6];
    QPushButton *dblFlatButt, *flatButt, *sharpButt, *dblSharpButt;
    QButtonGroup *noteGroup, *octaveGroup;

    TnotesList m_notes;
    void paintEvent(QPaintEvent *);
    void setNoteName(char noteNr, char octNr, char accNr);
    void setNameText();
    QString noteToRichText(Tnote note);
    void setButtons(Tnote note);

private slots:
    void noteWasChanged(int noteNr);
    void accidWasChanged();
    void octaveWasChanged(int octNr);


};

#endif // TNOTENAME_H
