/**
 * @file
 * @brief Source code to play a test sound
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @section LICENSE
 *
 * Copyright (c) 2008-2016 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of OpenShot Audio
 * Library (libopenshot-audio), an open-source project dedicated to delivering
 * high quality audio editing and playback solutions to the world. For more
 * information visit <http://www.openshot.org/>.
 *
 * OpenShot Audio Library (libopenshot-audio) is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version
 * 3 of the License, or (at your option) any later version.
 *
 * OpenShot Audio Library (libopenshot-audio) is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 *
 * @mainpage OpenShot Audio Library C++ API
 *
 * Welcome to the OpenShot Audio Library C++ API.  This library is used by libopenshot to enable audio
 * features, which powers the <a href="http://www.openshot.org">OpenShot Video Editor</a> application.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "JuceHeader.h"

void list_devices(juce::AudioDeviceManager* manager) {
    std::cout << "Detected audio devices:  "
              << "* - Currently-selected device\n\n";

    const auto& curPtr = manager->getCurrentAudioDevice();
    const auto& types = manager->getAvailableDeviceTypes();

    for (auto i = 0, e = types.size(); i < e; ++i) {
        const auto& typePtr = types[i];
        std::cout << "Type " << typePtr->getTypeName() << ":\n";

        for (const auto& deviceName :  typePtr->getDeviceNames()) {
            // Place a * next to the current output device
            bool is_default = (deviceName == curPtr->getName());
            char mark = is_default ? '*' : '-';

            std::cout << "  " << mark << " " << deviceName << "\n";
        }
    }
    std::cout << "\n";
}

//==============================================================================
int main(int argc, char* argv[])
{

    juce::ignoreUnused (argc, argv);

    // Initialize default audio device
    std::cout << "Initialising audio hardware management interface.\n\n";
    juce::AudioDeviceManager deviceManager;

    auto error = deviceManager.initialise (
        0,        // numInputChannelsNeeded
        2,        // numOutputChannelsNeeded
        nullptr,  // savedState
        true      // selectDefaultDeviceOnFailure
    );

    // Output error (if any)
    if (error.isNotEmpty()) {
        std::cout << "Error on initialise(): "
                  << error.toStdString() << std::endl;
    }

    list_devices(&deviceManager);

    std::cout << "You should hear five test tones, "
              << "precisely 1 second apart.\n";

    using sec = std::chrono::seconds;
    std::this_thread::sleep_for(sec(1));

    auto start = std::chrono::steady_clock::now();

    // Play test tones
    for (auto s = static_cast<sec>(1), e = static_cast<sec>(5); s <= e; ++s) {
        std::cout << s.count() << (s.count() < 5 ? "... " : ".\n\n") << std::flush;
        deviceManager.playTestSound();
        std::this_thread::sleep_until(start + s);
    }

    std::cout << "Demo complete. Shutting down device manager.\n";
    std::this_thread::sleep_for(sec(1));

    // Stop audio devices
    deviceManager.closeAudioDevice();
    deviceManager.removeAllChangeListeners();
    deviceManager.dispatchPendingMessages();

    return 0;
}
