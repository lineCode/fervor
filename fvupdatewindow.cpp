#include "fvupdatewindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

#include "include\ksyncrequest.h"
#include "include\misc.h"

#include "ui_fvupdatewindow.h"
#include "fvupdater.h"
#include "fvavailableupdate.h"
#include "define.h"


FvUpdateWindow::FvUpdateWindow(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::FvUpdateWindow)
{
	m_ui->setupUi(this);

	// Delete on close
	setAttribute(Qt::WA_DeleteOnClose, true);

	// Set the "new version is available" string
	QString newVersString = m_ui->newVersionIsAvailableLabel->text()
        .arg(QString::fromUtf8(FV_APP_NAME));
	m_ui->newVersionIsAvailableLabel->setText(newVersString);
    m_ui->download_progress_->hide();

    FvAvailableUpdate* fv = FvUpdater::sharedUpdater()->GetProposedUpdate();
    file_url_ = fv->GetEnclosureUrl().toString();
	// Connect buttons
// 	connect(m_ui->installUpdateButton, SIGNAL(clicked()),
// 			FvUpdater::sharedUpdater(), SLOT(InstallUpdate()));
	qui::CheckedConnect(m_ui->skipThisVersionButton, SIGNAL(clicked()),
			FvUpdater::sharedUpdater(), SLOT(SkipUpdate()));
    qui::CheckedConnect(m_ui->remindMeLaterButton, SIGNAL(clicked()),
			FvUpdater::sharedUpdater(), SLOT(RemindMeLater()));
    qui::CheckedConnect(fv, SIGNAL(signal_ReleaseNoteUpdated()),
        this, SLOT(slot_ReleaseNoteUpdated()));
}

FvUpdateWindow::~FvUpdateWindow()
{
    KHttpCancelDownload(file_url_, this);

    // m_ui->textBrowser->stop();
	delete m_ui;
}

bool FvUpdateWindow::UpdateWindowWithCurrentProposedUpdate()
{
	FvAvailableUpdate* proposedUpdate = FvUpdater::sharedUpdater()->GetProposedUpdate();
	if (! proposedUpdate) {
		return false;
	}

	QString downloadString = m_ui->wouldYouLikeToDownloadLabel->text()
			.arg(FV_APP_NAME, proposedUpdate->GetEnclosureVersion(), FV_APP_VERSION);
	m_ui->wouldYouLikeToDownloadLabel->setText(downloadString);

    UpdateReleaseNote();
//    m_ui->textBrowser->setHtml("");

// 	m_ui->releaseNotesWebView->stop();
// 	m_ui->releaseNotesWebView->load(proposedUpdate->GetReleaseNotesLink());

	return true;
}

void FvUpdateWindow::closeEvent(QCloseEvent* event)
{
	FvUpdater::sharedUpdater()->updaterWindowWasClosed();
	event->accept();
}

void FvUpdateWindow::signal_downloadSignal(int evt, QUrl, int progress, QNetworkReply::NetworkError)
{
    switch (evt)
    {
    case HttpDownload_Event_Finished:
        if (QMessageBox::question(this, QStringLiteral("软件更新"),
            QStringLiteral("已下载完成，现在更新吗？")) == QMessageBox::Yes)
        {
            // 启动更新文件
            QProcess pro;
            QStringList args;
            pro.start(saved_file_url_, args);
        }

        // 关闭
        close();
        break;

    case HttpDownload_Event_Progress:
        m_ui->download_progress_->setValue(progress);
        break;

    default:
        break;
    }
}

void FvUpdateWindow::on_installUpdateButton_clicked()
{
    // 不能多次点击
    // qobject_cast<QWidget*>(sender())->setEnabled(false);
    m_ui->installUpdateButton->setEnabled(false);

    // 显示更新进度条
    m_ui->download_progress_->show();

    // FvAvailableUpdate* fv = FvUpdater::sharedUpdater()->GetProposedUpdate();

    // 下载文件
    saved_file_url_ = QDir::currentPath() + "/" + QFileInfo(file_url_).fileName();
    KHttpDownload(file_url_, saved_file_url_, this,
        SLOT(signal_downloadSignal(int, QUrl, int, QNetworkReply::NetworkError)));
}

void FvUpdateWindow::slot_ReleaseNoteUpdated()
{
    UpdateReleaseNote();
}

void FvUpdateWindow::UpdateReleaseNote()
{
    FvAvailableUpdate* fv = FvUpdater::sharedUpdater()->GetProposedUpdate();

    QString sHtml = qui::ReadTextFile(fv->UpdateHistoryHtmlFile());
    m_ui->textBrowser->setHtml(sHtml.toUtf8());
}
