#ifndef VK_FETCHER_H
#define VK_FETCHER_H

#include <string>

#include "music_fetcher.h"
#include "streamsong.h"
#include "mpdpp.h"



class VKFetcher : public MusicFetcher
{
    typedef std::pair<bool, std::string> Result;
    typedef std::map<std::string, std::string> PostParams;
public:
    VKFetcher();

    virtual MPD::SongList *GetList() override;

    void SetCount(const std::string &);
	std::string GetCount();



private:



    static const char* inputParamNames[];

    static const char *authURL;
    static const char *searchBaseUrl;
    std::string parseHtml(const std::string &data, const std::string &begin, const std::string &end);
    std::string getFormInputValue(const std::string &data, const std::string &name);
    std::string getFormActionValue(const std::string &data, const std::string &name);
    std::string getUrlParamValue(const std::string &data, const std::string &name);

	std::string getAccessToken();
	Result fetch();
};

#endif // VK_FETCHER_H
