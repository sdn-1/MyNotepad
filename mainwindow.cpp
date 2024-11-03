#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <AboutDialog.h>
#include <SearchDialog.h>
#include <ReplaceDialog.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QColorDialog>
#include <QFontDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    textChanged = false;
    on_actionNew_triggered();

    //ui->statusbar;
    statusLabel.setMaximumWidth(150);
    statusLabel.setText(" length: " + QString::number(0) + "   lines:  " + QString::number(1));
    ui->statusbar->addPermanentWidget(&statusLabel);

    statusCursorLabel.setMaximumWidth(150);
    statusCursorLabel.setText(" length: " + QString::number(0) + "   lines:  " + QString::number(1));
    ui->statusbar->addPermanentWidget(&statusCursorLabel);

    QLabel *author = new QLabel(ui->statusbar);
    author->setText("  沈丹娜");
    ui->statusbar->addPermanentWidget(author);

    ui->actionCopy->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionPaste->setEnabled(false);

    QPlainTextEdit::LineWrapMode mode = ui->textEdit->lineWrapMode();

    if ( mode == QTextEdit::NoWrap ) {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        ui->actionLineWrap->setCheckable(false);
    } else {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        ui->actionLineWrap->setCheckable(true);
    }

    ui->actionShowStatusBar->setChecked(true);
    ui->actionShowToolBar->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//关于对话框
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg;
    dlg.exec();
}

//查找对话框
void MainWindow::on_actionFind_triggered()
{
    SearchDialog dlg;
    dlg.exec();
}

//替换对话框
void MainWindow::on_actionReplace_triggered()
{
    ReplaceDialog dlg;
    dlg.exec();
}

//新建
void MainWindow::on_actionNew_triggered()
{
    if (!userEditConfirmed())
        return;
    filePath = "";
    ui->textEdit->clear();
    this->setWindowTitle(tr("新建文本文件 - 编辑器"));
    textChanged = false;
}

//打开文件
void MainWindow::on_actionOpen_triggered()
{
    if (!userEditConfirmed())
        return;

    QString filename = QFileDialog::getOpenFileName(this, "打开文件", ".", tr("Text files (*.txt) ;; All (*.*)"));
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "..", "打开文件失败");
        return;
    }

    filePath = filename;

    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->insertPlainText(text);
    file.close();

    this->setWindowTitle(QFileInfo(filename).absoluteFilePath());

    textChanged = false;
}

//保存
// void MainWindow::on_actionSave_triggered()
// {

//     QFile file(filePath);
//     if (!file.open(QFile::ReadOnly | QFile::Text)) {
//         QMessageBox::warning(this, "..", "打开文件失败");

//         QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".", tr("Text files (*.txt) "));

//         QFile file(filename);
//         if (!file.open(QFile::WriteOnly | QFile::Text)) {
//             QMessageBox::warning(this, "..", "保存文件失败");
//             return;
//         }
//         filePath = filename;
//     }

//     QTextStream out(&file);
//     QString text = ui->textEdit->toPlainText();
//     out << text;
//     file.flush();
//     file.close();

//     this->setWindowTitle(QFileInfo(filePath).absoluteFilePath());

// }
void MainWindow::on_actionSave_triggered()
{
    QString text = ui->textEdit->toPlainText();
    bool isNewFile = false; // 标记是否为新文件

    // 检查filePath是否有效且文件可读取，文件不存在时进入
    QFile testFile(filePath);
    if (!testFile.exists() || !testFile.open(QFile::ReadOnly | QFile::Text)) {
        isNewFile = true; // 标记为新文件

        QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".", tr("Text files (*.txt);;All Files (*)"));
        if (filename.isEmpty()) {
            // 用户取消了保存操作
            return;
        }

        filePath = filename;
    }

    // 尝试以写模式打开文件
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) { // 使用QFile::Truncate来确保文件被清空
        QMessageBox::warning(this, "保存文件失败", "无法保存文件到指定路径");
        return;
    }

    QTextStream out(&file);
    out << text;
    file.flush();
    file.close();

    // 更新窗口标题
    this->setWindowTitle(QFileInfo(filePath).fileName() + " - 未命名" + (isNewFile ? " (新文件)" : ""));
    // 或者简单地使用绝对路径，不包含"(新文件)"标记
    // this->setWindowTitle(QFileInfo(filePath).absoluteFilePath());

    textChanged = false;
}

