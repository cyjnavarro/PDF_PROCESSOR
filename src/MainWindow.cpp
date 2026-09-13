#include "MainWindow.h"
#include "FileModel.h"
#include "PdfProcessor.h"
#include "StringConverter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QGridLayout>
#include <QDesktopServices>
#include <QPrinter>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QImage>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      stackedWidget_(nullptr),
      homePage_(nullptr),
      mergePage_(nullptr),
      fileList_(nullptr),
      statusLabel_(nullptr),
      addButton_(nullptr),
      clearButton_(nullptr),
      mergeButton_(nullptr),
      upButton_(nullptr),
      downButton_(nullptr),
      homeButton_(nullptr),
      mergeHomeButton_(nullptr),
      imagesToPdfButton_(nullptr),
      documentsToPdfButton_(nullptr),
      excelToPdfButton_(nullptr),
      printPdfButton_(nullptr),
      fileCollection_(std::make_unique<FileCollection>()),
      pdfProcessor_(std::make_unique<PdfProcessor>()) {
    setWindowTitle("PDF Workspace");
    setupUI();
    setupConnections();
    setAcceptDrops(true);
    resize(900, 650);
}

MainWindow::~MainWindow() = default;

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    QStringList files;
    foreach (QUrl url, event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            files.append(url.toLocalFile());
        }
    }

    if (!files.isEmpty()) {
        addFilesToCollection(files);
        setStatus("Files added via drag-and-drop.");
        stackedWidget_->setCurrentWidget(mergePage_);
    }
}

