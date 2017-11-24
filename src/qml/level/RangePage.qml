/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017 by Tomasz Bojczuk (seelook@gmail.com)          *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9
import QtQuick.Controls 2.2

import Score 1.0
import "../"


Flickable {
  width: parent.width; height: parent.height
  clip: true
  contentHeight: rangeGrid.height + Noo.fontSize() * 2
  contentWidth: Math.max(width, Noo.fontSize() * 35)

  ScrollBar.vertical: ScrollBar { active: !Noo.isAndroid() }

  Connections {
    target: creator
    onUpdateLevel: {
      rangeScore.setNote(rangeScore.scoreObj.note(0), creator.loNote)
      rangeScore.setNote(rangeScore.scoreObj.note(1), creator.hiNote)
    }
  }

  Grid {
    id: rangeGrid
    columns:  parent.width > Noo.fontSize() * 50 ? 2 : 1
    width: parent.width
    spacing: Noo.fontSize() / 4
    horizontalItemAlignment: Grid.AlignHCenter
    topPadding: Noo.fontSize()

    Frame {
        width: rangeGrid.columns === 1 ? parent.width * 0.98 : parent.width * 0.49
        Column {
          width: parent.width
          spacing: Noo.fontSize() / 2
          Text { text: qsTr("note range:"); color: activPal.text; anchors.horizontalCenter: parent.horizontalCenter }
          Score {
            id: rangeScore
            height: Noo.fontSize() * 20
            width: Noo.fontSize() * 12
            anchors.horizontalCenter: parent.horizontalCenter
            meter: Tmeter.NoMeter
            clef: creator.clef
            scoreObj.clefType: creator.clef
            Component.onCompleted: {
              rangeScore.addNote(creator.loNote)
              rangeScore.addNote(creator.hiNote)
            }
          }
          Tile {
            TcuteButton {
              text: qsTr("adjust fret range")
              anchors.horizontalCenter: parent.horizontalCenter
            }
            description: qsTr("Adjust fret range in a level to currently selected note range")
          }
        }
    }

    Column {
      width: rangeGrid.columns === 1 ? parent.width * 0.98 : parent.width * 0.49
      spacing: Noo.fontSize() / 2
      Frame {
          width: parent.width
          Column {
            width: parent.width
            spacing: Noo.fontSize() / 2
            Text { text: qsTr("fret range:"); color: activPal.text; anchors.horizontalCenter: parent.horizontalCenter }
            Row {
              anchors.horizontalCenter: parent.horizontalCenter
              spacing: Noo.fontSize() / 2
              Text { text: qsTr("from"); color: activPal.text; anchors.verticalCenter: parent.verticalCenter }
              SpinBox {
                id: fromFretSpin
                from: 0; to: GLOB.fretNumber
                value: creator.loFret
              }
              Text { text: qsTr("to"); color: activPal.text; anchors.verticalCenter: parent.verticalCenter }
              SpinBox {
                id: toFretSpin
                from: 0; to: GLOB.fretNumber
                value: creator.hiFret
              }
            }
            Tile {
              TcuteButton {
                text: qsTr("adjust note range")
                anchors.horizontalCenter: parent.horizontalCenter
              }
              description: qsTr("Adjust note range in a level to currently selected fret range")
            }
          }
      }
      Item { width: parent.width; height: rangeGrid.columns === 1 ? 0 : Noo.fontSize() * 3 } // spacer only for 2 columns
      Tile {
        description: qsTr("Uncheck strings if you want to skip them in an exam.")
        Column {
          width: parent.width
          spacing: Noo.fontSize() / 4
          Text { text: qsTr("available strings:"); color: activPal.text; anchors.horizontalCenter: parent.horizontalCenter }
          Grid {
            spacing: Noo.fontSize() / 4
            anchors.horizontalCenter: parent.horizontalCenter
            columns: Math.ceil(GLOB.tuning.stringNumber / 2.0)
            horizontalItemAlignment: Grid.AlignHCenter
            Repeater {
              model: GLOB.tuning.stringNumber
              TcheckBox {
                checked: creator.usedStrings & Math.pow(2, index)
                text: index + 1
                font { family: "nootka"; pixelSize: Noo.fontSize() * 2 }
                onClicked: creator.usedStrings = checked ? creator.usedStrings | Math.pow(2, index) : creator.usedStrings & ~Math.pow(2, index)
              }
            }
          }
        }
      }
    }
  }
}
