#include <vector>
#include <filesystem>

// OpenAL is included in common.hpp
#include <common.hpp>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

/* 
* This example program shows how to use OpenAL and dr_wav to load and play a WAV file.
* NOTE: Limited to max 2 channels and 44.1-48kHz, only tested on Windows but this should be cross-platform
* Usage: wav_play.exe "Your WAV Sound Filename Here.wav"
* OR just drag and drop a WAV sound file onto the executable
*/

int main(int argc, char** argv)
{
	// We are doing a lot of checking here to make sure the filename is correct and the actual file exists
	// You may skip this part if you're certain you're not going to have typos, but it's never bad to check

	// Check if file argument is given
	if (argc < 2) {
		std::cout << "Invalid arguments, no file given" << std::endl;
		std::cin.get();
		return -1;
	}

	// Converting filename argument to string for easier use
	std::string filename = argv[1];
	// If filename doesn't have an extension we can't be sure if it's a WAV file, so we abort
	if (!std::filesystem::path(filename).has_extension()) {
		std::cout << "Invalid arguments, no file extension" << std::endl;
		std::cin.get();
		return -1;
	} else {
		// Check if filename extension is WAV, if not we abort
		if (std::filesystem::path(filename).extension() != ".wav") {
			std::cout << "Invalid arguments, file extension is not WAV" << std::endl;
			std::cin.get();
			return -1;
		}
	}

	// Checking whether the file actually exists and that it is a regular file
	if (!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename)) {
		std::cout << "Invalid arguments, file either doesn't exist or isn't a regular file" << std::endl;
		std::cin.get();
		return -1;
	}


	// Now we're getting to the more interesting part, getting OpenAL device and creating OpenAL context
	// OpenAL device could be your motherboard's internal sound chip or PCIe soundcard for example, but since we are using OpenAL-Soft it's going to use the CPU
	// OpenAL-Soft is mostly interchangeable with original OpenAL or some other variant, as long as you don't use any Soft specific extensions such as 32-bit float formats
	// The OpenAL context is very familiar to OpenGL one, it is tied to the device and global to the thread it was created in. Simply put it allows us to use OpenAL.

	// Open default OpenAL device, if you got multiple devices you can pass a device name as parameter to choose one
	auto device = alcOpenDevice(NULL);
	if (!device) {
		std::cout << "Failed to open OpenAL device" << std::endl;
		std::cin.get();
		return -1;
	}

	// Create OpenAL context for the device, NULL passed to attribute list
	auto context = alcCreateContext(device, NULL);
	if (!context) {
		std::cout << "Failed to create OpenAL context" << std::endl;
		std::cin.get();
		return -1;
	}

	// Make the created context currently used/active one
	if (!alcMakeContextCurrent(context)) {
		std::cout << "Failed to make OpenAL context current" << std::endl;
		std::cin.get();
		return -1;
	}


	// We have now initialized OpenAL and it's ready to do work for us
	// Now we load the WAV file given as argument to the program using dr_wav
	// We're also going to print out some information regarding the loaded file

	// Open the file and get a pointer to it
	auto drwav = drwav_open_file(filename.c_str());
	if (!drwav) {
		std::cout << "Failed to open file" << std::endl;
		std::cin.get();
		return -1;
	}

	ALuint channels = drwav->channels;
	ALuint samplerate = drwav->sampleRate;

	// Creating a vector that can hold the data
	std::vector<drwav_int16> data(drwav->totalSampleCount);
	auto read = drwav_read_s16(drwav, drwav->totalSampleCount, static_cast<drwav_int16*>(data.data()));
	if (read == 0) {
		std::cout << "Failed to read file" << std::endl;
		std::cin.get();
		return -1;
	}

	// Since the data is now read into a vector, we can close the WAV file
	drwav_close(drwav);

	std::cout << "[" << filename << "]" << std::endl;
	std::cout << "Channels: " << channels << std::endl;
	std::cout << "Samplerate: " << samplerate << std::endl;

	// Here we calculate the amount of samples
	// We shift the amount of bits per sample (in this case 16) to right by 3 and multiply the result with the amount of total frames
	auto byteSize = (16 >> 3);
	auto samples = read * byteSize;

	// Let's also print the samples out
	std::cout << "Samples: " << samples << std::endl;

	// We have to let OpenAL know the right format for the sound data
	// Since we always read the data as 16-bit integers we only have to worry if the sound data is either mono or stereo
	auto format = (channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;


	// We have loaded the WAV file so now we have to push that data to OpenAL and play it
	// Firstly we create an OpenAL buffer where the data will be going to, then we fill that buffer with the data
	// We will then create an OpenAL source which will use the buffer's data to play the sound data

	// Create 1 buffer, the buffer's ID is stored in the "buffer" variable
	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	// Check that previous function call was succesful
	OAL_Check("alGenBuffers");

	// Fill the buffer with data
	alBufferData(buffer, format, data.data(), samples, samplerate);
	OAL_Check("alBufferData");

	// Create 1 source, similar to buffer the source's ID is going to be stored in a variable
	ALuint source = 0;
	alGenSources(1, &source);
	OAL_Check("alGenSources");

	// Sources can have parameters that control how the sound is played, these include AL_LOOPING, AL_PITCH, AL_GAIN and many others
	alSourcei(source, AL_LOOPING, AL_FALSE);
	OAL_Check("alSourcei - AL_LOOPING");

	// The buffer ID is also given as a parameter to the source
	alSourcei(source, AL_BUFFER, buffer);
	OAL_Check("alSourcei - AL_BUFFER");

	// At last, we will be playing the source
	alSourcePlay(source);
	OAL_Check("alSourcePlay");

	// Get the source state and see if it succesfully started playing
	ALint state = 0;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	OAL_Check("alGetSourcei");
	if (state != AL_PLAYING) {
		std::cout << "Failed to play OpenAL source" << std::endl;
		std::cin.get();
		return -1;
	}

	// While loop that will keep checking whether the source is still playing before we move on
	while (state == AL_PLAYING) {
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		OAL_Check("alGetSourcei - while");
	}

	// The source has now stopped playing, we will now clean up and exit the program cleanly

	// Delete the source
	alDeleteSources(1, &source);
	OAL_Check("alDeleteSources");

	// Delete the buffer
	alDeleteBuffers(1, &buffer);
	OAL_Check("alDeleteBuffers");

	// Set current OpenAL context to nothing
	alcMakeContextCurrent(NULL);

	// Destroy the context
	alcDestroyContext(context);

	// Finally we close the device
	alcCloseDevice(device);

	return 0;
}
