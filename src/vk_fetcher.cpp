#include "vk_fetcher.h"
#include "curl_handle.h"

const char *VKFetcher::authURL = "https://oauth.vk.com/authorize?client_id=3803853&scope=audio&redirect_uri=https://oauth.vk.com/blank.html&display=popup&v=5&response_type=token";

VKFetcher::VKFetcher()
{
}


MPD::SongList &VKFetcher::GetList()
{

}

std::string VKFetcher::getAccessToken()
{
    std::string authUrl = authURL;
    std::string data;
    CURLcode code = Curl::perform(data, authUrl);
}
