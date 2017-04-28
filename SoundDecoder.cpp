#include "SoundDecoder.h"

#include <assert.h>
#include <iostream>
#include <sstream>

#include "Sound.h"

namespace SoundTest
{
    QAudioFormat GetWavFormat()
    {
        QAudioFormat desiredFormat;

        desiredFormat.setChannelCount(2);
        desiredFormat.setCodec("audio/pcm");
        desiredFormat.setByteOrder(QAudioFormat::LittleEndian);
        //desiredFormat.setByteOrder(QAudioFormat::BigEndian);
        desiredFormat.setSampleType(QAudioFormat::SignedInt);
        //desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
        desiredFormat.setSampleRate(44100); //8000
        desiredFormat.setSampleSize(16);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

        if (!info.isFormatSupported(desiredFormat))
        {
            std::cout << "Default WAV sound format is not supported, trying to use nearest." << std::endl;
            return info.nearestFormat(desiredFormat);
        }

        return desiredFormat;
    }
}

using namespace SoundTest;

SoundDecoder::SoundDecoder(const QString & file_name, QByteArray & output_array) : inputFile(file_name), outputArray(output_array), outputFile(&outputArray)
{
    audioDecoder.reset(new QAudioDecoder(this));
    audioDecoder->setAudioFormat(GetWavFormat());

    connect(audioDecoder.get(), &QAudioDecoder::bufferReady, this, &SoundDecoder::bufferReady);
    //Cannot figure out why this connect does not compile.
    //connect(audioDecoder.get(), &QAudioDecoder::error, this, &SoundDecoder::onError);
    connect(audioDecoder.get(), SIGNAL(error(QAudioDecoder::Error)), this, SLOT(onError(QAudioDecoder::Error)));
    connect(audioDecoder.get(), &QAudioDecoder::stateChanged, this, &SoundDecoder::stateChanged);
    connect(audioDecoder.get(), &QAudioDecoder::finished, this, &SoundDecoder::finished);
    connect(audioDecoder.get(), &QAudioDecoder::positionChanged, this, &SoundDecoder::updateProgress);
    connect(audioDecoder.get(), &QAudioDecoder::durationChanged, this, &SoundDecoder::updateProgress);
}

void SoundDecoder::PrintCodecs()
{
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    std::cout << "Supported Codecs : ";
    auto codecs = info.supportedCodecs();

    for (const QString & c : codecs)
    {
        std::cout << " " << c.toStdString();
    }

    std::cout << std::endl;
}

void SoundDecoder::Start()
{
    if (inputFile.open(QIODevice::ReadOnly))
    {
        outputFile.open(QIODevice::WriteOnly);

        audioDecoder->setSourceDevice(&inputFile);

        //std::cerr << "XXX: start\n";
        audioDecoder->start();
        //std::cerr << "XXX: finish start\n";
    }
    else
    {
        std::cout << QString("Cannot open file '%1' for reading.").arg(inputFile.fileName()).toStdString() << std::endl;
        emit done();
    }
}

void SoundDecoder::bufferReady()
{
    // read a buffer from audio decoder
    QAudioBuffer buffer = audioDecoder->read();
    if (buffer.isValid())
    {
        std::cout << "Audio buffer byte count = " << buffer.byteCount() << std::endl;

        outputFile.write((const char *)buffer.data(), buffer.byteCount());
    }
    else
    {
        std::cout << "Audio buffer is invalid." << std::endl;
        emit done();
    }
}

void SoundDecoder::onError(QAudioDecoder::Error error)
{
    switch (error)
    {
    case QAudioDecoder::NoError:
        return;
    case QAudioDecoder::ResourceError:
        std::cout << "QAudioDecoder Resource error" << std::endl;
        break;
    case QAudioDecoder::FormatError:
        std::cout << "QAudioDecoder Format error" << std::endl;
        break;
    case QAudioDecoder::AccessDeniedError:
        std::cout << "QAudioDecoder Access denied error" << std::endl;
        break;
    case QAudioDecoder::ServiceMissingError:
        std::cout << "QAudioDecoder Service missing error" << std::endl;
        break;
    }

    emit done();
}

void SoundDecoder::stateChanged(QAudioDecoder::State newState)
{
    switch (newState) {
    case QAudioDecoder::DecodingState:
        std::cout << "Decoding sound..." << std::endl;
        break;
    case QAudioDecoder::StoppedState:
        std::cout << "Sound decoding stopped." << std::endl;
        outputFile.close();
        emit done();
        break;
    }
}

void SoundDecoder::finished()
{
    std::cout <<  "Sound decoding finished." << std::endl;
}

void SoundDecoder::updateProgress()
{
}
