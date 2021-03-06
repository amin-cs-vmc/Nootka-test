/***************************************************************************
 *   Copyright (C) 2013-2015 by Tomasz Bojczuk                             *
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


#ifndef TRTAUDIOABSTRACT_H
#define TRTAUDIOABSTRACT_H


#include "nootkasoundglobal.h"
#include "rt/RtAudio.h"
#include "taudioobject.h"
#include <QString>

class TaudioParams;


/** 
 * Common class for RtAudio input/outputs classes.
 * To get it working, a subclass has to set static call back method of type @p callBackType
 * where audio data will be send.
 * To start, @p updateAudioParams() has to be invoked to initialize/refresh outside audio parameters
 * then @p open() and @p startAudio() do their tasks
 * It has protected @member ao() which can emits signals
 * It can switch from/to JACK or ASIO with @p setJACKorASIO(bool) method
 * but use @p initJACKorASIO(bool) before first creation of @class TrtAudio to set preferred API
 * It works with split callbacks by default
 * - stream is opened for input and when output is demanded
 * - input is closed and output opened.
 * When @p updateAudioParams() will detect @p TaudioParams::forwardInput
 * or ASIO API then duplex stream works.
 * @p TaudioParams::forwardInput will work properly only when input and output
 * are on the same audio device or ASIO or JACK is set.
 * In other case @p TaudioParams::forwardInput doesn't work with different devices
 * due to syncing issues of RtAudio.
 */
class NOOTKASOUND_EXPORT TrtAudio
{

public:
	enum EdevType {	e_input, e_output	};
	typedef bool (*callBackType)(void*, unsigned int, const RtAudioStreamStatus&);

	TrtAudio(TaudioParams *audioP, TrtAudio::EdevType type, TrtAudio::callBackType cb);
	~TrtAudio();

  enum EaudioState { e_duplex, e_playing, e_listening, e_iddle };
	
	QString deviceName() { if (m_type == e_input) return m_inDevName; else return m_outDevName; }
	
	void open() { openStream(); }
	void startAudio() { startStream(); }
	void stopAudio() { stopStream(); }
	void terminate() { closeStream(); }
	static void apiStopOrClose(); /** This method stops or closes audio stream depends on current API. */

  static bool areStreamsSplit() { return m_areSplit; }
	
#if defined (Q_OS_LINUX) || defined (Q_OS_WIN)
	static void setJACKorASIO(bool jack);
	static bool isJACKorASIO() { return m_JACKorASIO; }

			/** Initializes only @p m_JACKorASIO, use it only before any RtAudio instance is created. */
	static void initJACKorASIO(bool useIt) { m_JACKorASIO = useIt; }
	
	static bool switchAPI(RtAudio::Api rtApi); /** Deletes current instance and creates a new one with given API. Returns @p TRUE on success. */
#endif
	
	static void createRtAudio(); /** Creates RtAudio instance. Once for whole application */
	
	static QString& inputName() { return m_inDevName; }
	static QString& outputName() { return m_outDevName; }
	
	void updateAudioParams();
	TaudioParams* audioParams() { return m_audioParams; }
	
	static void printSupportedFormats(RtAudio::DeviceInfo &devInfo); /** Prints in console supported audio formats. */
	static void printSupportedSampleRates(RtAudio::DeviceInfo &devInfo); /** Prints in console supported sample rates. */
	static QString currentRtAPI(); /** Returns current API of rtAudio device. */
	static unsigned int bufferFrames() { return m_bufferFrames; } /** Number of frames (samples) in single callback */
	
			/** Returns instance of RtAudio device. 
				* It is single instance for entire application */
	static RtAudio* rtDevice() { return m_rtAduio; }
	
	static quint32 sampleRate() { return m_sampleRate; } /** Common sample rate for duplex callback */
	static quint32 inRate() { return m_inSR; } /** Sample rate of input stream when split */
	static quint32 outRate() { return m_outSR; } /** Sample rate of output stream when split */
	static EaudioState state() { return m_state; } /** Current state of audio device */
	
protected:		
	static RtAudio::StreamOptions *streamOptions;
	RtAudio::StreamParameters* streamParams() { if (m_type == e_input) return m_inParams; else return m_outParams; }
	
			/** switch that forces updating procedure of audio devices.
				* Used when in device is created a few times during run-time.
				* Device name is unchanged and it is only case to perform update. */
	static bool forceUpdate;
	
