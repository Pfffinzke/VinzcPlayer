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
 vector<string> files = vector<string>();
int number_song;
int played_song;
std::string filename;
std::string menu_path;
int current_song;

int song_choice[4];
int vote[5]={0,0,0,0,0};

int next_chosen_song;

bool choice;
bool next_button = false;
bool menu;
bool next_bool;
bool next_pushed = false;
bool b_parsing;
bool time_to_vote;
float time_before_vote;

sf::Clock clock_sprite;
sf::Clock clock_autoplay;


/*function... might want it in some class?*/




int getdir (string dir, vector<string> &files)
{
    files.clear();
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
   
std::cout << "parsing directory" << dir << std::endl;
    getdir(dir,files);

	//Start loop

	//Second step : list all these files in Json

  	std::string text = "{\"ID\": \"0\",\"path\": \"James\", \"Song Name\": \"Bond\", \"artist\": \"yop\" , \"play\": 0 }\n";
  	std::ofstream outFile;
  	string json_name = string(input_dir+".json");
  
  	
	outFile.open("output.json");
	root.clear();
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
	next_pushed=false;
	std::cout << "first song is vlc playing" << std::endl;
	played_song++;
	NextSongPool();

	return 1;

}

void New_folder_First_song(){
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

std::cout << "first song init ok" << std::endl;
	root[current_song]["play"] = 1;
	std::cout << "first song call vlc play" << std::endl;
	multimedia->play(filename);
	next_pushed=false;
	std::cout << "first song is vlc playing" << std::endl;
	played_song++;
	NextSongPool();

	
}


