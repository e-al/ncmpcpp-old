#include "music_fetcher.h"

MusicFetcher::MusicFetcher()
{
}

std::string MusicFetcher::GetArtist()
{
    return searchMap["artist"];
}

std::string MusicFetcher::GetTitle()
{
    return searchMap["title"];
}

std::string MusicFetcher::GetAlbum()
{
    return searchMap["album"];
}

std::string MusicFetcher::GetComposer()
{
    return searchMap["composer"];
}

std::string MusicFetcher::GetPerformer()
{
    return searchMap["performer"];
}

std::string MusicFetcher::GetGenre()
{
    return searchMap["genre"];
}

std::string MusicFetcher::GetYear()
{
    return searchMap["year"];
}


void MusicFetcher::SetArtist(const std::string &str)
{
    searchMap["artist"] = str;
}

void MusicFetcher::SetTitle(const std::string &str)
{
    searchMap["title"] = str;
}

void MusicFetcher::SetAlbum(const std::string &str)
{
    searchMap["album"] = str;
}

void MusicFetcher::SetComposer(const std::string &str)
{
    searchMap["composer"] = str;
}

void MusicFetcher::SetPerformer(const std::string &str)
{
    searchMap["performer"] = str;
}

void MusicFetcher::SetGenre(const std::string &str)
{
    searchMap["genre"] = str;
}

void MusicFetcher::SetYear(const std::string &str)
{
    searchMap["year"] = str;
}


