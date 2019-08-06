//mp3reader.cpp
//snags the tag off the last 128 bytes of an MP3 file.

#include "MP3tag.h"
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
#include <unistd.h>
#include <cmath>

#include "SoundFileReaderMp3.hpp"
#include "mp3reader.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cstdint>
#include <random>



using namespace std;


Json::Reader reader;
Json::Value root;
Json::StyledStreamWriter writer;

int number_song;
int played_song;
std::string filename;
int current_song;

int song_choice[4];
int vote[5]={0,0,0,0,0};

int next_chosen_song;

bool choice;
bool next_bool;
/*function... might want it in some class?*/




int getdir (string dir, vector<string> &files)
{
    
	DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
	if ( strstr( dirp->d_name, ".mp3" )){
            	cout <<  "found a .mp3 file:" << dirp->d_name << endl;
        	files.push_back(dir+string(dirp->d_name));
}
    }
    closedir(dp);
    return 0;
}


int read_json()
{
	int fileNameLength = 1024;
	int mp3TagSize = 128;
	TAGdata tagStruct;
	char  fileName[fileNameLength+1];
	ifstream mp3File;
	char buffer[mp3TagSize+1];
	//first step : list all mp3 files in given directory
 	string dir = string("./music/");
    vector<string> files = vector<string>();

    getdir(dir,files);

	//Start loop

	//Second step : list all these files in Json

  	std::string text = "{\"ID\": \"0\",\"path\": \"James\", \"Song Name\": \"Bond\", \"artist\": \"yop\" , \"play\": 0 }\n";
  	std::ofstream outFile;
	outFile.open("output.json");
	
	for (unsigned int i = 0;i < files.size();i++) {

		std::cout << "Start loop" << std::endl;
 		// Parse JSON and print errors if needed
  		if(!reader.parse(text, root[i])) {
    		std::cout << reader.getFormattedErrorMessages();
			std::cout << "parse error" << std::endl;
    		exit(1);
  		} else {
    			// Read and modify the json data
				root[i]["ID"] = i;
				root[i]["path"] = files[i] ;
				
				//root[i]["path"] += files[i];
				//Third step parse ID3 tags to fill JSON	
				//stringstream s;	
				//s << "./music/" << files[i].c_str();
				//fileName = char("./music/" + files[i].c_str());
				cout << root[i]["path"] << endl;
				mp3File.open(files[i].c_str());
				buffer[mp3TagSize] = '\0';  //just a precaution
				
				if (mp3File){
					cout << "Successful open of " << files[i].c_str() << endl;
				}else{
					cerr << "***Aborting*** Could not open " << files[i].c_str() << endl;
					return 1;
				}

				//move file pointer to 128 bytes off the end of the file.
				mp3File.seekg(-(mp3TagSize),ios::end);
				
				if(mp3File.fail()){
					cerr << "Could not seek, Aborting " << endl;
					return 1;
				}
 
				//get the data
				mp3File.read(reinterpret_cast <char *>(&tagStruct),mp3TagSize);
	
				if(mp3File.fail()){
					cerr << "Could not read after seeking, Aborting " << endl;
					return 1;
				}
				mp3File.close();

				root[i]["Song Name"] = tagStruct.title ;
    			root[i]["artist"] = tagStruct.artist;
				root[i]["play"] = 0;
				root[i]["choice"] = 0;

    			// Write the output to a file
    			
    			writer.write(outFile, root[i]);
  			
  				}
	}
	 outFile.close();

// fourth step chose a random song to play


	return 0;
}

std::string getResourcePath(const std::string& executableDir, const std::string& resourceRelativeName) {
	// windows users may have to change "/" for "\"
	std::size_t pos = executableDir.find_last_of("/");
	std::string prefix = executableDir.substr (0, pos+1);
	return prefix + resourceRelativeName;
}

void togglePlayPause(sf::Music& music) {
	sf::SoundSource::Status musicStatus = music.getStatus();
	if(musicStatus == sf::SoundSource::Status::Paused) {
		music.play();
		std::cout << "music play" << std::endl;
	} else if(musicStatus == sf::SoundSource::Status::Playing) {
		music.pause();
		std::cout << "music pause" << std::endl;
	}
}

