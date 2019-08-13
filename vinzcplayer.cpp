//mp3reader.cpp
//snags the tag off the last 128 bytes of an MP3 file.

#include "MP3tag.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <jsoncpp/json/json.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <cmath>
#include "id3.h"
#include "tagHelper.h" 

#include "vlc.hpp"
#include "vinzcplayer.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cstdint>
#include <random>



using namespace std;

static vlc *multimedia = nullptr;
Json::Reader reader;
Json::Value root;
Json::StyledStreamWriter writer;

int number_song;
int played_song;
std::string filename;
std::string menu_path;
int current_song;

int song_choice[4];
int vote[5]={0,0,0,0,0};

int next_chosen_song;

bool choice;
bool menu;
bool next_bool;
bool b_parsing;

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


int read_json(string input_dir)
{
	//TAGdata tagStruct;
	//char  fileName[fileNameLength+1];
	//ifstream mp3File;
	b_parsing = true;
	//first step : list all mp3 files in given directory
 	string dir = string("./"+input_dir+"/");
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
				
				//Third step parse ID3 tags to fill JSON	

				cout << root[i]["path"] << endl;

				tagHelper th(const_cast<char*>(files[i].c_str()));

    			uint32 length;
    			length = th.getTrackLength();
    			printf("The track length is: %isec\n", length);
				root[i]["lenght"] = length;

    			char* title;
    			title = th.getValue(ID3FID_TITLE);
    			printf("The title of the song is: %s\n", title);
				root[i]["Song Name"] = title ;
    			delete title; //always remember to delete this

    			char* artist;
    			artist = th.getValue(ID3FID_LEADARTIST);
    			printf("The artist of the song is: %s\n", artist);
				root[i]["artist"] = artist;
    			delete artist; //always remember to delete this

				root[i]["play"] = 0;
				root[i]["choice"] = 0;

    			// Write the output to a file
    			
    			writer.write(outFile, root[i]);
  			
  				}
	}
	number_song = root.size();
	b_parsing = false;
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


int FirstSong() {
	//music.pause();
	std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, number_song);
	int first_random_song = distr(eng);
	while ((root[first_random_song]["play"]==1)&&(played_song<=number_song)){
	first_random_song = distr(eng);
	}
	current_song = first_random_song;
	filename = root[current_song]["path"].asString();
	std::cout << "first song filename" << filename << std::endl;
	if (!multimedia->initialize())
		return -1;
std::cout << "first song init ok" << std::endl;
	root[current_song]["play"] = 1;
	std::cout << "first song call vlc play" << std::endl;
	multimedia->play(filename);
	
	std::cout << "first song is vlc playing" << std::endl;
	played_song++;
	NextSongPool();

	return 1;

}

int NextSong() {
	multimedia->stop();
	next_bool=false;
	current_song = song_choice[next_chosen_song];
	filename = root[current_song]["path"].asString();

	std::cout << "START: " << filename << std::endl;
	
	
	root[current_song]["play"] = 1;
	multimedia->play(filename);
	
	// chosen song is played reset vote
			vote[1]=0;
			vote[2]=0;
			vote[3]=0;
			vote[4]=0;
	played_song++;
	NextSongPool();

return 1;

}


