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

#include <iomanip>
#include <iostream>

#include "display.h"
#include "global.h"
#include "helpers.h"
#include "playlist.h"
#include "search_engine.h"
#include "settings.h"
#include "status.h"
#include "vk_fetcher.h"

using Global::MainHeight;
using Global::MainStartY;

SearchEngine *mySearcher = new SearchEngine;

std::vector<std::string> SearchEngine::ConstraintsRegularNames =
{
	"Any:",
	"Artist:",
	"Title:",
	"Album:",
	"Filename:",
	"Composer:",
	"Performer:",
	"Genre:",
	"Year:",
	"Comment:"
};

std::vector<std::string> SearchEngine::ConstraintsVkNames =
{
	"Any:",
	"Artist:",
	"Title:",
	"Count:",
	"Genre:",
	"Vk Group:",
	"Vk User:"
};

std::vector<SearchEngine::Constraint *> SearchEngine::regularConstraints =
{
	new SearchEngine::Constraint("Any:"),
	new SearchEngine::Constraint("Artist:", 'a'),
	new SearchEngine::Constraint("Title:", 't'),
	new SearchEngine::Constraint("Album:", 'b'),
	new SearchEngine::Constraint("Composer:", 'c'),
	new SearchEngine::Constraint("Performer:", 'p'),
	new SearchEngine::Constraint("Genre:", 'g'),
	new SearchEngine::Constraint("Year:", 'd'),
	new SearchEngine::Constraint("Comment:", 'C'),
	new SearchEngine::Constraint("Filename:", 'g'),
};

std::vector<SearchEngine::Constraint *> SearchEngine::vkConstraints =
{
	new SearchEngine::Constraint("Any:"),
	new SearchEngine::Constraint("Artist:", 'a'),
	new SearchEngine::Constraint("Title:", 't'),
	new SearchEngine::Constraint("Count:"),
	new SearchEngine::Constraint("Genre:", 'g'),
	new SearchEngine::Constraint("Vk Group:"),
	new SearchEngine::Constraint("Vk User:")
};

const char *SearchEngine::SearchModes[] =
{
	"Match if tag contains searched phrase (no regexes)",
	"Match if tag contains searched phrase (regexes supported)",
	"Match only if both values are the same",
	0
};

const char *SearchEngine::SearchSources[] =
{
    "Database",
    "Current playlist",
    "vk.com",
    0
};

size_t SearchEngine::StaticOptions = 19;
size_t SearchEngine::ResetButton = 15;
size_t SearchEngine::SearchButton = 14;

void SearchEngine::Init()
{
	static Display::ScreenFormat sf = { this, &Config.song_list_format };

	w = new Menu< std::pair<Buffer *, MPD::Song *> >(0, MainStartY, COLS, MainHeight, "", Config.main_color, brNone);
	w->HighlightColor(Config.main_highlight_color);
	w->CyclicScrolling(Config.use_cyclic_scrolling);
	w->CenteredCursor(Config.centered_cursor);
	w->SetItemDisplayer(Display::SearchEngine);
	w->SetItemDisplayerUserData(&sf);
	w->SetSelectPrefix(&Config.selected_item_prefix);
	w->SetSelectSuffix(&Config.selected_item_suffix);
	w->SetGetStringFunction(SearchEngineOptionToString);
    SearchMode = &SearchModes[Config.search_engine_default_search_mode];
	//SearchSource = &SearchSources[Config.search_engine_default_source];
	SearchSource = &SearchSources[2]; // TODO: grandma
//	ConstraintsNames = &ConstraintsVkNames;

	constraintsMap[SearchSources[0]] = &regularConstraints;
	constraintsMap[SearchSources[1]] = &regularConstraints;
	constraintsMap[SearchSources[2]] = &vkConstraints;

    itsConstraints = constraintsMap[*SearchSource];

	//itsConstraints.resize(ConstraintsNames->size());
	isInitialized = 1;
}

void SearchEngine::Resize()
{
	size_t x_offset, width;
	GetWindowResizeParams(x_offset, width);
	w->Resize(width, MainHeight);
	w->MoveTo(x_offset, MainStartY);
    w->SetTitle(Config.columns_in_search_engine && Config.titles_visibility ? Display::Columns(w->GetWidth()) : "");
	hasToBeResized = 0;
}

