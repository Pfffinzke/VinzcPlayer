//MP3tag.h

#ifndef __MP3tag_h
#define __MP3tag_h

struct TAGdata
{
	char tag[3];
	char title[30];
 	char artist[30];
	char album[30];
	char year[4];
	char comment[30];
	int genre;
};

#endif

