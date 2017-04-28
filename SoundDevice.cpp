#include "Sound.h"
#include "SoundDevice.h"
#include <iostream>

using namespace SoundTest;

SoundDevice::SoundDevice() : audioOutput(GetWavFormat(), this)
{
    ClearFlags();

    connect(&audioOutput, SIGNAL(notify()), this, SLOT(onNotify()), Qt::DirectConnection);
    connect(&audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChanged(QAudio::State)), Qt::DirectConnection);
}

void SoundDevice::Start(QIODevice * p_device, float duration, bool loop)
{
    Lock lock(playMutex);

    //sound restarts without stop() call
    //audioOutput.stop();

    audioOutput.reset();

    pDevice = p_device;

    //SeekToStart();

    ClearFlags();

    loopWithStateChanged = loop;

    //looks like it does not work on Android
    bool useOnNotify = false;

    if (useOnNotify)
    {
        if (loop)
        {
            if (std::isnan(duration))
            {
                loopWithStateChanged = true;
            }
            else
            {
                loopWithNotify = true;

                audioOutput.setNotifyInterval((int)(duration * 1000));
            }
        }

        if (!loopWithNotify)
        {
            audioOutput.setNotifyInterval(100 * 1000);
        }
    }

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