void SearchEngine::SwitchTo()
{
	using Global::myScreen;
	using Global::myLockedScreen;
	
	if (myScreen == this)
	{
		Reset();
		return;
	}
	
	if (!isInitialized)
		Init();
	
	if (myLockedScreen)
		UpdateInactiveScreen(this);
	
	if (hasToBeResized || myLockedScreen)
		Resize();
	
	if (w->Empty())
		Prepare();

	if (myScreen != this && myScreen->isTabbable())
		Global::myPrevScreen = myScreen;
	myScreen = this;
	Global::RedrawHeader = 1;
	
	if (!w->Back().first)
	{
		*w << XY(0, 0) << "Updating list...";
		UpdateFoundList();
	}
}

std::basic_string<my_char_t> SearchEngine::Title()
{
	return U("Search engine");
}

void SearchEngine::EnterPressed()
{
	size_t option = w->Choice();
	if (option > itsConstraints->size() && option < SearchButton)
		w->Current().first->Clear();
	if (option < SearchButton)
		LockStatusbar();
	
	if (option < itsConstraints->size())
	{
		Statusbar() << fmtBold << itsConstraints->at(option)->GetName() << fmtBoldEnd << ' ';
        itsConstraints->at(option)->Set(Global::wFooter->GetString(itsConstraints->at(option)->Get()));
		w->Current().first->Clear();
		*w->Current().first << fmtBold << std::setw(10) << std::left << itsConstraints->at(option)->GetName() << fmtBoldEnd << ' ';
		ShowTag(*w->Current().first, itsConstraints->at(option)->Get());
	}
	else if (option == itsConstraints->size() + 1)
	{
        if (!*++SearchSource)
        {
			SearchSource = &SearchSources[0];
        }
		bool constraintsChanged = false;

		if (!strcmp(*SearchSource, SearchSources[2]))
		{
			Prepare();
			w->Goto(itsConstraints->size() + 1);
			constraintsChanged = true;
		}
		else if(ConstraintsNames != &ConstraintsRegularNames)
		{
			Prepare();
			w->Goto(itsConstraints->size() + 1);
			constraintsChanged = true;
		}
		if (!constraintsChanged)
		{
			*w->at(itsConstraints->size() + 1).first << fmtBold << "Search in:" << fmtBoldEnd << ' ' << *SearchSource;
		}
	}
	else if (option == itsConstraints->size() + 2)
	{
		if (!*++SearchMode)
			SearchMode = &SearchModes[0];
		*w->Current().first << fmtBold << "Search mode:" << fmtBoldEnd << ' ' << *SearchMode;
	}
	else if (option == SearchButton)
	{
		ShowMessage("Searching...");
		if (w->Size() > StaticOptions)
			Prepare();
		Search();
		if (!w->Back().first)
		{
			if (Config.columns_in_search_engine)
				w->SetTitle(Config.titles_visibility ? Display::Columns(w->GetWidth()) : "");
			size_t found = w->Size()-SearchEngine::StaticOptions;
			found += 3; // don't count options inserted below
			w->InsertSeparator(ResetButton+1);
			w->InsertOption(ResetButton+2, std::make_pair(static_cast<Buffer *>(0), static_cast<MPD::Song *>(0)), 1, 1);
			w->at(ResetButton+2).first = new Buffer();
			*w->at(ResetButton+2).first << Config.color1 << "Search results: " << Config.color2 << "Found " << found  << (found > 1 ? " songs" : " song") << clDefault;
			w->InsertSeparator(ResetButton+3);
			UpdateFoundList();
			ShowMessage("Searching finished!");
			if (Config.block_search_constraints_change)
				for (size_t i = 0; i < StaticOptions-4; ++i)
					w->Static(i, 1);
			w->Scroll(wDown);
			w->Scroll(wDown);
		}
		else
			ShowMessage("No results found");
	}
	else if (option == ResetButton)
	{
		Reset();
	}
	else
		w->Bold(w->Choice(), myPlaylist->Add(*w->Current().second, w->isBold(), 1));
	
	if (option < SearchButton)
		UnlockStatusbar();
}

void SearchEngine::SpacePressed()
{
	if (w->Current().first)
		return;
	
	if (Config.space_selects)
	{
		w->Select(w->Choice(), !w->isSelected());
		w->Scroll(wDown);
		return;
	}

	int newId = myPlaylist->Add(*w->Current().second, w->isBold(), 0);
	if (newId)
	{
		w->Bold(w->Choice(), true);
	}
	w->Scroll(wDown);
}

