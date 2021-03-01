// SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PLASMAPASS_ABBREVIATIONS_H
#define PLASMAPASS_ABBREVIATIONS_H

#include <QVector>

class QStringList;
class QStringRef;
class QString;

namespace PlasmaPass
{
bool matchesAbbreviation(const QStringRef &word, const QStringRef &typed);

bool matchesPath(const QStringRef &path, const QStringRef &typed);

/**
 * @brief Matches a path against a list of search fragments.
 * @return -1 when no match is found, otherwise a positive integer, higher values mean lower quality
 */
int matchPathFilter(const QVector<QStringRef> &toFilter, const QVector<QStringRef> &text);

}

#endif
