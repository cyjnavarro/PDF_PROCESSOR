#include "MainWindow.h"
#include "FileModel.h"
#include "PdfProcessor.h"
#include "StringConverter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
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
#include <QPrinter>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QImage>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QSpinBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QPdfView>
#include <QNativeGestureEvent>
#include <QWheelEvent>

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
    splitPdfButton_(nullptr),
      fileCollection_(std::make_unique<FileCollection>()),
      pdfProcessor_(std::make_unique<PdfProcessor>()) {
    setWindowTitle("PDF Workspace");
    setupUI();
    setupConnections();
    setAcceptDrops(true);
    resize(900, 650);
}

MainWindow::~MainWindow() = default;

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    auto* pdfView = qobject_cast<QPdfView*>(watched);
    if (pdfView == nullptr) {
        return QMainWindow::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Wheel) {
        auto* wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->modifiers().testFlag(Qt::ControlModifier)) {
            pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
            const qreal zoomDelta = wheelEvent->angleDelta().y() > 0 ? 0.1 : -0.1;
            pdfView->setZoomFactor(qBound(0.25, pdfView->zoomFactor() + zoomDelta, 4.0));
            return true;
        }
    }

    if (event->type() == QEvent::NativeGesture) {
        auto* gestureEvent = static_cast<QNativeGestureEvent*>(event);
        if (gestureEvent->gestureType() == Qt::ZoomNativeGesture) {
            pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
            pdfView->setZoomFactor(qBound(
                0.25,
                pdfView->zoomFactor() + static_cast<qreal>(gestureEvent->value()),
                4.0));
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

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
            padding: 6px;
            outline: none;
        }
        QListWidget::item {
            background-color: #1b222c;
            border: 1px solid #27313d;
            border-radius: 7px;
            margin: 3px 0;
        }
        QListWidget::item:hover { background-color: #222d3a; }
        QListWidget::item:selected {
            background-color: #243b53;
            border: 1px solid #3b82f6;
        }
        QWidget#fileRow { background-color: transparent; }
        QLabel#fileBadge {
            background-color: #b42318;
            color: #ffffff;
            border-radius: 4px;
            padding: 4px 5px;
            font-size: 9px;
            font-weight: 700;
        }
        QLabel#fileName {
            color: #e5e7eb;
            font-size: 11px;
        }
        QPushButton#removeFileButton {
            background-color: transparent;
            color: #9ca3af;
            border: 1px solid transparent;
            border-radius: 14px;
            padding: 0;
            font-size: 13px;
            font-weight: 700;
        }
        QPushButton#removeFileButton:hover {
            background-color: #3b1f2b;
            color: #fca5a5;
            border: 1px solid #7f1d1d;
        }
        QPushButton#removeFileButton:pressed {
            background-color: #7f1d1d;
            color: #ffffff;
        }
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
    splitPdfButton_ = new QPushButton(homePage_);
    setHomeButtonStyle(splitPdfButton_, "Split PDF", "Create separate PDFs from page ranges");

    toolsLayout->addWidget(mergeHomeButton_, 0, 0);
    toolsLayout->addWidget(imagesToPdfButton_, 0, 1);
    toolsLayout->addWidget(splitPdfButton_, 1, 0, 1, 2);
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
    fileList_->setSpacing(2);
    fileList_->setUniformItemSizes(true);
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
    connect(splitPdfButton_, &QPushButton::clicked, this, &MainWindow::onSplitPdf);
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

