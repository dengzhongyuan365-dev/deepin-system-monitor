// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "block_dev_item_widget.h"
#include "chart_view_widget.h"
#include "common/common.h"
#include "ddlog.h"

#include <QPen>
#include <QPainter>
#include <QPalette>
#include <QtMath>

#include <DApplication>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <DApplicationHelper>
#else
#include <DGuiApplicationHelper>
#endif

using namespace DDLog;
DWIDGET_USE_NAMESPACE
using namespace common::format;

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;

BlockDevItemWidget::BlockDevItemWidget(QWidget *parent) : QWidget(parent)
{
    qCDebug(app) << "BlockDevItemWidget constructor";
    m_memChartWidget = new ChartViewWidget(ChartViewWidget::ChartViewTypes::MEM_CHART, this);
    m_memChartWidget->setSpeedAxis(true);
    m_memChartWidget->setData1Color(readColor);
    m_memChartWidget->setData2Color(writeColor);
}

BlockDevItemWidget::~BlockDevItemWidget()
{
    qCDebug(app) << "BlockDevItemWidget destructor";
    if (m_memChartWidget) {
        qCDebug(app) << "Deleting m_memChartWidget";
        delete m_memChartWidget;
        m_memChartWidget = nullptr;
    }
}

void BlockDevItemWidget::updateWidgetGeometry()
{
    // qCDebug(app) << "BlockDevItemWidget updateWidgetGeometry";
    QFont font = DApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    int fontHeight = QFontMetrics(font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    if (m_mode == TITLE_HORIZONTAL) {
        qCDebug(app) << "BlockDevItemWidget updateWidgetGeometry m_mode == TITLE_HORIZONTAL";
        m_memChartWidget->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        qCDebug(app) << "BlockDevItemWidget updateWidgetGeometry m_mode == TITLE_VERTICAL";
        m_memChartWidget->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
    }
}

void BlockDevItemWidget::fontChanged(const QFont &font)
{
    qCDebug(app) << "BlockDevItemWidget font changed";
    m_font = font;
    updateWidgetGeometry();
}

void BlockDevItemWidget::resizeEvent(QResizeEvent *event)
{
    // qCDebug(app) << "BlockDevItemWidget resizeEvent";
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void BlockDevItemWidget::updateData(const BlockDevice &info)
{
    // qCDebug(app) << "BlockDevItemWidget updateData for" << info.deviceName();
    m_blokeDeviceInfo = info;
    m_memChartWidget->addData1(info.readSpeed());
    m_memChartWidget->addData2(info.writeSpeed());
    m_memChartWidget->update();

    this->update();
}

void BlockDevItemWidget::activeItemWidget(bool isShow)
{
    qCDebug(app) << "BlockDevItemWidget activeItemWidget" << isShow;
    m_isActive = isShow;
    update();
}

void BlockDevItemWidget::mousePressEvent(QMouseEvent *event)
{
    qCDebug(app) << "BlockDevItemWidget mousePressEvent";
    QWidget::mousePressEvent(event);
    emit clicked(m_blokeDeviceInfo.deviceName());
}

void BlockDevItemWidget::paintEvent(QPaintEvent *event)
{
    // qCDebug(app) << "BlockDevItemWidget paintEvent";
    QWidget::paintEvent(event);

    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    QPainter painter(this);
    QFont font = DApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    painter.setFont(font);
    painter.setRenderHint(QPainter:: Antialiasing, true);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto *dAppHelper = DApplicationHelper::instance();
#else
    auto *dAppHelper = DGuiApplicationHelper::instance();
#endif
    auto palette = dAppHelper->applicationPalette();

    QString deviceName = m_blokeDeviceInfo.deviceName().data();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int deviceNameWidth = painter.fontMetrics().width(deviceName);
#else
    int deviceNameWidth = painter.fontMetrics().horizontalAdvance(deviceName);
#endif
    int deviceNameHeight = painter.fontMetrics().height();
    QRect devtitleRect(curXMargin, margin, deviceNameWidth, deviceNameHeight);

    const QColor &textColor = palette.color(DPalette::TextTips);
    if (m_isActive && m_mode == TITLE_VERTICAL) {
        // qCDebug(app) << "BlockDevItemWidget paintEvent m_isActive && m_mode == TITLE_VERTICAL";
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(DPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(DPalette::Highlight));
    } else {
        // qCDebug(app) << "BlockDevItemWidget paintEvent m_isActive == false";
        painter.setPen(textColor);
    }
    painter.drawText(devtitleRect, deviceName);
    painter.setPen(textColor);

    QString readTitle = QString("%1 %2")
                        .arg(tr("Read"))
                        .arg(formatUnit_memory_disk(m_blokeDeviceInfo.readSpeed(), B, 1, true));

    QString writeTitle = QString("%1 %2")
                         .arg(tr("Write"))
                         .arg(formatUnit_memory_disk(m_blokeDeviceInfo.writeSpeed(), B, 1, true));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int readTitleWidth = painter.fontMetrics().width(readTitle);
#else
    int readTitleWidth = painter.fontMetrics().horizontalAdvance(readTitle);
#endif
    int readTitleHeight = painter.fontMetrics().height();
    if (m_mode == TITLE_HORIZONTAL) {
        // qCDebug(app) << "BlockDevItemWidget paintEvent m_mode == TITLE_HORIZONTAL";
        QRect memtitleRect(sectionSize + devtitleRect.right() + spacing * 2, devtitleRect.y() + deviceNameHeight, readTitleWidth, readTitleHeight);
        painter.drawText(memtitleRect, Qt::AlignLeft | Qt::AlignVCenter, readTitle);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRect swaptitleRect(sectionSize + memtitleRect.right() + 2 * spacing, memtitleRect.y() + readTitleHeight, painter.fontMetrics().width(writeTitle), painter.fontMetrics().height());
#else
        QRect swaptitleRect(sectionSize + memtitleRect.right() + 2 * spacing, memtitleRect.y() + readTitleHeight, painter.fontMetrics().horizontalAdvance(writeTitle), painter.fontMetrics().height());
#endif
        painter.drawText(swaptitleRect, Qt::AlignLeft | Qt::AlignVCenter, writeTitle);

        painter.setPen(Qt::NoPen);
        painter.setBrush(readColor);
        painter.drawEllipse(devtitleRect.right() + spacing, memtitleRect.y() + qCeil((memtitleRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(writeColor);
        painter.drawEllipse(memtitleRect.right() + spacing, swaptitleRect.y() + qCeil((swaptitleRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    } else {
        // qCDebug(app) << "BlockDevItemWidget paintEvent m_mode == TITLE_VERTICAL";
        painter.setPen(Qt::NoPen);
        painter.setBrush(readColor);

        QRect readStrRect(devtitleRect.left() + spacing + sectionSize, devtitleRect.y() + deviceNameHeight, readTitleWidth, readTitleHeight);
        painter.drawEllipse(devtitleRect.left(), readStrRect.y() + qCeil((readStrRect.height() - sectionSize) / 2.0), sectionSize, sectionSize); // 读硬盘速度的颜色提示
        painter.setPen(palette.color(DPalette::TextTips));
        painter.drawText(readStrRect, readTitle);

        painter.setPen(Qt::NoPen);
        painter.setBrush(writeColor);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRect writeStrRect(devtitleRect.left() + spacing + sectionSize, readStrRect.y() + readTitleHeight, painter.fontMetrics().width(writeTitle), painter.fontMetrics().height());
#else
        QRect writeStrRect(devtitleRect.left() + spacing + sectionSize, readStrRect.y() + readTitleHeight, painter.fontMetrics().horizontalAdvance(writeTitle), painter.fontMetrics().height());
#endif
        painter.drawEllipse(devtitleRect.left(), writeStrRect.y() + qCeil((writeStrRect.height() - sectionSize) / 2.0), sectionSize, sectionSize); // 写硬盘速度的颜色提示
        painter.setPen(palette.color(DPalette::TextTips));
        painter.drawText(writeStrRect, writeTitle);
    }
}

void BlockDevItemWidget::setMode(int mode)
{
    qCDebug(app) << "BlockDevItemWidget setMode" << mode;
    m_mode = mode;
    this->update();
}



