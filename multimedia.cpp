#include "multimedia.hpp"
#include <iostream>
#include <vlc/vlc.h>


libvlc_instance_t *vlc_instance = nullptr;
libvlc_media_player_t *media_player = nullptr;

bool paused = false;

Multimedia::Multimedia()
{
	
	initialize();
}

Multimedia::~Multimedia()
{

	stop();
	release();
}

void Multimedia::pause() const
{
		/* toggle pause auto */
	
	if (paused){
		paused=false;
	}
	else{
	paused=true;
	}
		libvlc_media_player_pause(media_player);
	
}

void Multimedia::stop()
{
	if (is_valid_media()) {
		libvlc_media_player_stop(media_player);
		libvlc_media_player_release(media_player);
		media_player = nullptr;
	}
}

void Multimedia::set_volume(int volume) const
{
	if (is_valid_media()) {
		libvlc_audio_set_volume(media_player, volume);
	}
}

int Multimedia::get_volume() const
{
	if (is_valid_media()) {
		return libvlc_audio_get_volume(media_player);
	}
	return 0;
}


bool Multimedia::is_valid_media() const
{
	return (media_player != nullptr);
}

bool Multimedia::is_mute() const
{
	return b_mute;
}


bool Multimedia::is_playing() const
{
	return is_valid_media() && libvlc_media_player_is_playing(media_player);
}


bool Multimedia::is_paused()
{
	return paused;
}


void Multimedia::set_position(const float position) const
{
	if (is_valid_media()) {
		libvlc_media_player_set_time(media_player, position);
	}
}

float Multimedia::get_position() const
{
	if (is_valid_media()) {
		return libvlc_media_player_get_time(media_player);
	}
	return 0;
}

bool Multimedia::initialize()
{
	static const char *def_argv[] = {
		"-vv",
	};

	static const int def_argc = sizeof(def_argv) / sizeof(def_argv[0]);



		//vlc_instance = libvlc_new(def_argc, def_argv);
		vlc_instance = libvlc_new(0, NULL);
	

	if (vlc_instance == nullptr) {
		std::cout << "Error create new" << "vlc_instance" << std::endl;
		return false;
	}
	return true;
}



int Multimedia::play(const std::string &media_file)
{
	//error = -1;
   
	if (vlc_instance == nullptr) {
		std::cout << "vlc_instance == nullptr, media file : " << media_file << std::endl;
		return error;
	}

	libvlc_media_t *media =
	    libvlc_media_new_path(vlc_instance, media_file.c_str());
		

	if (media == nullptr) {
		std::cout << "media  == nullptr, media file : " << media_file << std::endl;
		return error;
	}

	if (is_valid_media()) {
		stop();
	}


	media_player = libvlc_media_player_new_from_media(media);

	if (media_player == nullptr) {
		std::cout << "media_player  == nullptr, media file : " << media_file << std::endl;
		return false;
	}

	libvlc_media_player_play(media_player);

	libvlc_media_release(media);
	//return 1;
}

void Multimedia::release()
{
	if (vlc_instance != nullptr) {
		libvlc_release(vlc_instance);
		vlc_instance = nullptr;
	}
}

