/*
 * nheko Copyright (C) 2017  Konstantinos Sideris <siderisk@auth.gr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

#include "Ripple.h"
#include "RoomInfoListItem.h"
#include "RoomState.h"
#include "Theme.h"

RoomInfoListItem::RoomInfoListItem(RoomState state, QString room_id, QWidget *parent)
    : QWidget(parent)
    , state_(state)
    , roomId_(room_id)
    , isPressed_(false)
    , maxHeight_(60)
    , unreadMsgCount_(0)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover);

	setFixedHeight(maxHeight_);

	QPainterPath path;
	path.addRect(0, 0, parent->width(), height());

	ripple_overlay_ = new RippleOverlay(this);
	ripple_overlay_->setClipPath(path);
	ripple_overlay_->setClipping(true);
}

void RoomInfoListItem::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	p.setRenderHint(QPainter::TextAntialiasing);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.setRenderHint(QPainter::Antialiasing);

	if (isPressed_)
		p.fillRect(rect(), QColor("#38A3D8"));
	else if (underMouse())
		p.fillRect(rect(), QColor(200, 200, 200, 128));
	else
		p.fillRect(rect(), QColor("#F8FBFE"));

	QFont font("Open Sans", 10);

	QFontMetrics metrics(font);
	p.setFont(font);
	p.setPen(QColor("#333"));

	QRect avatarRegion(Padding, Padding, IconSize, IconSize);

	// Description line
	int bottom_y = avatarRegion.center().y() + metrics.height() / 2 + Padding / 2;

	if (width() > ui::sidebar::SmallSize) {
		if (isPressed_) {
			QPen pen(QColor("white"));
			p.setPen(pen);
		}

		auto name = metrics.elidedText(state_.resolveName(), Qt::ElideRight, (width() - IconSize - 2 * Padding) * 0.8);
		p.drawText(QPoint(2 * Padding + IconSize, avatarRegion.center().y() - metrics.height() / 2), name);

		if (!isPressed_) {
			QPen pen(QColor("#5d6565"));
			p.setPen(pen);
		}

		double descPercentage = 0.95;

		if (unreadMsgCount_ > 0)
			descPercentage = 0.8;

		auto description = metrics.elidedText(state_.resolveTopic(), Qt::ElideRight, width() * descPercentage - 2 * Padding - IconSize);
		p.drawText(QPoint(2 * Padding + IconSize, bottom_y), description);
	}

	p.setPen(Qt::NoPen);

	// We using the first letter of room's name.
	if (roomAvatar_.isNull()) {
		QBrush brush;
		brush.setStyle(Qt::SolidPattern);
		brush.setColor("#eee");

		p.setPen(Qt::NoPen);
		p.setBrush(brush);

		p.drawEllipse(avatarRegion.center(), IconSize / 2, IconSize / 2);

		font.setPointSize(13);
		p.setFont(font);
		p.setPen(QColor("#333"));
		p.setBrush(Qt::NoBrush);
		p.drawText(avatarRegion.translated(0, -1), Qt::AlignCenter, QChar(state_.resolveName()[0]));
	} else {
		p.save();

		QPainterPath path;
		path.addEllipse(Padding, Padding, IconSize, IconSize);
		p.setClipPath(path);

		p.drawPixmap(avatarRegion, roomAvatar_);
		p.restore();
	}

	if (unreadMsgCount_ > 0) {
		QColor textColor("white");
		QColor backgroundColor("#38A3D8");

		QBrush brush;
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(backgroundColor);

		if (isPressed_)
			brush.setColor(textColor);

		p.setBrush(brush);
		p.setPen(Qt::NoPen);

		QFont msgFont("Open Sans", 8);
		msgFont.setStyleName("Bold");

		p.setFont(msgFont);

		int diameter = 20;

		QRectF r(width() - diameter - Padding, bottom_y - diameter / 2 - 5, diameter, diameter);

		if (width() == ui::sidebar::SmallSize)
			r = QRectF(width() - diameter - 5, height() - diameter - 5, diameter, diameter);

		p.setPen(Qt::NoPen);
		p.drawEllipse(r);

		p.setPen(QPen(textColor));

		if (isPressed_)
			p.setPen(QPen(backgroundColor));

		p.setBrush(Qt::NoBrush);
		p.drawText(r.translated(0, -0.5), Qt::AlignCenter, QString::number(unreadMsgCount_));
	}
}

void RoomInfoListItem::updateUnreadMessageCount(int count)
{
	unreadMsgCount_ += count;
	repaint();
}

void RoomInfoListItem::clearUnreadMessageCount()
{
	unreadMsgCount_ = 0;
	repaint();
}

void RoomInfoListItem::setPressedState(bool state)
{
	if (!isPressed_ && state) {
		isPressed_ = state;
		update();
	} else if (isPressed_ && !state) {
		isPressed_ = state;
		update();
	}
}

void RoomInfoListItem::setState(const RoomState &new_state)
{
	state_ = new_state;
	repaint();
}

void RoomInfoListItem::mousePressEvent(QMouseEvent *event)
{
	emit clicked(roomId_);

	setPressedState(true);

	// Ripple on mouse position by default.
	QPoint pos = event->pos();
	qreal radiusEndValue = static_cast<qreal>(width()) / 3;

	Ripple *ripple = new Ripple(pos);

	ripple->setRadiusEndValue(radiusEndValue);
	ripple->setOpacityStartValue(0.15);
	ripple->setColor(QColor("white"));
	ripple->radiusAnimation()->setDuration(200);
	ripple->opacityAnimation()->setDuration(400);

	ripple_overlay_->addRipple(ripple);
}

RoomInfoListItem::~RoomInfoListItem()
{
}
