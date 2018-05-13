/*
 * Borrowed from KDevelop (kdevplatform/language/interfaces/abbreviations.h)
 *
 * Copyright 2014 Sven Brauch <svenbrauch@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PLASMAPASS_ABBREVIATIONS_H
#define PLASMAPASS_ABBREVIATIONS_H

class QStringList;
class QStringRef;
class QString;

namespace PlasmaPass {

bool matchesAbbreviation(const QStringRef& word, const QString& typed);

bool matchesPath(const QString& path, const QString& typed);

/**
 * @brief Matches a word against a list of search fragments.
 * The word will be split at separation characters (space, / and \c @) and
 * the resulting fragments will be matched one-by-one against the typed fragments.
 * If all typed fragments can be matched against a fragment in word in the right order
 * (skipping is allowed), true will be returned.
 * @param word the word to search in
 * @param typedFragments the fragments which were typed
 * @return bool true if match, else false
 */
bool matchesAbbreviationMulti(const QString& word, const QStringList& typedFragments);

#if 0
int matchPathFilter(const Path& toFilter, const QStringList& text, const Path& prefixPath)
#endif

}

#endif
