#include "EncodedSound.h"

#include <iostream>
#include <sstream>

namespace SoundTest
{
    std::shared_ptr<Sound> CreateEncodedSound(const QString & file_name)
    {
        return std::make_shared<EncodedSound>(file_name);
    }
}

using namespace SoundTest;

EncodedSound::EncodedSound(const QString & file_name) : audioOutput(GetWavFormat(), this)
{
    std::cerr << "XXX: Begin create decoder\n";
    audioDecoder.reset(new QAudioDecoder(this));
    std::cerr << "XXX: End create decoder\n";
    audioDecoder->setAudioFormat(GetWavFormat());
    //audioDecoder.setSourceDevice();
    std::cerr << "XXX: Setting source file name\n";
    audioDecoder->setSourceFilename(file_name);

    connect(audioDecoder.get(), SIGNAL(bufferReady()), this, SLOT(bufferReady()));
    connect(audioDecoder.get(), SIGNAL(error(QAudioDecoder::Error)), this, SLOT(error(QAudioDecoder::Error)));
    connect(audioDecoder.get(), SIGNAL(stateChanged(QAudioDecoder::State)), this, SLOT(stateChanged(QAudioDecoder::State)));
    connect(audioDecoder.get(), SIGNAL(finished()), this, SLOT(finished()));
    connect(audioDecoder.get(), SIGNAL(positionChanged(qint64)), this, SLOT(updateProgress()));
    connect(audioDecoder.get(), SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress()));

    connect(&audioOutput, &QAudioOutput::stateChanged, this, [this](QAudio::State state)
    {
        if (state == QAudio::IdleState)
        {
            //emit done();
        }
    });
}

//QAudioFormat EncodedSound::GetWavFormat()
//{
//    QAudioFormat desiredFormat;

//    desiredFormat.setChannelCount(2);
//    desiredFormat.setCodec("audio/x-raw");
//    desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
//    desiredFormat.setSampleRate(48000);
//    desiredFormat.setSampleSize(16);

//    return desiredFormat;
//}

QAudioFormat EncodedSound::GetWavFormat()
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
    //std::cout << "Supported Codecs : " << info.supportedCodecs();
    auto codecs = info.supportedCodecs();

    if (!info.isFormatSupported(desiredFormat))
    {
        std::cout << "Default WAV sound format is not supported, trying to use nearest." << std::endl;
        return info.nearestFormat(desiredFormat);
    }

    return desiredFormat;
}

void EncodedSound::Start()
{
    //there is also NAN define in cmath.h
    CommonStart(std::numeric_limits<float>::quiet_NaN(), false, volumeCorrection);
}

void EncodedSound::Start(float duration)
{
    CommonStart(duration, false, volumeCorrection);
}

void EncodedSound::Loop(float duration)
{
    CommonStart(duration, true, volumeCorrection);
}

void EncodedSound::Stop()
{
    audioDecoder->stop();

    pAudioStream = nullptr;
}

void EncodedSound::SetVolume(double val)
{
    volumeCorrection = val;
}

void EncodedSound::CommonStart(float duration, bool loop, double volume)
{
    Q_UNUSED(duration)
    Q_UNUSED(loop)
    Q_UNUSED(volume)

    std::cerr << "XXX: start\n";
    audioDecoder->start();
    std::cerr << "XXX: finish start\n";

    pAudioStream = audioOutput.start();
}

void EncodedSound::Convert(const QString & file_name)
{
    outputFile.setFileName(file_name);

    if (!outputFile.open(QIODevice::WriteOnly))
    {
        throw SoundException(QString("Cannot open file '%1' for writing.").arg(file_name));
    }

    audioDecoder->start();
}

void EncodedSound::bufferReady()
{
    // read a buffer from audio decoder
    QAudioBuffer buffer = audioDecoder->read();
    if (buffer.isValid())
    {
        std::cout << "Audio buffer byte count = " << buffer.byteCount() << std::endl;

        if (pAudioStream != nullptr)
        {
            pAudioStream->write((const char *)buffer.data(), buffer.byteCount());
        }
        else
        {
            outputFile.write((const char *)buffer.data(), buffer.byteCount());
        }
    }
    else
    {
        std::cout << "Audio buffer is invalid." << std::endl;
    }
}

void EncodedSound::error(QAudioDecoder::Error error)
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

void EncodedSound::stateChanged(QAudioDecoder::State newState)
{
    switch (newState) {
    case QAudioDecoder::DecodingState:
        std::cout << "Decoding sound..." << std::endl;
        break;
    case QAudioDecoder::StoppedState:
        std::cout << "Sound decoding stopped." << std::endl;
        break;
    }
}

void EncodedSound::finished()
{
    std::cout <<  "Sound decoding finished." << std::endl;

    if (pAudioStream != nullptr)
    {
        pAudioStream = nullptr;
    }
    else
    {
        outputFile.close();
    }

    //emit done();
}

void EncodedSound::updateProgress()
{
}