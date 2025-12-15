#ifndef DIFFVIEWWIDGET_H
#define DIFFVIEWWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QComboBox>

class SyntaxManager;
class ThemeManager;
class DiffSyntaxHighlighter;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class DiffViewWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit DiffViewWidget(QWidget *parent = nullptr);
    ~DiffViewWidget();
    
    // Set managers
    void setSyntaxManager(SyntaxManager *manager);
    void setThemeManager(ThemeManager *manager);
    
    // Display content
    void setContent(const QString &content, const QString &filePath);
    void clear();
    
    // Get current selections
    QString currentTheme() const;
    QString currentSyntax() const;
    
signals:
    void themeChanged(const QString &themeName);
    void syntaxChanged(const QString &syntaxName);
    
private slots:
    void onThemeComboChanged(int index);
    void onSyntaxComboChanged(int index);
    
private:
    void setupUI();
    void populateThemeComboBox();
    void populateSyntaxComboBox();
    void applySyntaxHighlighting();
    
    // UI Components
    QTextEdit *m_diffDisplay;
    QComboBox *m_themeComboBox;
    QComboBox *m_syntaxComboBox;
    
    // Managers
    SyntaxManager *m_syntaxManager;
    ThemeManager *m_themeManager;
    DiffSyntaxHighlighter *m_diffHighlighter;  // 改用 DiffSyntaxHighlighter
    
    // Current state
    QString m_currentFilePath;
};

#endif // DIFFVIEWWIDGET_H
