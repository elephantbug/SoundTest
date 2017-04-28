#pragma once

#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QAudioOutput>
#include <QException>
#include <QFile>
#include <QBuffer>
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

        bool isLooping() const
        {
            return loop_;
        }

        void loop(bool val)
        {
            loop_ = val;
        }

        static void PrintCodecs();

    //public slots:
        void bufferReady();
        void stateChanged(QAudioDecoder::State newState);
        void finished();
        void onStateChanged(QAudio::State);
        void onNotify();
        void updateProgress();

    signals:

        void done();

    private slots:
        void onError(QAudioDecoder::Error error);

    private:

        //looks like QAudioDecoder is not currently supported on Android.
        QFile inputFile;
        std::unique_ptr<QAudioDecoder> audioDecoder;
        QAudioOutput audioOutput;
        QIODevice * pAudioStream = nullptr;
        QByteArray outputArray;
        QBuffer outputFile;

        void CommonStart(float duration, bool loop, double volume);

        double volumeCorrection = 1.0;

        bool isActive = false;

        bool loop_ = false;
    };
}
