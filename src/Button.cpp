#include "Button.h"
#include <QPainter>
#include <QPainterPath>
#include <QOverload>
#include "DesignSystem.h"

Button::Button(QString btnText, qreal textSize, QWidget* parent)
	:QPushButton(parent),
	m_radius(6),
	m_margin(4),
	m_hovered(false),
	m_pressed(false),
	baseColor(DesignSystem::instance()->primaryColor())
{
	setCursor(Qt::PointingHandCursor);
	QFont font;
	font.setPointSizeF(textSize);
	setFont(font);
	setText(btnText);

	connect(DesignSystem::instance(), &DesignSystem::themeChanged, this, [this]()
		{
			baseColor = DesignSystem::instance()->primaryColor();
			update();
		});
}

Button::~Button()
{
	if (m_svgRenderer)
	{
		delete m_svgRenderer;
	}
}

void Button::setSvgIcon(const QString& iconPath)
{
	m_svgRenderer = new QSvgRenderer(iconPath, this);
	update();
}

void Button::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (!m_clickTimer.isValid() || m_clickTimer.elapsed() >= m_clickIntervalMs)
		{
			m_clickTimer.restart();

			QRectF buttonRect = QRectF(m_margin, m_margin,
				width() - 2 * m_margin,
				height() - 2 * m_margin);
			Ripple* ripple = new Ripple(buttonRect, m_radius, this);
			m_ripples.append(ripple);

			QParallelAnimationGroup* rippleAnimationGroup = new QParallelAnimationGroup(this);

			QPropertyAnimation* offsetAnimation = new QPropertyAnimation(ripple, "m_offset");
			offsetAnimation->setDuration(animTime);
			offsetAnimation->setEasingCurve(QEasingCurve::InOutSine);

			QPropertyAnimation* opacityAnimation = new QPropertyAnimation(ripple, "m_opacity");
			opacityAnimation->setDuration(animTime + 300);
			opacityAnimation->setEasingCurve(QEasingCurve::InOutSine);

			rippleAnimationGroup->addAnimation(offsetAnimation);
			rippleAnimationGroup->addAnimation(opacityAnimation);

			if (rippleAnimationGroup->state() != QAbstractAnimation::Running)
			{
				ripple->setBeginValue(0, 0.5);
			}

			offsetAnimation->setStartValue(ripple->offset());
			offsetAnimation->setEndValue(m_margin - 2);

			opacityAnimation->setStartValue(ripple->opacity());
			opacityAnimation->setEndValue(0.0);

			connect(ripple, &Ripple::offsetChanged, this, QOverload<>::of(&Button::update));
			connect(ripple, &Ripple::opacityChanged, this, QOverload<>::of(&Button::update));

			rippleAnimationGroup->start(QAbstractAnimation::DeleteWhenStopped);

			connect(rippleAnimationGroup, &QParallelAnimationGroup::finished, this, [this, ripple]()
				{
					m_ripples.removeOne(ripple);
					ripple->deleteLater();
				});

			m_pressed = true;
			update();
		}
	}
	QPushButton::mousePressEvent(event);
}

void Button::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_pressed = false;
		update();
	}
	QPushButton::mouseReleaseEvent(event);
}

void Button::enterEvent(QEnterEvent* event)
{
	m_hovered = true;
	update();
	QPushButton::enterEvent(event);
}

void Button::leaveEvent(QEvent* event)
{
	m_hovered = false;
	update();
	QPushButton::leaveEvent(event);
}

void Button::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QRectF buttonRect;
	bool hasIcon = (m_svgRenderer != nullptr);

	if (hasIcon)
	{
		int diameter = qMin(width(), height());
		buttonRect = QRectF(m_margin, m_margin, diameter - 2 * m_margin, diameter - 2 * m_margin);

		if (m_hovered || m_pressed)
		{
			QColor hoverColor = baseColor;
			hoverColor.setAlphaF(m_pressed ? 0.15 : 0.08);
			painter.setBrush(hoverColor);
			painter.setPen(Qt::NoPen);
			painter.drawEllipse(buttonRect);
		}
	}
	else
	{
		buttonRect = QRectF(m_margin, m_margin, width() - 2 * m_margin, height() - 2 * m_margin);

		if (m_hovered || m_pressed)
		{
			QColor hoverColor = baseColor;
			hoverColor.setAlphaF(m_pressed ? 0.15 : 0.08);
			painter.setBrush(hoverColor);
			painter.setPen(Qt::NoPen);
			painter.drawRoundedRect(buttonRect, m_radius, m_radius);
		}
	}

	if (hasIcon)
	{
		QSizeF iconSize = buttonRect.size() * m_scaleFactor;
		QRectF iconRect = buttonRect;
		iconRect.setSize(iconSize);
		iconRect.moveCenter(buttonRect.center());
		m_svgRenderer->render(&painter, iconRect.toRect());
	}
	else
	{
		painter.setPen(DesignSystem::instance()->currentTheme().textColor);
		painter.setFont(font());
		painter.drawText(buttonRect, Qt::AlignCenter, text());
	}

	QColor rippleColor = baseColor;
	painter.setPen(Qt::NoPen);
	for (Ripple* ripple : m_ripples)
	{
		rippleColor.setAlphaF(ripple->opacity());
		painter.setBrush(rippleColor);

		int rippleOffset = ripple->offset();
		QRectF outerRect = buttonRect.adjusted(
			-rippleOffset, -rippleOffset,
			+rippleOffset, +rippleOffset
		);

		QRectF innerRect = buttonRect;

		QPainterPath outerPath;
		if (hasIcon)
			outerPath.addEllipse(outerRect);
		else
			outerPath.addRoundedRect(outerRect, m_radius, m_radius);

		QPainterPath innerPath;
		if (hasIcon)
			innerPath.addEllipse(innerRect);
		else
			innerPath.addRoundedRect(innerRect, m_radius, m_radius);

		QPainterPath ringPath = outerPath.subtracted(innerPath);
		painter.drawPath(ringPath);
	}
}
