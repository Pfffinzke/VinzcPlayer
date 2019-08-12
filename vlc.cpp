#include "vlc.hpp"
#include <iostream>
#include <vlc/vlc.h>


libvlc_instance_t *vlc_instance = nullptr;
libvlc_media_player_t *media_player = nullptr;

bool paused = false;

vlc::vlc()
{
	
	initialize();
}

vlc::~vlc()
{

	stop();
	release();
}

void vlc::pause() const
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

void vlc::stop()
{
	if (is_valid_media()) {
		libvlc_media_player_stop(media_player);
		libvlc_media_player_release(media_player);
		media_player = nullptr;
	}
}


bool vlc::is_valid_media() const
{
	return (media_player != nullptr);
}

bool vlc::is_playing() const
{
	return is_valid_media() && libvlc_media_player_is_playing(media_player);
}


bool vlc::is_paused()
{
	return paused;
}


void vlc::set_position(const float position) const
{
	if (is_valid_media()) {
		libvlc_media_player_set_time(media_player, position);
	}
}

float vlc::get_position() const
{
	if (is_valid_media()) {
		return libvlc_media_player_get_time(media_player);
	}
	return 0;
}

bool vlc::initialize()
{
	static const char *def_argv[] = {
		"-vv",
	};

	static const int def_argc = sizeof(def_argv) / sizeof(def_argv[0]);



		//vlc_instance = libvlc_new(def_argc, def_argv);
		vlc_instance = libvlc_new(0, NULL);
	

	if (vlc_instance == nullptr) {
		std::cout << "Error creating new" << "vlc_instance" << std::endl;
		return false;
	}
	return true;
}



int vlc::play(const std::string &media_file)
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

void vlc::release()
{
	if (vlc_instance != nullptr) {
		libvlc_release(vlc_instance);
		vlc_instance = nullptr;
	}
}

