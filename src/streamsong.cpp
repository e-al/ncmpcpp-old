#include "streamsong.h"

MPD::StreamSong::StreamSong():Song()
{

}

void MPD::StreamSong::SetFilename(const std::string newFName)
{
	newFName.copy(itsFile, newFName.size() + 1, 0);
	itsFile[newFName.size()] = 0;
}

std::string MPD::StreamSong::GetName(unsigned) const
{
	if (itsSlash != std::string::npos)
		return MyFilename()+itsSlash+1;
	else
		return MyFilename();
}

void MPD::StreamSong::Localize()
{
	isLocalised = 1;
	return;
}

std::string MPD::StreamSong::GetTag(mpd_tag_type, unsigned) const
{
	if (itsTags)
	{
		TagMap::const_iterator it = itsTags->find(std::make_pair(type, pos));
		if (it != itsTags->end())
			return it->second;
	}
//	const char *tag = mpd_song_get_tag(itsSong, type, pos);
//	return tag ? tag : "";
	return "";
}

const char *MPD::StreamSong::MyFilename() const
{
	return itsFile;
}
