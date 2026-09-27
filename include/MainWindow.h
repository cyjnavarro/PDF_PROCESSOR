#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>
#include <memory>

class FileCollection;
class PdfProcessor;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onAddPdfFiles();
    void onClearFiles();
    void onMergeFiles();
    void onMoveUp();
    void onMoveDown();
    void onRemoveFile();
    void onOpenHome();
    void onOpenMergePage();
    void onConvertImagesToPdf();
    void onSplitPdf();

private:
    void setupUI();
    void setupHomePage();
    void setupMergePage();
    void setHomeButtonStyle(QPushButton* button, const QString& title, const QString& subtitle);
    void updateFileList();
    void updateButtonState();
    void setStatus(const QString& message);
    void setupConnections();
    void addFilesToCollection(const QStringList& files);
    bool exportImagesToPdf(const QStringList& imageFiles, const QString& outputPath, QString& errorMessage);

    // UI Widgets
    QStackedWidget* stackedWidget_;
    QWidget* homePage_;
    QWidget* mergePage_;
    QListWidget* fileList_;
    QLabel* statusLabel_;
    QPushButton* addButton_;
    QPushButton* clearButton_;
    QPushButton* mergeButton_;
    QPushButton* upButton_;
    QPushButton* downButton_;
    QPushButton* homeButton_;
    QPushButton* mergeHomeButton_;
    QPushButton* imagesToPdfButton_;
    QPushButton* splitPdfButton_;

    // Business logic
    std::unique_ptr<FileCollection> fileCollection_;
    std::unique_ptr<PdfProcessor> pdfProcessor_;
};
