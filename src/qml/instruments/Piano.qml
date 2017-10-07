/** This file is part of Nootka (http://nootka.sf.net)               *
 * Copyright (C) 2017 by Tomasz Bojczuk (seelook@gmail.com)          *
 * on the terms of GNU GPLv3 license (http://www.gnu.org/licenses)   */

import QtQuick 2.9

import Nootka 1.0


TpianoBg {

  anchors.fill: parent

  keyWidth: Noo.fontSize() * 2
  onSelectedRectChanged: {
    selectedKey.x = keyRect.x
    selectedKey.y = keyWidth + keyRect.height - height * 0.3
    selectedKey.width = keyRect.width
  }

  Rectangle {
    id: selectedKey
    color: GLOB.selectedColor
    height: parent.height * 0.3
    radius: width * 0.2
    visible: x > 0
  }

  Rectangle {
    color: GLOB.fingerColor
    width: keyRect.width
    height: keyRect.height
    radius: width * 0.2
    x: keyRect.x
    y: keyRect.y
    visible: active
  }
}