int FirstSong(sf::Music& music) {
	music.pause();
	std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, number_song);
	int first_random_song = distr(eng);

	while ((root[first_random_song]["play"]==1)&&(played_song<=number_song)){
	first_random_song = distr(eng);
	}
	current_song = first_random_song;
	filename = root[current_song]["path"].asString();

	if (!music.openFromFile(filename)) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
	}
	root[current_song]["play"] = 1;
	music.play();
	
	played_song++;
	NextSongPool(music);

	return 1;

}

int NextSong(sf::Music& music) {
	music.pause();
	next_bool=false;
	current_song = song_choice[next_chosen_song];
	filename = root[current_song]["path"].asString();
	if (!music.openFromFile(filename)) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
	}
	
	root[current_song]["play"] = 1;
	music.play();
	// chosen song is played reset vote
			vote[1]=0;
			vote[2]=0;
			vote[3]=0;
			vote[4]=0;
	played_song++;
	NextSongPool(music);

return 1;

}


void NextSongPool(sf::Music& music) {
	std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, number_song-1);
		
	for (unsigned int i = 1;i <= 4;i++) {

		int next_random_song = distr(eng);
		// as long as we don't find a non played song or non selected song, we random the next song
		while (((root[next_random_song]["play"]==1)||(root[next_random_song]["choice"]==1))&&(played_song+3<=number_song)){
			
			next_random_song = distr(eng);
		}
	
		song_choice[i] = next_random_song;
		root[song_choice[i]]["choice"] = 1;
		//next_random_song = distr(eng);

	}

	for (unsigned int i = 0;i <= number_song;i++) {
		//reset chosen songs for next call
		root[i]["choice"]=0;
	}

	return;
	
}

void Autoplay(sf::Music& music) {

	if (music.getStatus()!=sf::Music::Status::Playing)
	NextSong(music);
	
}


int ChosenSong(int vote[]){
	int counter = 1;
    int largest = 0;
	int next_song;

    while (counter <= 4)
    {


        if (largest < vote[counter])
        {
            largest = vote[counter];
			next_song=counter;
        }
		counter++;
    }
	//in case no song have been voted for, we take the first one
	if (largest == 0)
	{
	
        largest = 1;
		next_song=1;
    }

	counter=0;
	return next_song;
}

