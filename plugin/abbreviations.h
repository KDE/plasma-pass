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

#include <QVector>

class QStringList;
class QStringRef;
class QString;

namespace PlasmaPass {

bool matchesAbbreviation(const QStringRef &word, const QStringRef &typed);

bool matchesPath(const QStringRef &path, const QStringRef &typed);

/**
 * @brief Matches a path against a list of search fragments.
 * @return -1 when no match is found, otherwise a positive integer, higher values mean lower quality
 */
int matchPathFilter(const QVector<QStringRef> &toFilter, const QVector<QStringRef> &text);

}

#endif
