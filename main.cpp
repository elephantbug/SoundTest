#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QObject>
#include <QSoundEffect>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <memory>
#include <functional>

#include "EncodedSound.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Sound Test");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Sound file to play."));

    QCommandLineOption reproduceBug("b", QCoreApplication::translate("main", "Reproduce QSoundEffect bug."));
    parser.addOption(reproduceBug);

    parser.process(app);

    if (parser.positionalArguments().length() == 1)
    {
        QString sound_file_name = parser.positionalArguments().at(0);

        std::cout << "Playing '" << sound_file_name.toStdString() << "' ";

        std::function<void ()> start_func;

        if (parser.isSet(reproduceBug))
        {
            std::cout << "with QSoundEffect..." << std::endl;

            std::shared_ptr<QSoundEffect> pSound = std::make_shared<QSoundEffect>();

            pSound->setSource(QUrl::fromLocalFile(sound_file_name));

            auto loop_sound_func = [&app, pSound]()
            {
                std::cout << "Is playing = " << pSound->isPlaying() << std::endl;
                if (!pSound->isPlaying())
                    app.quit();
            };

            QObject::connect(pSound.get(), &QSoundEffect::playingChanged, &app, loop_sound_func,
                Qt::QueuedConnection);

            start_func = [pSound]() { pSound->play(); };
        }
        else
        {
            std::cout << "with QAudioOutput..." << std::endl;

            std::shared_ptr<SoundTest::EncodedSound> pSound = std::make_shared<SoundTest::EncodedSound>(sound_file_name);

            QObject::connect(pSound.get(), &SoundTest::EncodedSound::done, &app, [&app]() {app.quit();});

            start_func = [pSound]() { pSound->Start(); };
        }

        // This will run the task from the application event loop.
        QTimer::singleShot(0, &app, start_func);

        return app.exec();
    }
    else
    {
        std::cout << "Usage: SoundTest <sound-file>" << std::endl;
    }
}
