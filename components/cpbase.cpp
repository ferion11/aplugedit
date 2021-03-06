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

#include <cstring>
#include "includes/generic.h"
#include "includes/renderarea.h"
#include "includes/cpbase.h"
#include "includes/cpplug.h"
#include "includes/cpconv.h"
#include "ui_hintdlg.h"

ZCPBase::ZCPBase(QWidget *parent, ZRenderArea *aOwner)
    : QWidget(parent)
{
    m_owner=aOwner;
}

bool ZCPBase::canConnectOut(ZCPBase *toFilter)
{
    Q_UNUSED(toFilter)
    return true;
}

bool ZCPBase::canConnectIn(ZCPBase *toFilter)
{
    Q_UNUSED(toFilter)
    return true;
}

void ZCPBase::deleteOutput(int idx)
{
    for (int i=0;i<fOutputs.count();i++) {
        if ((fOutputs.at(i)->toPin!=-1) && (fOutputs.at(i)->toFilter)) {
            if (i==idx) { // disconnect specified output from input
                fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                        ->links.removeAll(CInpLink(i,this));
                fOutputs.at(i)->toFilter=nullptr;
                fOutputs.at(i)->toPin=-1;
            } else if (i>idx) { // shift pin number
                int lnk = fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                          ->links.indexOf(CInpLink(i,this));
                if (lnk>=0)
                    fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)->links[lnk].fromPin--;
            }
        }
    }
    auto *out = fOutputs.takeAt(idx);
    m_owner->repaintConn();
    out->deleteLater();
}

void ZCPBase::deleteComponent()
{
    for (const auto &inp : qAsConst(fInputs)) {
        while (!(inp->links.isEmpty())) {
            const auto link = inp->links.constLast();
            if ((link.fromPin!=-1) && (link.fromFilter!=nullptr)) {
                link.fromFilter->fOutputs[link.fromPin]->toFilter=nullptr;
                link.fromFilter->fOutputs[link.fromPin]->toPin=-1;
            }
            inp->links.removeLast();
        }
    }
    for (int i=0;i<fOutputs.count();i++) {
        if ((fOutputs.at(i)->toPin!=-1) && (fOutputs.at(i)->toFilter!=nullptr)) {
            fOutputs.at(i)->toFilter->fInputs.at(fOutputs.at(i)->toPin)
                    ->links.removeAll(CInpLink(i,this));
        }
        fOutputs[i]->toFilter=nullptr;
        fOutputs[i]->toPin=-1;
    }
    m_owner->repaintConn();
    deleteLater();
}

void ZCPBase::mouseInPin(const QPoint & mx, int &aPinNum, PinType &aPinType, ZCPBase* &aFilter)
{
    // Note: x and y must be in global screen coordinate system.
    QRect r;
    QPoint p;
    aPinNum=-1;
    aPinType=PinType::ptInput;
    aFilter=nullptr;
    const auto cplist = ownerArea()->findComponents<ZCPBase*>();
    for (const auto &c : cplist)
    {
        int j = 0;
        for (const auto &a : qAsConst(c->fInputs))
        {
            p=QPoint(m_owner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+zcpPinSize/2,
                                                 a->ownerFilter->y()+a->relCoord.y()+zcpPinSize/2)));
            r=QRect(p.x()-zcpPinSize,p.y()-zcpPinSize,2*zcpPinSize,2*zcpPinSize);
            if (r.contains(mx.x(),mx.y()))
            {
                aPinNum=j;
                aPinType=PinType::ptInput;
                aFilter=c;
                return;
            }
            j++;
        }

        j = 0;
        for (const auto &a : qAsConst(c->fOutputs))
        {
            p=QPoint(m_owner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+zcpPinSize/2,
                                                 a->ownerFilter->y()+a->relCoord.y()+zcpPinSize/2)));
            r=QRect(p.x()-zcpPinSize,p.y()-zcpPinSize,2*zcpPinSize,2*zcpPinSize);
            if (r.contains(mx.x(),mx.y()))
            {
                aPinNum=j;
                aPinType=PinType::ptOutput;
                aFilter=c;
                return;
            }
            j++;
        }
    }
}