int main() {

	read_json();
	number_song = root.size();
	std::cout << "number of songs: " << number_song << std::endl;



	//filename = root[distr(eng)]["path"].asString();

	std::cout << "START: " << filename << std::endl;
	// Register our custom sf::SoundFileReader for mp3 files.
	// This is the preferred way of adding support for a new audio format.
	// Other formats will be handled by their respective readers.
	sf::SoundFileFactory::registerReader<audio::SoundFileReaderMp3>();

	// Load a music to play
	sf::Music music;



	if (!FirstSong(music)) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
	}

	
	sf::Time song_lenght = music.getDuration();


	
	/*if (!music.openFromFile(filename)) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
	}*/

	// Now we are sure we can play the file
	// Set up the main window

	sf::Sprite sprite;
	sf::Texture texture;
	sf::Vector2u dimensions;

	// See if we can find the image
	
		texture.loadFromFile("WWDIYJimmyTArt.png");
		sprite.setTexture(texture);
	//	dimensions = texture.getSize();

		dimensions.x = 1200;
		dimensions.y = 800;
	

	// Create the main window
	sf::RenderWindow window(sf::VideoMode(dimensions.x, dimensions.y), "SFML window");

		//get information about the joystick
	sf::Joystick::Identification id = sf::Joystick::getIdentification(0);
	std::cout << "\nVendor ID: " << id.vendorId << "\nProduct ID: " << id.productId << std::endl;
	sf::String controller("Joystick Use: " + id.name);
	window.setTitle(controller);//easily tells us what controller is connected

	// Start the music
	//music.play();
	bool choice;

	// Start the game loop
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		Autoplay(music);
		sf::Time elapsed = music.getPlayingOffset();
		sf::Time start_voting_time = sf::seconds(10.0f);
		sf::Time end_voting_time = sf::seconds(30.0f);
		//std::cout << "song time. " << song_lenght.asSeconds() << "remaining time :  " << elapsed.asSeconds() << std::endl;


		// propose 4 choices
		if((elapsed.asSeconds()>=start_voting_time.asSeconds())&&(elapsed.asSeconds()<=end_voting_time.asSeconds())) {
			choice = true;
			}
			else 
			{
			choice = false;

			}
		if((elapsed.asSeconds()>=30)&&(elapsed.asSeconds()<=40)) {

			next_chosen_song = ChosenSong(vote);
			next_bool=true;
			}

		//next_song_pool(music);	
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			} else if(event.type == sf::Event::KeyPressed) {
				// the user interface: SPACE pauses and plays, ESC quits
				switch (event.key.code) {
					case sf::Keyboard::Space:
						togglePlayPause(music);
						break;
					case sf::Keyboard::Escape:
						window.close();
						break;
					case sf::Keyboard::N:
						if (!NextSong(music)) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
					}
						break;

					case sf::Keyboard::A:
						if (choice) {
							vote[1]++;
					}
						break;

					case sf::Keyboard::Z:
						if (choice) {
							vote[2]++;
					}
						break;

					case sf::Keyboard::E:
						if (choice) {
							vote[3]++;
					}
						break;

					case sf::Keyboard::R:
						if (choice) {
							vote[4]++;
					}
						break;

					default:
						break;
				}
			}
			else if(event.type == sf::Event::JoystickButtonPressed) {
				switch (event.joystickButton.button) {
					case 0 :
						if (choice) {
							vote[1]++;
						}
						break;

					case 1 :
						if (choice) {
							vote[2]++;
						}
						break;

					case 2 :
						if (choice) {
							vote[3]++;
						}
						break;

					case 3 :
						if (choice) {
							vote[4]++;
						}
						break;

					case 4:
						if (!NextSong(music)) {
							std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
							return EXIT_FAILURE;
						}
						break;

					default:
						break;
				}
			


			}
		}


		// Clear screen
		window.clear();
		// Draw the sprite
		window.draw(sprite);
		sf::Font font;
		font.loadFromFile("ALBA.ttf");
		sf::Font font_neon;
		font_neon.loadFromFile("Neon.ttf");
		float thickness = 4.f;
		

		sf::Text Title;
		sf::Text Artist;
		sf::Text CurrentText;
		sf::String CurrentSongText = root[current_song]["Song Name"].asString();
		sf::String CurrentArtistText = root[current_song]["artist"].asString();
		//Title.setOutlineColor(sf::Color::White);
    	//Title.setOutlineThickness(thickness);
		//Artist.setOutlineColor(sf::Color::White);
    	//Artist.setOutlineThickness(thickness);
		//CurrentText.setOutlineColor(sf::Color::White);
    	//CurrentText.setOutlineThickness(thickness);


		Artist.setFont(font_neon);
		Artist.setString(CurrentArtistText);
		Artist.setColor(sf::Color::White);
		Artist.setPosition(700.0f, 70.0f);
		Artist.setCharacterSize(24);
		Artist.setStyle(sf::Text::Bold);

		Title.setFont(font_neon);
		Title.setString(CurrentSongText);
		Title.setColor(sf::Color::White);
		Title.setPosition(700.0f, 90.0f);
		Title.setCharacterSize(24);
		Title.setStyle(sf::Text::Bold);
		
		CurrentText.setFont(font);
		CurrentText.setString("Current playing song");
		CurrentText.setColor(sf::Color::Red);
		CurrentText.setPosition(700.0f, 45.0f);
		CurrentText.setCharacterSize(26);
		CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		window.draw(Artist);
		window.draw(Title);
		window.draw(CurrentText);
		

		sf::Text Choice1_Title;
		sf::Text Choice1_Artist;
		sf::Text Choice1_CurrentText;
		sf::Text Choice1_Vote;

		sf::Text Choice2_Title;
		sf::Text Choice2_Artist;
		sf::Text Choice2_CurrentText;
		sf::Text Choice2_Vote;

		sf::Text Choice3_Title;
		sf::Text Choice3_Artist;
		sf::Text Choice3_CurrentText;
		sf::Text Choice3_Vote;

		sf::Text Choice4_Title;
		sf::Text Choice4_Artist;
		sf::Text Choice4_CurrentText;
		sf::Text Choice4_Vote;

		sf::Text Vote_CurrentText;
		sf::Text Vote_Time;

		sf::Text Next_Title;
		sf::Text Next_Artist;
		sf::Text Next_CurrentText;

		// update Choices info


		Choice1_CurrentText.setFont(font);
		Choice1_CurrentText.setString("Next song 1");
		Choice1_CurrentText.setColor(sf::Color::Green);
		Choice1_CurrentText.setPosition(50.0f, 345.0f);
		Choice1_CurrentText.setCharacterSize(26);
		Choice1_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice1_Artist.setFont(font_neon);
		Choice1_Artist.setString(root[song_choice[1]]["artist"].asString());
		Choice1_Artist.setColor(sf::Color::Blue);
		Choice1_Artist.setPosition(50.0f, 370.0f);
		Choice1_Artist.setCharacterSize(24);
		Choice1_Artist.setStyle(sf::Text::Bold);

		Choice1_Title.setFont(font_neon);
		Choice1_Title.setString(root[song_choice[1]]["Song Name"].asString());
		Choice1_Title.setColor(sf::Color::Blue);
		Choice1_Title.setPosition(50.0f, 390.0f);
		Choice1_Title.setCharacterSize(24);
		Choice1_Title.setStyle(sf::Text::Bold);

		Choice1_Vote.setFont(font_neon);
		Choice1_Vote.setString(std::to_string(vote[1]));
		Choice1_Vote.setColor(sf::Color::Blue);
		Choice1_Vote.setPosition(50.0f, 430.0f);
		Choice1_Vote.setCharacterSize(20+(vote[1]/2));
		Choice1_Vote.setStyle(sf::Text::Bold);

		Choice2_CurrentText.setFont(font);
		Choice2_CurrentText.setString("Next song 2");
		Choice2_CurrentText.setColor(sf::Color::Green);
		Choice2_CurrentText.setPosition(300.0f, 345.0f);
		Choice2_CurrentText.setCharacterSize(26);
		Choice2_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice2_Artist.setFont(font_neon);
		Choice2_Artist.setString(root[song_choice[2]]["artist"].asString());
		Choice2_Artist.setColor(sf::Color::Blue);
		Choice2_Artist.setPosition(300.0f, 370.0f);
		Choice2_Artist.setCharacterSize(24);
		Choice2_Artist.setStyle(sf::Text::Bold);

		Choice2_Title.setFont(font_neon);
		Choice2_Title.setString(root[song_choice[2]]["Song Name"].asString());
		Choice2_Title.setColor(sf::Color::Blue);
		Choice2_Title.setPosition(300.0f, 390.0f);
		Choice2_Title.setCharacterSize(24);
		Choice2_Title.setStyle(sf::Text::Bold);

		Choice2_Vote.setFont(font_neon);
		Choice2_Vote.setString(std::to_string(vote[2]));
		Choice2_Vote.setColor(sf::Color::Blue);
		Choice2_Vote.setPosition(300.0f, 430.0f);
		Choice2_Vote.setCharacterSize(20+(vote[2]/2));
		Choice2_Vote.setStyle(sf::Text::Bold);

		Choice3_CurrentText.setFont(font);
		Choice3_CurrentText.setString("Next song 3");
		Choice3_CurrentText.setColor(sf::Color::Green);
		Choice3_CurrentText.setPosition(550.0f, 345.0f);
		Choice3_CurrentText.setCharacterSize(26);
		Choice3_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice3_Artist.setFont(font_neon);
		Choice3_Artist.setString(root[song_choice[3]]["artist"].asString());
		Choice3_Artist.setColor(sf::Color::Blue);
		Choice3_Artist.setPosition(550.0f, 370.0f);
		Choice3_Artist.setCharacterSize(24);
		Choice3_Artist.setStyle(sf::Text::Bold);

		Choice3_Title.setFont(font_neon);
		Choice3_Title.setString(root[song_choice[3]]["Song Name"].asString());
		Choice3_Title.setColor(sf::Color::Blue);
		Choice3_Title.setPosition(550.0f, 390.0f);
		Choice3_Title.setCharacterSize(24);
		Choice3_Title.setStyle(sf::Text::Bold);

		Choice3_Vote.setFont(font_neon);
		Choice3_Vote.setString(std::to_string(vote[3]));
		Choice3_Vote.setColor(sf::Color::Blue);
		Choice3_Vote.setPosition(550.0f, 430.0f);
		Choice3_Vote.setCharacterSize(20+(vote[3]/2));
		Choice3_Vote.setStyle(sf::Text::Bold);

		Choice4_CurrentText.setFont(font);
		Choice4_CurrentText.setString("Next song 4");
		Choice4_CurrentText.setColor(sf::Color::Green);
		Choice4_CurrentText.setPosition(800.0f, 345.0f);
		Choice4_CurrentText.setCharacterSize(26);
		Choice4_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice4_Artist.setFont(font_neon);
		Choice4_Artist.setString(root[song_choice[4]]["artist"].asString());
		Choice4_Artist.setColor(sf::Color::Blue);
		Choice4_Artist.setPosition(800.0f, 370.0f);
		Choice4_Artist.setCharacterSize(24);
		Choice4_Artist.setStyle(sf::Text::Bold);

		Choice4_Title.setFont(font_neon);
		Choice4_Title.setString(root[song_choice[4]]["Song Name"].asString());
		Choice4_Title.setColor(sf::Color::Blue);
		Choice4_Title.setPosition(800.0f, 390.0f);
		Choice4_Title.setCharacterSize(24);
		Choice4_Title.setStyle(sf::Text::Bold);

		Choice4_Vote.setFont(font_neon);
		Choice4_Vote.setString(std::to_string(vote[4]));
		Choice4_Vote.setColor(sf::Color::Blue);
		Choice4_Vote.setPosition(800.0f, 430.0f);
		Choice4_Vote.setCharacterSize(20+(vote[4]/2));
		Choice4_Vote.setStyle(sf::Text::Bold);


		// next song :
	
		Next_CurrentText.setFont(font);
		Next_CurrentText.setString("Next song");
		Next_CurrentText.setColor(sf::Color::Green);
		Next_CurrentText.setPosition(600.0f, 650.0f);
		Next_CurrentText.setCharacterSize(26);
		Next_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Next_Artist.setFont(font_neon);
		Next_Artist.setString(root[song_choice[next_chosen_song]]["artist"].asString());
		Next_Artist.setColor(sf::Color::White);
		Next_Artist.setPosition(600.0f, 670.0f);
		Next_Artist.setCharacterSize(24);
		Next_Artist.setStyle(sf::Text::Bold);

		Next_Title.setFont(font_neon);
		Next_Title.setString(root[song_choice[next_chosen_song]]["Song Name"].asString());
		Next_Title.setColor(sf::Color::White);
		Next_Title.setPosition(600.0f, 690.0f);
		Next_Title.setCharacterSize(24);
		Next_Title.setStyle(sf::Text::Bold);
	

		// print option text
	if(choice==true){
		// set voting time
		Vote_CurrentText.setFont(font_neon);
		Vote_CurrentText.setString("TIME TO VOTE");
		Vote_CurrentText.setColor(sf::Color::Green);
		Vote_CurrentText.setPosition(700.0f, 250.0f);
		Vote_CurrentText.setCharacterSize(28);
		Vote_CurrentText.setStyle(sf::Text::Bold);

		Vote_Time.setFont(font_neon);
		Vote_Time.setString(std::to_string((int)(end_voting_time.asSeconds()-elapsed.asSeconds())));
		Vote_Time.setColor(sf::Color::Red);
		Vote_Time.setPosition(750.0f, 300.0f);
		Vote_Time.setCharacterSize(44);
		Vote_Time.setStyle(sf::Text::Bold);

		//display text
		window.draw(Choice1_Artist);
		window.draw(Choice1_Title);
		window.draw(Choice1_CurrentText);
		window.draw(Choice1_Vote);
		window.draw(Choice2_Artist);
		window.draw(Choice2_Title);
		window.draw(Choice2_CurrentText);
		window.draw(Choice2_Vote);
		window.draw(Choice3_Artist);
		window.draw(Choice3_Title);
		window.draw(Choice3_CurrentText);
		window.draw(Choice3_Vote);
		window.draw(Choice4_Artist);
		window.draw(Choice4_Title);
		window.draw(Choice4_CurrentText);
		window.draw(Choice4_Vote);
		window.draw(Vote_CurrentText);
		window.draw(Vote_Time);
	}
	// print next song
	if(next_bool==true){

		window.draw(Next_Artist);
		window.draw(Next_Title);
		window.draw(Next_CurrentText);
		}
		
		// Update the window
		window.display();
	}

	return EXIT_SUCCESS;
}

