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

#include "includes/generic.h"
#include "includes/cprate.h"
#include "includes/ratedialog.h"

ZCPRate::ZCPRate(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this, QSL("in"));
    registerInput(fInp);

    fOut=new ZCPOutput(this, QSL("out"));
    registerOutput(fOut);

    fCtlOut=new ZCPOutput(this, QSL("ctl"),CStructures::PinClass::pcCTL);
    registerOutput(fCtlOut);
}

ZCPRate::~ZCPRate() = default;

QSize ZCPRate::minimumSizeHint() const
{
    return QSize(180,65);
}

void ZCPRate::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/3);
    fCtlOut->relCoord=QPoint(width()-zcpPinSize/2,2*height()/3);
}

int ZCPRate::getRate() const
{
    return m_rate;
}

void ZCPRate::doInfoGenerate(QTextStream & stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << Qt::endl;
    stream << QSL("  type rate") << Qt::endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << Qt::endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << Qt::endl;
        stream << QSL("    rate ") << m_rate << Qt::endl;
        stream << QSL("  }") << Qt::endl;
    }
    if (!m_converter.isEmpty())
        stream << QSL("  converter \"") << m_converter << QSL("\"") << Qt::endl;
    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << Qt::endl;
    stream << Qt::endl;
    if (fCtlOut->toFilter) {
        stream << QSL("ctl.") << objectName() << QSL(" {") << Qt::endl;
        fCtlOut->toFilter->doCtlGenerate(stream,warnings);
        stream << QSL("}") << Qt::endl;
        stream << Qt::endl;
    }
}

void ZCPRate::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("SRC"));

    QString s = QSL("%1 Hz").arg(m_rate);
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,s);

    p.restore();
}

void ZCPRate::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));
    m_rate = json.toObject().value(QSL("rate")).toInt();
    m_converter = json.toObject().value(QSL("converter")).toString();
}

QJsonValue ZCPRate::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());
    data.insert(QSL("rate"),m_rate);
    data.insert(QSL("converter"),m_converter);
    return data;
}

void ZCPRate::showSettingsDlg()
{
    ZRateDialog d(window());
    d.setParams(m_rate,m_converter);

    if (d.exec()==QDialog::Rejected) return;

    Q_EMIT componentChanged(this);

    m_rate=d.getRate();
    m_converter=d.getConverter();
    update();
}