void ZCPBase::redrawPins(QPainter & painter)
{
    realignPins();

    painter.save();
    QFont n = painter.font();
    n.setBold(false);
    painter.setFont(n);
    for (const auto & a : qAsConst(fInputs))
    {
        QPen penPin=QPen(a->getPinColor());
        QBrush brshPin=QBrush(a->getPinColor(),Qt::SolidPattern);
        painter.setPen(penPin);
        painter.setBrush(brshPin);

        painter.fillRect(QRect(   a->relCoord.x()-zcpPinSize/2,
                                  a->relCoord.y()-zcpPinSize/2,
                                  zcpPinSize,
                                  zcpPinSize),brshPin);
        painter.drawText(QPoint(  a->relCoord.x()+zcpPinSize/2+1,
                                  a->relCoord.y()+painter.fontMetrics().height()/4),
                         a->pinName);

    }
    for (const auto & a : qAsConst(fOutputs))
    {
        QPen penPin=QPen(a->getPinColor());
        QBrush brshPin=QBrush(a->getPinColor(),Qt::SolidPattern);
        painter.setPen(penPin);
        painter.setBrush(brshPin);

        painter.fillRect(QRect(   a->relCoord.x()-zcpPinSize/2,
                                  a->relCoord.y()-zcpPinSize/2,
                                  zcpPinSize,
                                  zcpPinSize),brshPin);
        painter.drawText(QPoint(  a->relCoord.x()-zcpPinSize/2-1 - painter.fontMetrics().boundingRect(a->pinName).width(),
                                  a->relCoord.y()+painter.fontMetrics().height()/4), a->pinName);
    }
    painter.restore();
}

ZRenderArea *ZCPBase::ownerArea() const
{
    return m_owner;
}

void ZCPBase::registerInput(ZCPInput *inp)
{
    fInputs.append(inp);
}

void ZCPBase::registerOutput(ZCPOutput *out)
{
    fOutputs.append(out);
}

bool ZCPBase::postLoadBind()
{
    for (const auto &a : qAsConst(fInputs)) {
        if (!(a->postLoadBind()))
            return false;
    }
    for (const auto &a : qAsConst(fOutputs)) {
        if (!(a->postLoadBind()))
            return false;
    }
    return true;
}

void ZCPBase::checkRecycle()
{
    if (frameGeometry().intersects(m_owner->m_recycle->frameGeometry()))
        deleteComponent();
}

void ZCPBase::showHintDlg()
{
    QDialog dlg(window());
    Ui::ZHintDialog ui;
    ui.setupUi(&dlg);

    ui.editHint->setText(m_hint);
    ui.checkShowHint->setChecked(m_hintShow);

    if (dlg.exec()==QDialog::Rejected) return;

    m_hint = ui.editHint->text();
    m_hintShow = ui.checkShowHint->isChecked();

    Q_EMIT componentChanged(this);
    update();
}

void ZCPBase::setBaseFont(QPainter &p, ZCPBase::FontType type) const
{
    QFont res = p.font();
    switch (type) {
        case ftTitle:
            res.setBold(true);
            res.setPointSize(res.pointSize()+1);
            p.setFont(res);
            p.setPen(Qt::black);
            break;
        case ftDesc:
            res.setBold(false);
            res.setPointSize(res.pointSize()-2);
            p.setFont(res);
            p.setPen(Qt::gray);
            break;
        case ftHint:
            res.setBold(false);
            res.setPointSize(res.pointSize()-3);
            p.setFont(res);
            p.setPen(Qt::blue);
            break;
    }
}

