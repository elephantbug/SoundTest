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

#include "SoundDevice.h"
#include "Sound.h"

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

        //It I use this->pDevice the sound does not play at all, or at least I do not hear it.
        auto pDevice = std::make_shared<SoundDevice>(44);

        QObject::connect(pDevice.get(), &SoundDevice::done, &loop, [&loop]() {loop.quit();});

        pDevice->Start(inputFile.get(), 0, false);

        //It does not make a sense to test the sound in loop if each thread has its own SoundDevice.
        /*
        for (int i = 0; i < iterCount; ++i)
        {
            std::cout << "Starting the sound.";

            pDevice->Start(inputFile.get(), 0, false);

            msleep(5000);

            pDevice->Stop();
        }
        */

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

    parser.process(app);

    bool loop = true;

    if (parser.positionalArguments().length() == 1)
    {
        QString sound_file_name = parser.positionalArguments().at(0);

        std::cout << "Playing '" << sound_file_name.toStdString() << "' ";

        std::function<void ()> start_func;

        std::shared_ptr<SoundDevice> pDevice = std::make_shared<SoundDevice>(44);

        {
            std::shared_ptr<QFile> inputFile = std::make_shared<QFile>(sound_file_name);

            if (inputFile->open(QIODevice::ReadOnly))
            {
                QObject::connect(pDevice.get(), &SoundDevice::done, &app, [&app]() {app.quit();});

                start_func = [pDevice, inputFile, loop]()
                {
                    pDevice->Start(inputFile.get(), 0, loop);
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
