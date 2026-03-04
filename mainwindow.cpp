    #include "mainwindow.h"
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QHeaderView>
    #include <QMessageBox>
    #include <QScrollArea>
    #include <QTimer>  
    #include <QLabel>     // ← Add this line (case-sensitive!)
    // ← REQUIRED for safe auto-load in Release

    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
        , combo(new QComboBox(this))           // ← created in initializer list
        , baseGrid(new QTableWidget(this))     // ← created in initializer list
        , solutionGrid(new QTableWidget(this)) // ← created in initializer list
        , log(new QTextEdit(this))             // ← created in initializer list
        , model()
    {
        setWindowTitle("Globus Medical RTS Pathfinding");
        resize(1650, 980); // wide enough for two full small maps side-by-side

        QWidget* content = new QWidget(this);
        QVBoxLayout* mainLayout = new QVBoxLayout(content);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(12, 12, 12, 12);

        // Top controls
        QHBoxLayout* top = new QHBoxLayout();
        for (const auto& f : mapList)
            combo->addItem(QString::fromStdString(f));

        QPushButton* btn = new QPushButton("Load Map & Find Paths");
        top->addWidget(new QLabel("Map:"));
        top->addWidget(combo);
        top->addWidget(btn);
        mainLayout->addLayout(top);

        // SIDE-BY-SIDE MAPS
        QHBoxLayout* mapsLayout = new QHBoxLayout();
        mapsLayout->setSpacing(25);

        QVBoxLayout* left = new QVBoxLayout();
        left->addWidget(new QLabel("<b>BASE MAP (before solution)</b>"));
        setupGrid(baseGrid, 64);
        left->addWidget(baseGrid);
        mapsLayout->addLayout(left);

        QVBoxLayout* right = new QVBoxLayout();
        right->addWidget(new QLabel("<b>SOLUTION MAP with Paths (S/T/# + colored paths | + = overlap)</b>"));
        setupGrid(solutionGrid, 64);
        right->addWidget(solutionGrid);
        mapsLayout->addLayout(right);

        mainLayout->addLayout(mapsLayout);

        // Log
        log->setReadOnly(true);
        log->setFontFamily("Consolas");
        log->setFontPointSize(9);
        mainLayout->addWidget(new QLabel("<b>Results & Full Path Details:</b>"));
        mainLayout->addWidget(log, 1);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidget(content);
        scrollArea->setWidgetResizable(true);
        setCentralWidget(scrollArea);

        connect(btn, &QPushButton::clicked, this, &MainWindow::loadAndSolve);

        // Auto-load on startup – delayed until Qt is fully ready (fixes Release crash)
        QTimer::singleShot(0, this, &MainWindow::loadAndSolve);
    }

    void MainWindow::setupGrid(QTableWidget* grid, int size)
    {
        grid->setRowCount(size);
        grid->setColumnCount(size);
        grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
        grid->horizontalHeader()->setVisible(false);
        grid->verticalHeader()->setVisible(false);
        grid->setShowGrid(true);

        // ====================== THIS IS THE MAGIC THAT MAKES BOXES SMALL ======================
        const int cellSize = 7; // ← change to 6 (tiny) or 8 (bigger) if you want

        QHeaderView* hHeader = grid->horizontalHeader();
        QHeaderView* vHeader = grid->verticalHeader();

        hHeader->setSectionResizeMode(QHeaderView::Fixed);
        vHeader->setSectionResizeMode(QHeaderView::Fixed);

        hHeader->setMinimumSectionSize(cellSize);
        vHeader->setMinimumSectionSize(cellSize);
        hHeader->setMaximumSectionSize(cellSize); // ← this line was missing before
        vHeader->setMaximumSectionSize(cellSize);

        hHeader->setDefaultSectionSize(cellSize);
        vHeader->setDefaultSectionSize(cellSize);

        // set every single cell size
        for (int i = 0; i < size; ++i) {
            grid->setRowHeight(i, cellSize);
            grid->setColumnWidth(i, cellSize);
        }

        grid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        grid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // fixed size = exactly the grid content
        int totalSize = size * cellSize + 2;
        grid->setFixedSize(totalSize, totalSize);
        grid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    void MainWindow::drawBaseGrid(QTableWidget* target, const BattleMatrix& bm)
    {
        target->clearContents();
        for (uint64_t i = 0; i < bm.getPositionsCount(); ++i) {
            auto [col, row] = bm.convertIndexToColRow(i);
            QTableWidgetItem* item = new QTableWidgetItem();
            QColor color = Qt::lightGray;
            QString text = ".";
            switch (bm.getPosition(i)) {
            case BattleMatrix::Start: color = Qt::green; text = "S"; break;
            case BattleMatrix::Target: color = Qt::red; text = "T"; break;
            case BattleMatrix::Elevated: color = Qt::darkGray; text = "#"; break;
            }
            item->setBackground(color);
            item->setText(text);
            item->setTextAlignment(Qt::AlignCenter);
            target->setItem(static_cast<int>(row), static_cast<int>(col), item);
        }
        // === TASK 2: INTRODUCED NULL-POINTER BUG ===
        //QTableWidgetItem* nullPtr = nullptr;
        //nullPtr->setBackground(Qt::yellow); // ← this will crash
    }

    void MainWindow::drawSolutionGrid(QTableWidget* target, const BattleMatrix& bm, const std::vector<std::vector<uint64_t>>& paths)
    {
        drawBaseGrid(target, bm);
        if (paths.empty()) return;

        std::vector<std::vector<int>> count(bm.getSize(), std::vector<int>(bm.getSize(), 0));
        for (const auto& p : paths)
            for (uint64_t idx : p) {
                auto [c, r] = bm.convertIndexToColRow(idx);
                if (bm.getPosition(idx) != BattleMatrix::Start && bm.getPosition(idx) != BattleMatrix::Target)
                    count[r][c]++;
            }

        QColor colors[5] = { Qt::blue, Qt::darkBlue, Qt::magenta, Qt::cyan, Qt::darkYellow };
        for (size_t p = 0; p < paths.size(); ++p) {
            QColor col = colors[p % 5];
            for (uint64_t idx : paths[p]) {
                auto [c, r] = bm.convertIndexToColRow(idx);
                QTableWidgetItem* item = target->item(static_cast<int>(r), static_cast<int>(c));
                if (!item || item->text() == "S" || item->text() == "T") continue;

                if (count[r][c] > 1) {
                    item->setBackground(Qt::darkRed);
                    item->setText("+");
                }
                else {
                    item->setBackground(col);
                    item->setText(QString::number(p + 1));
                }
            }
        }
    }

    void MainWindow::appendRichLog(const BattleMatrix& bm, const std::vector<std::vector<uint64_t>>& paths, bool success)
    {
        if (success && !paths.empty()) {
            log->append("<font color='green'><b>SUCCESS: " + QString::number(paths.size()) + " collision-free path(s) found</b></font>");
            for (size_t i = 0; i < paths.size(); ++i) {
                log->append("<b>Path " + QString::number(i + 1) + "</b> (" +
                    QString::number(paths[i].size()) + " positions / " +
                    QString::number(paths[i].size() - 1) + " steps)");

                QString coords;
                for (size_t j = 0; j < paths[i].size(); ++j) {
                    auto [col, row] = bm.convertIndexToColRow(paths[i][j]);
                    coords += QString("(%1,%2)").arg(col).arg(row);
                    if (j < paths[i].size() - 1) coords += " → ";
                }
                log->append(coords);
            }
        }
        else {
            log->append("<font color='red'><b>No valid collision-free solution found</b></font>");
        }
    }

    void MainWindow::loadAndSolve()
    {
        std::string jsonPath = combo->currentText().toStdString();
        log->clear();
        log->append("<b>=== Loading " + QString::fromStdString(jsonPath) + " ===</b>");

        auto optBM = model.loadFromJson(jsonPath);
        if (!optBM) {
            QMessageBox::warning(this, "Error", "Failed to load map");
            return;
        }

        BattleMatrix bm = *optBM;
        drawBaseGrid(baseGrid, bm);
        log->append("Base map loaded (" + QString::number(bm.getSize()) + "×" + QString::number(bm.getSize()) + ")");

        BattleMatrix bmOut;
        std::vector<std::vector<uint64_t>> paths;
        bool success = model.moveStartsToTargets(bmOut, paths);

        drawSolutionGrid(solutionGrid, bmOut, paths);
        appendRichLog(bmOut, paths, success);
    }