void ZCPBase::showCtxMenu(const QPoint &pos)
{
    QMenu menu;
    addCtxMenuItems(&menu);
    if (!menu.isEmpty())
        menu.addSeparator();

    QAction* ac = menu.addAction(tr("Hint..."));
    connect(ac,&QAction::triggered,this,&ZCPBase::showHintDlg);
    ac->setEnabled(isHintSupported());

    ac = menu.addAction(tr("Properties..."));
    connect(ac,&QAction::triggered,this,[this](){
        showSettingsDlg();
    });
    ac->setEnabled(needSettingsDlg());

    menu.exec(pos);
}

void ZCPBase::mouseMoveEvent(QMouseEvent * event)
{
    if (m_isDragging)
    {
        move(QPoint(x() + event->pos().x() - m_relCorner.x(),
                    y() + event->pos().y() - m_relCorner.y()));
        m_owner->repaintConn();
        m_owner->resize(m_owner->minimumSizeHint());
        update();
        Q_EMIT componentChanged(this);
    } else {
        m_owner->refreshConnBuilder(event->pos());
    }
}

int ZCPBase::paintBase(QPainter &p, bool isGrowable)
{
    p.save();

    QPen pn=QPen(Qt::black);
    pn.setWidth(2);
    p.setPen(pn);
    p.setBrush(QBrush(Qt::white,Qt::SolidPattern));

    p.drawRect(rect());

    redrawPins(p);

    int hintHeight = 0;

    if (!m_hint.isEmpty()) {
        setBaseFont(p,ftHint);
        hintHeight = height()/3;
        if (isGrowable)
            hintHeight = p.fontMetrics().height()+5;
        QRect hrect(0,0,width(),hintHeight);

        p.drawText(hrect,Qt::AlignCenter,m_hint);
    }

    p.restore();

    return hintHeight;
}

void ZCPBase::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    Q_UNUSED(warnings)

    doHintGenerate(stream);
}

void ZCPBase::doCtlGenerate(QTextStream &stream, QStringList &warnings, bool softvol) const
{
    Q_UNUSED(stream)
    Q_UNUSED(warnings)
    Q_UNUSED(softvol)
}

void ZCPBase::doHintGenerate(QTextStream &stream) const
{
    if (!m_hint.isEmpty()) {
        QString opt = QSL("off");
        if (m_hintShow)
            opt = QSL("on");

        stream << QSL("  hint {") << Qt::endl;
        stream << QSL("    show %1").arg(opt) << Qt::endl;
        stream << QSL("    description \"%1\"").arg(m_hint) << Qt::endl;
        stream << QSL("  }") << Qt::endl;
    }
}

QString ZCPBase::getHint() const
{
    return m_hint;
}

void ZCPBase::mousePressEvent(QMouseEvent * event)
{
    raise();
    QPoint mx=mapToGlobal(event->pos());

    if (event->button()==Qt::RightButton)
    {
        showCtxMenu(mx);
        return;
    }

    ZCPBase* dFlt = nullptr;
    int pNum = 0;
    PinType pType = PinType::ptInput;
    mouseInPin(mx,pNum,pType,dFlt);
    Q_EMIT componentChanged(this);
    if (pNum<0) {
        m_relCorner=event->pos();
        m_isDragging=true;
        return;
    }
    m_isDragging=false;
    if (pType==PinType::ptInput) {
        m_owner->initConnBuilder(PinType::ptInput,pNum,dFlt->fInputs.at(pNum),nullptr,dFlt);
    } else {
        m_owner->initConnBuilder(PinType::ptOutput,pNum,nullptr,dFlt->fOutputs.at(pNum),dFlt);
    }
}

void ZCPBase::mouseReleaseEvent(QMouseEvent * event)
{
    QPoint mx=mapToGlobal(event->pos());
    ZCPBase* dFlt = nullptr;
    int pNum = 0;
    PinType pType = PinType::ptInput;
    mouseInPin(mx,pNum,pType,dFlt);
    if (pNum==-1) {
        bool f=m_isDragging;
        m_isDragging=false;
        if (!f) {
            m_owner->doneConnBuilder(true,PinType::ptInput,-1,nullptr,nullptr,nullptr);
        } else {
            checkRecycle();
        }
        m_isDragging=f;
        return;
    }
    if (pType==PinType::ptInput) {
        m_owner->doneConnBuilder(false,PinType::ptInput,pNum,dFlt->fInputs.at(pNum),nullptr,dFlt);
    } else {
        m_owner->doneConnBuilder(false,PinType::ptOutput,pNum,nullptr,dFlt->fOutputs.at(pNum),dFlt);
    }
    Q_EMIT componentChanged(this);
}