//另存
void MainWindow::on_actionSaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".", tr("Text files (*.txt) "));

    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "..", "保存文件失败");
        return;
    }

    filePath = filename;
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();

    this->setWindowTitle(QFileInfo(filePath).absoluteFilePath());
}


void MainWindow::on_textEdit_textChanged()
{
    if (!textChanged) {
        this->setWindowTitle("*" + this->windowTitle());
        textChanged = true;
    }
}

bool MainWindow::userEditConfirmed()
{
    if (textChanged) {
        QString path = (filePath != "") ? filePath : "无标题.txt";

        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Question);
        msg.setWindowTitle("...");
        msg.setWindowFlag(Qt::Drawer);
        msg.setText(QString("是否将更改保存到\n") + "\"" + path + "\"  ? ");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        int r = msg.exec();
        switch (r) {
        case QMessageBox::Yes:
            on_actionSave_triggered();
            break;
        case QMessageBox::No:
            textChanged = false;
            break;
        case QMessageBox::Cancel:
            return false;
        }
    }

    return true;
}

//redo，undo，cut，copy，paste
void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}

void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
    ui->actionPaste->setEnabled(true);
}

void MainWindow::on_actionCopy_triggered()
{
    ui->textEdit->copy();
    ui->actionPaste->setEnabled(true);
}

void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

//设置初始化为不能选中
void MainWindow::on_textEdit_copyAvailable(bool b)
{
    ui->actionCopy->setEnabled(b);
    ui->actionCut->setEnabled(b);
}


void MainWindow::on_textEdit_redoAvailable(bool b)
{
    ui->actionRedo->setEnabled(b);
}


void MainWindow::on_textEdit_undoAvailable(bool b)
{
    ui->actionUndo->setEnabled(b);
}

//字体
void MainWindow::on_actionFont_triggered()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, this);

    if (ok)
        ui->textEdit->setFont(font);
}

//字体颜色
void MainWindow::on_actionFontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
    if (color.isValid()) {
        ui->textEdit->setStyleSheet(QString("QPlainTextEdit {color: %1}").arg(color.name()));
    }
}


void MainWindow::on_actionBackgroundColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
    if (color.isValid()) {
        ui->textEdit->setStyleSheet(QString("QPlainTextEdit {backgroundcolor: %1}").arg(color.name()));
    }
}


void MainWindow::on_actionFontBackgroundColor_triggered()
{

}


void MainWindow::on_actionLineWrap_triggered()
{
    QPlainTextEdit::LineWrapMode mode = ui->textEdit->lineWrapMode();

    if ( mode == QTextEdit::NoWrap ) {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        ui->actionLineWrap->setCheckable(true);
    } else {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        ui->actionLineWrap->setCheckable(false);
    }
}

//工具栏，状态栏
void MainWindow::on_actionShowToolBar_triggered()
{
    bool visible = ui->toolBar->isVisible();
    ui->toolBar->setVisible(!visible);
    ui->actionShowToolBar->setChecked(!visible);
}


void MainWindow::on_actionShowStatusBar_triggered()
{
    bool visible = ui->statusbar->isVisible();
    ui->statusbar->setVisible(!visible);
    ui->actionShowStatusBar->setChecked(!visible);
}

//全选
void MainWindow::on_actionSelectAll_triggered()
{
    ui->textEdit->selectAll();
}

//退出
void MainWindow::on_actionExit_triggered()
{
    //exit(0);
    if (userEditConfirmed())
        exit(0);
}

