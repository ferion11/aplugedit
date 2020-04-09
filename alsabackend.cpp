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
*   Parts of this code from ALSA project aplay.c utility.                 *
*                                                                         *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#include <QApplication>
#include <alsa/asoundlib.h>
#include "includes/generic.h"
#include "includes/alsabackend.h"

ZAlsaBackend::ZAlsaBackend(QObject *parent)
    : QObject(parent)
{
}

ZAlsaBackend::~ZAlsaBackend() = default;

ZAlsaBackend *ZAlsaBackend::instance()
{
    static QPointer<ZAlsaBackend> inst;
    static QAtomicInteger<bool> initializedOnce(false);

    if (inst.isNull()) {
        if (initializedOnce.testAndSetAcquire(false,true)) {
            inst = new ZAlsaBackend(QApplication::instance());
            return inst.data();
        }

        qCritical() << "Accessing to gAlsa after destruction!!!";
        return nullptr;
    }

    return inst.data();
}

void ZAlsaBackend::initialize()
{
    enumerateCards();
}

void ZAlsaBackend::reloadGlobalConfig()
{
    snd_config_update_free_global();
    snd_config_update();
}

void ZAlsaBackend::snd_lib_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...)
{
    va_list arg;
    va_start(arg, fmt);
    QString msg = QString::vasprintf(fmt,arg);
    if (err != 0)
        msg.append(QSL(": %1").arg(QString::fromUtf8(snd_strerror(err))));
    va_end(arg);

    msg = QSL("ALSA: %1: %2 (%3:%4)")
            .arg(QString::fromUtf8(function),msg,QString::fromUtf8(file))
            .arg(line);

    auto alsa = gAlsa;
    if (alsa)
        Q_EMIT alsa->alsaErrorMsg(msg);
}

void ZAlsaBackend::setupErrorLogger()
{
    snd_lib_error_set_handler(&snd_lib_error_handler);
}

void ZAlsaBackend::enumerateCards()
{
    m_cards.clear();

    static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    int card = -1;
    int err;
    int dev;
    snd_ctl_t *handle = nullptr;

    snd_ctl_card_info_t *info = nullptr;
    snd_pcm_info_t *pcminfo = nullptr;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    if (snd_card_next(&card) < 0 || card < 0) {
        qWarning() << "no soundcards found!";
        return;
    }
    // **** List of Hardware Devices ****
    while (card >= 0) {
        QString name = QSL("hw:%1").arg(card);

        if ((err = snd_ctl_open(&handle, name.toLatin1().constData(), 0)) < 0) {
            qWarning() << QSL("control open (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
        } else {
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                qWarning() << QSL("control hardware info (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
            } else {

                m_cards << CCardItem(QString::fromUtf8(snd_ctl_card_info_get_id(info)),
                                     QString::fromUtf8(snd_ctl_card_info_get_name(info)),
                                     card);
                dev = -1;
                while (true) {
                    unsigned int count;
                    if (snd_ctl_pcm_next_device(handle, &dev)<0)
                        qWarning() << "snd_ctl_pcm_next_device";
                    if (dev < 0)
                        break;
                    snd_pcm_info_set_device(pcminfo, static_cast<unsigned int>(dev));
                    snd_pcm_info_set_subdevice(pcminfo, 0);
                    snd_pcm_info_set_stream(pcminfo, stream);
                    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                            qWarning() << QSL("control digital audio info (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
                        continue;
                    }
                    count = snd_pcm_info_get_subdevices_count(pcminfo);
                    m_cards.last().devices << CDeviceItem(dev,static_cast<int>(count),
                                                          QString::fromUtf8(snd_pcm_info_get_name(pcminfo)));
                }
            }
            if (handle)
                snd_ctl_close(handle);
        }
        if (snd_card_next(&card) < 0)
            break;
    }
}

QVector<CCardItem> ZAlsaBackend::cards() const
{
    return m_cards;
}

QVector<CPCMItem> ZAlsaBackend::pcmList() const
{
    QVector<CPCMItem> res;

    const char *filter = "Output";

    void **hints;
    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return res;

    void **n = hints;
    while (*n != nullptr) {
        QScopedPointer<char,QScopedPointerPodDeleter> name(snd_device_name_get_hint(*n, "NAME"));
        QScopedPointer<char,QScopedPointerPodDeleter> descr(snd_device_name_get_hint(*n, "DESC"));
        QScopedPointer<char,QScopedPointerPodDeleter> io(snd_device_name_get_hint(*n, "IOID"));
        if (!(io != nullptr && strcmp(io.data(), filter) != 0)) {
            QStringList descList;
            if (descr != nullptr)
                descList = QString::fromUtf8(descr.data()).split('\n');
            res.append(CPCMItem(QString::fromUtf8(name.data()),descList));
        }
        n++;
    }
    snd_device_name_free_hint(hints);

    return res;
}

bool ZAlsaBackend::getCardNumber(const QString& name, QString &cardId, unsigned int *devNum, unsigned int *subdevNum) const
{
    int err;
    QByteArray deviceName = name.toUtf8();
    snd_ctl_t *ctl;
    snd_ctl_card_info_t *info;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_t *pcminfo;
    snd_pcm_info_alloca(&pcminfo);

    err = snd_ctl_open(&ctl, deviceName.constData(), SND_CTL_NONBLOCK);
    if (err < 0) {
        qDebug() << "Failed to open ctl device: " << deviceName << snd_strerror(err);
        return false;
    }

    err = snd_ctl_card_info(ctl,info);
    if (err < 0) {
        qDebug() << "Failed to get ctl device info: " << deviceName << snd_strerror(err);
        snd_ctl_close(ctl);
        return false;
    }

    cardId = QString::fromUtf8(snd_ctl_card_info_get_id(info));

    err = snd_ctl_pcm_info(ctl,pcminfo);
    if (err < 0) {
        qDebug() << "Failed to get ctl device pcm info: " << deviceName << snd_strerror(err);
        snd_ctl_close(ctl);
        return false;
    }

    *devNum = snd_pcm_info_get_device(pcminfo);
    *subdevNum = snd_pcm_info_get_subdevice(pcminfo);

    snd_ctl_close(ctl);

    return true;
}

CCardItem::CCardItem(const QString &aCardID, const QString &aCardName, int aCardNum)
{
    cardName = aCardName;
    cardNum = aCardNum;
    cardID = aCardID;
}

CDeviceItem::CDeviceItem(int aDevNum, int aSubdevices, const QString& aName)
{
    devNum = aDevNum;
    subdevices = aSubdevices;
    devName = aName;
}

CPCMItem::CPCMItem(const QString &aName)
{
    name = aName;
}

CPCMItem::CPCMItem(const QString &aName, const QStringList &aDescription)
{
    name = aName;
    description = aDescription;
}

bool CPCMItem::operator==(const CPCMItem &s) const
{
    return (name == s.name);
}

bool CPCMItem::operator!=(const CPCMItem &s) const
{
    return !operator==(s);
}
