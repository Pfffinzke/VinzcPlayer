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
	int play(const std::string &media_file);
	bool is_playing() const;
	bool is_paused();

	void stop();
	void pause() const;

	void set_position(const float position) const;
	float get_position() const;
	//	bool exist_audio();
	bool is_valid_media() const;

};
