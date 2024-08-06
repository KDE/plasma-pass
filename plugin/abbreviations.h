// SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PLASMAPASS_ABBREVIATIONS_H
#define PLASMAPASS_ABBREVIATIONS_H

#include <QStringList>
#include <QVector>
class QString;

namespace PlasmaPass
{
bool matchesAbbreviation(const QStringView &word, const QStringView &typed);

bool matchesPath(const QStringView &path, const QStringView &typed);

/**
 * @brief Matches a path against a list of search fragments.
 * @return -1 when no match is found, otherwise a positive integer, higher values mean lower quality
 */
int matchPathFilter(const QVector<QStringView> &toFilter, const QVector<QStringView> &text);
}

#endif
