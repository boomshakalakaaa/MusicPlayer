#include "customwidget.h"


// 可移动窗体
CustomMoveWidget::CustomMoveWidget(QWidget *parent) :
    QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
}

CustomMoveWidget::~CustomMoveWidget()
{

}

/*CustomMoveWidget::mouseMoveEvent 鼠标点击移动*/
void CustomMoveWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_mousePressed && (e->buttons() && Qt::LeftButton))
    {
        this->move(e->globalPos() - mousePoint);
        e->accept();
    }
}

void CustomMoveWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void CustomMoveWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_mousePressed = false;
}
