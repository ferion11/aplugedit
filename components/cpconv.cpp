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

#include "includes/cpbase.h"
#include "includes/cpconv.h"
#include "includes/convdialog.h"

ZCPConv::ZCPConv(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp = new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut = new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
    m_converter=alcLinear;
    m_format=QSL("S16_LE");
}

void ZCPConv::setConverterType(ZCPConv::ConverterType type)
{
    m_converter = type;
}

ZCPConv::~ZCPConv() = default;

QSize ZCPConv::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPConv::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPConv::doInfoGenerate(QTextStream & stream) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    QString conv = QSL("linear");
    switch (m_converter) {
        case alcLinear: conv=QSL("linear"); break;
        case alcFloat: conv=QSL("lfloat"); break;
        case alcIEC958: conv=QSL("iec958"); break;
        case alcMuLaw: conv=QSL("mulaw"); break;
        case alcALaw: conv=QSL("alaw"); break;
        case alcImaADPCM: conv=QSL("adpcm"); break;
    }
    stream << QSL("  type ") << conv << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        if (m_converter!=alcIEC958)
            stream << QSL("    format ") << m_format << endl;
        stream << QSL("  }") << endl;
    }
    stream << QSL("}") << endl;
    stream << endl;
    if (fOut->toFilter)
        fOut->toFilter->doGenerate(stream);
}

void ZCPConv::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QPen pn=QPen(Qt::black);
    QPen op=p.pen();
    QBrush ob=p.brush();
    QFont of=p.font();
    pn.setWidth(2);
    p.setPen(pn);
    p.setBrush(QBrush(Qt::white,Qt::SolidPattern));

    p.drawRect(rect());

    redrawPins(p);

    QFont n=of;
    n.setBold(true);
    n.setPointSize(n.pointSize()+1);
    p.setFont(n);
    p.drawText(rect(),Qt::AlignCenter,QSL("Converter"));

    n.setBold(false);
    n.setPointSize(n.pointSize()-3);
    p.setPen(QPen(Qt::gray));
    p.setFont(n);
    
    QString conv = QSL("linear");
    switch (m_converter) {
        case alcLinear: conv=QSL("linear"); break;
        case alcFloat: conv=QSL("linear<->float"); break;
        case alcIEC958: conv=QSL("linear<->IEC958 frames"); break;
        case alcMuLaw: conv=QSL("linear<->MuLaw"); break;
        case alcALaw: conv=QSL("linear<->ALaw"); break;
        case alcImaADPCM: conv=QSL("linear<->ImaADPCM"); break;
    }
    p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,conv);

    p.setFont(of);
    p.setBrush(ob);
    p.setPen(op);
}

void ZCPConv::readFromStreamLegacy( QDataStream & stream )
{
    ZCPBase::readFromStreamLegacy(stream);
    stream.readRawData(reinterpret_cast<char*>(&m_converter),sizeof(m_converter));
    stream >> m_format;
}

void ZCPConv::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    auto convEnum = QMetaEnum::fromType<ConverterType>();
    m_converter = static_cast<ConverterType>(convEnum.keyToValue(json.toObject().value(QSL("converter"))
                                                                 .toString().toLatin1().constData()));

    m_format = json.toObject().value(QSL("format")).toString();
}

QJsonValue ZCPConv::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"),ZCPBase::storeToJson());

    auto convEnum = QMetaEnum::fromType<ConverterType>();
    data.insert(QSL("converter"),convEnum.valueToKey(m_converter));

    data.insert(QSL("format"),m_format);

    return data;
}

void ZCPConv::showSettingsDlg()
{
    ZConvDialog d(window());
    QString conv = QSL("linear");
    switch (m_converter) {
        case alcLinear: conv=QSL("linear"); break;
        case alcFloat: conv=QSL("linear<->float"); break;
        case alcIEC958: conv=QSL("linear<->IEC958 frames"); break;
        case alcMuLaw: conv=QSL("linear<->MuLaw"); break;
        case alcALaw: conv=QSL("linear<->ALaw"); break;
        case alcImaADPCM: conv=QSL("linear<->ImaADPCM"); break;
    }
    d.alConverter->setText(conv);

    int fmtidx=d.alFormat->findText(m_format,Qt::MatchStartsWith | Qt::MatchCaseSensitive);
    if ((fmtidx>=0) && (fmtidx<d.alFormat->count()))
        d.alFormat->setCurrentIndex(fmtidx);

    if (d.exec()==QDialog::Rejected)
        return;

    Q_EMIT componentChanged(this);
    m_format=d.alFormat->currentText().section(QSL(" "),0,0);
    update();
}