#include "PlaybackController.h"

namespace alors
{

namespace
{
    constexpr double beatsPerCycle = 4.0;
}

PlaybackController::PlaybackController (const juce::File& userLibraryFolder)
    : engine (buildSampleBank (userLibraryFolder)),
      scheduler (engine)
{
    deviceError = deviceManager.initialiseWithDefaultDevices (0, 2);
    deviceManager.addAudioCallback (&engine);
}

PlaybackController::~PlaybackController()
{
    scheduler.stop();
    deviceManager.removeAudioCallback (&engine);
}

void PlaybackController::play (Pattern pattern)
{
    scheduler.play (std::move (pattern));
}

void PlaybackController::stop()
{
    scheduler.stop();
}

bool PlaybackController::preview (const std::string& soundName, int variant)
{
    if (auto index = engine.getSampleBank().find (soundName, variant))
        return engine.preview (*index);

    return false;
}

void PlaybackController::setBpm (double bpm)
{
    scheduler.setCyclesPerSecond (bpm / 60.0 / beatsPerCycle);
}

double PlaybackController::getBpm() const
{
    return scheduler.getCyclesPerSecond() * 60.0 * beatsPerCycle;
}

bool PlaybackController::isPlaying() const
{
    return scheduler.isPlaying();
}

double PlaybackController::getCurrentCycle() const
{
    return scheduler.getCurrentCycle();
}

std::vector<SoundInfo> PlaybackController::getSounds() const
{
    return engine.getSampleBank().sounds();
}

std::shared_ptr<const SampleBank> PlaybackController::buildSampleBank (const juce::File& userLibraryFolder)
{
    auto bank = std::make_shared<SampleBank>();
    bank->addBuiltInKit();

    if (userLibraryFolder.isDirectory())
        bank->loadFolder (userLibraryFolder);

    return bank;
}

}
