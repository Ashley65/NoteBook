//
// Created for TaskHelper - Embedded Note Attachments
//

#include "helpers/NoteAttachmentRenderer.h"
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <QDir>
#include <QDebug>

NoteAttachmentRenderer::FileType NoteAttachmentRenderer::detectFileType(const QString& fileName)
{
    const QString ext = QFileInfo(fileName).suffix().toLower();

    static const QStringList imageExts = { "png", "jpg", "jpeg", "gif", "svg", "webp", "bmp", "ico" };
    if (imageExts.contains(ext)) return FileType::Image;

    if (ext == "pdf") return FileType::Pdf;

    static const QStringList codeExts = {
        "py", "cpp", "c", "h", "hpp", "cs", "js", "ts", "jsx", "tsx",
        "html", "css", "json", "sql", "sh", "bat", "ps1", "rs", "go",
        "java", "xml", "yaml", "yml", "txt", "ini", "env", "toml"
    };
    if (codeExts.contains(ext)) return FileType::Code;

    if (ext == "csv" || ext == "tsv") return FileType::Csv;

    return FileType::Generic;
}

QString NoteAttachmentRenderer::formatFileSize(qint64 bytes)
{
    if (bytes <= 0) return "0 B";
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1));
}

QString NoteAttachmentRenderer::renderImageCard(const QString& filePath, const QString& fileName, qint64 fileSize)
{
    const QString safeName = fileName.toHtmlEscaped();
    const QString safeSize = formatFileSize(fileSize);
    const QString fileUrl = QUrl::fromLocalFile(filePath).toString();
    const QString encodedPath = QString::fromUtf8(QUrl::toPercentEncoding(filePath));

    QString html =
        "<table width=\"100%\" style=\"margin: 14px 0; background-color: #171824; border: 1px solid #2B2D3F; border-radius: 8px;\" cellpadding=\"8\" cellspacing=\"0\">"
        "  <tr>"
        "    <td style=\"text-align: center; border: none;\">"
        "      <a href=\"action://open-file?path=%%CARD_ENCODED_PATH%%\">"
        "        <img src=\"%%CARD_FILE_URL%%\" width=\"480\" style=\"border-radius: 6px; border: 1px solid #232536;\" />"
        "      </a>"
        "    </td>"
        "  </tr>"
        "  <tr>"
        "    <td style=\"font-size: 11px; color: #8C92A4; text-align: center; border: none; padding-top: 4px;\">"
        "      <b>📷 %%CARD_NAME%%</b> (%%CARD_SIZE%%) &bull; "
        "      <a href=\"action://open-file?path=%%CARD_ENCODED_PATH%%\" style=\"color: #818CF8; text-decoration: none; font-weight: bold;\">Open Full Size</a>"
        "    </td>"
        "  </tr>"
        "</table>";

    html.replace("%%CARD_ENCODED_PATH%%", encodedPath);
    html.replace("%%CARD_FILE_URL%%", fileUrl);
    html.replace("%%CARD_NAME%%", safeName);
    html.replace("%%CARD_SIZE%%", safeSize);
    return html;
}

QString NoteAttachmentRenderer::renderPdfCard(const QString& filePath, const QString& fileName, qint64 fileSize)
{
    const QString safeName = fileName.toHtmlEscaped();
    const QString safeSize = formatFileSize(fileSize);
    const QString encodedPath = QString::fromUtf8(QUrl::toPercentEncoding(filePath));

    QString html =
        "<table width=\"100%\" style=\"margin: 14px 0; background-color: #1A1C2B; border: 1px solid #2D3044; border-radius: 8px;\" cellpadding=\"12\">"
        "  <tr>"
        "    <td width=\"44\" style=\"background-color: #2F1E24; text-align: center; vertical-align: middle; border: none; border-radius: 6px;\">"
        "      <span style=\"font-size: 16px; color: #EF4444; font-weight: bold; font-family: sans-serif;\">PDF</span>"
        "    </td>"
        "    <td style=\"border: none; padding-left: 12px; vertical-align: middle;\">"
        "      <div style=\"font-size: 14px; font-weight: bold; color: #F3F4F6;\">%%CARD_NAME%%</div>"
        "      <div style=\"font-size: 11px; color: #9CA3AF; margin-top: 3px;\">PDF Document &bull; %%CARD_SIZE%%</div>"
        "    </td>"
        "    <td width=\"100\" style=\"text-align: right; vertical-align: middle; border: none;\">"
        "      <a href=\"action://open-pdf?path=%%CARD_ENCODED_PATH%%\" style=\"background-color: #3B82F6; color: #FFFFFF; padding: 6px 14px; text-decoration: none; font-size: 11px; font-weight: bold; border-radius: 4px;\">Quick View</a>"
        "    </td>"
        "  </tr>"
        "</table>";

    html.replace("%%CARD_NAME%%", safeName);
    html.replace("%%CARD_SIZE%%", safeSize);
    html.replace("%%CARD_ENCODED_PATH%%", encodedPath);
    return html;
}

