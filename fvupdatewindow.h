#ifndef FVUPDATEWINDOW_H
#define FVUPDATEWINDOW_H

#include <QWidget>
#include <QNetworkReply>

class QGraphicsScene;

namespace Ui {
class FvUpdateWindow;
}

class FvUpdateWindow : public QWidget
{
	Q_OBJECT
	
public:
	explicit FvUpdateWindow(QWidget *parent = 0);
	~FvUpdateWindow();

	// Update the current update proposal from FvUpdater
	bool UpdateWindowWithCurrentProposedUpdate();

	void closeEvent(QCloseEvent* event);

protected:
    void UpdateReleaseNote();

protected slots:
    void signal_downloadSignal(int evt, QUrl, int progress, QNetworkReply::NetworkError);

private slots:
void on_installUpdateButton_clicked();
void slot_ReleaseNoteUpdated();

private:
	Ui::FvUpdateWindow*	m_ui;

    // 网络更新地址
    QString     file_url_;
    
    // 保存到的本地地址
    QString     saved_file_url_;
};

#endif // FVUPDATEWINDOW_H
