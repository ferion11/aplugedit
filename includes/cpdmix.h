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

#ifndef CPDMIX_H
#define CPDMIX_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class ZCPDMix : public ZCPBase
{
    Q_OBJECT
private:
    ZCPInput* fInp;
    ZCPOutput* fOut;

public:
    ZCPDMix(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPDMix() override;

    QSize minimumSizeHint() const override;
    bool canConnectOut(ZCPBase * toFilter) override;

protected:
    void paintEvent (QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream) const override;
};
#endif