QString NoteAttachmentRenderer::renderCodeCard(const QString& filePath, const QString& fileName, qint64 fileSize)
{
    const QString safeName = fileName.toHtmlEscaped();
    const QString safeSize = formatFileSize(fileSize);
    const QString encodedPath = QString::fromUtf8(QUrl::toPercentEncoding(filePath));

    QString snippet = "";
    int lineCount = 0;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd() && lineCount < 20) {
            const QString line = in.readLine();
            snippet += line.toHtmlEscaped() + "\n";
            lineCount++;
        }
        if (!in.atEnd()) {
            snippet += "... (remaining lines truncated) ...\n";
        }
        file.close();
    } else {
        snippet = "(Unable to read file content)";
    }

    QString html =
        "<table width=\"100%\" style=\"margin: 14px 0; background-color: #141520; border: 1px solid #2D3044; border-radius: 8px;\" cellpadding=\"0\" cellspacing=\"0\">"
        "  <tr>"
        "    <td style=\"background-color: #1F2232; padding: 8px 12px; border-bottom: 1px solid #2D3044; color: #A5B4FC; font-family: Consolas, monospace; font-size: 12px;\">"
        "      <b>%%CARD_NAME%%</b> <span style=\"color: #6B7280; font-size: 11px;\">(%%CARD_SIZE%%)</span>"
        "    </td>"
        "    <td style=\"background-color: #1F2232; padding: 8px 12px; border-bottom: 1px solid #2D3044; text-align: right;\">"
        "      <a href=\"action://open-file?path=%%CARD_ENCODED_PATH%%\" style=\"color: #818CF8; text-decoration: none; font-size: 11px; font-weight: bold;\">Open</a>"
        "    </td>"
        "  </tr>"
        "  <tr>"
        "    <td colspan=\"2\" style=\"padding: 12px; border: none;\">"
        "      <pre style=\"margin: 0; font-family: Consolas, monospace; font-size: 12px; color: #E2E8F0; line-height: 1.4;\">%%CARD_SNIPPET%%</pre>"
        "    </td>"
        "  </tr>"
        "</table>";

    html.replace("%%CARD_NAME%%", safeName);
    html.replace("%%CARD_SIZE%%", safeSize);
    html.replace("%%CARD_ENCODED_PATH%%", encodedPath);
    html.replace("%%CARD_SNIPPET%%", snippet);
    return html;
}

