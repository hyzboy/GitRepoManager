#include "BottomDockWidget.h"
#include <QWidget>
#include <QVBoxLayout>

BottomDockWidget::BottomDockWidget(QWidget *parent)
    : QDockWidget("Bottom Panel", parent)
{
    setupUI();
}

BottomDockWidget::~BottomDockWidget()
{
}

void BottomDockWidget::setupUI()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Add empty placeholder (can be replaced with actual widgets)
    layout->addWidget(new QWidget());
    
    setWidget(content);
}
