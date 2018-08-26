#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QObject>
#include <QSoundEffect>
#include <QByteArray>
#include <QFile>
#include <QThread>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <memory>
#include <functional>

#include "EncodedSound.h"
#include "SoundDevice.h"
#include "SoundDecoder.h"

namespace SoundTest
{
    class SoundTestThread : public QThread
    {
    public:

        SoundTestThread(std::shared_ptr<SoundDevice> d, std::shared_ptr<QFile> f, int iter_count) :
            pDevice(d), inputFile(f), iterCount(iter_count)
        {
        }

    protected:

        void run() override;

    private:

        std::shared_ptr<SoundDevice> pDevice;
        std::shared_ptr<QFile> inputFile;
        int iterCount;
    };

    void SoundTestThread::run()
    {
        QEventLoop loop;

        std::cout << "Starting the sound." << std::endl;

        auto pDevice = std::make_shared<SoundDevice>(44);

        QObject::connect(pDevice.get(), &SoundDevice::done, &loop, [&loop]() {loop.quit();});

        pDevice->Start(inputFile.get(), 0, false);

//        for (int i = 0; i < iterCount; ++i)
//        {
//            std::cout << "Starting the sound.";

//            pDevice->Start(inputFile.get(), 0, false);

//            msleep(5000);

//            pDevice->Stop();
//        }

        std::cout << "Starting the event loop." << std::endl;

        loop.exec();
    }
}