void SearchEngine::MouseButtonPressed(MEVENT me)
{
	if (w->Empty() || !w->hasCoords(me.x, me.y) || size_t(me.y) >= w->Size())
		return;
	if (me.bstate & (BUTTON1_PRESSED | BUTTON3_PRESSED))
	{
		if (!w->Goto(me.y))
			return;
		w->Refresh();
		if ((me.bstate & BUTTON3_PRESSED || w->Choice() > itsConstraints->size()) && w->Choice() < StaticOptions)
			EnterPressed();
		else if (w->Choice() >= StaticOptions)
		{
			if (me.bstate & BUTTON1_PRESSED)
			{
				size_t pos = w->Choice();
				SpacePressed();
				if (pos < w->Size()-1)
					w->Scroll(wUp);
			}
			else
				EnterPressed();
		}
	}
	else
		Screen< Menu< std::pair<Buffer *, MPD::Song *> > >::MouseButtonPressed(me);
}

MPD::Song *SearchEngine::CurrentSong()
{
	return !w->Empty() ? w->Current().second : 0;
}

void SearchEngine::GetSelectedSongs(MPD::SongList &v)
{
	if (w->Empty())
		return;
	std::vector<size_t> selected;
	w->GetSelected(selected);
	if (selected.empty() && w->Choice() >= StaticOptions)
		selected.push_back(w->Choice());
	for (std::vector<size_t>::const_iterator it = selected.begin(); it != selected.end(); ++it)
		v.push_back(new MPD::Song(*w->at(*it).second));
}

void SearchEngine::ApplyFilter(const std::string &s)
{
	w->ApplyFilter(s, StaticOptions, REG_ICASE | Config.regex_type);
}

void SearchEngine::UpdateFoundList()
{
	bool bold = 0;
	for (size_t i = StaticOptions; i < w->Size(); ++i)
	{
		for (size_t j = 0; j < myPlaylist->Items->Size(); ++j)
		{
			if (myPlaylist->Items->at(j).GetHash() == w->at(i).second->GetHash())
			{
				bold = 1;
				break;
			}
		}
		w->Bold(i, bold);
		bold = 0;
	}
}

void SearchEngine::Scroll(int input)
{
	size_t pos = w->Choice();
	
	// above the reset button
	if (pos < ResetButton)
	{
		if (Keypressed(input, Key.UpAlbum) || Keypressed(input, Key.UpArtist))
			w->Highlight(0);
		else if (Keypressed(input, Key.DownAlbum) || Keypressed(input, Key.DownArtist))
			w->Highlight(ResetButton);
	}
	// reset button
	else if (pos == ResetButton)
	{
		if (Keypressed(input, Key.UpAlbum) || Keypressed(input, Key.UpArtist))
			w->Highlight(0);
		else if (Keypressed(input, Key.DownAlbum) || Keypressed(input, Key.DownArtist))
			w->Highlight(StaticOptions); // first search result
	}
	// we are in the search results at this point
	else if (pos >= StaticOptions)
	{
		if (Keypressed(input, Key.UpAlbum))
		{
			if (pos == StaticOptions)
			{
				w->Highlight(ResetButton);
				return;
			}
			else
			{
				std::string album = w->at(pos).second->GetAlbum();
				while (pos > StaticOptions)
					if (w->at(--pos).second->GetAlbum() != album)
						break;
			}
		}
		else if (Keypressed(input, Key.DownAlbum))
		{
			std::string album = w->at(pos).second->GetAlbum();
			while (pos < w->Size() - 1)
				if (w->at(++pos).second->GetAlbum() != album)
					break;
		}
		else if (Keypressed(input, Key.UpArtist))
		{
			if (pos == StaticOptions)
			{
				w->Highlight(0);
				return;
			}
			else
			{
				std::string artist = w->at(pos).second->GetArtist();
				while (pos > StaticOptions)
					if (w->at(--pos).second->GetArtist() != artist)
						break;
			}
		}
		else if (Keypressed(input, Key.DownArtist))
		{
			std::string artist = w->at(pos).second->GetArtist();
			while (pos < w->Size() - 1)
				if (w->at(++pos).second->GetArtist() != artist)
					break;
		}
		w->Highlight(pos);
	}
}

void SearchEngine::SelectAlbum()
{
	size_t pos = w->Choice();
	if (pos < StaticOptions)
		return;		// not on a song
	
	std::string album = w->at(pos).second->GetAlbum();

	// select song under cursor
	w->Select(pos, 1);

	// go up
	while (pos > StaticOptions)
	{
		if (w->at(--pos).second->GetAlbum() != album)
			break;
		else
			w->Select(pos, 1);
	}

	// go down
	while (pos < w->Size() - 1)
	{
		if (w->at(++pos).second->GetAlbum() != album)
			break;
		else
			w->Select(pos, 1);
	}
}

