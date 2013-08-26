#ifndef STREAMSONG_H
#define STREAMSONG_H

#include "song.h"

namespace MPD {
	class StreamSong : public Song
	{
	public:
		StreamSong();

		void SetFilename(const std::string);

		std::string GetFile(unsigned = 0) const{return std::string("");}
		std::string GetDirectory(unsigned = 0) const{return std::string("");}
		std::string GetLength(unsigned = 0) const{return std::string("");}



		unsigned GetTotalLength() const { return 0;}
		unsigned GetPosition() const { return 0;}
		unsigned GetID() const { return 0;}

		time_t GetMTime() const { return 0;}

		void SetPosition(unsigned){return;};

		void Localize();

	private:

		std::string GetTag(mpd_tag_type, unsigned) const;

		/// Used internally for handling filename, since we don't have
		/// write access to file string in mpd_song, manage our own if
		/// localization was done and there is localized filename that
		/// is different than the original one.
		///
		const char *MyFilename() const;
	};
}

#endif // STREAMSONG_H
