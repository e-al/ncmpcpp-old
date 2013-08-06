#ifndef VK_FETCHER_H
#define VK_FETCHER_H



#include "music_fetcher.h"
#include "mpdpp.h"

class VKFetcher : public MusicFetcher
{
    typedef std::pair<bool, std::string> Result;
public:
    VKFetcher();

    virtual MPD::SongList& GetList() override;
    std::string getAccessToken();
    Result fetch();

private:


    static const char *authURL;
};

#endif // VK_FETCHER_H
