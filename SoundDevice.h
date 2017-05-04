#pragma once

#include <QAudioOutput>
#include <QIODevice>

#include <mutex>

namespace SoundTest
{
    class SoundDevice : public QObject
    {
        Q_OBJECT

    public:

        SoundDevice();

        void Start(QIODevice * p_device, float duration, bool loop);

        void Stop();

        qreal GetVolume() const
        {
            return audioOutput.volume();
        }

        void SetVolume(qreal val)
        {
            audioOutput.setVolume(val);
        }

    signals:

        void done();

    public slots:
        void onStateChanged(QAudio::State);
        void onNotify();

    private:

        void SeekToStart()
        {
            pDevice->seek(0); //no need to skip 44 bytes of wav header
        }

        void ClearFlags()
        {
            loopWithNotify = false;
            loopWithStateChanged = false;
        }

        typedef std::lock_guard<std::recursive_mutex> Lock;

        std::recursive_mutex playMutex;

        QIODevice * pDevice = nullptr;

        QAudioOutput audioOutput;

        bool loopWithNotify;

        bool loopWithStateChanged;

        bool tracingEnabled = false;
    };
}