void MainWindow::onSplitPdf() {
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Select PDF to split", "", "PDF Files (*.pdf);;All Files (*)");

    if (filePath.isEmpty()) {
        return;
    }

    QPdfDocument document;
    if (document.load(filePath) != QPdfDocument::Error::None || document.pageCount() == 0) {
        QMessageBox::critical(this, "Open Failed", "The selected PDF could not be opened.");
        return;
    }

    QDialog viewerDialog(this);
    viewerDialog.setWindowTitle("View and Split PDF");
    viewerDialog.resize(900, 720);

    auto* dialogLayout = new QVBoxLayout(&viewerDialog);
    auto* titleLabel = new QLabel(
        QString("%1  |  %2 pages").arg(QFileInfo(filePath).fileName()).arg(document.pageCount()),
        &viewerDialog);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    dialogLayout->addWidget(titleLabel);

    auto* pdfView = new QPdfView(&viewerDialog);
    pdfView->setDocument(&document);
    pdfView->setPageMode(QPdfView::PageMode::SinglePage);
    pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
    pdfView->setPageSpacing(12);
    pdfView->installEventFilter(this);
    dialogLayout->addWidget(pdfView, 1);

    auto* zoomLayout = new QHBoxLayout();
    auto* zoomOutButton = new QPushButton("-", &viewerDialog);
    auto* zoomInButton = new QPushButton("+", &viewerDialog);
    auto* fitButton = new QPushButton("Fit", &viewerDialog);
    auto* zoomLabel = new QLabel("Fit to view", &viewerDialog);
    zoomOutButton->setFixedWidth(36);
    zoomInButton->setFixedWidth(36);
    fitButton->setFixedWidth(55);
    zoomLayout->addWidget(zoomOutButton);
    zoomLayout->addWidget(zoomInButton);
    zoomLayout->addWidget(fitButton);
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addStretch();
    dialogLayout->addLayout(zoomLayout);

    auto* navigationLayout = new QHBoxLayout();
    auto* previousButton = new QPushButton("Previous", &viewerDialog);
    auto* nextButton = new QPushButton("Next", &viewerDialog);
    auto* pageSpinBox = new QSpinBox(&viewerDialog);
    pageSpinBox->setRange(1, document.pageCount());
    pageSpinBox->setValue(1);
    pageSpinBox->setFixedWidth(70);
    auto* pageLabel = new QLabel(QString("of %1").arg(document.pageCount()), &viewerDialog);
    navigationLayout->addWidget(previousButton);
    navigationLayout->addWidget(nextButton);
    navigationLayout->addStretch();
    navigationLayout->addWidget(new QLabel("Page", &viewerDialog));
    navigationLayout->addWidget(pageSpinBox);
    navigationLayout->addWidget(pageLabel);
    dialogLayout->addLayout(navigationLayout);

    auto* nameLabel = new QLabel("Output file name:", &viewerDialog);
    dialogLayout->addWidget(nameLabel);
    auto* nameEdit = new QLineEdit(QFileInfo(filePath).completeBaseName(), &viewerDialog);
    nameEdit->setPlaceholderText("Example: project-chapter");
    dialogLayout->addWidget(nameEdit);

    auto* rangeLabel = new QLabel("Split ranges (example: 1-3, 4-6):", &viewerDialog);
    dialogLayout->addWidget(rangeLabel);
    auto* rangeEdit = new QLineEdit("1-" + QString::number(document.pageCount()), &viewerDialog);
    rangeEdit->setPlaceholderText("1-3, 4-6");
    dialogLayout->addWidget(rangeEdit);

    auto* dialogButtons = new QDialogButtonBox(&viewerDialog);
    auto* cancelButton = dialogButtons->addButton(QDialogButtonBox::Cancel);
    auto* splitButton = dialogButtons->addButton("Split PDF", QDialogButtonBox::AcceptRole);
    dialogLayout->addWidget(dialogButtons);

    QObject::connect(previousButton, &QPushButton::clicked, [&]() {
        pageSpinBox->setValue(pageSpinBox->value() - 1);
    });
    QObject::connect(nextButton, &QPushButton::clicked, [&]() {
        pageSpinBox->setValue(pageSpinBox->value() + 1);
    });
    QObject::connect(zoomOutButton, &QPushButton::clicked, [&]() {
        pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
        pdfView->setZoomFactor(qMax(0.25, pdfView->zoomFactor() - 0.15));
        zoomLabel->setText(QString("%1%").arg(qRound(pdfView->zoomFactor() * 100)));
    });
    QObject::connect(zoomInButton, &QPushButton::clicked, [&]() {
        pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
        pdfView->setZoomFactor(qMin(4.0, pdfView->zoomFactor() + 0.15));
        zoomLabel->setText(QString("%1%").arg(qRound(pdfView->zoomFactor() * 100)));
    });
    QObject::connect(fitButton, &QPushButton::clicked, [&]() {
        pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
        zoomLabel->setText("Fit to view");
    });
    QObject::connect(pageSpinBox, &QSpinBox::valueChanged, [&](int page) {
        pdfView->pageNavigator()->jump(page - 1, QPointF(0, 0));
    });
    QObject::connect(pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged,
        [&](int page) {
            pageSpinBox->setValue(page + 1);
        });
    QObject::connect(cancelButton, &QPushButton::clicked, &viewerDialog, &QDialog::reject);

    QString rangeText;
    QString outputBaseName;
    QString outputDirectory;
    QObject::connect(splitButton, &QPushButton::clicked, [&]() {
        const QString candidateName = nameEdit->text().trimmed();
        if (candidateName.isEmpty() || candidateName == "." || candidateName == ".." ||
            candidateName.contains(QRegularExpression(R"([\\/:*?"<>|])"))) {
            QMessageBox::warning(&viewerDialog, "Invalid File Name",
                "Enter a file name without path separators or Windows-invalid characters.");
            return;
        }

        const QString selectedDirectory = QFileDialog::getExistingDirectory(
            &viewerDialog, "Choose output folder for split PDFs");
        if (!selectedDirectory.isEmpty()) {
            rangeText = rangeEdit->text();
            outputBaseName = candidateName;
            outputDirectory = selectedDirectory;
            viewerDialog.accept();
        }
    });

    if (viewerDialog.exec() != QDialog::Accepted || rangeText.trimmed().isEmpty()) {
        return;
    }

    std::vector<std::pair<size_t, size_t>> pageRanges;
    for (const QString& range : rangeText.split(',', Qt::SkipEmptyParts)) {
        const QStringList bounds = range.trimmed().split('-', Qt::SkipEmptyParts);
        bool startOk = false;
        bool endOk = false;
        const int startPage = bounds.value(0).trimmed().toInt(&startOk);
        const int endPage = bounds.size() > 1
            ? bounds.value(1).trimmed().toInt(&endOk)
            : startPage;

        if (!startOk || (bounds.size() > 1 && !endOk) || bounds.size() > 2 ||
            startPage < 1 || endPage < startPage || static_cast<size_t>(endPage) >
                static_cast<size_t>(document.pageCount())) {
            QMessageBox::warning(this, "Invalid Page Range",
                "Use ranges like 1-3, 4-6. Page numbers must be within the PDF.");
            return;
        }

        pageRanges.emplace_back(static_cast<size_t>(startPage - 1), static_cast<size_t>(endPage - 1));
    }

    setStatus("Splitting PDF...");
    const bool success = pdfProcessor_->splitFile(
        StringConverter::toUtf8(filePath.toStdWString()),
        pageRanges,
        StringConverter::toUtf8(outputDirectory.toStdWString()),
        StringConverter::toUtf8(outputBaseName.toStdWString()),
        [this](size_t current, size_t total, const std::string& fileName) {
            setStatus(QString("Creating part %1/%2: %3")
                .arg(current).arg(total).arg(QString::fromStdString(fileName)));
            QApplication::processEvents();
        });

    if (!success) {
        QMessageBox::critical(this, "Split Failed", "The PDF could not be split.");
        setStatus("PDF split failed.");
        return;
    }

    QMessageBox::information(this, "Split Complete",
        QString("Created %1 PDF part(s) in the selected folder.").arg(pageRanges.size()));
    setStatus("PDF split complete.");
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

void MainWindow::onRemoveFile() {
    auto* removeButton = qobject_cast<QPushButton*>(sender());
    if (removeButton == nullptr) {
        return;
    }

    const int index = removeButton->property("fileIndex").toInt();
    if (index < 0 || index >= static_cast<int>(fileCollection_->getCount())) {
        return;
    }

    fileCollection_->remove(static_cast<size_t>(index));
    updateFileList();
    setStatus("File removed from merge list.");
}

void MainWindow::updateFileList() {
    if (fileList_ == nullptr) {
        return;
    }

    fileList_->clear();
    const auto& files = fileCollection_->getFiles();
    for (int index = 0; index < static_cast<int>(files.size()); ++index) {
        auto* item = new QListWidgetItem(fileList_);
        auto* rowWidget = new QWidget(fileList_);
        rowWidget->setObjectName("fileRow");
        auto* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 4, 6, 4);
        rowLayout->setSpacing(10);

        auto* fileBadge = new QLabel("PDF", rowWidget);
        fileBadge->setObjectName("fileBadge");
        fileBadge->setAlignment(Qt::AlignCenter);
        fileBadge->setFixedSize(34, 24);
        auto* fileLabel = new QLabel(QString::fromStdString(files.at(index).getFilename()), rowWidget);
        fileLabel->setObjectName("fileName");
        fileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        fileLabel->setToolTip(QString::fromStdString(files.at(index).getPath()));
        auto* removeButton = new QPushButton("X", rowWidget);
        removeButton->setObjectName("removeFileButton");
        removeButton->setFixedSize(28, 28);
        removeButton->setToolTip("Remove this file");
        removeButton->setProperty("fileIndex", index);

        rowLayout->addWidget(fileBadge);
        rowLayout->addWidget(fileLabel);
        rowLayout->addStretch();
        rowLayout->addWidget(removeButton);
        item->setSizeHint(QSize(0, 44));
        fileList_->setItemWidget(item, rowWidget);

        connect(removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveFile);
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
