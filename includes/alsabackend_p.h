/***************************************************************************
*   Copyright (C) 2006 - 2020 by kernelonline@gmail.com                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
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
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************
*                                                                         *
*   Parts of this code from ALSA project utilities:                       *
*   amixer, aplay, alsactl.                                               *
*                                                                         *
*   Copyright (c) by Takashi Iwai <tiwai@suse.de>                         *
*   Copyright (c) by Abramo Bagnara <abramo@alsa-project.org>             *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#ifndef ALSABACKENDPRIVATE_H
#define ALSABACKENDPRIVATE_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QHash>
#include "alsastructures.h"

class ZAlsaBackend;

class ZAlsaBackendPrivate : public QObject
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(ZAlsaBackendPrivate)
    Q_DECLARE_PUBLIC(ZAlsaBackend)
    ZAlsaBackend* q_ptr { nullptr };

    void addDebugOutputPrivate(const QString& msg);
    void closeAllMixerCtls();

public:
    explicit ZAlsaBackendPrivate(ZAlsaBackend *parent);
    ~ZAlsaBackendPrivate() override;

    QVector<CCardItem> m_cards;
    QVector<CCTLItem> m_mixerCtl;
    QStringList m_alsaWarnings;
    QTimer m_mixerPollTimer;
    QStringList m_debugMessages;
    QMutex m_loggerMutex;

    snd_ctl_t* getMixerCtl(const QString &ctlName) const;
    void addAlsaWarning(const QString& msg);
    void enumerateCards();
    void enumerateMixers();
    static void snd_lib_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...);
    static bool lessThanMixerItem(const CMixerItem &a, const CMixerItem &b);
    QVector<int> findRelatedMixerItems(const CMixerItem &base, const QVector<CMixerItem> &items, int *topScore) const;

    static void stdConsoleOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private Q_SLOTS:
    void pollMixerEvents();

};

#endif // ALSABACKENDPRIVATE_H
