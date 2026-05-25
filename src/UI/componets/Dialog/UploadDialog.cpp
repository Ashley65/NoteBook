//
// Created by DevAccount on 18/05/2026.
//


#include "UI/components/Dialogs/UploadDialog.h"
#include "UI/components/Dialogs/DropAreaWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QLabel>

UploadDialog::UploadDialog(QWidget* parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Upload Attachments"));
    setMinimumSize(520, 420);

    auto* mainLayout = new QVBoxLayout(this);

    auto* title = new QLabel(tr("Drop files here or choose them manually"), this);
    title->setStyleSheet("font-size: 14px; font-weight: 600;");
    mainLayout->addWidget(title);

    m_dropArea = new DropAreaWidget(this);
    m_dropArea->setAllowedExtensions(QStringList() << "pdf" << "png" << "jpg" << "jpeg" << "txt" << "md" << "docx" << "xlsx");

    connect(m_dropArea, &DropAreaWidget::filesDropped, this, &UploadDialog::onFilesDropped);
    mainLayout->addWidget(m_dropArea);

    connect(m_dropArea, &DropAreaWidget::browseRequested,
        this, &UploadDialog::onChooseFiles);


    m_fileList = new QListWidget(this);
    mainLayout->addWidget(m_fileList, 1);

    m_statusLabel = new QLabel(tr("No files selected"), this);
    mainLayout->addWidget(m_statusLabel);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    mainLayout->addWidget(m_progress);

    auto* buttonRow = new QHBoxLayout();

    m_btnChoose = new QPushButton(tr("Choose Files"), this);
    m_btnUpload = new QPushButton(tr("Upload"), this);
    m_btnCancel = new QPushButton(tr("Cancel"), this);

    buttonRow->addWidget(m_btnChoose);
    buttonRow->addWidget(m_btnUpload);
    buttonRow->addStretch();
    buttonRow->addWidget(m_btnCancel);

    mainLayout->addLayout(buttonRow);

    connect(m_btnChoose, &QPushButton::clicked, this, &UploadDialog::onChooseFiles);
    connect(m_btnUpload, &QPushButton::clicked, this, &UploadDialog::onUploadPressed);
    connect(m_btnCancel, &QPushButton::clicked, this, &UploadDialog::reject);

}

void UploadDialog::setWorkspaceId(const QUuid& wsId)
{
    workspaceId_ = wsId;
}

void UploadDialog::setProjectId(const QUuid& projectId)
{
    projectId_ = projectId;
}

void UploadDialog::setFileFilter(const QString& filter)
{
        fileFilter_ = filter;
}

void UploadDialog::onChooseFiles()
{
    const QStringList selectedFiles = QFileDialog::getOpenFileNames(
       this,
       tr("Select Files to Upload"),
       QString(),
       fileFilter_
   );

    if (selectedFiles.isEmpty()) {
        return;
    }

    onFilesDropped(selectedFiles);

}

void UploadDialog::onUploadPressed()
{
    if (m_selectedFiles.isEmpty()) {
        m_statusLabel->setText(tr("Please choose files first"));
        return;
    }

    emit uploadRequested(m_selectedFiles, workspaceId_, projectId_);
    accept();
}

void UploadDialog::onFilesDropped(const QStringList& filePaths)
{
    m_selectedFiles.clear();
    m_fileList->clear();

    for (const QString& path : filePaths) {
        QFileInfo info(path);
        if (!info.exists() || !info.isFile()) {
            continue;
        }
        const QString abs = info.absoluteFilePath();
        m_selectedFiles << abs;
        m_fileList->addItem(info.fileName());
    }

    m_statusLabel->setText(tr("%1 file(s) ready to upload").arg(m_selectedFiles.size()));
}