void SearchEngine::Prepare()
{

	for (size_t i = 0; i < w->Size(); ++i)
	{
		if (i == itsConstraints->size() || i == SearchButton-1 || i == ResetButton+1 || i == ResetButton+3) // separators
			continue;
		delete (*w)[i].first;
		delete (*w)[i].second;
	}
//	if (newConstraints)
////	if (newConstraints != ConstraintsNames)
//	{
//		ConstraintsNames = newConstraints;
//		itsConstraints.resize(ConstraintsNames->size());
//	}

    itsConstraints = constraintsMap[*SearchSource];

	StaticOptions = itsConstraints->size() + 9;
	ResetButton = itsConstraints->size() + 5;
	SearchButton = itsConstraints->size() + 4;

	w->SetTitle("");
	w->Clear();
	w->ResizeList(StaticOptions-3);
	
	w->IntoSeparator(itsConstraints->size());
	w->IntoSeparator(SearchButton-1);
	
	for (size_t i = 0; i < StaticOptions-3; ++i)
	{
		if (i == itsConstraints->size() || i == SearchButton-1) // separators
			continue;
		(*w)[i].first = new Buffer();
	}
	
	for (size_t i = 0; i < itsConstraints->size(); ++i)
	{
		*(*w)[i].first << fmtBold << std::setw(10) << std::left << itsConstraints->at(i)->GetName() << fmtBoldEnd << ' ';
		ShowTag(*(*w)[i].first, itsConstraints->at(i)->Get());
	}
	
	*w->at(itsConstraints->size()+1).first << fmtBold << "Search in:" << fmtBoldEnd << ' ' << *SearchSource;
	*w->at(itsConstraints->size()+2).first << fmtBold << "Search mode:" << fmtBoldEnd << ' ' << *SearchMode;
	
	*w->at(SearchButton).first << "Search";
	*w->at(ResetButton).first << "Reset";
}

void SearchEngine::Reset()
{
	for (size_t i = 0; i < itsConstraints->size(); ++i)
		itsConstraints->at(i)->Set("");
	w->Reset();
	Prepare();
	ShowMessage("Search state reset");
}

