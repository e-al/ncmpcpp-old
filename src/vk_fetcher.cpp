#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iostream>

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

void print(boost::property_tree::ptree const& pt)
{
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }
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
	searchUrl += Curl::escape(GetAny());
	searchUrl += "%20";
    searchUrl += Curl::escape(GetArtist());
    searchUrl += "%20";
    searchUrl += Curl::escape(GetTitle());

	std::string count = Curl::escape(GetCount());
	try
	{
		boost::lexical_cast<int>(count);
	}
	catch (boost::bad_lexical_cast &)
	{
		count = "50";
	}

	searchUrl += "&count=";
	searchUrl += count;

    searchUrl += "&auto_complete=0&lyrics=0&";
	if (GetTitle() == "" && GetAny() == "" && GetArtist() != "")
    {
        searchUrl += "performer_only=1&";
    }
	searchUrl += "sort=2&access_token=";
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
	auto response = pt.get_child("response");
	response.pop_front();

	MPD::Song *newSong = nullptr;
    try
    {
		for (auto song: response)
        {
			newSong = new MPD::Song();
			if (newSong)
			{
				newSong->SetFilename(song.second.get<std::string>("url"));
				newSong->SetArtist(song.second.get<std::string>("artist"));
				newSong->SetTitle(song.second.get<std::string>("title"));
//				std::cout << song.second.get<std::string>("title") << std::endl;
				songList->push_back(newSong);
			}
        }
    }
    catch(boost::exception& e)
    {
        std::cout << dynamic_cast<std::exception &>(e).what() <<std::endl;
    }

//    for (auto item : pt.get_child("response.items"))
//    {
//        std::cout << item.second.data() << std::endl;
//    }
	int i = 0;
	return songList;
}

void VKFetcher::SetCount(const std::string &str)
{
	AddOption("count", str);
}

std::string VKFetcher::GetCount()
{
	return GetOption("count");
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