void MainWindow::setupUI() {
    stackedWidget_ = new QStackedWidget(this);
    setCentralWidget(stackedWidget_);

    setupHomePage();
    setupMergePage();
    stackedWidget_->setCurrentWidget(homePage_);

    QString darkStylesheet = R"(
        QMainWindow { background-color: #0d1117; }
        QWidget { background-color: #0d1117; color: #c9d1d9; }
        QStackedWidget { background-color: #0d1117; }
        QListWidget {
            background-color: #161b22;
            color: #c9d1d9;
            border: 1px solid #30363d;
            border-radius: 6px;
        }
        QListWidget::item:hover { background-color: #21262d; }
        QListWidget::item:selected { background-color: #1f6feb; }
        QPushButton {
            background-color: #21262d;
            color: #c9d1d9;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 8px 14px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #30363d;
            border: 1px solid #58a6ff;
        }
        QPushButton:pressed { background-color: #1f6feb; }
        QPushButton:disabled {
            color: #6e7681;
            background-color: #0d1117;
        }
        QLabel { color: #c9d1d9; }
        QToolTip {
            background-color: #161b22;
            color: #c9d1d9;
            border: 1px solid #30363d;
        }
    )";

    setStyleSheet(darkStylesheet);
}

void MainWindow::setupHomePage() {
    homePage_ = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(homePage_);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(18);

    QLabel* titleLabel = new QLabel("PDF Workspace", homePage_);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("Choose a task to get started.", homePage_);
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(11);
    subtitleLabel->setFont(subtitleFont);
    layout->addWidget(subtitleLabel);

    QGridLayout* toolsLayout = new QGridLayout();
    toolsLayout->setSpacing(18);

    mergeHomeButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(mergeHomeButton_, "Merge PDFs", "Combine multiple PDFs into one file");
    imagesToPdfButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(imagesToPdfButton_, "Convert Images to PDF", "Turn collections of images into a PDF");
    documentsToPdfButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(documentsToPdfButton_, "Convert Docs to PDF", "Open doc workflows and export as PDF");
    excelToPdfButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(excelToPdfButton_, "Convert Excel to PDF", "Prepare spreadsheets for PDF export");
    printPdfButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(printPdfButton_, "Print PDF", "Open a selected PDF in its default viewer");

    toolsLayout->addWidget(mergeHomeButton_, 0, 0);
    toolsLayout->addWidget(imagesToPdfButton_, 0, 1);
    toolsLayout->addWidget(documentsToPdfButton_, 1, 0);
    toolsLayout->addWidget(excelToPdfButton_, 1, 1);
    toolsLayout->addWidget(printPdfButton_, 2, 0, 1, 2);
    toolsLayout->setColumnStretch(0, 1);
    toolsLayout->setColumnStretch(1, 1);

    layout->addLayout(toolsLayout);
    layout->addStretch();
    stackedWidget_->addWidget(homePage_);
}

void MainWindow::setHomeButtonStyle(QPushButton* button, const QString& title, const QString& subtitle) {
    button->setMinimumHeight(110);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    button->setStyleSheet(QString(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1f2937, stop:1 #111827);"
        "   border: 1px solid #374151;"
        "   border-radius: 12px;"
        "   padding: 18px;"
        "   text-align: left;"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #23314d, stop:1 #172033);"
        "   border: 1px solid #60a5fa;"
        "}"
    ));
    button->setText(title + "\n" + subtitle);
    button->setToolTip(title);
}

void MainWindow::setupMergePage() {
    mergePage_ = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(mergePage_);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    homeButton_ = new QPushButton("Back to Home", mergePage_);
    homeButton_->setMaximumWidth(130);
    mainLayout->addWidget(homeButton_);

    QLabel* titleLabel = new QLabel("Merge PDF Files", mergePage_);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QLabel* filesLabel = new QLabel("Selected Files:", mergePage_);
    QFont normalFont = filesLabel->font();
    normalFont.setPointSize(10);
    filesLabel->setFont(normalFont);
    mainLayout->addWidget(filesLabel);

    fileList_ = new QListWidget(mergePage_);
    fileList_->setMinimumHeight(220);
    fileList_->setFont(normalFont);
    mainLayout->addWidget(fileList_);

    statusLabel_ = new QLabel("Ready", mergePage_);
    statusLabel_->setFont(normalFont);
    statusLabel_->setStyleSheet("color: #c9d1d9; padding: 5px;");
    mainLayout->addWidget(statusLabel_);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(8);

    addButton_ = new QPushButton("Add PDFs", mergePage_);
    clearButton_ = new QPushButton("Clear Files", mergePage_);
    upButton_ = new QPushButton("Up", mergePage_);
    downButton_ = new QPushButton("Down", mergePage_);
    mergeButton_ = new QPushButton("Merge & Save As", mergePage_);

    addButton_->setMinimumHeight(32);
    clearButton_->setMinimumHeight(32);
    upButton_->setMinimumHeight(32);
    downButton_->setMinimumHeight(32);
    mergeButton_->setMinimumHeight(32);

    buttonsLayout->addWidget(addButton_);
    buttonsLayout->addWidget(clearButton_);
    buttonsLayout->addWidget(upButton_);
    buttonsLayout->addWidget(downButton_);
    buttonsLayout->addWidget(mergeButton_);

    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    stackedWidget_->addWidget(mergePage_);
}

void MainWindow::setupConnections() {
    connect(homeButton_, &QPushButton::clicked, this, &MainWindow::onOpenHome);
    connect(mergeHomeButton_, &QPushButton::clicked, this, &MainWindow::onOpenMergePage);
    connect(imagesToPdfButton_, &QPushButton::clicked, this, &MainWindow::onConvertImagesToPdf);
    connect(documentsToPdfButton_, &QPushButton::clicked, this, &MainWindow::onConvertDocumentsToPdf);
    connect(excelToPdfButton_, &QPushButton::clicked, this, &MainWindow::onConvertExcelToPdf);
    connect(printPdfButton_, &QPushButton::clicked, this, &MainWindow::onPrintPdf);
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddPdfFiles);
    connect(clearButton_, &QPushButton::clicked, this, &MainWindow::onClearFiles);
    connect(mergeButton_, &QPushButton::clicked, this, &MainWindow::onMergeFiles);
    connect(upButton_, &QPushButton::clicked, this, &MainWindow::onMoveUp);
    connect(downButton_, &QPushButton::clicked, this, &MainWindow::onMoveDown);
}

void MainWindow::addFilesToCollection(const QStringList& files) {
    for (const QString& filePath : files) {
        std::string utf8Path = StringConverter::toUtf8(filePath.toStdWString());
        PdfFile pdfFile(utf8Path);
        std::string errorMsg;
        if (pdfFile.validate(errorMsg)) {
            fileCollection_->add(pdfFile);
        }
    }
    updateFileList();
}

bool MainWindow::exportImagesToPdf(const QStringList& imageFiles, const QString& outputPath) {
    if (imageFiles.isEmpty()) {
        return false;
    }

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QPainter painter;
    if (!painter.begin(&printer)) {
        return false;
    }

    const QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);

    for (int i = 0; i < imageFiles.size(); ++i) {
        if (i > 0) {
            printer.newPage();
        }

        QImage image(imageFiles.at(i));
        if (image.isNull()) {
            continue;
        }

        const QImage scaled = image.scaled(pageRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QRectF targetRect(
            (pageRect.width() - scaled.width()) / 2.0,
            (pageRect.height() - scaled.height()) / 2.0,
            static_cast<double>(scaled.width()),
            static_cast<double>(scaled.height()));

        painter.drawImage(targetRect, scaled);
    }

    painter.end();
    return true;
}

void MainWindow::launchDocumentForPrint(const QString& filePath) {
    const QUrl url = QUrl::fromLocalFile(filePath);
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, "Open Failed",
            "The file could not be opened with the default application. Please open it manually and use Print to PDF.");
    }
}

void MainWindow::onOpenHome() {
    stackedWidget_->setCurrentWidget(homePage_);
    setStatus("Home");
}

void MainWindow::onOpenMergePage() {
    stackedWidget_->setCurrentWidget(mergePage_);
    setStatus("Ready to merge PDFs.");
}

void MainWindow::onConvertImagesToPdf() {
    const QStringList imageFiles = QFileDialog::getOpenFileNames(
        this, "Select images to convert", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All Files (*)");

    if (imageFiles.isEmpty()) {
        return;
    }

    const QString outputPath = QFileDialog::getSaveFileName(
        this, "Save converted PDF as", "converted_images.pdf", "PDF Files (*.pdf);;All Files (*)");

    if (outputPath.isEmpty()) {
        return;
    }

    setStatus("Converting images to PDF...");
    const bool success = exportImagesToPdf(imageFiles, outputPath);
    if (!success) {
        QMessageBox::critical(this, "Conversion Failed", "The images could not be converted to PDF.");
        setStatus("Image conversion failed.");
        return;
    }

    QMessageBox::information(this, "Success", "Images were converted to PDF successfully.");
    setStatus("Image conversion complete.");
}

void MainWindow::onConvertDocumentsToPdf() {
    const QStringList files = QFileDialog::getOpenFileNames(
        this, "Select document files", "", "Documents (*.doc *.docx *.odt *.rtf *.txt *.pdf);;All Files (*)");

    if (files.isEmpty()) {
        return;
    }

    const QString selectedFile = files.first();
    QMessageBox::information(this, "Document Workflow",
        "Open the selected document in your default app and use Save as PDF or Print to PDF.");
    launchDocumentForPrint(selectedFile);
    setStatus("Document workflow started.");
}

void MainWindow::onConvertExcelToPdf() {
    const QStringList files = QFileDialog::getOpenFileNames(
        this, "Select Excel files", "", "Excel Files (*.xlsx *.xls *.csv);;All Files (*)");

    if (files.isEmpty()) {
        return;
    }

    const QString selectedFile = files.first();
    QMessageBox::information(this, "Excel Workflow",
        "Open the selected spreadsheet in Excel or LibreOffice and export or print it to PDF.");
    launchDocumentForPrint(selectedFile);
    setStatus("Excel workflow started.");
}

void MainWindow::onPrintPdf() {
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Select PDF to print", "", "PDF Files (*.pdf);;All Files (*)");

    if (filePath.isEmpty()) {
        return;
    }

    QMessageBox::information(this, "Print PDF",
        "The selected PDF will be opened in the default application so you can print it from there.");
    launchDocumentForPrint(filePath);
    setStatus("Opened PDF for printing.");
}

void MainWindow::onAddPdfFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Select PDFs to merge", "", "PDF Files (*.pdf);;All Files (*)");

    if (!files.isEmpty()) {
        addFilesToCollection(files);
        setStatus("Files added to merge list.");
    }
}

void MainWindow::onClearFiles() {
    fileCollection_->clear();
    updateFileList();
    setStatus("Ready");
}

void MainWindow::onMergeFiles() {
    if (fileCollection_->isEmpty()) {
        QMessageBox::warning(this, "No Files", "Please select at least one PDF file first.");
        return;
    }

    QString outputPath = QFileDialog::getSaveFileName(this,
        "Save merged PDF as", "", "PDF Files (*.pdf);;All Files (*)");

    if (outputPath.isEmpty()) {
        return;
    }

    setStatus("Merging files...");

    auto stringPaths = fileCollection_->toStringPaths();
    std::string utf8Output = StringConverter::toUtf8(outputPath.toStdWString());

    bool success = pdfProcessor_->mergeFiles(stringPaths, utf8Output,
        [this](size_t current, size_t total, const std::string& fileName) {
            QString message = QString("Processing %1/%2: %3")
                .arg(current).arg(total).arg(QString::fromStdString(fileName));
            setStatus(message);
            QApplication::processEvents();
        });

    if (!success) {
        QMessageBox::critical(this, "Merge Failed",
            "The merge operation failed. Check the selected files and try again.");
        setStatus("Merge failed.");
        return;
    }

    QMessageBox::information(this, "Success", "PDF files merged successfully.");
    setStatus("Merge complete.");
    fileCollection_->clear();
    updateFileList();
}

void MainWindow::onMoveUp() {
    int selected = fileList_->currentRow();
    if (selected > 0 && selected < static_cast<int>(fileCollection_->getCount())) {
        fileCollection_->moveUp(selected);
        updateFileList();
        fileList_->setCurrentRow(selected - 1);
        updateButtonState();
    }
}

void MainWindow::onMoveDown() {
    int selected = fileList_->currentRow();
    if (selected >= 0 && selected < static_cast<int>(fileCollection_->getCount()) - 1) {
        fileCollection_->moveDown(selected);
        updateFileList();
        fileList_->setCurrentRow(selected + 1);
        updateButtonState();
    }
}

void MainWindow::updateFileList() {
    if (fileList_ == nullptr) {
        return;
    }

    fileList_->clear();
    for (const auto& file : fileCollection_->getFiles()) {
        fileList_->addItem(QString::fromStdString(file.getFilename()));
    }
    updateButtonState();
}

void MainWindow::updateButtonState() {
    if (fileList_ == nullptr || clearButton_ == nullptr || mergeButton_ == nullptr ||
        upButton_ == nullptr || downButton_ == nullptr) {
        return;
    }

    const bool hasFiles = !fileCollection_->isEmpty();
    const int selected = fileList_->currentRow();
    const bool hasSelection = selected >= 0;

    clearButton_->setEnabled(hasFiles);
    mergeButton_->setEnabled(hasFiles);
    upButton_->setEnabled(hasSelection && selected > 0);
    downButton_->setEnabled(hasSelection && selected < fileList_->count() - 1);
}

void MainWindow::setStatus(const QString& message) {
    if (statusLabel_ != nullptr) {
        statusLabel_->setText(message);
    }
}
