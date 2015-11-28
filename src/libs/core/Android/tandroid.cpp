/***************************************************************************
 *   Copyright (C) 2015 by Tomasz Bojczuk                                  *
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


#include "tandroid.h"
#include <QtAndroidExtras/qandroidfunctions.h>
#include <QtAndroidExtras/qandroidjnienvironment.h>


void Tandroid::setScreenLockDisabled() {
  QAndroidJniObject activity = QtAndroid::androidActivity();
  if (activity.isValid()) {
    QAndroidJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
    if (window.isValid()) {
      const int FLAG_KEEP_SCREEN_ON = 128;
      const int FLAG_FULLSCREEN = 1024;
//      const int FLAG_FORCE_NOT_FULLSCREEN = 2048;
      window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON | FLAG_FULLSCREEN);
      QAndroidJniEnvironment env;
      if (env->ExceptionCheck())
        env->ExceptionClear();
    }
  }
}


QString Tandroid::getExternalPath() {
  QAndroidJniObject extDirObject =
      QAndroidJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
  QAndroidJniObject externalPath = extDirObject.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
  QAndroidJniEnvironment env;
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
  }
  return externalPath.toString();
}


QString Tandroid::accountName() {
  return "fake";
//  return QAndroidJniObject::callStaticObjectMethod<jstring>
//              ("net/sf/nootka/account",
//               "getName").toString();
}
