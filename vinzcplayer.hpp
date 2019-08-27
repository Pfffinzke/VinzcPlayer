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
    int read_json(string input_dir);
    std::string getResourcePath(const std::string& executableDir, const std::string& resourceRelativeName);
    void NextSong();
    int FirstSong();
    void NextSongPool();
    void Autoplay();
    int ChosenSong(int vote[]);
void New_folder_First_song();




