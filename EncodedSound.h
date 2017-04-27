#pragma once

#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QAudioOutput>
#include <QException>
#include <QFile>
#include <memory>

#include "Sound.h"

namespace SoundTest
{
    class SoundException //: public QException
    {
    public:

        SoundException(const QString & message) : message_(message)
        {
        }

        //void raise() const override { throw *this; }

        //SoundException *clone() const override { return new SoundException(*this); }

    private:

        QString message_;
    };

    class EncodedSound : public QObject, public Sound
    {
        Q_OBJECT

    public:

        EncodedSound(const QString & file_name);

        virtual void Start() override;

        virtual void Start(float duration) override;

        virtual void Loop(float duration) override;

        virtual void Stop() override;

        virtual void SetVolume(double val) override;

        void Convert(const QString & file_name);

        bool isLooping() const
        {
            return loop_;
        }

        void loop(bool val)
        {
            loop_ = val;
        }

    signals:

        void done();

    public slots:
        void bufferReady();
        void error(QAudioDecoder::Error error);
        void stateChanged(QAudioDecoder::State newState);
        void finished();
        void onStateChanged(QAudio::State);
        void onNotify();

    private slots:
        void updateProgress();

    private:

        QAudioFormat GetWavFormat();

        //looks like QAudioDecoder is not currently supported on Android.
        std::unique_ptr<QAudioDecoder> audioDecoder;
        QAudioOutput audioOutput;
        QIODevice * pAudioStream = nullptr;
        QFile outputFile;

        void CommonStart(float duration, bool loop, double volume);

        double volumeCorrection = 1.0;

        bool isActive = false;

        bool loop_ = false;
    };
}