using namespace SoundTest;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Sound Test");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Sound file to play."));

    QCommandLineOption reproduceBugOption("b", QCoreApplication::translate("main", "Reproduce QSoundEffect bug."));
    parser.addOption(reproduceBugOption);

    QCommandLineOption useEncodedSoundClassOption("e", QCoreApplication::translate("main", "Use EncodedSound class."));
    parser.addOption(useEncodedSoundClassOption);

    QCommandLineOption rawOption("r", QCoreApplication::translate("main", "Play raw sound (do not decode)."));
    parser.addOption(rawOption);

    QCommandLineOption loopSoundOption("l", QCoreApplication::translate("main", "Loop the sound."));
    parser.addOption(loopSoundOption);

    QCommandLineOption traceOption("t", QCoreApplication::translate("main", "Enable tracing."));
    parser.addOption(traceOption);

    QCommandLineOption multipleThreadsOption("m", QCoreApplication::translate("main", "Multiple threads."));
    parser.addOption(multipleThreadsOption);

    QCommandLineOption iterCountOption("i", QCoreApplication::translate("main", "Iteration count."));
    parser.addOption(iterCountOption);

    parser.process(app);

    EnableTracing(parser.isSet(traceOption));

    if (parser.positionalArguments().length() == 1)
    {
        QString sound_file_name = parser.positionalArguments().at(0);

        std::cout << "Playing '" << sound_file_name.toStdString() << "' ";

        std::function<void ()> start_func;

        if (parser.isSet(reproduceBugOption))
        {
            std::cout << "with QSoundEffect..." << std::endl;

            std::shared_ptr<QSoundEffect> pSound = std::make_shared<QSoundEffect>();

            pSound->setSource(QUrl::fromLocalFile(sound_file_name));

            std::function<void ()> status_func;

            if (parser.isSet(loopSoundOption))
            {
                status_func = [&app, pSound]()
                {
                    std::cout << "Is playing = " << pSound->isPlaying() << std::endl;

                    if (!pSound->isPlaying())
                    {
                        pSound->play();
                    }
                };
            }
            else
            {
                status_func = [&app, pSound]()
                {
                    std::cout << "Is playing = " << pSound->isPlaying() << std::endl;

                    if (!pSound->isPlaying())
                    {
                        app.quit();
                    }
                };
            }

            QObject::connect(pSound.get(), &QSoundEffect::playingChanged, &app, status_func,
                Qt::QueuedConnection);

            start_func = [pSound]() { pSound->play(); };
        }
        else if (parser.isSet(useEncodedSoundClassOption))
        {
            std::cout << "with QAudioOutput..." << std::endl;

            SoundTest::EncodedSound::PrintCodecs();

            std::shared_ptr<SoundTest::EncodedSound> pSound = std::make_shared<SoundTest::EncodedSound>(sound_file_name);

            QObject::connect(pSound.get(), &SoundTest::EncodedSound::done, &app, [&app]() {app.quit();});

            pSound->loop(parser.isSet(loopSoundOption));

            start_func = [pSound]() { pSound->Start(); };
        }
        else if (parser.isSet(rawOption))
        {
            std::shared_ptr<SoundDevice> pDevice = std::make_shared<SoundDevice>(44);

            if (parser.isSet(multipleThreadsOption))
            {
                int iter_count = 1;//1000000;

                if (parser.isSet(iterCountOption))
                {
                    iter_count = parser.value(iterCountOption).toInt();
                }

                start_func = [&app, pDevice, sound_file_name, iter_count]()
                {
                    std::vector<SoundTestThread *> threads;

                    constexpr size_t thread_count = 4;

                    for (size_t i = 0; i < thread_count; ++i)
                    {
                        std::shared_ptr<QFile> inputFile = std::make_shared<QFile>(sound_file_name);

                        if (inputFile->open(QIODevice::ReadOnly))
                        {
                            auto thread = new SoundTestThread(pDevice, inputFile, iter_count);

                            thread->start();

                            threads.push_back(thread);
                        }
                        else
                        {
                            std::cout << "Cannot open input file '" << sound_file_name.toStdString() << " for reading." << std::endl;

                            app.quit();
                        }
                    }

                    for (auto * t : threads)
                    {
                        t->wait();

                        delete t;
                    }

                    app.quit();
                };
            }
            else
            {
                std::shared_ptr<QFile> inputFile = std::make_shared<QFile>(sound_file_name);

                if (inputFile->open(QIODevice::ReadOnly))
                {
                    QObject::connect(pDevice.get(), &SoundDevice::done, &app, [&app]() {app.quit();});

                    start_func = [pDevice, inputFile]()
                    {
                        pDevice->Start(inputFile.get(), 0, false);
                    };
                }
                else
                {
                    std::cout << "Cannot open input file '" << sound_file_name.toStdString() << " for reading." << std::endl;

                    start_func = [&app]()
                    {
                        app.quit();
                    };
                }
            }
        }
        else
        {
            std::cout << "with audio decoder..." << std::endl;

            std::shared_ptr<QByteArray> pSoundBuffer = std::make_shared<QByteArray>();

            std::shared_ptr<SoundDecoder> pDecoder = std::make_shared<SoundDecoder>(sound_file_name, *(pSoundBuffer.get()));

            start_func = [pSoundBuffer, pDecoder]()
            {
                pDecoder->Start();
            };

            std::shared_ptr<SoundDevice> pDevice = std::make_shared<SoundDevice>();

            std::shared_ptr<QBuffer> in = std::make_shared<QBuffer>(pSoundBuffer.get());

            bool loop = parser.isSet(loopSoundOption);

            std::function<void ()> next_func = [in, pSoundBuffer, pDevice, loop]()
            {
                std::cout << "Sound buffer size = " << pSoundBuffer->length() << std::endl;

                in->open(QIODevice::ReadOnly);

                pDevice->Start(in.get(), 0, loop);
            };

            QObject::connect(pDecoder.get(), &SoundDecoder::done, &app, next_func);

            QObject::connect(pDevice.get(), &SoundDevice::done, &app, [&app]() {app.quit();});
        }

        // This will run the task from the application event loop.
        QTimer::singleShot(0, &app, start_func);

        return app.exec();
    }
    else
    {
        std::cout << "Usage: SoundTest <sound-file>" << std::endl;
    }

    return 1;
}
