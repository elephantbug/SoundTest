#pragma once

#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QAudioOutput>
#include <QException>
#include <QFile>
#include <QBuffer>
#include <memory>

namespace SoundTest
{
    class SoundDecoder : public QObject
    {
        Q_OBJECT

    public:

        SoundDecoder(const QString & file_name, QByteArray & outputArray);

        void Start();

        static void PrintCodecs();

    signals:

        void done();

    private slots:
        void onError(QAudioDecoder::Error error);

    private:

        void bufferReady();
        void stateChanged(QAudioDecoder::State newState);
        void finished();
        void updateProgress();

        QFile inputFile;
        std::unique_ptr<QAudioDecoder> audioDecoder;
        QByteArray & outputArray;
        QBuffer outputFile;
    };
}
