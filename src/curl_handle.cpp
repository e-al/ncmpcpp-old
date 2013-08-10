/***************************************************************************
 *   Copyright (C) 2008-2012 by Andrzej Rybczak                            *
 *   electricityispower@gmail.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "curl_handle.h"

#ifdef HAVE_CURL_CURL_H

#include <cstdlib>
#include <pthread.h>

namespace
{
	size_t write_data(char *buffer, size_t size, size_t nmemb, void *data)
	{
		size_t result = size*nmemb;
		static_cast<std::string *>(data)->append(buffer, result);
		return result;
	}
}

CURLcode Curl::perform(std::string &data, const std::string &URL, const std::string &referer, unsigned timeout, const std::string &postData, std::bitset<16> options)
{
    //std::string headerData;
	CURLcode result;
	CURL *c = curl_easy_init();
	curl_easy_setopt(c, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(c, CURLOPT_USERAGENT, "ncmpcpp " VERSION);

	if (!referer.empty())
    {
		curl_easy_setopt(c, CURLOPT_REFERER, referer.c_str());
    }

    if (options[0] == 1)
    {
        curl_easy_setopt(c, CURLOPT_POST, 1);
    }
    if (options[1] == 1)
    {
        curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1);
    }
    if (options[2] == 1)
    {
        curl_easy_setopt(c, CURLOPT_HEADER, 1);
    }

    if (postData.size() != 0)
    {
        curl_easy_setopt(c, CURLOPT_POSTFIELDS, postData.c_str());
    }
    // TODO: Сделать что-нибудь с записью куков даже когда они не нужны.
    curl_easy_setopt(c, CURLOPT_COOKIEFILE, "/tmp/ncmpcpp_vk_cookies");
    result = curl_easy_perform(c);

    curl_easy_setopt(c, CURLOPT_COOKIEJAR, "/tmp/ncmpcpp_vk_cookies");
    curl_easy_cleanup(c);
    return result;
}

std::string Curl::escape(const std::string &s)
{
	char *cs = curl_easy_escape(0, s.c_str(), s.length());
	std::string result(cs);
	curl_free(cs);
	return result;
}

#endif // HAVE_CURL_CURL_H

