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

#ifndef CPPLUG_H
#define CPPLUG_H

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class ZCPPlug : public ZCPBase
{
    Q_OBJECT
private:
    ZCPInput* fInp { nullptr };
    ZCPOutput* fOut { nullptr };

public:
    ZCPPlug(QWidget *parent, ZRenderArea *aOwner);
    ~ZCPPlug() override;

    QSize minimumSizeHint() const override;

protected:
    void paintEvent (QPaintEvent * event) override;
    void realignPins() override;
    void doInfoGenerate(QTextStream & stream) const override;

};

#endif // CPPLUG_H