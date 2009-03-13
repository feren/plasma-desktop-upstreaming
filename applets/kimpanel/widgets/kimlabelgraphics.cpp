#include "kimlabelgraphics.h"
#include "paintutils.h"
#include <plasma/theme.h>

KIMLabelGraphics::KIMLabelGraphics(QGraphicsItem *parent)
    :QGraphicsWidget(parent),
     m_hoverEffect(false),
     m_drawCursor(false),
     m_cursorPos(0)
{
    setAcceptHoverEvents(true);
    connect(Plasma::Theme::defaultTheme(),SIGNAL(themeChanged()),
            this,SLOT(generatePixmap()));
}

KIMLabelGraphics::~KIMLabelGraphics()
{

}

void KIMLabelGraphics::setDrawCursor(bool to_draw)
{
    m_drawCursor = to_draw;
    generatePixmap();
}

void KIMLabelGraphics::setCursorPos(int pos)
{
    if (m_drawCursor) {
        m_cursorPos = pos;
        generatePixmap();
    }
}

void KIMLabelGraphics::setText(const QString &text)
{
    m_text = text;
    generatePixmap();
}

void KIMLabelGraphics::setLabel(const QString &label)
{
    m_label = label;
    generatePixmap();
}

void KIMLabelGraphics::generatePixmap()
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    QSize old_size = m_pixmap.size();

    QPixmap text_pixmap;
    QPixmap textReversed_pixmap;
    QPixmap label_pixmap;
    QPixmap labelReversed_pixmap;
    QPixmap cursor_pixmap;

    QString text;

    QSize size(0,0);

    if (m_drawCursor) {
        text = m_text.left(m_cursorPos) + "I" + m_text.mid(m_cursorPos);
    } else {
        text = m_text;
    }

    if (text.isEmpty() && m_label.isEmpty()) {
        m_pixmap = QPixmap();
        m_reversedPixmap = QPixmap();
    } else {
        if (!text.isEmpty()) {
            text_pixmap = renderText(text);
            textReversed_pixmap = renderText(text,
                    theme->color(Plasma::Theme::BackgroundColor),
                    theme->color(Plasma::Theme::TextColor));
            size.setWidth(size.width() + text_pixmap.width());
            size.setHeight(text_pixmap.height());
        }
        if (!m_label.isEmpty()) {
            labelReversed_pixmap = renderText(m_label);
            label_pixmap = renderText(m_label,
                    theme->color(Plasma::Theme::BackgroundColor),
                    theme->color(Plasma::Theme::TextColor));
            size.setWidth(size.width() + label_pixmap.width());
            size.setHeight(label_pixmap.height());
        }
        m_pixmap = QPixmap(size);
        m_reversedPixmap = QPixmap(size);
        m_pixmap.fill(Qt::transparent);
        m_reversedPixmap.fill(Qt::transparent);
        QPainter p(&m_pixmap);
        QPainter p1(&m_reversedPixmap);
        if (!label_pixmap.isNull()) {
            p.drawPixmap(0,0,label_pixmap);
            p1.drawPixmap(0,0,labelReversed_pixmap);
            if (!text_pixmap.isNull()) {
                p.drawPixmap(label_pixmap.width(),0,text_pixmap);
                p1.drawPixmap(label_pixmap.width(),0,textReversed_pixmap);
            }
        } else {
            p.drawPixmap(0,0,text_pixmap);
            p1.drawPixmap(0,0,textReversed_pixmap);
        }
        //kDebug() << m_pixmap.size();
    }

    if (m_pixmap.size() != old_size) {
        updateGeometry();
    }
}

void KIMLabelGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    int h_spacing = (boundingRect().height() - m_pixmap.height())/2;
    if (m_hoverEffect && (m_states & HoverState)) {
        painter->drawPixmap(0,h_spacing,
            m_reversedPixmap);
    } else {
        painter->drawPixmap(0,h_spacing,
            m_pixmap);
    }
}

void KIMLabelGraphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsWidget::mousePressEvent(event);
        return;
    }

    m_states |= PressedState;
    m_clickStartPos = scenePos();

    if (boundingRect().contains(event->pos())) {
        emit pressed(true);
    }

//    update();
}

void KIMLabelGraphics::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (~m_states & PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    if (boundingRect().contains(event->pos())) {
        if (~m_states & HoverState) {
            m_states |= HoverState;
            update();
        }
    } else {
        if (m_states & HoverState) {
            m_states &= ~HoverState;
            update();
        }
    }
}

void KIMLabelGraphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (~m_states & PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    m_states &= ~PressedState;

    //don't pass click when the mouse was moved
    bool handled = m_clickStartPos != scenePos();
    if (!handled) {
        if (boundingRect().contains(event->pos())) {
            emit clicked();
        }
        emit pressed(false);
    }

    update();
}

void KIMLabelGraphics::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEffect(true);
    update();

    QGraphicsWidget::hoverEnterEvent(event);
}

void KIMLabelGraphics::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_states &= ~HoverState; // Will be set once progress is zero again ...
    hoverEffect(false);
    update();

    QGraphicsWidget::hoverLeaveEvent(event);
}

void KIMLabelGraphics::hoverEffect(bool show)
{
    if (show) {
        m_states |= HoverState;
    }

#if 0
    fadeIn = show;
    const int FadeInDuration = 150;

    if (hoverAnimId != -1) {
        Animator::self()->stopCustomAnimation(hoverAnimId);
    }

    hoverAnimId = Animator::self()->customAnimation(
        40 / (1000 / FadeInDuration), FadeInDuration,
        Animator::EaseOutCurve, q, "hoverAnimationUpdate");
#endif
}

// vim: sw=4 sts=4 et tw=100