void NextSong() {
	multimedia->stop();
	next_bool=false;
	current_song = song_choice[next_chosen_song];
	filename = root[current_song]["path"].asString();

	std::cout << "START: " << filename << std::endl;
	
	
	root[current_song]["play"] = 1;
	multimedia->play(filename);
	next_pushed=false;
	// chosen song is played reset vote
			vote[1]=0;
			vote[2]=0;
			vote[3]=0;
			vote[4]=0;
	played_song++;
	NextSongPool();

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
  /* debug
std::cout << "START: CALL AUTOPLAY "  << std::endl;
std::cout << "AUTOPLAY : is playing "  << multimedia->is_playing() << std::endl;
std::cout << "AUTOPLAY : is paused "  << multimedia->is_paused() << std::endl;
std::cout << "AUTOPLAY : is finished "  << multimedia->is_finished() << std::endl;
*/

	if (((!multimedia->is_playing())&&!multimedia->is_paused()&&!next_pushed)||multimedia->is_finished()){

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
	
	// sprite for voting buttons

	sf::Texture texturej1;
  texturej1.loadFromFile("spriteJ1.png");
  sf::IntRect rectSourceSprite(0, 0, 150, 150);
  sf::Sprite spritej1(texturej1,rectSourceSprite);
  spritej1.setPosition(50, 280);

  sf::Texture texturej2;
  texturej2.loadFromFile("spriteJ2.png");
  sf::Sprite spritej2(texturej2,rectSourceSprite);
  spritej2.setPosition(250, 200);
  
  sf::Texture texturej3;
  texturej3.loadFromFile("spriteJ3.png");
  sf::Sprite spritej3(texturej3,rectSourceSprite);
  spritej3.setPosition(450, 280);

  sf::Texture texturej4;
  texturej4.loadFromFile("spriteJ4.png");
  sf::Sprite spritej4(texturej4,rectSourceSprite);
  spritej4.setPosition(650, 200);
  
  sf::Font font;
	font.loadFromFile("ALBA.ttf");
	sf::Font font_neon;
	font_neon.loadFromFile("Neon.ttf");
	float thickness = 3.0f;
  
  sf::Text menu_txt;
  menu_txt.setFont(font);
	menu_txt.setString("Press - M - for changing music directory");
	menu_txt.setColor(sf::Color::White);
	menu_txt.setOutlineColor(sf::Color::Black);
	menu_txt.setOutlineThickness(2.0f);
	menu_txt.setPosition(400.0f, 570.0f);
	menu_txt.setCharacterSize(23);


// Start the Menu loop
	while (window.isOpen())
	{
		if (menu){
		// Process events
		sf::Event event;
		
		//path choice(music);	
		while (window.pollEvent(event)) {
		  //std::remove("output.json");
		 /* for (unsigned int i = 0;i < files.size();i++) {
		    delete root[i];
		  }
		  */
		  
      
      
			if (event.type == sf::Event::Closed) {
				window.close();
			} else if(event.type == sf::Event::KeyPressed) {
				// the user interface: SPACE pauses and plays, ESC quits
				switch (event.key.code) {
					case sf::Keyboard::F1:
					  menu_path = "hip-hop";
					  //root.clear();
					  read_json(menu_path);
					  break;
					
					case sf::Keyboard::F2:
					  menu_path = "Funk-Disco";
					  //root.clear();
					  read_json(menu_path);
					  break;
					
					case sf::Keyboard::F3:
					  menu_path = "latino";
					  //root.clear();
					  read_json(menu_path);
					  break;
					  
					case sf::Keyboard::F4:
					  menu_path = "party";
					  //root.clear();
					  read_json(menu_path);
					  break;
					  
					case sf::Keyboard::F5:
					  menu_path = "full";
					  //root.clear();
					  read_json(menu_path);
					  break;
					  
					case sf::Keyboard::M:
					  menu = false;
					  break;
					  
					case sf::Keyboard::Escape:
					  std::cout << "press escape - escape" << std::endl;
						window.close();
						break;
					case sf::Keyboard::Enter:
					  played_song=0;
					  New_folder_First_song();
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
		

		
	sf::Texture texButton;
	texButton.loadFromFile("boutton.png");
	// Create a sprite
	
	float button_line = first_line + 20;
	float end_line = 500.0;
	
	sf::Sprite spriteButton1;
	spriteButton1.setTexture(texButton);
	spriteButton1.setPosition(100, button_line);
	spriteButton1.scale(0.8,0.8);

	sf::Sprite spriteButton2;
	spriteButton2.setTexture(texButton);
	spriteButton2.setPosition(400, button_line);
	spriteButton2.scale(0.8,0.8);
	
	sf::Sprite spriteButton3;
	spriteButton3.setTexture(texButton);
	spriteButton3.setPosition(100, button_line*2);
	spriteButton3.scale(0.8,0.8);

	sf::Sprite spriteButton4;
	spriteButton4.setTexture(texButton);
	spriteButton4.setPosition(400, button_line*2);
	spriteButton4.scale(0.8,0.8);
	
	sf::Sprite spriteButton5;
	spriteButton5.setTexture(texButton);
	spriteButton5.setPosition(250, button_line*3);
	spriteButton5.scale(0.8,0.8);


	window.draw(spriteButton1);
	window.draw(spriteButton2);
	window.draw(spriteButton3);
	window.draw(spriteButton4);
	window.draw(spriteButton5);


		sf::Text Welcome;
		sf::Text Path_Choice1;
		sf::Text Path_Choice2;
		sf::Text Path_Choice3;
		sf::Text Path_Choice4;
		sf::Text Path_Choice5;

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
		Path_Choice1.setPosition(135.0f, button_line+10);
		Path_Choice1.setCharacterSize(30);
		Path_Choice1.setStyle(sf::Text::Bold );

		Path_Choice2.setFont(font);
		Path_Choice2.setString("F2 : Disco-Funk");
		Path_Choice2.setColor(sf::Color::Green);
		Path_Choice2.setOutlineColor(sf::Color::Black);
		Path_Choice2.setOutlineThickness(thickness);
		Path_Choice2.setPosition(420.0f, button_line+10);
		Path_Choice2.setCharacterSize(30);
		Path_Choice2.setStyle(sf::Text::Bold);
		
		Path_Choice3.setFont(font);
		Path_Choice3.setString("F3 : Latino");
		Path_Choice3.setColor(sf::Color::Green);
		Path_Choice3.setOutlineColor(sf::Color::Black);
		Path_Choice3.setOutlineThickness(thickness);
		Path_Choice3.setPosition(135.0f, button_line*2+10);
		Path_Choice3.setCharacterSize(30);
		Path_Choice3.setStyle(sf::Text::Bold );

		Path_Choice4.setFont(font);
		Path_Choice4.setString("F4 : Party !");
		Path_Choice4.setColor(sf::Color::Green);
		Path_Choice4.setOutlineColor(sf::Color::Black);
		Path_Choice4.setOutlineThickness(thickness);
		Path_Choice4.setPosition(440.0f, button_line*2+10);
		Path_Choice4.setCharacterSize(30);
		Path_Choice4.setStyle(sf::Text::Bold);
		
		Path_Choice5.setFont(font);
		Path_Choice5.setString("F5 : Full Playlist");
		Path_Choice5.setColor(sf::Color::Green);
		Path_Choice5.setOutlineColor(sf::Color::Black);
		Path_Choice5.setOutlineThickness(thickness);
		Path_Choice5.setPosition(270.0f, button_line*3+10);
		Path_Choice5.setCharacterSize(30);
		Path_Choice5.setStyle(sf::Text::Bold );


		Path_Selected.setFont(font_neon);
		Path_Selected.setString(menu_path);
		Path_Selected.setColor(sf::Color::White);
		Path_Selected.setOutlineColor(sf::Color::Black);
		Path_Selected.setOutlineThickness(thickness);
		Path_Selected.setPosition(520.0f, end_line);
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
		Parsing.setPosition(650.0f, end_line);
		Parsing.setCharacterSize(24);
		Parsing.setStyle(sf::Text::Bold);

		Confirm.setFont(font_neon);
		Confirm.setString("Press Enter to confirm");
		Confirm.setColor(sf::Color::Green);
		Confirm.setOutlineColor(sf::Color::Black);
		Confirm.setOutlineThickness(thickness);
		Confirm.setPosition(520.0f, end_line+25);
		Confirm.setCharacterSize(24);
		Confirm.setStyle(sf::Text::Bold);

		//draw text
		window.draw(Welcome);
		window.draw(Path_Choice1);
		window.draw(Path_Choice2);
		window.draw(Path_Choice3);
		window.draw(Path_Choice4);
		window.draw(Path_Choice5);
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
		
		Autoplay();

		//elapsed time is a % from 0.0 to 1.0
		float elapsed  = multimedia->get_position()/1000;

		float start_voting_time = 60 ;
		float end_voting_time = 90;
		//std::cout << "song time. " << song_lenght.asSeconds() << "remaining time :  " << elapsed.asSeconds() << std::endl;


time_before_vote = start_voting_time - elapsed;

if (time_before_vote >= 0)
{time_to_vote=true;
}
else
{time_to_vote=false;
}


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
			std::cout << "next chosen song. " << next_chosen_song << std::endl;
			next_bool=true;
			next_button=true;
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
						if (next_button){
						  next_button = false;
						  next_pushed=true;
						NextSong();
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
					if (next_button){
						  next_button = false;
						  next_pushed = true;
						  NextSong();
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
		window.draw(menu_txt);
		sf::Font font;
		font.loadFromFile("ALBA.ttf");
		sf::Font font_neon;
		font_neon.loadFromFile("Neon.ttf");
		float thickness = 4.f;
		
		sf::RectangleShape rectangleplay(sf::Vector2f(300, 10));
		rectangleplay.setFillColor(sf::Color::White);
		rectangleplay.setOutlineColor(sf::Color::Black);
		rectangleplay.setOutlineThickness(1.0f);
		rectangleplay.setPosition(400, first_line+42);
		
		// update the playing bar with time
		float total_time;
		float current_time;
		float current_curser;
	
		total_time = multimedia->get_lenght();
		current_time = multimedia->get_current_time();
		current_curser = (current_time*300.0)/total_time;
		
	  sf::RectangleShape rectangleplaycurrent(sf::Vector2f(current_curser, 10));
		rectangleplaycurrent.setFillColor(sf::Color::Yellow);
		rectangleplaycurrent.setOutlineThickness(0.0f);
		rectangleplaycurrent.setPosition(400, first_line+42);
		
		
		sf::Text Title;
		sf::Text Artist;
		sf::Text CurrentText;
		sf::String CurrentSongText = root[current_song]["Song Name"].asString();
		sf::String CurrentArtistText = root[current_song]["artist"].asString();

		sf::Text timevote;
		sf::Text countdown;


		timevote.setFont(font_neon);
		timevote.setString("Next vote in :    s");
		timevote.setColor(sf::Color::Red);
		timevote.setPosition(400.0f, first_line+55);
		timevote.setCharacterSize(20);
		timevote.setStyle(sf::Text::Bold);
		timevote.setOutlineColor(sf::Color::Black);
		timevote.setOutlineThickness(3.0f);

		countdown.setFont(font_neon);
		countdown.setString(std::to_string((int)(time_before_vote)));
		countdown.setColor(sf::Color::Red);
		countdown.setPosition(540.0f, first_line+55);
		countdown.setCharacterSize(20);
		countdown.setStyle(sf::Text::Bold);
		countdown.setOutlineColor(sf::Color::Black);
		countdown.setOutlineThickness(3.0f);



		Artist.setFont(font_neon);
		Artist.setString(CurrentArtistText);
		Artist.setColor(sf::Color::White);
		Artist.setPosition(400.0f, first_line-15);
		Artist.setCharacterSize(24);
		Artist.setStyle(sf::Text::Bold);
		Artist.setOutlineColor(sf::Color::Black);
		Artist.setOutlineThickness(3.0f);

		Title.setFont(font_neon);
		Title.setString(CurrentSongText);
		Title.setColor(sf::Color::White);
		Title.setPosition(400.0f, first_line+10);
		Title.setCharacterSize(24);
		Title.setStyle(sf::Text::Bold);
		Title.setOutlineColor(sf::Color::Black);
		Title.setOutlineThickness(3.0f);
		
		CurrentText.setFont(font);
		CurrentText.setString("Current playing song");
		CurrentText.setColor(sf::Color::Cyan);
		CurrentText.setPosition(400.0f, first_line-40);
		CurrentText.setCharacterSize(26);
		CurrentText.setStyle(sf::Text::Bold | sf::Text::Underlined);
		CurrentText.setOutlineColor(sf::Color::Black);
		CurrentText.setOutlineThickness(3.0f);
		
		window.draw(CurrentText);
		window.draw(Artist);
		window.draw(Title);
		window.draw(spriteLogo);
		window.draw(rectangleplay);
		window.draw(rectangleplaycurrent);
		//window.draw(spriteJimmy);

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
float delta = 80.0f;

		Choice1_CurrentText.setFont(font);
		Choice1_CurrentText.setString("Next song 1");
		Choice1_CurrentText.setColor(sf::Color::White);
		Choice1_CurrentText.setPosition(40.0f, 120.0f+delta);
		Choice1_CurrentText.setOutlineColor(sf::Color::Black);
		Choice1_CurrentText.setOutlineThickness(3.0f);
		Choice1_CurrentText.setCharacterSize(24);
		Choice1_CurrentText.setStyle(sf::Text::Bold);

		Choice1_Artist.setFont(font_neon);
		Choice1_Artist.setString(root[song_choice[1]]["artist"].asString());
		Choice1_Artist.setColor(sf::Color(85,85,255,255));
		Choice1_Artist.setOutlineThickness(2.0f);
		Choice1_Artist.setOutlineColor(sf::Color::Black);
		Choice1_Artist.setPosition(40.0f, 150.0f+delta);
		Choice1_Artist.setCharacterSize(22);
		Choice1_Artist.setStyle(sf::Text::Bold);

		Choice1_Title.setFont(font_neon);
		Choice1_Title.setString(root[song_choice[1]]["Song Name"].asString());
		Choice1_Title.setColor(sf::Color(85,85,255,255));
		Choice1_Title.setOutlineThickness(2.0f);
		Choice1_Title.setOutlineColor(sf::Color::Black);
		Choice1_Title.setPosition(40.0f, 170.0f+delta);
		Choice1_Title.setCharacterSize(18);
		Choice1_Title.setStyle(sf::Text::Bold);

		Choice1_Vote.setFont(font_neon);
		Choice1_Vote.setString(std::to_string(vote[1]));
		Choice1_Vote.setColor(sf::Color::Blue);
		Choice1_Vote.setOutlineThickness(3.0f);
		Choice1_Vote.setOutlineColor(sf::Color::Black);
		Choice1_Vote.setPosition(100.0f, 520.0f);
		Choice1_Vote.setCharacterSize(40);
		Choice1_Vote.setStyle(sf::Text::Bold);

		Choice2_CurrentText.setFont(font);
		Choice2_CurrentText.setString("Next song 2");
		Choice2_CurrentText.setColor(sf::Color::White);
		Choice2_CurrentText.setOutlineColor(sf::Color::Black);
		Choice2_CurrentText.setOutlineThickness(3.0f);
		Choice2_CurrentText.setPosition(230.0f, 280.0f+delta);
		Choice2_CurrentText.setCharacterSize(24);
		Choice2_CurrentText.setStyle(sf::Text::Bold );

		Choice2_Artist.setFont(font_neon);
		Choice2_Artist.setString(root[song_choice[2]]["artist"].asString());
		Choice2_Artist.setColor(sf::Color::Green);
		Choice2_Artist.setOutlineColor(sf::Color::Black);
		Choice2_Artist.setOutlineThickness(2.0f);
		Choice2_Artist.setPosition(230.0f, 310.0f+delta);
		Choice2_Artist.setCharacterSize(22);
		Choice2_Artist.setStyle(sf::Text::Bold);

		Choice2_Title.setFont(font_neon);
		Choice2_Title.setString(root[song_choice[2]]["Song Name"].asString());
		Choice2_Title.setColor(sf::Color::Green);
		Choice2_Title.setOutlineColor(sf::Color::Black);
		Choice2_Title.setOutlineThickness(2.0f);
		Choice2_Title.setPosition(230.0f, 330.0f+delta);
		Choice2_Title.setCharacterSize(18);
		Choice2_Title.setStyle(sf::Text::Bold);

		Choice2_Vote.setFont(font_neon);
		Choice2_Vote.setString(std::to_string(vote[2]));
		Choice2_Vote.setColor(sf::Color::Green);
		Choice2_Vote.setOutlineColor(sf::Color::Black);
		Choice2_Vote.setOutlineThickness(3.0f);
		Choice2_Vote.setPosition(280.0f, 520.0f);
		Choice2_Vote.setCharacterSize(40);
		Choice2_Vote.setStyle(sf::Text::Bold);

		Choice3_CurrentText.setFont(font);
		Choice3_CurrentText.setString("Next song 3");
		Choice3_CurrentText.setColor(sf::Color::White);
		Choice3_CurrentText.setOutlineColor(sf::Color::Black);
		Choice3_CurrentText.setOutlineThickness(3.0f);
		Choice3_CurrentText.setPosition(420.0f, 120.0f+delta);
		Choice3_CurrentText.setCharacterSize(24);
		Choice3_CurrentText.setStyle(sf::Text::Bold );

		Choice3_Artist.setFont(font_neon);
		Choice3_Artist.setString(root[song_choice[3]]["artist"].asString());
		Choice3_Artist.setColor(sf::Color::Red);
		Choice3_Artist.setOutlineColor(sf::Color::Black);
		Choice3_Artist.setOutlineThickness(2.0f);
		Choice3_Artist.setPosition(420.0f, 150.0f+delta);
		Choice3_Artist.setCharacterSize(22);
		Choice3_Artist.setStyle(sf::Text::Bold);

		Choice3_Title.setFont(font_neon);
		Choice3_Title.setString(root[song_choice[3]]["Song Name"].asString());
		Choice3_Title.setColor(sf::Color::Red);
		Choice3_Title.setOutlineColor(sf::Color::Black);
		Choice3_Title.setOutlineThickness(2.0f);
		Choice3_Title.setPosition(420.0f, 170.0f+delta);
		Choice3_Title.setCharacterSize(18);
		Choice3_Title.setStyle(sf::Text::Bold);

		Choice3_Vote.setFont(font_neon);
		Choice3_Vote.setString(std::to_string(vote[3]));
		Choice3_Vote.setColor(sf::Color::Red);
		Choice3_Vote.setOutlineColor(sf::Color::Black);
		Choice3_Vote.setOutlineThickness(3.0f);
		Choice3_Vote.setPosition(460.0f, 520.0f);
		Choice3_Vote.setCharacterSize(40);
		Choice3_Vote.setStyle(sf::Text::Bold);

		Choice4_CurrentText.setFont(font);
		Choice4_CurrentText.setString("Next song 4");
		Choice4_CurrentText.setColor(sf::Color::White);
		Choice4_CurrentText.setOutlineColor(sf::Color::Black);
		Choice4_CurrentText.setOutlineThickness(3.0f);
		Choice4_CurrentText.setPosition(610.0f, 280.0f+delta);
		Choice4_CurrentText.setCharacterSize(24);
		Choice4_CurrentText.setStyle(sf::Text::Bold );

		Choice4_Artist.setFont(font_neon);
		Choice4_Artist.setString(root[song_choice[4]]["artist"].asString());
		Choice4_Artist.setColor(sf::Color::Yellow);
		Choice4_Artist.setOutlineColor(sf::Color::Black);
		Choice4_Artist.setOutlineThickness(2.0f);
		Choice4_Artist.setPosition(610.0f, 310.0f+delta);
		Choice4_Artist.setCharacterSize(22);
		Choice4_Artist.setStyle(sf::Text::Bold);

		Choice4_Title.setFont(font_neon);
		Choice4_Title.setString(root[song_choice[4]]["Song Name"].asString());
		Choice4_Title.setColor(sf::Color::Yellow);
		Choice4_Title.setOutlineColor(sf::Color::Black);
		Choice4_Title.setOutlineThickness(2.0f);
		Choice4_Title.setPosition(610.0f, 330.0f+delta);
		Choice4_Title.setCharacterSize(18);
		Choice4_Title.setStyle(sf::Text::Bold);

		Choice4_Vote.setFont(font_neon);
		Choice4_Vote.setString(std::to_string(vote[4]));
		Choice4_Vote.setColor(sf::Color::Yellow);
		Choice4_Vote.setOutlineColor(sf::Color::Black);
		Choice4_Vote.setOutlineThickness(3.0f);
		Choice4_Vote.setPosition(640.0f, 520.0f);
		Choice4_Vote.setCharacterSize(40);
		Choice4_Vote.setStyle(sf::Text::Bold);

		sf::RectangleShape rectanglevote1(sf::Vector2f(100, -(5+vote[1])));
		rectanglevote1.setFillColor(sf::Color(50, 50, 250));
		rectanglevote1.setOutlineColor(sf::Color::Black);
		rectanglevote1.setOutlineThickness(3.0f);
		rectanglevote1.setPosition(80, 575);

		sf::RectangleShape rectanglevote2(sf::Vector2f(100, -(5+vote[2])));
		rectanglevote2.setFillColor(sf::Color(50, 250, 50));
		rectanglevote2.setOutlineColor(sf::Color::Black);
		rectanglevote2.setOutlineThickness(3.0f);
		rectanglevote2.setPosition(260, 575);

		sf::RectangleShape rectanglevote3(sf::Vector2f(100, -(5+vote[3])));
		rectanglevote3.setFillColor(sf::Color(250, 50, 50));
		rectanglevote3.setOutlineColor(sf::Color::Black);
		rectanglevote3.setOutlineThickness(3.0f);
		rectanglevote3.setPosition(440, 575);

		sf::RectangleShape rectanglevote4(sf::Vector2f(100, -(5+vote[4])));
		rectanglevote4.setFillColor(sf::Color(250, 250, 50));
		rectanglevote4.setOutlineColor(sf::Color::Black);
		rectanglevote4.setOutlineThickness(3.0f);
		rectanglevote4.setPosition(620, 575);


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
	  
	  // voting buttons
  
//std::cout << "elapsed time" << clock_sprite.getElapsedTime().asSeconds() << std::endl;
    if (clock_sprite.getElapsedTime().asSeconds() > 1.0f){
      if (rectSourceSprite.left == 150)
        {rectSourceSprite.left = 0;
        clock_sprite.restart();}
      else
        {rectSourceSprite.left += 150;
        clock_sprite.restart();}
      spritej1.setTextureRect(rectSourceSprite);
      spritej2.setTextureRect(rectSourceSprite);
      spritej3.setTextureRect(rectSourceSprite);
      spritej4.setTextureRect(rectSourceSprite);
    }
    
    
		// set voting time
		Vote_CurrentText.setFont(font_neon);
		Vote_CurrentText.setString("TIME TO VOTE");
		Vote_CurrentText.setColor(sf::Color::Red);
		Vote_CurrentText.setOutlineColor(sf::Color::Black);
		Vote_CurrentText.setOutlineThickness(3.0f);
		Vote_CurrentText.setPosition(300.0f, 440.0f);
		Vote_CurrentText.setCharacterSize(35);
		Vote_CurrentText.setStyle(sf::Text::Bold);

		Vote_Time.setFont(font_neon);
		Vote_Time.setString(std::to_string((int)((end_voting_time-elapsed))));
		Vote_Time.setColor(sf::Color::Red);
		Vote_Time.setOutlineColor(sf::Color::Black);
		Vote_Time.setOutlineThickness(4.0f);
		Vote_Time.setPosition(390.0f, 480.0f);
		Vote_Time.setCharacterSize(50);
		Vote_Time.setStyle(sf::Text::Bold);

		//display text
		window.draw(rectanglevote1);
		window.draw(rectanglevote2);
		window.draw(rectanglevote3);
		window.draw(rectanglevote4);
		window.draw(Choice1_CurrentText);
		window.draw(Choice1_Artist);
		window.draw(Choice1_Title);
		window.draw(Choice1_Vote);
		window.draw(spritej1);
		window.draw(Choice2_CurrentText);
		window.draw(Choice2_Artist);
		window.draw(Choice2_Title);
		window.draw(Choice2_Vote);
		window.draw(spritej2);
		window.draw(Choice3_CurrentText);
		window.draw(Choice3_Artist);
		window.draw(Choice3_Title);
		window.draw(Choice3_Vote);
		window.draw(spritej3);
		window.draw(Choice4_CurrentText);
		window.draw(Choice4_Artist);
		window.draw(Choice4_Title);
		window.draw(Choice4_Vote);
		window.draw(spritej4);
		window.draw(Vote_CurrentText);
		window.draw(Vote_Time);
		
	}
	// print next song
	if(next_bool==true){

		window.draw(Next_Artist);
		window.draw(Next_Title);
		window.draw(Next_CurrentText);
		}

	if(time_to_vote==true)
{

		window.draw(timevote);
		window.draw(countdown);
		
		}

		
		// Update the window
		window.display();
	}
	}

	return EXIT_SUCCESS;

}
