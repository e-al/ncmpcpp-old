#ifndef MUSICFETCHER_H
#define MUSICFETCHER_H

#include <string>
#include <map>

#include "mpdpp.h"


class MusicFetcher
{
    typedef std::map<std::string, std::string> SearchMap;
public:
    MusicFetcher();

    virtual MPD::SongList *GetList() = 0;

    virtual std::string GetArtist();
    virtual std::string GetTitle();
    virtual std::string GetAlbum();
    virtual std::string GetComposer();
    virtual std::string GetPerformer();
    virtual std::string GetGenre();
    virtual std::string GetYear();
	virtual std::string GetAny();

    virtual void SetArtist(const std::string &);
    virtual void SetTitle(const std::string &);
    virtual void SetAlbum(const std::string &);
    virtual void SetComposer(const std::string &);
    virtual void SetPerformer(const std::string &);
    virtual void SetGenre(const std::string &);
    virtual void SetYear(const std::string &);
	virtual void SetAny(const std::string &);

protected:
	void AddOption(const std::string &name, const std::string &value);
	std::string GetOption(const std::string &name);
private:
    SearchMap searchMap;
};

#endif // MUSICFETCHER_H