QString NoteAttachmentRenderer::renderCsvCard(const QString& filePath, const QString& fileName, qint64 fileSize)
{
    const QString safeName = fileName.toHtmlEscaped();
    const QString safeSize = formatFileSize(fileSize);
    const QString encodedPath = QString::fromUtf8(QUrl::toPercentEncoding(filePath));

    QString tableRows = "";
    int rowCount = 0;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        const QChar delimiter = fileName.endsWith(".tsv", Qt::CaseInsensitive) ? '\t' : ',';

        while (!in.atEnd() && rowCount < 8) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            const QStringList cells = line.split(delimiter);
            tableRows += "<tr>";
            for (const QString& cell : cells) {
                const QString safeCell = cell.trimmed().toHtmlEscaped();
                if (rowCount == 0) {
                    tableRows += QString("<th style=\"background-color: #1F2232; color: #93C5FD; padding: 6px 10px; border: 1px solid #2D3044; text-align: left;\">%1</th>").arg(safeCell);
                } else {
                    const QString bg = (rowCount % 2 == 0) ? "#141520" : "#181A28";
                    tableRows += QString("<td style=\"background-color: %1; color: #D1D5DB; padding: 5px 10px; border: 1px solid #2D3044;\">%2</td>").arg(bg, safeCell);
                }
            }
            tableRows += "</tr>";
            rowCount++;
        }
        file.close();
    }

    if (tableRows.isEmpty()) {
        tableRows = "<tr><td style=\"padding: 10px; color: #9CA3AF;\">(Empty data file)</td></tr>";
    }

    QString html =
        "<table width=\"100%\" style=\"margin: 14px 0; border: 1px solid #2D3044; border-radius: 8px;\" cellpadding=\"0\" cellspacing=\"0\">"
        "  <tr>"
        "    <td>"
        "      <table width=\"100%\" style=\"border-collapse: collapse; font-size: 12px; font-family: Consolas, sans-serif;\" cellpadding=\"6\">"
        "        %%CARD_TABLE_ROWS%%"
        "      </table>"
        "    </td>"
        "  </tr>"
        "  <tr>"
        "    <td style=\"background-color: #1A1C28; padding: 6px 12px; font-size: 11px; color: #9CA3AF; border-top: 1px solid #2D3044;\">"
        "      <b>%%CARD_NAME%%</b> &bull; %%CARD_SIZE%% &bull; "
        "      <a href=\"action://open-file?path=%%CARD_ENCODED_PATH%%\" style=\"color: #818CF8; text-decoration: none; font-weight: bold;\">Open Full File</a>"
        "    </td>"
        "  </tr>"
        "</table>";

    html.replace("%%CARD_TABLE_ROWS%%", tableRows);
    html.replace("%%CARD_NAME%%", safeName);
    html.replace("%%CARD_SIZE%%", safeSize);
    html.replace("%%CARD_ENCODED_PATH%%", encodedPath);
    return html;
}

QString NoteAttachmentRenderer::renderGenericCard(const QString& filePath, const QString& fileName, qint64 fileSize)
{
    const QString safeName = fileName.toHtmlEscaped();
    const QString safeSize = formatFileSize(fileSize);
    const QString ext = QFileInfo(fileName).suffix().toUpper();
    const QString safeExt = ext.isEmpty() ? "FILE" : ext.left(4).toHtmlEscaped();
    const QString encodedPath = QString::fromUtf8(QUrl::toPercentEncoding(filePath));

    QString html =
        "<table width=\"100%\" style=\"margin: 14px 0; background-color: #1A1C2B; border: 1px solid #2D3044; border-radius: 8px;\" cellpadding=\"12\">"
        "  <tr>"
        "    <td width=\"44\" style=\"background-color: #212435; text-align: center; vertical-align: middle; border: none; border-radius: 6px;\">"
        "      <span style=\"font-size: 13px; color: #60A5FA; font-weight: bold; font-family: sans-serif;\">%%CARD_EXT%%</span>"
        "    </td>"
        "    <td style=\"border: none; padding-left: 12px; vertical-align: middle;\">"
        "      <div style=\"font-size: 14px; font-weight: bold; color: #F3F4F6;\">%%CARD_NAME%%</div>"
        "      <div style=\"font-size: 11px; color: #9CA3AF; margin-top: 3px;\">Attachment &bull; %%CARD_SIZE%%</div>"
        "    </td>"
        "    <td width=\"90\" style=\"text-align: right; vertical-align: middle; border: none;\">"
        "      <a href=\"action://open-file?path=%%CARD_ENCODED_PATH%%\" style=\"background-color: #2563EB; color: #FFFFFF; padding: 6px 14px; text-decoration: none; font-size: 11px; font-weight: bold; border-radius: 4px;\">Open</a>"
        "    </td>"
        "  </tr>"
        "</table>";

    html.replace("%%CARD_EXT%%", safeExt);
    html.replace("%%CARD_NAME%%", safeName);
    html.replace("%%CARD_SIZE%%", safeSize);
    html.replace("%%CARD_ENCODED_PATH%%", encodedPath);
    return html;
}

