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
***************************************************************************/

#ifndef CPLADSPA_H
#define CPLADSPA_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"
#include "ladspadialog.h"

class ZCPLADSPA : public ZCPBase
{
    Q_OBJECT
private:
    QString m_plugLabel;
    QString m_plugID;
    QString m_plugName;
    QString m_plugLibrary;
    ZLADSPAControlItems m_plugControls;
    int searchSampleRate();

public:
    ZCPInput* fInp;
    ZCPOutput* fOut;

    ZCPLADSPA(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPLADSPA() override;

    void readFromStreamLegacy(QDataStream & stream) override;
    void readFromJson(const QJsonValue& json) override;
    QJsonValue storeToJson() const override;

    QSize minimumSizeHint() const override;
protected:
    void paintEvent (QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream) const override;
    void showSettingsDlg() override;
};
#endif
