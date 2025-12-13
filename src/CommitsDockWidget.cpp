#include "CommitsDockWidget.h"
#include <QWidget>
#include <QVBoxLayout>

CommitsDockWidget::CommitsDockWidget(QWidget *parent)
    : QDockWidget("Commits", parent)
{
    setupUI();
}

CommitsDockWidget::~CommitsDockWidget()
{
}

void CommitsDockWidget::setupUI()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Add empty placeholder (can be replaced with actual widgets)
    layout->addWidget(new QWidget());
    
    setWidget(content);
}
