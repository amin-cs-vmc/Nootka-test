/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017-2018 by Tomasz Bojczuk (seelook@gmail.com)     *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9
import QtQuick.Controls 2.2

import Score 1.0
import Nootka 1.0
import Nootka.Main 1.0
import "score"


Score {
  id: mainScore

  width: parent.width

  property alias showNamesAct: mainObj.showNamesAct
  property alias extraAccidsAct: mainObj.extraAccidsAct
  property alias zoomInAct: mainObj.zoomInAct
  property alias zoomOutAct: mainObj.zoomOutAct
  property alias openXmlAct: mainObj.openXmlAct
  property alias saveXmlAct: mainObj.saveXmlAct
  property alias deleteLastAct: mainObj.deleteLastAct
  property alias clearScoreAct: mainObj.clearScoreAct
  property alias recModeAct: mainObj.recModeAct
  property alias playAct: mainObj.playAct
  property alias scoreActions: mainObj.scoreActions
  property alias melodyActions: mainObj.melodyActions
  property alias keyName: keyName

  scoreObj.meter: GLOB.rhythmsEnabled && !GLOB.singleNoteMode ? Tmeter.Meter_4_4 : Tmeter.NoMeter
  focus: true

  clef: GLOB.clefType
  enableDoubleAccids: GLOB.enableDoubleAccids
  scoreObj.showNoteNames: GLOB.namesOnScore
  scoreObj.nameColor: GLOB.nameColor
  scoreObj.nameStyle: GLOB.noteNameStyle
  scoreObj.enableDoubleAccidentals: GLOB.enableDoubleAccids
  scoreObj.enharmNotesEnabled: GLOB.showEnharmNotes
  scoreObj.bgColor: activPal.base
  scoreObj.enableTechnical: GLOB.instrument.type === Tinstrument.Bandoneon

  TmainScoreObject {
    id: mainObj
    openXmlAct.shortcut: Shortcut { sequence: StandardKey.Open; enabled: !GLOB.singleNoteMode && !GLOB.isExam }
    saveXmlAct.shortcut: Shortcut { sequence: StandardKey.Save; enabled: !GLOB.singleNoteMode && !GLOB.isExam }
    zoomOutAct.shortcut: Shortcut { sequence: StandardKey.ZoomOut; enabled: !GLOB.singleNoteMode }
    zoomInAct.shortcut: Shortcut { sequence: StandardKey.ZoomIn; enabled: !GLOB.singleNoteMode }
    playAct.shortcut: Shortcut { sequence: " "; enabled: !GLOB.singleNoteMode && !GLOB.isExam }
    recModeAct.shortcut: Shortcut { sequence: "Ctrl+ "; enabled: !GLOB.singleNoteMode && !GLOB.isExam }
    recModeAct.text: recordMode ? qsTr("Note by note") : qsTr("Edit")
    recModeAct.icon: recordMode ? "record" : "stopMelody"
    randMelodyAct.shortcut: Shortcut { sequence: "Ctrl+M"; enabled: !GLOB.singleNoteMode && !GLOB.isExam }
    onMelodyGenerate: nootkaWindow.showDialog(Nootka.MelodyGenerator)
  }

  Timer { id: zoomTimer; interval: 500 }
  MouseArea {
    anchors.fill: parent
    z: -1
    onWheel: {
      if (wheel.modifiers & Qt.ControlModifier) {
          if (wheel.angleDelta.y > 0) {
              if (!zoomTimer.running) {
                zoomInAct.trigger()
                zoomTimer.running = true
              }
          } else if (wheel.angleDelta.y < 0) {
              if (!zoomTimer.running) {
                zoomOutAct.trigger()
                zoomTimer.running = true
              }
          }
      } else
          wheel.accepted = false
    }
  }

  Text {
    id: keyName
    parent: firstStaff
    visible: GLOB.showKeyName && enableKeySign
    x: clef === Tclef.PianoStaffClefs ? 7.5 : 5.5
    y: clef === Tclef.PianoStaffClefs ? 3.7 : 6.2
    color: activPal.text
    font { family: "Sans"; pixelSize: 2 }
    text: GLOB.showKeyName && enableKeySign ? mainObj.keyNameText : ""
  }

  // private
  property var scordature: null
  property var notePrompt: null

  Connections {
    target: SOUND
    onInitialized: {
      mainObj.scoreObject = scoreObj
      singleNote = Qt.binding(function() { return GLOB.singleNoteMode })
      scoreObj.allowAdding = Qt.binding(function() { return !GLOB.singleNoteMode })
      enableKeySign = Qt.binding(function() { return GLOB.keySignatureEnabled })
      updateScord()
      var t = Qt.createComponent("qrc:/StatusTip.qml")
      t.createObject(nootkaWindow)
    }
  }
  Connections {
    target: GLOB
    onClefTypeChanged: score.clef = GLOB.clefType
  }
  Connections {
    target: GLOB.tuning
    onScordatureChanged: updateScord()
  }
  function updateScord() {
    if (scordature) {
      scordature.destroy()
      scordature = null
    }
    if (GLOB.tuning.scordature && GLOB.instrument.isGuitar && GLOB.instrument.type !== Tinstrument.BassGuitar) {
        var c = Qt.createComponent("qrc:/score/Scordature.qml")
        scordature = c.createObject(firstStaff)
        firstStaff.scordSpace = scordature.height
    } else
        firstStaff.scordSpace = 0
  }

  onSingleNoteChanged: {
    if (singleNote) {
        var c = Qt.createComponent("qrc:/score/NotePrompt.qml")
        notePrompt = c.createObject(scoreObj.note(0))
    } else
        notePrompt.destroy()
  }

  Rectangle { // note highlight
    id: noteHighlight
    parent: currentNote
    visible: currentNote != null && !singleNote
    width: currentNote ? (currentNote.width - currentNote.alterWidth) * 1.5 : 0
    height: currentNote ? Math.min(12.0, currentNote.notePosY + 6.0) : 0
    x: currentNote ? -width * 0.25 : 0
    y: currentNote ? Math.min(currentNote.height - height, Math.max(0.0, currentNote.notePosY - height / 2.0)) : 0
    color: Noo.alpha(activPal.highlight, 75)
    z: -1
    radius: width / 3.0
  }

  Shortcut {
    enabled: !GLOB.singleNoteMode
    sequence: StandardKey.MoveToNextChar
    onActivated: {
      if (currentNote)
        currentNote = scoreObj.getNext(currentNote)
      else
        currentNote = scoreObj.note(0)
    }
  }
  Shortcut {
    enabled: !GLOB.singleNoteMode
    sequence: StandardKey.MoveToPreviousChar
    onActivated: {
      if (currentNote)
        currentNote = scoreObj.getPrev(currentNote)
      else
        currentNote = scoreObj.note(notesCount - 1)
    }
  }

}
