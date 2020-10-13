#include <string>
#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>

// Converts OpenAL error code to an error string that can be understood better
std::string OAL_ErrorToString(const ALuint& error)
{
	std::string err;
	switch (error) {
		case AL_NO_ERROR: err = "None"; break;
		case AL_INVALID_NAME: err = "Invalid name"; break;
		case AL_INVALID_ENUM: err = "Invalid enum"; break;
		case AL_INVALID_VALUE: err = "Invalid value"; break;
		case AL_INVALID_OPERATION: err = "Invalid operation"; break;
		case AL_OUT_OF_MEMORY: err = "Out of memory"; break;
		default: err = "Unknown error"; break;
	}
	return err;
}

// Function for checking if the last OpenAL function was executed succesfully, if not we show an error in console and exit
void OAL_Check(const std::string& str)
{
	ALenum err = 0;
	if ((err = alGetError()) != AL_NO_ERROR) {
		std::cout << "OpenAL check for [" << str << "] failed, error: " << OAL_ErrorToString(err) << " [" << err << "]" << std::endl;
		std::cin.get();
		std::quick_exit(-1);
	}
}