void ZCPBase::readFromJson(const QJsonValue &json)
{
    m_hint = json.toObject().value(QSL("hint")).toString();
    m_hintShow = json.toObject().value(QSL("hintShow")).toBool(true);

    const QJsonArray inputs = json.toObject().value(QSL("inputs")).toArray();
    for (int i=0;i<fInputs.count();i++)
        fInputs.at(i)->readFromJson(inputs.at(i));

    const QJsonArray outputs = json.toObject().value(QSL("outputs")).toArray();
    for (int i=0;i<fOutputs.count();i++)
        fOutputs.at(i)->readFromJson(outputs.at(i));
}

QJsonValue ZCPBase::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("hint"),m_hint);
    data.insert(QSL("hintShow"),m_hintShow);

    QJsonArray inputs;
    for (int i=0;i<fInputs.count();i++)
        inputs.append(fInputs.at(i)->storeToJson());
    data.insert(QSL("inputs"),inputs);

    QJsonArray outputs;
    for (int i=0;i<fOutputs.count();i++)
        outputs.append(fOutputs.at(i)->storeToJson());
    data.insert(QSL("outputs"),outputs);

    return data;
}

bool ZCPBase::isFloatConverterPresent() const
{
    if (searchPluginForward(ZCPPlug::staticMetaObject.className()) != nullptr)
        return true;

    if (auto *plug = searchPluginForward(ZCPConv::staticMetaObject.className())) {
        if (auto *conv = qobject_cast<ZCPConv*>(plug)) {
            if (conv->getConverterType() == ZCPConv::ConverterType::alcFloat)
                return true;
        }
    }
    return false;
}

QSize ZCPBase::sizeHint() const
{
    return minimumSizeHint();
}

ZCPBase *ZCPBase::searchPluginBackward(const char *targetClass, const ZCPBase *node,
                                       QSharedPointer<QStringList> searchStack) const
{
    if (searchStack.isNull())
        searchStack.reset(new QStringList());

    if (node == nullptr) node = this;

    if (searchStack->contains(node->objectName())) return nullptr;
    searchStack->append(node->objectName());

    for (const auto& inp: qAsConst(node->fInputs)) {
        for (const auto& link : qAsConst(inp->links)) {
            if (link.fromFilter) {
                if (std::strcmp(targetClass,link.fromFilter->metaObject()->className()) == 0) {
                    searchStack->removeLast();
                    return link.fromFilter; // found
                }

                if (auto *component = searchPluginBackward(targetClass,link.fromFilter,searchStack)) {
                    searchStack->removeLast();
                    return component;
                }
            }
        }
    }
    searchStack->removeLast();
    return nullptr;
}

ZCPBase *ZCPBase::searchPluginForward(const char *targetClass, const ZCPBase *node,
                                      QSharedPointer<QStringList> searchStack) const
{
    if (searchStack.isNull())
        searchStack.reset(new QStringList());

    if (node == nullptr) node = this;

    if (searchStack->contains(node->objectName())) return nullptr;
    searchStack->append(node->objectName());

    for (const auto& out: qAsConst(node->fOutputs)) {
        if (out->toFilter) {
            if (std::strcmp(targetClass,out->toFilter->metaObject()->className()) == 0) {
                searchStack->removeLast();
                return out->toFilter; // found
            }

            if (auto *component = searchPluginForward(targetClass,out->toFilter)) {
                searchStack->removeLast();
                return component;
            }
        }
    }
    searchStack->removeLast();
    return nullptr;
}

