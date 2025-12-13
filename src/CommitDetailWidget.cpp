#include "CommitDetailWidget.h"
#include <QVBoxLayout>
#include <QLabel>

CommitDetailWidget::CommitDetailWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

CommitDetailWidget::~CommitDetailWidget()
{
}

void CommitDetailWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Placeholder label
    QLabel *placeholderLabel = new QLabel("Commit Details", this);
    layout->addWidget(placeholderLabel);
    
    layout->addStretch();
}
