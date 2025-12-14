#ifndef GITDIFFPROVIDER_H
#define GITDIFFPROVIDER_H

#include <QObject>
#include <QString>
#include <QList>
#include <git2.h>

// File change information
struct FileChange {
    enum Status {
        Added,
        Deleted,
        Modified,
        Renamed,
        Copied,
        Unknown
    };
    
    QString path;
    Status status;
    
    QString statusString() const {
        switch (status) {
            case Added: return "[A] ";
            case Deleted: return "[D] ";
            case Modified: return "[M] ";
            case Renamed: return "[R] ";
            case Copied: return "[C] ";
            default: return "[?] ";
        }
    }
};

// RAII wrapper for git_commit
class GitCommitGuard {
public:
    GitCommitGuard(git_repository *repo, const git_oid *oid);
    ~GitCommitGuard();
    
    git_commit* get() const { return m_commit; }
    operator bool() const { return m_commit != nullptr; }
    
    // Prevent copying
    GitCommitGuard(const GitCommitGuard&) = delete;
    GitCommitGuard& operator=(const GitCommitGuard&) = delete;
    
private:
    git_commit *m_commit;
};

// RAII wrapper for git_tree
class GitTreeGuard {
public:
    explicit GitTreeGuard(git_tree *tree);
    ~GitTreeGuard();
    
    git_tree* get() const { return m_tree; }
    operator bool() const { return m_tree != nullptr; }
    
    // Prevent copying
    GitTreeGuard(const GitTreeGuard&) = delete;
    GitTreeGuard& operator=(const GitTreeGuard&) = delete;
    
private:
    git_tree *m_tree;
};

// RAII wrapper for git_diff
class GitDiffGuard {
public:
    explicit GitDiffGuard(git_diff *diff);
    ~GitDiffGuard();
    
    git_diff* get() const { return m_diff; }
    operator bool() const { return m_diff != nullptr; }
    
    // Prevent copying
    GitDiffGuard(const GitDiffGuard&) = delete;
    GitDiffGuard& operator=(const GitDiffGuard&) = delete;
    
private:
    git_diff *m_diff;
};

// Git diff provider class
class GitDiffProvider : public QObject
{
    Q_OBJECT
    
public:
    explicit GitDiffProvider(QObject *parent = nullptr);
    ~GitDiffProvider();
    
    // Set the repository to work with
    void setRepository(git_repository *repo);
    
    // Get list of changed files in a commit
    QList<FileChange> getChangedFiles(const git_oid &oid);
    
    // Get diff content for a specific file in a commit
    QString getFileDiff(const git_oid &oid, const QString &filePath);
    
    // Get file content for initial commit (no parent)
    QString getFileContent(const git_oid &oid, const QString &filePath);
    
    // Check if commit is initial commit (has no parent)
    bool isInitialCommit(const git_oid &oid);
    
private:
    git_repository *m_repo;
    
    // Helper methods
    FileChange::Status convertGitStatus(git_delta_t status) const;
    QString extractFileDiff(const QString &fullPatch, const QString &filePath) const;
};

#endif // GITDIFFPROVIDER_H