	void deleteOutParams() { delete m_outParams; m_outParams = 0; }
	void deleteInParams() { delete m_inParams; m_inParams = 0; }

			/** Sets pointer to appropriate callback method to 0. */
	void resetCallBack() { if (m_type == e_input) m_cbIn = 0; else m_cbOut = 0; }
	
			/** Examines available sample rates to check more appropriate. 
				* 44100 48000 88200 ... 192000. If not the latest from the list is taken. 
				* @param sampleRate is setting. */
	quint32 determineSampleRate(RtAudio::DeviceInfo& devInfo);
	
	bool openStream();
	bool startStream();
  bool play();
  bool listen();
	static void stopStream();
	static void closeStream();
	static void abortStream();
	static bool isOpened(); /** Checks rtAudio instance and returns @p TRUE when it is opened. */
	static bool isRunning(); /** Checks rtAudio instance and returns @p TRUE when it is running. */
	
			/** Static instance of 'signal emitter'
				* It emits following signals:
				* @p streamOpened() - when stream is opening
				*/
	static TaudioObject* ao() { return m_ao; }
	
	static bool hasCallBackIn() { return (bool)m_cbIn; }
	static bool hasCallBackOut() { return (bool)m_cbOut; }
	
	static bool getDeviceInfo(RtAudio::DeviceInfo &devInfo, int id);
	static RtAudio::Api getCurrentApi(); /** Returns current RtAudio API is instance exists or @p RtAudio::UNSPECIFIED */
	static unsigned int getDeviceCount(); /** Returns number of available audio devices or 0 if none or error occurred. */
	static int getDefaultIn(); /** Returns default input device for current API or -1 if error. */
	static int getDefaultOut(); /** Returns default output device for current API or -1 if error. */

			/** Converts device name of @p devInf determining proper encoding which depends on current API. */
	static QString convDevName(RtAudio::DeviceInfo& devInf) {
		if (getCurrentApi() == RtAudio::WINDOWS_WASAPI)
			return QString::fromUtf8(devInf.name.data());
		else
			return QString::fromLocal8Bit(devInf.name.data());
	}
#if defined(Q_OS_WIN)
	void restartASIO();
#endif

private:
	static int duplexCallBack(void *outBuffer, void *inBuffer, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void*) {
    if (m_cbOut) {
      if (m_cbOut(outBuffer, nBufferFrames, status))
        if (m_cbIn)
          m_cbIn(inBuffer, nBufferFrames, status);
    } else
        if (m_cbIn)
          m_cbIn(inBuffer, nBufferFrames, status);
    return 0;
  }

	static int passInputCallBack(void *outBuffer, void *inBuffer, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void*) {
    qint16 *in = (qint16*)inBuffer;
    qint16 *out = (qint16*)outBuffer;
    if (m_cbOut(outBuffer, nBufferFrames, status)) // none playing is performed
        for (int i = 0; i < nBufferFrames; i++) { // then forward input
            *out++ = *(in + i); // left channel
            *out++ = *(in + i); // right channel
        }
    m_cbIn(inBuffer, nBufferFrames, status);
    return 0;
  }

  static int playCallBack(void *outBuffer, void*, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void*) {
    if (m_cbOut(outBuffer, nBufferFrames, status))
      ao()->emitPlayingFinished();
    return 0;
  }

  static int listenCallBack(void*, void *inBuffer, unsigned int nBufferFrames, double, RtAudioStreamStatus status, void*) {
    m_cbIn(inBuffer, nBufferFrames, status);
    return 0;
  }
	
private:
	TaudioParams													*m_audioParams;
	static RtAudio					 							*m_rtAduio;
	EdevType															 m_type;
  static bool                            m_areSplit;
  static EaudioState                     m_state;
	static QString  											 m_inDevName, m_outDevName;
	static RtAudio::StreamParameters 			*m_inParams;
	static RtAudio::StreamParameters 			*m_outParams;
  static quint32 												 m_sampleRate; /** Common sample rate (for duplex callback) */
  static quint32                         m_inSR, m_outSR;
	static unsigned int										 m_bufferFrames;
	static bool														 m_isAlsaDefault;
	static callBackType										 m_cbIn, m_cbOut;
	static TaudioObject										*m_ao;
	static RtAudioCallback								 m_callBack;
	static bool														 m_JACKorASIO;
};

#endif // TRTAUDIOABSTRACT_H
