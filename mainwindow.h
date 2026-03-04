#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model.h"              // Assuming this defines BattleMatrix and Model

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <vector>
#include <string>
#include <QLabel>     // ← Add this line (case-sensitive!)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;   // good to have virtual destructor

private slots:
    void loadAndSolve();

private:
    void setupGrid(QTableWidget* grid, int size);
    void drawBaseGrid(QTableWidget* target, const BattleMatrix& bm);
    void drawSolutionGrid(QTableWidget* target, const BattleMatrix& bm,
        const std::vector<std::vector<uint64_t>>& paths);
    void appendRichLog(const BattleMatrix& bm,
        const std::vector<std::vector<uint64_t>>& paths,
        bool success);

    // Widgets – initialized to nullptr for safety
    QTableWidget* baseGrid = nullptr;
    QTableWidget* solutionGrid = nullptr;
    QComboBox* combo = nullptr;
    QTextEdit* log = nullptr;

    // Model instance
    Model model;

    // Available map files
    std::vector<std::string> mapList = {
        "map.json",
        "map2.json",
        "map3.json",
        "map_undefined.json"
    };
};

#endif // MAINWINDOW_H