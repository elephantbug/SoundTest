#include "EncodedSound.h"

#include <assert.h>
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

EncodedSound::EncodedSound(const QString & file_name) : inputFile(file_name), audioOutput(GetWavFormat(), this), outputFile(&outputArray)
{
    //std::cerr << "XXX: Begin create decoder\n";
    audioDecoder.reset(new QAudioDecoder(this));
    //std::cerr << "XXX: End create decoder\n";
    audioDecoder->setAudioFormat(GetWavFormat());
    //audioDecoder.setSourceDevice();
    //std::cerr << "XXX: Setting source file name\n";
    //audioDecoder->setSourceFilename(file_name);

    connect(audioDecoder.get(), &QAudioDecoder::bufferReady, this, &EncodedSound::bufferReady);
    //Cannot figure out why this connect does not compile.
    //connect(audioDecoder.get(), &QAudioDecoder::error, this, &EncodedSound::onError);
    connect(audioDecoder.get(), SIGNAL(error(QAudioDecoder::Error)), this, SLOT(onError(QAudioDecoder::Error)));
    connect(audioDecoder.get(), &QAudioDecoder::stateChanged, this, &EncodedSound::stateChanged);
    connect(audioDecoder.get(), &QAudioDecoder::finished, this, &EncodedSound::finished);
    connect(audioDecoder.get(), &QAudioDecoder::positionChanged, this, &EncodedSound::updateProgress);
    connect(audioDecoder.get(), &QAudioDecoder::durationChanged, this, &EncodedSound::updateProgress);

    connect(&audioOutput, &QAudioOutput::stateChanged, this, &EncodedSound::onStateChanged);
    connect(&audioOutput, &QAudioOutput::notify, this, &EncodedSound::onNotify);
}

void EncodedSound::PrintCodecs()
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

    if (inputFile.open(QIODevice::ReadOnly))
    {
        outputFile.open(QIODevice::WriteOnly);

        audioDecoder->setSourceDevice(&inputFile);

        //std::cerr << "XXX: start\n";
        audioDecoder->start();
        //std::cerr << "XXX: finish start\n";

        pAudioStream = audioOutput.start();
    }
    else
    {
        std::cout << QString("Cannot open file '%1' for reading.").arg(inputFile.fileName()).toStdString() << std::endl;
        emit done();
    }
}

void EncodedSound::bufferReady()
{
    // read a buffer from audio decoder
    QAudioBuffer buffer = audioDecoder->read();
    if (buffer.isValid())
    {
        std::cout << "Audio buffer byte count = " << buffer.byteCount() << std::endl;

        assert(pAudioStream != nullptr);

        pAudioStream->write((const char *)buffer.data(), buffer.byteCount());
        outputFile.write((const char *)buffer.data(), buffer.byteCount());
    }
    else
    {
        std::cout << "Audio buffer is invalid." << std::endl;
        emit done();
    }
}

void EncodedSound::onError(QAudioDecoder::Error error)
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

void EncodedSound::onStateChanged(QAudio::State state)
{
    switch (state)
    {
    case QAudio::ActiveState:
        std::cout << "QAudio::ActiveState" << std::endl;
        isActive = true;
        break;

    case QAudio::SuspendedState:
        std::cout << "QAudio::ActiveState" << std::endl;
        break;

    case QAudio::StoppedState:
        std::cout << "QAudio::StoppedState" << std::endl;
        break;

    case QAudio::IdleState:

        std::cout << "QAudio::IdleState" << std::endl;

        std::cout << "QAudioDecoder::State=" << (audioDecoder->state() == QAudioDecoder::StoppedState ? "Stoppped" : "Decoding") << std::endl;

        if (isActive)
        {
            if (audioDecoder != nullptr)
            {
                audioDecoder->stop();

                audioDecoder.reset(nullptr);

                inputFile.close();

                outputFile.close();
            }

            if (loop_)
            {
                pAudioStream->write(outputArray.data(), outputArray.length());
            }
            else
            {
                isActive = false;

                emit done();
            }
        }

//        if (loopWithStateChanged)
//        {
//            //looks like we need to call start() at this point, because others do not take an effect
//            //audioOutput.suspend();
//            //audioOutput.reset();
//            //audioOutput.resume();

//            SeekToStart();

//            audioOutput.start(pDevice);
//        }

        break;
    }
}

void EncodedSound::onNotify()
{
    std::cout << "onNotify()" << std::endl;

    //qDebug() << "loopWithNotify =" << loopWithNotify;

    //probably onNotify() way is better, because we do not need to call start() here
//    if (loopWithNotify)
//    {
//        SeekToStart();
//    }
}
