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

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0


Item {
  id: nootkaLabel
  property color bgColor: "white"
  property alias active: mouseArea.hoverEnabled

  signal clicked()

  height: parent.height
  width: logo.width

  Rectangle {
    id: bgRect
    height: parent.height
    width: logo.width
    color: bgColor
  }

  Image {
    id: logo
    smooth: true
    anchors.top: parent.Top
    source: Tpath.pix("logo")
    sourceSize.height: nootkaLabel.height
  }

  Text {
    y: nootkaLabel.height * 0.72
    anchors.horizontalCenter: logo.horizontalCenter
    font.pixelSize: nootkaLabel.height * 0.2
    color: bgColor
    text: Noo.version()
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    onEntered: bgColor = randColor()
    onExited: bgColor = "white"
    onClicked: nootkaLabel.clicked()
  }

  function randColorValue() { return (80 + Math.random() * 150) / 255 }
  function randColor() { return Qt.rgba(randColorValue(), randColorValue(), randColorValue(), 1) }
}