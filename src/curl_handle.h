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

#ifndef _CURL_HANDLE_H
#define _CURL_HANDLE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_CURL_CURL_H

#include <string>
#include <bitset>
#include "curl/curl.h"

namespace Curl
{
    CURLcode perform(std::string &data, const std::string &URL, const std::string &referer = "", unsigned timeout = 10, const std::string &postData = "", std::bitset<16> options = 0);
	std::string escape(const std::string &s);

    const int POST = (1 << 0);
    const int FOLLOW_LOCATION = (1 << 1);
    const int INCLUDE_HEADER = (1 << 2);
}

#endif // HAVE_CURL_CURL_H

#endif
