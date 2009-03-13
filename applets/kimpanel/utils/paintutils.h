/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef PAINTUTILS_H
#define PAINTUTILS_H

#include <plasma/theme.h>

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEngine>
#include <QPixmap>

QPixmap renderText(QString text, 
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), QColor bgColor = Qt::transparent,
    const QFont &ft = qApp->font());

#endif // PAINTUTILS_H