QString NoteAttachmentRenderer::renderMissingCard(const QString& fileName)
{
    const QString safeName = fileName.toHtmlEscaped();
    return QString(
        "<div style=\"margin: 12px 0; padding: 10px 14px; background-color: #26211C; border: 1px solid #573B1B; border-radius: 6px; color: #FBBF24; font-size: 12px;\">"
        "  ⚠️ <b>Attachment not found:</b> <code>%1</code> (Attach this file to your project or note to view it)"
        "</div>"
    ).arg(safeName);
}

QString NoteAttachmentRenderer::processNoteMarkdown(const QString& rawMarkdown,
                                                   const QList<FileAttachment>& availableAttachments)
{
    QString result = rawMarkdown;

    // Pattern 1: Obsidian wikilink embed: ![[filename.ext]] or ![[filename.ext|options]]
    static const QRegularExpression wikiEmbedRegex(R"(!\[\[([^\]|]+)(?:\|([^\]]+))?\]\])");

    // Helper lambda to find attachment by filename or path
    auto findAttachment = [&](const QString& target) -> FileAttachment {
        const QString cleanTarget = target.trimmed();
        for (const auto& att : availableAttachments) {
            if (att.fileName.compare(cleanTarget, Qt::CaseInsensitive) == 0 ||
                QFileInfo(att.relativePath).fileName().compare(cleanTarget, Qt::CaseInsensitive) == 0) {
                return att;
            }
        }
        // Direct path check if it's already a local path
        if (QFile::exists(cleanTarget)) {
            FileAttachment direct;
            direct.fileName = QFileInfo(cleanTarget).fileName();
            direct.relativePath = cleanTarget;
            direct.fileSize = QFileInfo(cleanTarget).size();
            return direct;
        }
        return {};
    };

    // Replace ![[...]]
    QRegularExpressionMatchIterator it = wikiEmbedRegex.globalMatch(result);
    // Collect replacements backwards or in order
    struct Replacement {
        int start;
        int length;
        QString html;
    };
    QList<Replacement> replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        const QString fileName = match.captured(1).trimmed();

        const FileAttachment att = findAttachment(fileName);
        QString cardHtml;

        if (att.relativePath.isEmpty() || !QFile::exists(att.relativePath)) {
            cardHtml = renderMissingCard(fileName);
        } else {
            const FileType type = detectFileType(att.fileName.isEmpty() ? fileName : att.fileName);
            switch (type) {
                case FileType::Image:
                    cardHtml = renderImageCard(att.relativePath, att.fileName.isEmpty() ? fileName : att.fileName, att.fileSize);
                    break;
                case FileType::Pdf:
                    cardHtml = renderPdfCard(att.relativePath, att.fileName.isEmpty() ? fileName : att.fileName, att.fileSize);
                    break;
                case FileType::Code:
                    cardHtml = renderCodeCard(att.relativePath, att.fileName.isEmpty() ? fileName : att.fileName, att.fileSize);
                    break;
                case FileType::Csv:
                    cardHtml = renderCsvCard(att.relativePath, att.fileName.isEmpty() ? fileName : att.fileName, att.fileSize);
                    break;
                case FileType::Generic:
                default:
                    cardHtml = renderGenericCard(att.relativePath, att.fileName.isEmpty() ? fileName : att.fileName, att.fileSize);
                    break;
            }
        }

        Replacement rep;
        rep.start = match.capturedStart();
        rep.length = match.capturedLength();
        rep.html = cardHtml;
        replacements.append(rep);
    }

    // Apply replacements backwards to preserve offsets
    for (int i = replacements.size() - 1; i >= 0; --i) {
        result.replace(replacements[i].start, replacements[i].length, replacements[i].html);
    }

    return result;
}
