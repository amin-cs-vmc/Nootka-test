/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017 by Tomasz Bojczuk (seelook@gmail.com)          *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import Score 1.0


Column {

    spacing: nootkaWindow.fontSize
    width: parent.width


    ListView {
      id: headList
      orientation: ListView.Horizontal
      spacing: nootkaWindow.fontSize
      width: parent.width

      model: ListModel {
        ListElement { head: QT_TR_NOOP("Score settings") }
        ListElement { head: QT_TR_NOOP("Key signatures") }
        ListElement { head: QT_TR_NOOP("Clefs") }
        ListElement { head: QT_TR_NOOP("Notes naming") }
      }

      delegate: Component {
        Button {
          text: (index + 1) + ". " + qsTranslate("TscoreSettings", head)
          onClicked: { swipePages.currentIndex = index; headList.currentIndex = index }
          highlighted: headList.currentIndex == index
          Component.onCompleted: headList.height = Math.max(height, headList.height)
        }
      }
    }
    StackLayout {
      id: swipePages
      height: parent.height - headList.height - nootkaWindow.fontSize
      width: parent.width

      Flickable { // 1st page (general)
        clip: true
        contentHeight: firstColumn.height + nootkaWindow.fontSize * 2
        width: parent.width
        height: parent.height
        Column {
          id: firstColumn
          width: parent.width
          spacing: nootkaWindow.fontSize / 2
          Frame {
            width: parent.width
            Column {
              spacing: nootkaWindow.fontSize / 2
              width: parent.width
              Tile {
                description: qsTranslate("TscoreSettings", "When enabled, a score displays only a single note.")
                CheckBox {
                  id: singleNoteModeChB
                  text: qsTranslate("TscoreSettings", "use single note only")
                  anchors.horizontalCenter: parent.horizontalCenter
                }
              }

              Tile {
                enabled: singleNoteModeChB.checked
                description: qsTranslate("TscoreSettings",
                                        "Shows enharmonic variants of notes.<br>i.e.: the note E is also Fb (F flat) <i>and</i> Dx (D with double sharp).")
                CheckBox {
                  id: showEnharmNotesChB
                  text: qsTranslate("TscoreSettings", "show enharmonic variants of notes")
                  anchors.horizontalCenter: parent.horizontalCenter
                }
              }
              Tile {
                enabled: singleNoteModeChB.checked
                Row {
                  spacing: nootkaWindow.fontSize
                  anchors.horizontalCenter: parent.horizontalCenter
                  Text {
                    color: enabled ? activPal.text : disdPal.text
                    text: qsTranslate("TscoreSettings", "color of enharmonic notes")
                    anchors.verticalCenter: parent.verticalCenter
                  }
                  ColorButton { id: enharmNoteColor }
                }
              }
            }
          }
          Tile {
            description: qsTr("Use rhythms or only bare note heads.")
            enabled: !singleNoteModeChB.checked
            CheckBox {
              id: enableRhythChB
              text: qsTr("enable rhythms")
              anchors.horizontalCenter: parent.horizontalCenter
            }
          }
          Tile {
            description: qsTranslate("TscoreSettings", "If checked, you can use double sharps and double flats.")
            CheckBox {
              id: doubleAccidsChB
              text: qsTranslate("TscoreSettings", "use double accidentals")
              anchors.horizontalCenter: parent.horizontalCenter
              checked: GLOB.enableDoubleAccids
            }
          }
          Tile {
            Row {
              spacing: nootkaWindow.fontSize
              anchors.horizontalCenter: parent.horizontalCenter
              Text { color: activPal.text; text: qsTranslate("TscoreSettings", "note-cursor color"); anchors.verticalCenter: parent.verticalCenter }
              ColorButton { id: pointerColorButt; color: GLOB.noteCursorColor }
            }
          }
        }
        Component.onCompleted: {
          enharmNoteColor.color = GLOB.enharmNoteColor
          showEnharmNotesChB.checked = GLOB.showEnharmNotes
          singleNoteModeChB.checked = GLOB.singleNoteMode
        }
      }

      Flickable { // 2nd page (key signatures)
        clip: true
        contentHeight: secondColumn.height + nootkaWindow.fontSize * 2
        width: parent.width
        height: parent.height
        Column {
          id: secondColumn
          width: parent.width
          spacing: nootkaWindow.fontSize / 2
          Tile {
            CheckBox {
              id: enableKeyChB
              text: qsTranslate("TscoreSettings", "enable key signature")
              anchors.horizontalCenter: parent.horizontalCenter
              checked: GLOB.keySignatureEnabled
            }
          }
          Frame {
            width: parent.width * 0.96
            anchors.horizontalCenter: parent.horizontalCenter
            Column {
              spacing: nootkaWindow.fontSize / 2
              width: parent.width
              Tile {
                enabled: enableKeyChB.checked
                CheckBox {
                  id: showKeyNamesChB
                  text: qsTranslate("TscoreSettings", "show names of key signature")
                  anchors.horizontalCenter: parent.horizontalCenter
                  checked: GLOB.showKeyName
                }
              }

              Tile {
                enabled: enableKeyChB.checked && showKeyNamesChB.checked
                Grid {
                  columns: parent.width < nootkaWindow.fontSize * 50 ? 1 : 2
                  spacing: nootkaWindow.fontSize
                  anchors.horizontalCenter: parent.horizontalCenter
                  NameStyleSelector {
                    id: keyNameStyleSel
                    seventhIsB: GLOB.seventhIsB
                  }
                  Column {
                    Text {
                      text: qsTranslate("TscoreSettings", "Naming extension")
                      anchors.horizontalCenter: parent.horizontalCenter
                      color: enabled ? activPal.text : disdPal.text
                    }
                    Row {
                      spacing: nootkaWindow.fontSize * 2
                      anchors.horizontalCenter: parent.horizontalCenter
                      KeySufixEdit { id: majKeySufixText; nameStyle: keyNameStyleSel.style }
                      KeySufixEdit { id: minKeySufixText; nameStyle: keyNameStyleSel.style; noteOne: 1; alterOne: 1; noteTwo: 5; alterTwo: 0 }
                    }
                  }
                }
                Component.onCompleted: {
                  majKeySufixText.sufix = GLOB.majorKeyNameSufix
                  minKeySufixText.sufix = GLOB.minorKeyNameSufix
                  keyNameStyleSel.style = GLOB.keyNameStyle
                }
              }
            }
          }
        }
      }

      Flickable { // 3rd page (clefs)
        clip: true
        contentHeight: clefsCol.height + nootkaWindow.fontSize * 2
        contentWidth: parent.width
        width: parent.width
        Column {
          id: clefsCol
          width: parent.width
          spacing: nootkaWindow.fontSize / 2
          Text {
            text: qsTranslate("TscoreSettings", "Select default clef for the application.") + "<br><b>" + qsTranslate("TscoreSettings", "Remember! Not all clefs are suitable for some possible tunings or instrument types!") + "<b>"
            textFormat: Text.StyledText
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
            wrapMode: Text.WordWrap
          }
          ClefMenu {
            id: clefs
            columns: width < nootkaWindow.fontSize * 40 ? 1 : 2
            Component.onCompleted: selClef = GLOB.clefType
          }
        }
      }
      Flickable { // 3rd page (clefs)
        clip: true
        contentWidth: parent.width
        width: parent.width
        NameStyleSelector {
          
        }
      }
    }

    function save() {
//       GLOB. = singleNoteModeChB
      GLOB.enableDoubleAccids = doubleAccidsChB.checked
      GLOB.noteCursorColor = pointerColorButt.color
      GLOB.keySignatureEnabled = enableKeyChB.checked
      if (GLOB.keySignatureEnabled) {
        GLOB.showKeyName = showKeyNamesChB.checked
        if (GLOB.showKeyName) {
          GLOB.majorKeyNameSufix = majKeySufixText.sufix
          GLOB.minorKeyNameSufix = minKeySufixText.sufix
          GLOB.keyNameStyle = keyNameStyleSel.style
          GLOB.updateKeySignatureNames()
        }
      }
    }

    function defaults() {
      singleNoteModeChB.checked = false
      showEnharmNotesChB.checked = false
      enharmNoteColor.color = Qt.rgba(0, 0.6352941176470588, 0.6352941176470588, 1)
      doubleAccidsChB.checked = false
      pointerColorButt.color = "pink"

      enableKeyChB.checked = false
    }
}