ZCPOutput::ZCPOutput(ZCPBase *parent, const QString &aPinName, CStructures::PinClass aPinClass)
    : QObject(parent)
    , pinClass(aPinClass)
    , ownerFilter(parent)
{
    pinName = aPinName;
}

void ZCPOutput::readFromJson(const QJsonValue &json)
{
    toFilter = nullptr;
    toPin = json.toObject().value(QSL("toPin")).toInt(-1);
    ffLogic = json.toObject().value(QSL("toFilter")).toString();
    pinClass = static_cast<CStructures::PinClass>(json.toObject().value(QSL("pinClass")).toInt(0));
}

QJsonValue ZCPOutput::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("toPin"),toPin);
    data.insert(QSL("pinClass"),static_cast<int>(pinClass));

    QString filter;
    if (toFilter)
        filter= toFilter->objectName();
    data.insert(QSL("toFilter"),filter);

    return data;
}

bool ZCPOutput::postLoadBind()
{
    if (ffLogic.isEmpty()) return true;
    auto *b = ownerFilter->ownerArea()->findChild<ZCPBase *>(ffLogic);
    if (b) {
        toFilter=b;
    } else {
        return false;
    }
    return true;
}

QColor ZCPOutput::getPinColor() const
{
    switch (pinClass) {
        case CStructures::PinClass::pcPCM: return Qt::blue;
        case CStructures::PinClass::pcCTL: return Qt::red;
    }
    return Qt::GlobalColor::gray;
}

ZCPInput::ZCPInput(ZCPBase *parent, const QString &aPinName, CStructures::PinClass aPinClass)
    : QObject(parent)
    , pinClass(aPinClass)
    , ownerFilter(parent)
{
    pinName = aPinName;
}

void ZCPInput::readFromJson(const QJsonValue &json)
{
    pinClass = static_cast<CStructures::PinClass>(json.toObject().value(QSL("pinClass")).toInt(0));

    const QJsonArray jarray = json.toObject().value(QSL("links")).toArray();
    for (const auto& item : jarray) {
        qint32 fromPin = item.toObject().value(QSL("fromPin")).toInt(-1);
        QString ffLogic = item.toObject().value(QSL("fromFilter")).toString();
        links.append(CInpLink(fromPin,nullptr,ffLogic));
    }
}

QJsonValue ZCPInput::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("pinClass"),static_cast<int>(pinClass));

    QJsonArray jlinks;
    for (const auto& link : qAsConst(links)) {
        QJsonObject item;
        item.insert(QSL("fromPin"),link.fromPin);

        QString filter;
        if (link.fromFilter)
            filter = link.fromFilter->objectName();
        item.insert(QSL("fromFilter"),filter);

        jlinks.append(item);
    }
    data.insert(QSL("links"),jlinks);
    return data;
}

bool ZCPInput::postLoadBind()
{
    bool failed = false;
    for (auto & link : links) {
        if (!(link.ffLogic.isEmpty())) {
            auto *b = ownerFilter->ownerArea()->findChild<ZCPBase *>(link.ffLogic);
            if (b) {
                link.fromFilter = b;
            } else {
                failed = true;
            }
        }
    }
    return (!failed);
}

QColor ZCPInput::getPinColor() const
{
    switch (pinClass) {
        case CStructures::PinClass::pcPCM: return Qt::blue;
        case CStructures::PinClass::pcCTL: return Qt::red;
    }
    return Qt::GlobalColor::gray;
}

CInpLink::CInpLink(qint32 aFromPin, ZCPBase *aFromFilter, const QString &affLogic)
{
    fromPin = aFromPin;
    fromFilter = aFromFilter;
    ffLogic = affLogic;
}

bool CInpLink::operator==(const CInpLink &s) const
{
    return ((fromPin == s.fromPin) &&
            (reinterpret_cast<qintptr>(fromFilter) == reinterpret_cast<qintptr>(s.fromFilter)));
}

bool CInpLink::operator!=(const CInpLink &s) const
{
    return !operator==(s);
}
