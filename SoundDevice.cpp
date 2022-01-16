#include "Sound.h"
#include "SoundDevice.h"
#include <iostream>
#include <QTimer>

using namespace SoundTest;

SoundDevice::SoundDevice(qint64 start_pos) : audioOutput(GetWavFormat(), this), startPos(start_pos)
{
    ClearFlags();

    //connect(&audioOutput, &QAudioSink::notify, this, &SoundDevice::onNotify, Qt::DirectConnection);
    connect(&audioOutput, &QAudioSink::stateChanged, this, &SoundDevice::onStateChanged);
}

void SoundDevice::Start(QIODevice * p_device, float duration, bool loop)
{
    Lock lock(playMutex);

    //sound restarts without stop() call
    //audioOutput.stop();

    audioOutput.reset();

    pDevice = p_device;

    SeekToStart();

    ClearFlags();

    loopWithStateChanged = loop;

    audioOutput.start(pDevice);
}

void SoundDevice::Stop()
{
    Lock lock(playMutex);

    if (pDevice != nullptr)
    {
        audioOutput.stop();

        pDevice = nullptr;

        ClearFlags();
    }
}

qreal SoundDevice::GetVolume() const
{
    Lock lock(playMutex);

    return audioOutput.volume();
}

void SoundDevice::SetVolume(qreal val)
{
    Lock lock(playMutex);

    audioOutput.setVolume(val);
}

void SoundDevice::onStateChanged(QAudio::State state)
{
    Lock lock(playMutex);

    switch (state)
    {
    case QAudio::ActiveState:
        std::cout << "QAudio::ActiveState" << std::endl;
        break;

    case QAudio::SuspendedState:
        std::cout << "QAudio::ActiveState" << std::endl;
        break;

    case QAudio::StoppedState:
        std::cout << "QAudio::StoppedState" << std::endl;
        break;

    case QAudio::IdleState:

        std::cout << "QAudio::IdleState" << std::endl;

        if (loopWithStateChanged)
        {
            //looks like we need to call start() at this point, because others do not take an effect
            //audioOutput.suspend();
            //audioOutput.reset();
            //audioOutput.resume();

            SeekToStart();

            //timer does not help
            //QTimer::singleShot(0, this, [this](){ audioOutput.start(pDevice);});
            audioOutput.start(pDevice);
        }
        else
        {
            emit done();
        }

        break;
    }
}

void SoundDevice::onNotify()
{
    Lock lock(playMutex);

    std::cout << "onNotify()" << std::endl;

    //probably onNotify() way is better, because we do not need to call start() here
    if (loopWithNotify)
    {
        SeekToStart();
    }
}
