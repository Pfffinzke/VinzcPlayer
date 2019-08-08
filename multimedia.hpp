#pragma once


#include <algorithm>
#include <string>
#include <vector>
#include <vlc/vlc.h>
#include <vlc/libvlc_version.h>
#include <assert.h>


class Multimedia {
      public:

	Multimedia();

	~Multimedia();

      private:
	
	
	int window_id = 0;
	
	bool b_mute = false;
	int error = 0; // 0 == ok, -1 == errr

      private:
	/* VLC lifetime */
	
	void release();


      public:
	/* Actions */
	
	bool initialize();
	int play(const std::string &media_file);
	void set_volume(const int volume) const;
	int get_volume() const;
	bool is_playing() const;
	bool is_paused();

	void stop();
	void pause() const;
	void toggle_mute();
	void mute();
	bool is_mute() const;
	void set_position(const float position) const;
	float get_position() const;
	//	bool exist_audio();
	bool is_valid_media() const;

};
