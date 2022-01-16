#include "Sound.h"

namespace SoundTest
{
    QAudioFormat GetWavFormat()
    {
        QAudioFormat desiredFormat;

        desiredFormat.setChannelCount(2);
        //desiredFormat.setCodec("audio/pcm");
        //desiredFormat.setByteOrder(QAudioFormat::LittleEndian);
        //desiredFormat.setByteOrder(QAudioFormat::BigEndian);
        //desiredFormat.setSampleType(QAudioFormat::SignedInt);
        desiredFormat.setSampleRate(44100); //8000
        //desiredFormat.setSampleSize(16);
        desiredFormat.setSampleFormat(QAudioFormat::Int16);

        /*
        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        //qDebug() << "Supported Codecs : " << info.supportedCodecs();

        if (!info.isFormatSupported(desiredFormat))
        {
            qWarning() << "Default format not supported - trying to use nearest";
            return info.nearestFormat(desiredFormat);
        }
        */

        return desiredFormat;
    }

    bool tracingEnabled = false;
}
