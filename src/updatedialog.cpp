#include "updatedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QFileInfo>

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("檢查更新");
    setMinimumWidth(500);
    setMinimumHeight(400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 標題標籤
    m_titleLabel = new QLabel("Qt 西洋棋更新", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);
    
    // 訊息標籤
    m_messageLabel = new QLabel("正在檢查更新...", this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_messageLabel);
    
    // 發行說明
    m_releaseNotesEdit = new QTextEdit(this);
    m_releaseNotesEdit->setReadOnly(true);
    m_releaseNotesEdit->hide();
    mainLayout->addWidget(m_releaseNotesEdit);
    
    // 進度條
    m_progressBar = new QProgressBar(this);
    m_progressBar->hide();
    mainLayout->addWidget(m_progressBar);
    
    // 按鈕佈局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_downloadButton = new QPushButton("下載更新", this);
    m_downloadButton->hide();
    connect(m_downloadButton, &QPushButton::clicked, this, &UpdateDialog::onDownloadClicked);
    buttonLayout->addWidget(m_downloadButton);
    
    m_cancelButton = new QPushButton("關閉", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &UpdateDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::setUpdateInfo(const QString& currentVersion, const QString& latestVersion,
                                 const QString& downloadUrl, const QString& releaseNotes)
{
    m_downloadUrl = downloadUrl;
    
    m_messageLabel->setText(QString("發現新版本！\n\n當前版本：v%1\n最新版本：v%2")
                           .arg(currentVersion, latestVersion));
    
    m_releaseNotesEdit->setPlainText(releaseNotes);
    m_releaseNotesEdit->show();
    
    m_downloadButton->show();
    m_cancelButton->setText("稍後");
}

void UpdateDialog::showCheckingUpdate()
{
    m_messageLabel->setText("正在檢查更新...");
    m_releaseNotesEdit->hide();
    m_downloadButton->hide();
    m_progressBar->hide();
    m_cancelButton->setText("取消");
}

void UpdateDialog::showNoUpdate()
{
    m_messageLabel->setText("您已經在使用最新版本！");
    m_releaseNotesEdit->hide();
    m_downloadButton->hide();
    m_progressBar->hide();
    m_cancelButton->setText("關閉");
}

void UpdateDialog::showError(const QString& errorMessage)
{
    m_messageLabel->setText("檢查更新時發生錯誤：\n" + errorMessage);
    m_releaseNotesEdit->hide();
    m_downloadButton->hide();
    m_progressBar->hide();
    m_cancelButton->setText("關閉");
}

void UpdateDialog::startDownload()
{
    m_messageLabel->setText("正在下載更新...");
    m_releaseNotesEdit->hide();
    m_downloadButton->setEnabled(false);
    m_progressBar->setValue(0);
    m_progressBar->show();
    m_cancelButton->setText("取消");
}

void UpdateDialog::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        m_progressBar->setValue(progress);
        
        // 顯示下載大小
        double receivedMB = bytesReceived / (1024.0 * 1024.0);
        double totalMB = bytesTotal / (1024.0 * 1024.0);
        m_messageLabel->setText(QString("正在下載更新...\n%.2f MB / %.2f MB").arg(receivedMB).arg(totalMB));
    }
}

void UpdateDialog::downloadComplete(const QString& filePath)
{
    m_filePath = filePath;
    m_progressBar->setValue(100);
    
    m_messageLabel->setText(QString("下載完成！\n\n檔案已儲存至：\n%1").arg(filePath));
    
    m_downloadButton->setText("開啟檔案位置");
    m_downloadButton->setEnabled(true);
    m_downloadButton->disconnect();
    connect(m_downloadButton, &QPushButton::clicked, [filePath]() {
        QFileInfo fileInfo(filePath);
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    });
    m_downloadButton->show();
    
    m_cancelButton->setText("關閉");
}

void UpdateDialog::downloadFailed(const QString& errorMessage)
{
    m_messageLabel->setText("下載失敗：\n" + errorMessage);
    m_progressBar->hide();
    m_downloadButton->setText("重試");
    m_downloadButton->setEnabled(true);
    m_cancelButton->setText("關閉");
}

void UpdateDialog::onDownloadClicked()
{
    emit downloadRequested();
}

void UpdateDialog::onCancelClicked()
{
    reject();
}