void NextSongPool() {
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

void Autoplay() {

	if ((!multimedia->is_playing())&&!multimedia->is_paused()){
		std::cout << "START: AUTOPLAY "  << std::endl;
		NextSong();
	}
	
	
	
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
	menu=true;
	menu_path = "intro";
	//read_json(argv[1]);
	read_json(menu_path);
	
	std::cout << "number of songs: " << number_song << std::endl;
	b_parsing=true;


	//filename = root[distr(eng)]["path"].asString();

	std::cout << "START: " << filename << std::endl;
	// Register our custom sf::SoundFileReader for mp3 files.
	// This is the preferred way of adding support for a new audio format.
	// Other formats will be handled by their respective readers.
	

	// Load a music to play
	//sf::Music music;





	
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

		dimensions.x = 800;
		dimensions.y = 600;
	

	// Create the main window
	sf::RenderWindow window(sf::VideoMode(dimensions.x, dimensions.y), "SFML window");

		//get information about the joystick
	sf::Joystick::Identification id = sf::Joystick::getIdentification(0);
	std::cout << "\nVendor ID: " << id.vendorId << "\nProduct ID: " << id.productId << std::endl;
	sf::String controller("Joystick Use: " + id.name);
	
	// Start the music
	bool choice;
	float first_line = 100.0;
	if (!FirstSong()) {
		std::cout << "check your file path. also only wav, flac, ogg and mp3 are supported." << std::endl;
		return EXIT_FAILURE;
	}

	sf::Texture texLogo;
	texLogo.loadFromFile("vinzcplayer-2.png");
	// Create a sprite
	sf::Sprite spriteLogo;
	spriteLogo.setTexture(texLogo);
	spriteLogo.setPosition(200, 20);



// Start the Menu loop
	while (window.isOpen())
	{
		if (menu){
		// Process events
		sf::Event event;
		

		
		//path choice(music);	
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed) {
				window.close();
			} else if(event.type == sf::Event::KeyPressed) {
				// the user interface: SPACE pauses and plays, ESC quits
				switch (event.key.code) {
					case sf::Keyboard::F1:
					menu_path = "hiphop";
					read_json(menu_path);
					
						break;
					case sf::Keyboard::F2:
					menu_path = "disco";
					read_json(menu_path);
					
						break;
					case sf::Keyboard::Escape:
					std::cout << "pressspace - escape" << std::endl;
						window.close();
						break;
					case sf::Keyboard::Enter:
					NextSong();
					menu = false;
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
		float thickness = 3.f;
		
	sf::Texture texButton;
	texButton.loadFromFile("boutton.png");
	// Create a sprite
	
	float end_line = 400.0;
	sf::Sprite spriteButton1;
	spriteButton1.setTexture(texButton);
	spriteButton1.setPosition(100, first_line);
	spriteButton1.scale(0.8,0.8);

	sf::Sprite spriteButton2;
	spriteButton2.setTexture(texButton);
	spriteButton2.setPosition(400, first_line);
	spriteButton2.scale(0.8,0.8);
	

	window.draw(spriteButton1);
	window.draw(spriteButton2);

		sf::Text Welcome;
		sf::Text Path_Choice1;
		sf::Text Path_Choice2;
		sf::Text Path_Selected;
		sf::Text Confirm;
		sf::Text Parsing;


		// update Choices info
		Welcome.setFont(font);
		Welcome.setString("Welcome");
		Welcome.setColor(sf::Color::Blue);
		Welcome.setOutlineColor(sf::Color::Black);
		Welcome.setOutlineThickness(thickness);
		Welcome.setPosition(300.0f, 50.0f);
		Welcome.setCharacterSize(45);
		Welcome.setStyle(sf::Text::Bold);

		Path_Choice1.setFont(font);
		Path_Choice1.setString("F1 : Hip-Hop");
		Path_Choice1.setColor(sf::Color::Green);
		Path_Choice1.setOutlineColor(sf::Color::Black);
		Path_Choice1.setOutlineThickness(thickness);
		Path_Choice1.setPosition(135.0f, first_line+10);
		Path_Choice1.setCharacterSize(30);
		Path_Choice1.setStyle(sf::Text::Bold );

		Path_Choice2.setFont(font);
		Path_Choice2.setString("F2 : Disco-Funk");
		Path_Choice2.setColor(sf::Color::Green);
		Path_Choice2.setOutlineColor(sf::Color::Black);
		Path_Choice2.setOutlineThickness(thickness);
		Path_Choice2.setPosition(410.0f, first_line+10);
		Path_Choice2.setCharacterSize(30);
		Path_Choice2.setStyle(sf::Text::Bold);

		Path_Selected.setFont(font_neon);
		Path_Selected.setString(menu_path);
		Path_Selected.setColor(sf::Color::White);
		Path_Selected.setOutlineColor(sf::Color::Black);
		Path_Selected.setOutlineThickness(thickness);
		Path_Selected.setPosition(120.0f, end_line);
		Path_Selected.setCharacterSize(24);
		Path_Selected.setStyle(sf::Text::Bold);

		Parsing.setFont(font_neon);

		if (b_parsing)
		{
			Parsing.setString("Wait...");
			Parsing.setColor(sf::Color::Red);
		}
		else
		{
			Parsing.setString("OK");
			Parsing.setColor(sf::Color::Green);
		}
		
		Parsing.setOutlineColor(sf::Color::Black);
		Parsing.setOutlineThickness(thickness);
		Parsing.setPosition(200.0f, end_line);
		Parsing.setCharacterSize(24);
		Parsing.setStyle(sf::Text::Bold);

		Confirm.setFont(font_neon);
		Confirm.setString("Press Enter to confirm");
		Confirm.setColor(sf::Color::Green);
		Confirm.setOutlineColor(sf::Color::Black);
		Confirm.setOutlineThickness(thickness);
		Confirm.setPosition(120.0f, end_line+25);
		Confirm.setCharacterSize(24);
		Confirm.setStyle(sf::Text::Bold);

		//draw text
		window.draw(Welcome);
		window.draw(Path_Choice1);
		window.draw(Path_Choice2);
		window.draw(Path_Selected);
		window.draw(Parsing);
		window.draw(Confirm);
		window.draw(spriteLogo);
		
		// Update the window0
		window.display();
	}

	// Start the game loop
	else
	{
		// Process events
		sf::Event event;
		
		//Autoplay();

		//elapsed time is a % from 0.0 to 1.0
		float elapsed  = multimedia->get_position()/1000;

		float start_voting_time = 10;
		float end_voting_time = 30;
		//std::cout << "song time. " << song_lenght.asSeconds() << "remaining time :  " << elapsed.asSeconds() << std::endl;


		// propose 4 choices
		if((elapsed >= start_voting_time)&&(elapsed <= end_voting_time)) {
			choice = true;
			}
			else 
			{
			choice = false;

			}
		if((elapsed >= end_voting_time)&&(elapsed<=end_voting_time+1)) {

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
					std::cout << "pressspace" << std::endl;
						multimedia->pause();
						break;
					case sf::Keyboard::Escape:
					std::cout << "pressspace - escape" << std::endl;
						window.close();
						break;
					case sf::Keyboard::N:
						std::cout << "pressspace - nextsong" << std::endl;
						if (!NextSong()) {
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

					case sf::Keyboard::M:
						menu = true;
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
						if (!NextSong()) {
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

		Artist.setFont(font_neon);
		Artist.setString(CurrentArtistText);
		Artist.setColor(sf::Color::White);
		Artist.setPosition(400.0f, first_line-15);
		Artist.setCharacterSize(24);
		Artist.setStyle(sf::Text::Bold);
		//Artist.setOutlineColor(sf::Color::Black);
		//Artist.setOutlineThickness(3.0f);

		Title.setFont(font_neon);
		Title.setString(CurrentSongText);
		Title.setColor(sf::Color::White);
		Title.setPosition(400.0f, first_line+10);
		Title.setCharacterSize(24);
		Title.setStyle(sf::Text::Bold);
		
		CurrentText.setFont(font);
		CurrentText.setString("Current playing song");
		CurrentText.setColor(sf::Color::Cyan);
		CurrentText.setPosition(400.0f, first_line-40);
		CurrentText.setCharacterSize(26);
		CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		window.draw(Artist);
		window.draw(Title);
		window.draw(CurrentText);
		window.draw(spriteLogo);
		

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
		Choice1_CurrentText.setPosition(400.0f, 150.0f);
		Choice1_CurrentText.setCharacterSize(26);
		Choice1_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice1_Artist.setFont(font_neon);
		Choice1_Artist.setString(root[song_choice[1]]["artist"].asString());
		Choice1_Artist.setColor(sf::Color::White);
		Choice1_Artist.setPosition(400.0f, 180.0f);
		Choice1_Artist.setCharacterSize(24);
		Choice1_Artist.setStyle(sf::Text::Bold);

		Choice1_Title.setFont(font_neon);
		Choice1_Title.setString(root[song_choice[1]]["Song Name"].asString());
		Choice1_Title.setColor(sf::Color::White);
		Choice1_Title.setPosition(400.0f, 200.0f);
		Choice1_Title.setCharacterSize(24);
		Choice1_Title.setStyle(sf::Text::Bold);

		Choice1_Vote.setFont(font_neon);
		Choice1_Vote.setString(std::to_string(vote[1]));
		Choice1_Vote.setColor(sf::Color::Yellow);
		Choice1_Vote.setPosition(700.0f, 150.0f);
		Choice1_Vote.setCharacterSize(20+(vote[1]/2));
		Choice1_Vote.setStyle(sf::Text::Bold);

		sf::Sprite spriteChoice1_Vote;
		spriteChoice1_Vote.setTextureRect(sf::IntRect(10, 10, 50, 30));
		spriteChoice1_Vote.setColor(sf::Color(255, 0, 255, 200));
		spriteChoice1_Vote.setPosition(100, 25);

		sf::RectangleShape rectanglevote1(sf::Vector2f(120, -(5+vote[1])));
		rectanglevote1.setFillColor(sf::Color(50, 50, 250));

		rectanglevote1.setPosition(100, 525);

		

		Choice2_CurrentText.setFont(font);
		Choice2_CurrentText.setString("Next song 2");
		Choice2_CurrentText.setColor(sf::Color::Green);
		Choice2_CurrentText.setPosition(400.0f, 250.0f);
		Choice2_CurrentText.setCharacterSize(26);
		Choice2_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice2_Artist.setFont(font_neon);
		Choice2_Artist.setString(root[song_choice[2]]["artist"].asString());
		Choice2_Artist.setColor(sf::Color::White);
		Choice2_Artist.setPosition(400.0f, 280.0f);
		Choice2_Artist.setCharacterSize(24);
		Choice2_Artist.setStyle(sf::Text::Bold);

		Choice2_Title.setFont(font_neon);
		Choice2_Title.setString(root[song_choice[2]]["Song Name"].asString());
		Choice2_Title.setColor(sf::Color::White);
		Choice2_Title.setPosition(400.0f, 300.0f);
		Choice2_Title.setCharacterSize(24);
		Choice2_Title.setStyle(sf::Text::Bold);

		Choice2_Vote.setFont(font_neon);
		Choice2_Vote.setString(std::to_string(vote[2]));
		Choice2_Vote.setColor(sf::Color::Yellow);
		Choice2_Vote.setPosition(700.0f, 250.0f);
		Choice2_Vote.setCharacterSize(20+(vote[2]/2));
		Choice2_Vote.setStyle(sf::Text::Bold);

		Choice3_CurrentText.setFont(font);
		Choice3_CurrentText.setString("Next song 3");
		Choice3_CurrentText.setColor(sf::Color::Green);
		Choice3_CurrentText.setPosition(400.0f, 350.0f);
		Choice3_CurrentText.setCharacterSize(26);
		Choice3_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice3_Artist.setFont(font_neon);
		Choice3_Artist.setString(root[song_choice[3]]["artist"].asString());
		Choice3_Artist.setColor(sf::Color::White);
		Choice3_Artist.setPosition(400.0f, 380.0f);
		Choice3_Artist.setCharacterSize(24);
		Choice3_Artist.setStyle(sf::Text::Bold);

		Choice3_Title.setFont(font_neon);
		Choice3_Title.setString(root[song_choice[3]]["Song Name"].asString());
		Choice3_Title.setColor(sf::Color::White);
		Choice3_Title.setPosition(400.0f, 400.0f);
		Choice3_Title.setCharacterSize(24);
		Choice3_Title.setStyle(sf::Text::Bold);

		Choice3_Vote.setFont(font_neon);
		Choice3_Vote.setString(std::to_string(vote[3]));
		Choice3_Vote.setColor(sf::Color::Yellow);
		Choice3_Vote.setPosition(700.0f, 350.0f);
		Choice3_Vote.setCharacterSize(20+(vote[3]/2));
		Choice3_Vote.setStyle(sf::Text::Bold);

		Choice4_CurrentText.setFont(font);
		Choice4_CurrentText.setString("Next song 4");
		Choice4_CurrentText.setColor(sf::Color::Green);
		Choice4_CurrentText.setPosition(400.0f, 450.0f);
		Choice4_CurrentText.setCharacterSize(26);
		Choice4_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Choice4_Artist.setFont(font_neon);
		Choice4_Artist.setString(root[song_choice[4]]["artist"].asString());
		Choice4_Artist.setColor(sf::Color::White);
		Choice4_Artist.setPosition(400.0f, 480.0f);
		Choice4_Artist.setCharacterSize(24);
		Choice4_Artist.setStyle(sf::Text::Bold);

		Choice4_Title.setFont(font_neon);
		Choice4_Title.setString(root[song_choice[4]]["Song Name"].asString());
		Choice4_Title.setColor(sf::Color::White);
		Choice4_Title.setPosition(400.0f, 500.0f);
		Choice4_Title.setCharacterSize(24);
		Choice4_Title.setStyle(sf::Text::Bold);

		Choice4_Vote.setFont(font_neon);
		Choice4_Vote.setString(std::to_string(vote[4]));
		Choice4_Vote.setColor(sf::Color::Yellow);
		Choice4_Vote.setPosition(700.0f, 450.0f);
		Choice4_Vote.setCharacterSize(20+(vote[4]/2));
		Choice4_Vote.setStyle(sf::Text::Bold);


		// next song :
	
		Next_CurrentText.setFont(font);
		Next_CurrentText.setString("Next song");
		Next_CurrentText.setColor(sf::Color::Green);
		Next_CurrentText.setPosition(400.0f, 150.0f);
		Next_CurrentText.setCharacterSize(26);
		Next_CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);

		Next_Artist.setFont(font_neon);
		Next_Artist.setString(root[song_choice[next_chosen_song]]["artist"].asString());
		Next_Artist.setColor(sf::Color::White);
		Next_Artist.setPosition(400.0f, 180.0f);
		Next_Artist.setCharacterSize(24);
		Next_Artist.setStyle(sf::Text::Bold);

		Next_Title.setFont(font_neon);
		Next_Title.setString(root[song_choice[next_chosen_song]]["Song Name"].asString());
		Next_Title.setColor(sf::Color::White);
		Next_Title.setPosition(400.0f, 200.0f);
		Next_Title.setCharacterSize(24);
		Next_Title.setStyle(sf::Text::Bold);
	

		// print option text
	if(choice==true){
		// set voting time
		Vote_CurrentText.setFont(font_neon);
		Vote_CurrentText.setString("TIME TO VOTE");
		Vote_CurrentText.setColor(sf::Color::Red);
		Vote_CurrentText.setOutlineColor(sf::Color::Black);
		Vote_CurrentText.setOutlineThickness(3.0f);
		Vote_CurrentText.setPosition(100.0f, 400.0f);
		Vote_CurrentText.setCharacterSize(35);
		Vote_CurrentText.setStyle(sf::Text::Bold);

		Vote_Time.setFont(font_neon);
		Vote_Time.setString(std::to_string((int)((end_voting_time-elapsed))));
		Vote_Time.setColor(sf::Color::Red);
		Vote_Time.setOutlineColor(sf::Color::Black);
		Vote_Time.setOutlineThickness(4.0f);
		Vote_Time.setPosition(150.0f, 420.0f);
		Vote_Time.setCharacterSize(50);
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
		window.draw(rectanglevote1);
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
	}

	return EXIT_SUCCESS;

}
