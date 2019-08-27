#pragma once


#include <algorithm>
#include <string>
#include <vector>
#include <vlc/vlc.h>
#include <vlc/libvlc_version.h>
#include <assert.h>


class vlc {
      public:

	vlc();

	~vlc();

      private:
	
	
	int error = 0; // 0 == ok, -1 == errr

      private:
	/* VLC lifetime */
	
	void release();


      public:
	/* Actions */
	
	bool initialize();
	float get_lenght();
	float get_current_time();
	int play(const std::string &media_file);
	int get_volume();
	void set_volume(int volume);
	bool is_playing() const;
	bool is_paused();
  bool is_finished();
	void stop();
	void pause() const;

	void set_position(const float position) const;
	float get_position() const;
	//	bool exist_audio();
	bool is_valid_media() const;

};
