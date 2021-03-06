#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QSqlError;
class MainWindowPrivate;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdd();
    void onDelete();
    void onUpdate();
    void onSelect();
    void onRevert();
    void onShow();

private:
    void setupUI();
    void setupModel();
    void showError(const QSqlError &err);
    MainWindowPrivate *d;
};
#endif // MAINWINDOW_H
