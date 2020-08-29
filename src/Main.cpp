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

//==============================================================================
int main()
{

	// Initialize audio devices
	std::cout << "Begin init" << std::endl;
	juce::AudioDeviceManager deviceManager;
	juce::String error = deviceManager.initialise (
		0, /* number of input channels */
		2, /* number of output channels */
		0, /* no XML settings.. */
		true  /* select default device on failure */
	);

	// Output error (if any)
	if (error.isNotEmpty()) {
		std::cout << "Error on initialise(): " << error.toStdString() << std::endl;
	}

	// Play test sound
	std::cout << "You should hear five test tones, precisely 1 second apart." << std::endl;
	auto start = std::chrono::steady_clock::now();
	using sec = std::chrono::seconds;
	for (auto s = static_cast<sec>(1); s <= static_cast<sec>(5); s++) {
		std::cout << s.count() << "... " << std::flush;
		deviceManager.playTestSound();
		std::this_thread::sleep_until(start + s);
	}

	std::cout << "\nAudio device probe results:\n";
	for (const auto t : deviceManager.getAvailableDeviceTypes()) {
		juce::String typeString;
		typeString << "Type " << t->getTypeName() << ":";
		std::cout << typeString << std::endl;
		for (const auto deviceName :  t->getDeviceNames()) {
			juce::String deviceString;
			deviceString << deviceName;
			std::cout << "  - " << deviceString << std::endl;
		}
	}
  std::cout << "\nDemo complete. Shutting down device manager.\n";

	// Stop audio devices
	deviceManager.closeAudioDevice();
	deviceManager.removeAllChangeListeners();
	deviceManager.dispatchPendingMessages();

	return 0;
}
