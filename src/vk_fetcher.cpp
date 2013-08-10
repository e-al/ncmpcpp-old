#include <boost/property_tree/json_parser.hpp>
#include <sstream>

#include "vk_fetcher.h"
#include "curl_handle.h"
#include "conv.h"


const char *VKFetcher::authURL = "https://oauth.vk.com/authorize?client_id=3803853&scope=audio&redirect_uri=https://oauth.vk.com/blank.html&display=mobile&v=5&response_type=token";
const char  *VKFetcher::searchBaseUrl = "https://api.vk.com/method/audio.search?";

const char *VKFetcher::inputParamNames[] =
{
    "ip_h",
    "_origin",
    "to",
    "expire",
    "email",
    "pass",
};

VKFetcher::VKFetcher()
{
}


MPD::SongList *VKFetcher::GetList()
{
    MPD::SongList *songList = new MPD::SongList();
    std::string accessToken = getAccessToken();
    if (accessToken.size() == 0)
    {
        return songList;
    }

    std::string searchUrl = searchBaseUrl;
    searchUrl += "q=";
    searchUrl += Curl::escape(GetArtist());
    searchUrl += "%20";
    searchUrl += Curl::escape(GetTitle());
    searchUrl += "&auto_complete=0&lyrics=0&";
    if (GetTitle() == "" && GetArtist() != "")
    {
        searchUrl += "performer_only=1&";
    }
    searchUrl += "sort=2&count=100&access_token=";
    searchUrl += accessToken;

    std::string data;
    CURLcode code = Curl::perform(data, searchUrl);
    if (code != CURLE_OK)
    {
        return songList;
    }

    std::stringstream ss;
    ss << data;

    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
//    for (auto song : pt.get_child(""))
//    {

//    }
    return songList;
}

std::string VKFetcher::getAccessToken()
{

    std::string url = authURL;
    std::string data;

    int curlOptions = Curl::FOLLOW_LOCATION | Curl::INCLUDE_HEADER;
    CURLcode code = Curl::perform(data, url, "", 10, "", curlOptions);
    if (code != CURLE_OK)
    {
        return "";
    }

    std::string accessToken = getUrlParamValue(data, "access_token");

    if (accessToken.size() == 0)
    {
        PostParams postParams;
        std::string param;

        for (auto i : inputParamNames)
        {
            param = getFormInputValue(data, i);
            if (param.size() != 0)
            {
                postParams[i] = param;
            }
            else
            {
                break;
            }
        }

        postParams["email"] = Curl::escape("+79889431831");
        postParams["pass"] = Curl::escape("afrqj'cc");

        std::string strPostParams;

        for (auto i : postParams)
        {
            strPostParams += i.first;
            strPostParams += "=";
            strPostParams += i.second;
            strPostParams += "&";
        }
        strPostParams.resize(strPostParams.size() - 1);

        url = getFormActionValue(data, "action");
        if (url.size() == 0)
        {
            return "";
        }

        code = Curl::perform(data, url, "", 10, strPostParams, curlOptions | Curl::POST);
        if (code != CURLE_OK)
        {
            return "";
        }

        accessToken = getUrlParamValue(data, "access_token");
    }   // access token checking

    return accessToken;
}

std::string VKFetcher::parseHtml(const std::string &data, const std::string &begin, const std::string &end)
{
    int paramPosBeg;
    int paramPosEnd;

    if ((paramPosBeg = data.find(begin)) != std::string::npos)
    {
        paramPosBeg = paramPosBeg + begin.size();
        paramPosEnd = data.find(end, paramPosBeg);

        return data.substr(paramPosBeg, paramPosEnd - paramPosBeg);
    }
    else
    {
        return "";
    }
}

std::string VKFetcher::getFormInputValue(const std::string &data, const std::string &name)
{
    return parseHtml(data, "name=\"" + name +  "\" value=\"", "\"");
}

std::string VKFetcher::getFormActionValue(const std::string &data, const std::string &name)
{
    return parseHtml(data, name + "=\"", "\"");
}

std::string VKFetcher::getUrlParamValue(const std::string &data, const std::string &name)
{
    return parseHtml(data, name + "=", "&" );
}
