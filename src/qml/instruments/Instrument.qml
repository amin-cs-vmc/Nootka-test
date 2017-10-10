/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017 by Tomasz Bojczuk (seelook@gmail.com)          *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9

import Nootka 1.0
// import "instruments"


Item {
  id: root
  property alias instrument: instrLoad.item
  property Score score

  signal note(var n)

  height: GLOB.instrument.isSax ? parent.height : parent.height / GLOB.instrument.heightPart
  width: parent.width * (GLOB.instrument.isSax ? 0.15 : 1)
  y: GLOB.instrument.isSax ? 0 : score.height + 1
  x: GLOB.instrument.isSax ? parent.width - width : 0
  z: 1

  Loader {
    id: instrLoad
    anchors.fill: parent
    source: "qrc:/instruments/" +  GLOB.instrument.qmlFile + ".qml"
    onLoaded: {
      Noo.instrument = instrument
      if (GLOB.instrument.type === Tinstrument.Piano)
        instrument.firstOctave = Noo.octave(score.scoreObj.lowestNote())
    }
  }

  Connections {
    target: instrLoad.item
    onNoteChanged: root.note(instrument.note)
  }
  Connections {
    target: score
    enabled: GLOB.instrument.type === Tinstrument.Piano
    onClefChanged: instrument.firstOctave = Noo.octave(score.scoreObj.lowestNote())
  }
}
