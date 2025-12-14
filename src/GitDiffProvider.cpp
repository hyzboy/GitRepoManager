#include "GitDiffProvider.h"
#include <QDebug>

// GitCommitGuard implementation
GitCommitGuard::GitCommitGuard(git_repository *repo, const git_oid *oid)
    : m_commit(nullptr)
{
    if (repo && oid) {
        git_commit_lookup(&m_commit, repo, oid);
    }
}

GitCommitGuard::~GitCommitGuard()
{
    if (m_commit) {
        git_commit_free(m_commit);
    }
}

// GitTreeGuard implementation
GitTreeGuard::GitTreeGuard(git_tree *tree)
    : m_tree(tree)
{
}

GitTreeGuard::~GitTreeGuard()
{
    if (m_tree) {
        git_tree_free(m_tree);
    }
}

// GitDiffGuard implementation
GitDiffGuard::GitDiffGuard(git_diff *diff)
    : m_diff(diff)
{
}

GitDiffGuard::~GitDiffGuard()
{
    if (m_diff) {
        git_diff_free(m_diff);
    }
}

// GitDiffProvider implementation
GitDiffProvider::GitDiffProvider(QObject *parent)
    : QObject(parent)
    , m_repo(nullptr)
{
}

GitDiffProvider::~GitDiffProvider()
{
}

void GitDiffProvider::setRepository(git_repository *repo)
{
    m_repo = repo;
}

bool GitDiffProvider::isInitialCommit(const git_oid &oid)
{
    if (!m_repo) {
        return false;
    }
    
    GitCommitGuard commit(m_repo, &oid);
    if (!commit) {
        return false;
    }
    
    return git_commit_parentcount(commit.get()) == 0;
}

QList<FileChange> GitDiffProvider::getChangedFiles(const git_oid &oid)
{
    QList<FileChange> changes;
    
    if (!m_repo) {
        qWarning() << "GitDiffProvider: No repository set";
        return changes;
    }
    
    GitCommitGuard commit(m_repo, &oid);
    if (!commit) {
        qWarning() << "GitDiffProvider: Failed to lookup commit";
        return changes;
    }
    
    // Get commit tree
    git_tree *commit_tree_ptr = nullptr;
    if (git_commit_tree(&commit_tree_ptr, commit.get()) != 0) {
        qWarning() << "GitDiffProvider: Failed to get commit tree";
        return changes;
    }
    GitTreeGuard commit_tree(commit_tree_ptr);
    
    // Check if this is initial commit
    unsigned int parent_count = git_commit_parentcount(commit.get());
    if (parent_count == 0) {
        // Initial commit - all files are added
        // For now, return empty list with a note
        // In a real implementation, you might want to traverse the tree
        return changes;
    }
    
    // Get parent commit
    git_commit *parent_commit_ptr = nullptr;
    if (git_commit_parent(&parent_commit_ptr, commit.get(), 0) != 0) {
        qWarning() << "GitDiffProvider: Failed to get parent commit";
        return changes;
    }
    GitCommitGuard parent_commit(m_repo, git_commit_id(parent_commit_ptr));
    git_commit_free(parent_commit_ptr);  // Free the temporary pointer
    
    if (!parent_commit) {
        return changes;
    }
    
    // Get parent tree
    git_tree *parent_tree_ptr = nullptr;
    if (git_commit_tree(&parent_tree_ptr, parent_commit.get()) != 0) {
        qWarning() << "GitDiffProvider: Failed to get parent tree";
        return changes;
    }
    GitTreeGuard parent_tree(parent_tree_ptr);
    
    // Create diff
    git_diff *diff_ptr = nullptr;
    if (git_diff_tree_to_tree(&diff_ptr, m_repo, parent_tree.get(), commit_tree.get(), nullptr) != 0) {
        qWarning() << "GitDiffProvider: Failed to create diff";
        return changes;
    }
    GitDiffGuard diff(diff_ptr);
    
    // Get changed files
    size_t num_deltas = git_diff_num_deltas(diff.get());
    for (size_t i = 0; i < num_deltas; ++i) {
        const git_diff_delta *delta = git_diff_get_delta(diff.get(), i);
        if (delta) {
            FileChange change;
            change.path = QString::fromUtf8(delta->new_file.path);
            change.status = convertGitStatus(delta->status);
            changes.append(change);
        }
    }
    
    return changes;
}