void SearchEngine::Search()
{
	bool constraints_empty = 1;
	for (size_t i = 0; i < itsConstraints->size(); ++i)
	{
		if (!itsConstraints->at(i)->Get().empty())
		{
			constraints_empty = 0;
			break;
		}
	}
	//TODO::grandma
    //if (constraints_empty)
    //	return;
	
	if (SearchSource == &SearchSources[2])
	{
		VKFetcher vkFetcher;
		if (!itsConstraints->at(0)->Get().empty())
            vkFetcher.SetAny(itsConstraints->at(0)->Get());
		if (!itsConstraints->at(1)->Get().empty())
            vkFetcher.SetArtist(itsConstraints->at(1)->Get());
		if (!itsConstraints->at(2)->Get().empty())
            vkFetcher.SetTitle(itsConstraints->at(2)->Get());
		if (!itsConstraints->at(3)->Get().empty())
            vkFetcher.SetCount(itsConstraints->at(3)->Get());
//		if (!itsConstraints[4].empty())
//			Mpd.AddSearchURI(itsConstraints[4]);
//		if (!itsConstraints[5].empty())
//			Mpd.AddSearch(MPD_TAG_COMPOSER, itsConstraints[5]);
//		if (!itsConstraints[6].empty())
//			Mpd.AddSearch(MPD_TAG_PERFORMER, itsConstraints[6]);
//		if (!itsConstraints[7].empty())
//			Mpd.AddSearch(MPD_TAG_GENRE, itsConstraints[7]);
//		if (!itsConstraints[8].empty())
//			Mpd.AddSearch(MPD_TAG_DATE, itsConstraints[8]);
//		if (!itsConstraints[9].empty())
//			Mpd.AddSearch(MPD_TAG_COMMENT, itsConstraints[9]);

		MPD::SongList *list = vkFetcher.GetList();

		if (!list)
		{
			return;
		}

		for (auto song: *list)
		{
			w->AddOption(std::make_pair(static_cast<Buffer *>(0), song));

		}
		list->clear();
		return;
	}

	if (SearchSource == &SearchSources[0] && (SearchMode == &SearchModes[0] || SearchMode == &SearchModes[2])) // use built-in mpd searching
	{
		Mpd.StartSearch(SearchMode == &SearchModes[2]);
		ConstraintsVector::iterator it = ++(itsConstraints->begin());
		ConstraintsVector::iterator endIt = --(itsConstraints->end());

		if (!itsConstraints->at(0)->Get().empty())
			Mpd.AddSearchAny(itsConstraints->at(0)->Get());

		int size = itsConstraints->size();
		if (!itsConstraints->at(size - 1)->Get().empty())
			Mpd.AddSearchURI(itsConstraints->at(size - 1)->Get());

		for (; it != endIt; ++it)
		{
			if (!(*it)->Get().empty())
            {
                Mpd.AddSearch(IntoTagItem((*it)->GetTag()), (*it)->Get());
            }

		}

		MPD::SongList results;
		Mpd.CommitSearch(results);
		for (MPD::SongList::const_iterator it = results.begin(); it != results.end(); ++it)
			w->AddOption(std::make_pair(static_cast<Buffer *>(0), *it));
		return;
	}

	MPD::SongList list;
	if (SearchSource == &SearchSources[0])
		Mpd.GetDirectoryRecursive("/", list);
	else // if(SearchSource == &SearchSources[1])  // TODO: grandma вернуть условие
	{
		list.reserve(myPlaylist->Items->Size());
		for (size_t i = 0; i < myPlaylist->Items->Size(); ++i)
			list.push_back(&(*myPlaylist->Items)[i]);
	}

	bool any_found = 1;
	bool found = 1;
	
	for (MPD::SongList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if (SearchMode != &SearchModes[2]) // match to pattern
		{
			regex_t rx;
			if (!itsConstraints->at(0)->Get().empty())
			{
				if (regcomp(&rx, itsConstraints->at(0)->Get().c_str(), REG_ICASE | Config.regex_type) == 0)
				{
					any_found =
						!regexec(&rx, (*it)->GetArtist().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetTitle().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetAlbum().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetName().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetComposer().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetPerformer().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetGenre().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetDate().c_str(), 0, 0, 0)
					||	!regexec(&rx, (*it)->GetComment().c_str(), 0, 0, 0);
				}
				regfree(&rx);
			}
			

			MPD::Song::GetFunction get;
			ConstraintsVector::iterator itConstraints = itsConstraints->begin() + 1;
            for (; itConstraints != itsConstraints->end(); ++itConstraints )
			{
				if (found && !(*itConstraints)->Get().empty())
				{
					get = toGetFunction((*itConstraints)->GetTag());
                    if (!regcomp(&rx, (*itConstraints)->Get().c_str(), REG_ICASE | Config.regex_type))
						found = !regexec(&rx, (*it)->GetTags(get).c_str(), 0, 0, 0);
					regfree(&rx);
				}
			}
		}
		else // match only if values are equal
		{
			CaseInsensitiveStringComparison cmp;
			
            if (!itsConstraints->at(0)->Get().empty())
				any_found =
                    !cmp((*it)->GetArtist(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetTitle(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetAlbum(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetName(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetComposer(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetPerformer(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetGenre(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetDate(), itsConstraints->at(0)->Get())
                ||	!cmp((*it)->GetComment(), itsConstraints->at(0)->Get());
			
            MPD::Song::GetFunction get;
            ConstraintsVector::iterator itConstraints = itsConstraints->begin() + 1;
            for (; itConstraints != itsConstraints->end(); ++itConstraints )
            {
                if (found && !(*itConstraints)->Get().empty())
                {
                    get = toGetFunction((*itConstraints)->GetTag());
                    found = !cmp((*itConstraints)->Get(), (*it)->GetTags(get));
                    //FIXME: сейчас будут браться ВСЕ теги у песни через запятую
                    // такое поведение не желательно
                }
            }
		}
		
		if (found && any_found)
		{
            MPD::Song *ss = (SearchSource == &SearchSources[0]) ? *it : new MPD::Song(**it);
			w->AddOption(std::make_pair(static_cast<Buffer *>(0), ss));
			list[it-list.begin()] = 0;
		}
		found = 1;
		any_found = 1;
	}
    if (SearchSource == &SearchSources[0]) // free song list only if it's database
		MPD::FreeSongList(list);
}

std::string SearchEngine::SearchEngineOptionToString(const std::pair<Buffer *, MPD::Song *> &pair, void *)
{
	if (!Config.columns_in_search_engine)
		return pair.second->toString(Config.song_list_format_dollar_free);
	else
		return Playlist::SongInColumnsToString(*pair.second, 0);
}

