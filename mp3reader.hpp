#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "json/json.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <cstring>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;




	int getdir (string dir, vector<string> &files);
    int read_json();
    std::string getResourcePath(const std::string& executableDir, const std::string& resourceRelativeName);
    void togglePlayPause(sf::Music& music);
    int NextSong(sf::Music& music);
    void NextSongPool(sf::Music& music);
    void Autoplay(sf::Music& music);







