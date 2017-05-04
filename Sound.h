#pragma once

#include <memory>
#include <QAudioFormat>

namespace SoundTest
{
    //! Represents a sound that is played when an action like ball movement or explosion is taken.
    /*! This class should be implemented as thread safe.*/
    struct Sound
    {
        //! Plays the sound. Used for events like start game, explosion, etc...
        virtual void Start() = 0;

        //! Chooses the playback speed, so that the entire sound will take <duration> seconds. Used for ball movements.
        virtual void Start(float duration) = 0;

        //! Loops starting <duration> seconds of the sound, until Stop() is called. Used for ball jumps.
        virtual void Loop(float duration) = 0;

        //! Stop playing the sound.
        virtual void Stop() = 0;

        //! Used to correct the volume of supplied wav files.
        virtual void SetVolume(double val) = 0;

        //! Virtual destructor for using with std::shared_ptr.
        virtual ~Sound() {}
    };

    std::shared_ptr<Sound> CreateEncodedSound(const QString & file_name);

    QAudioFormat GetWavFormat();

    extern bool tracingEnabled;

    inline bool IsTraceEnabled()
    {
        return tracingEnabled;
    }

    inline void EnableTracing(bool val)
    {
        tracingEnabled = val;
    }
}