QString GitDiffProvider::getFileDiff(const git_oid &oid, const QString &filePath)
{
    if (!m_repo || filePath.isEmpty()) {
        return "No file selected or repository not available.";
    }
    
    GitCommitGuard commit(m_repo, &oid);
    if (!commit) {
        return "Failed to lookup commit.";
    }
    
    // Check if this is initial commit
    if (git_commit_parentcount(commit.get()) == 0) {
        // For initial commit, show file content
        return getFileContent(oid, filePath);
    }
    
    // Get parent commit
    git_commit *parent_commit_ptr = nullptr;
    if (git_commit_parent(&parent_commit_ptr, commit.get(), 0) != 0) {
        return "Failed to get parent commit.";
    }
    GitCommitGuard parent_commit(m_repo, git_commit_id(parent_commit_ptr));
    git_commit_free(parent_commit_ptr);
    
    if (!parent_commit) {
        return "Failed to lookup parent commit.";
    }
    
    // Get trees
    git_tree *commit_tree_ptr = nullptr;
    git_tree *parent_tree_ptr = nullptr;
    
    if (git_commit_tree(&commit_tree_ptr, commit.get()) != 0 ||
        git_commit_tree(&parent_tree_ptr, parent_commit.get()) != 0) {
        if (commit_tree_ptr) git_tree_free(commit_tree_ptr);
        if (parent_tree_ptr) git_tree_free(parent_tree_ptr);
        return "Failed to get commit trees.";
    }
    
    GitTreeGuard commit_tree(commit_tree_ptr);
    GitTreeGuard parent_tree(parent_tree_ptr);
    
    // Create diff
    git_diff *diff_ptr = nullptr;
    if (git_diff_tree_to_tree(&diff_ptr, m_repo, parent_tree.get(), commit_tree.get(), nullptr) != 0) {
        return "Failed to create diff.";
    }
    GitDiffGuard diff(diff_ptr);
    
    // Generate patch
    git_buf patch_buf = GIT_BUF_INIT;
    if (git_diff_to_buf(&patch_buf, diff.get(), GIT_DIFF_FORMAT_PATCH) != 0) {
        return "Failed to generate diff.";
    }
    
    QString fullPatch = QString::fromUtf8(patch_buf.ptr, patch_buf.size);
    git_buf_dispose(&patch_buf);
    
    // Extract specific file diff
    QString output = extractFileDiff(fullPatch, filePath);
    
    if (output.isEmpty()) {
        return "File not found in commit diff.";
    }
    
    return output;
}

QString GitDiffProvider::getFileContent(const git_oid &oid, const QString &filePath)
{
    if (!m_repo || filePath.isEmpty()) {
        return "No file or repository available.";
    }
    
    GitCommitGuard commit(m_repo, &oid);
    if (!commit) {
        return "Failed to lookup commit.";
    }
    
    // Get commit tree
    git_tree *tree_ptr = nullptr;
    if (git_commit_tree(&tree_ptr, commit.get()) != 0) {
        return "Failed to get commit tree.";
    }
    GitTreeGuard tree(tree_ptr);
    
    // Get file entry
    git_tree_entry *entry = nullptr;
    if (git_tree_entry_bypath(&entry, tree.get(), filePath.toUtf8().constData()) != 0) {
        return "File not found in commit.";
    }
    
    // Get blob
    git_blob *blob = nullptr;
    int result = git_blob_lookup(&blob, m_repo, git_tree_entry_id(entry));
    git_tree_entry_free(entry);
    
    if (result != 0) {
        return "Failed to read file content.";
    }
    
    // Read content
    const void *content = git_blob_rawcontent(blob);
    size_t size = git_blob_rawsize(blob);
    QString text = QString::fromUtf8(static_cast<const char*>(content), size);
    
    git_blob_free(blob);
    
    return text;
}

FileChange::Status GitDiffProvider::convertGitStatus(git_delta_t status) const
{
    switch (status) {
        case GIT_DELTA_ADDED:
            return FileChange::Added;
        case GIT_DELTA_DELETED:
            return FileChange::Deleted;
        case GIT_DELTA_MODIFIED:
            return FileChange::Modified;
        case GIT_DELTA_RENAMED:
            return FileChange::Renamed;
        case GIT_DELTA_COPIED:
            return FileChange::Copied;
        default:
            return FileChange::Unknown;
    }
}

QString GitDiffProvider::extractFileDiff(const QString &fullPatch, const QString &filePath) const
{
    QString output;
    QStringList lines = fullPatch.split('\n');
    bool inTargetFile = false;
    
    for (const QString &line : lines) {
        // Check if this line starts a new file diff
        if (line.startsWith("diff --git")) {
            // Check if it's our target file
            if (line.contains(" b/" + filePath)) {
                inTargetFile = true;
                output += line + '\n';
            } else if (!output.isEmpty() && inTargetFile) {
                // We were in target file but now moved to another file
                break;
            } else {
                inTargetFile = false;
            }
        } else if (inTargetFile) {
            output += line + '\n';
        }
    }
    
    // Remove trailing newline if present
    if (output.endsWith('\n')) {
        output.chop(1);
    }
    
    return output;
}
