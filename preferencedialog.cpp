#include "colorsamplemodel.h"
#include"common.h"
#include "preferencedialog.h"
#include "ui_preferencedialog.h"

#include <QAbstractTableModel>
#include <QColorDialog>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog),
    m_model(),
    m_colorMap()
{
    m_model.setColorMap(&m_colorMap);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->sampleEdit->setText(QDir::homePath());
    ui->sampleTable->setModel(&m_model);
    ui->sampleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->sampleTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->bootSize, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->sizeAbsolute, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->sizeRelative, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->bootPos, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->posAbsolute, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->posRelative, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));

    connect(ui->boxClrBg, SIGNAL(clicked()), this, SLOT(selectBoxColor()));
    connect(ui->boxClrFg, SIGNAL(clicked()), this, SLOT(selectBoxColor()));

    connect(ui->clrBgMark, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrBgNormal, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgHidden, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgMark, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgNormal, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgReadonly, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgSystem, SIGNAL(clicked()), this, SLOT(selectViewColor()));

    connect(ui->boxFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(changeFont()));
    connect(ui->boxFontBold, SIGNAL(clicked()), this, SLOT(changeFont()));
    connect(ui->boxFontSize, SIGNAL(valueChanged(int)), this, SLOT(changeFont()));

    connect(ui->viewFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(changeFont()));
    connect(ui->viewFontBold, SIGNAL(clicked()), this, SLOT(changeFont()));
    connect(ui->viewFontSize, SIGNAL(valueChanged(int)), this, SLOT(changeFont()));

    connect(ui->importAppearance, SIGNAL(clicked()), this, SLOT(importAppearance()));
    connect(ui->exportAppearance, SIGNAL(clicked()), this, SLOT(exportAppearance()));
    connect(ui->termBrowse, SIGNAL(clicked()), this, SLOT(browseApp()));
    connect(ui->editorBrowse, SIGNAL(clicked()), this, SLOT(browseApp()));

    QSettings settings;
    QString strValue;
    QSize size;
    QPoint point;
    QRadioButton *radioBtn;

    //>>>>> 起動と終了
    // 終了時の確認ダイアログ
    ui->confirmExit->setChecked(settings.value(IniKey_ConfirmExit).toBool());
    // 起動時のサイズ
    strValue = settings.value(IniKey_BootSizeSpec).toString();
    if (strValue.isEmpty()) {
        ui->bootSize->setChecked(false);
    }
    else {
        ui->bootSize->setChecked(true);
        radioBtn = findChild<QRadioButton*>(strValue);
        if (radioBtn == NULL) {
            radioBtn = ui->sizeLast;
        }
        radioBtn->setChecked(true);
    }
    size = settings.value(IniKey_BootSizeAbs).toSize();
    ui->absoluteWidth->setValue(size.width());
    ui->absoluteHeight->setValue(size.height());
    size = settings.value(IniKey_BootSizeRel).toSize();
    ui->relativeWidth->setValue(size.width());
    ui->relativeHeight->setValue(size.height());
    // 起動時の位置
    strValue = settings.value(IniKey_BootPosSpec).toString();
    if (strValue.isEmpty()) {
        ui->bootPos->setChecked(false);
    }
    else {
        ui->bootPos->setChecked(true);
        radioBtn = findChild<QRadioButton*>(strValue);
        if (radioBtn == NULL) {
            radioBtn = ui->posLast;
        }
        radioBtn->setChecked(true);
    }
    point = settings.value(IniKey_BootPosAbs).toPoint();
    ui->absoluteLeft->setValue(point.x());
    ui->absoluteTop->setValue(point.y());
    point = settings.value(IniKey_BootPosRel).toPoint();
    ui->relativeLeft->setValue(point.x());
    ui->relativeTop->setValue(point.y());
    // 起動時の設定削除
    ui->resetOnBoot->setChecked(settings.value(IniKey_ResetOnBoot).toBool());

    //>>>>> 色とフォント
    loadAppearance(settings);

    //>>>>> ファイル操作
    // 確認ダイアログの表示
    ui->confirmCopy->setChecked(settings.value(IniKey_ConfirmCopy).toBool());
    ui->confirmDelete->setChecked(settings.value(IniKey_ConfirmDelete).toBool());
    ui->confirmMove->setChecked(settings.value(IniKey_ConfirmMove).toBool());
    ui->confirmRename->setChecked(settings.value(IniKey_ConfirmRename).toBool());
    // 完了ダイアログの自動クローズ
    ui->autoCloseCopy->setChecked(settings.value(IniKey_AutoCloseCopy).toBool());
    ui->autoCloseDelete->setChecked(settings.value(IniKey_AutoCloseDelete).toBool());
    ui->autoCloseMove->setChecked(settings.value(IniKey_AutoCloseMove).toBool());
    ui->autoCloseRename->setChecked(settings.value(IniKey_AutoCloseRename).toBool());
    // 上書き時の既定動作
    strValue = settings.value(IniKey_DefaultOnCopy).toString();
    if (strValue.isEmpty()) {
        strValue = "owDefIfNew";
    }
    radioBtn = findChild<QRadioButton*>(strValue);
    if (radioBtn == NULL) {
        radioBtn = ui->owDefIfNew;
    }
    radioBtn->setChecked(true);
    ui->moveAfterCreate->setChecked(settings.value(IniKey_MoveAfterCreateFolder).toBool());
    ui->openAfterCreate->setChecked(settings.value(IniKey_OpenAfterCreateFile).toBool());

    //>>>>> パス設定
    // エディタ
    ui->editorOpt->setText(settings.value(IniKey_EditorOption).toString());
    ui->editorPath->setText(settings.value(IniKey_EditorPath).toString());
    // ターミナル
    ui->termOpt->setText(settings.value(IniKey_TerminalOption).toString());
    ui->termPath->setText(settings.value(IniKey_TerminalPath).toString());
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::saveAppearance(QSettings &settings)
{
    QFont font = ui->sampleEdit->font();
    QPalette palette = ui->sampleEdit->palette();
    settings.setValue(IniKey_BoxColorBg, palette.base().color());
    settings.setValue(IniKey_BoxColorFg, palette.text().color());
    settings.setValue(IniKey_BoxFont, font);

    settings.setValue(IniKey_ViewColorBgMark, m_colorMap["clrBgMark"]);
    settings.setValue(IniKey_ViewColorBgNormal, m_colorMap["clrBgNormal"]);
    settings.setValue(IniKey_ViewColorFgHidden, m_colorMap["clrFgHidden"]);
    settings.setValue(IniKey_ViewColorFgMark, m_colorMap["clrFgMark"]);
    settings.setValue(IniKey_ViewColorFgNormal, m_colorMap["clrFgNormal"]);
    settings.setValue(IniKey_ViewColorFgReadonly, m_colorMap["clrFgReadonly"]);
    settings.setValue(IniKey_ViewColorFgSystem, m_colorMap["clrFgSystem"]);
    settings.setValue(IniKey_ViewFont, m_model.font());
}

void PreferenceDialog::loadAppearance(QSettings &settings)
{
    QPalette palette;
    QColor color;
    QFont font;

    //>>>> アドレスボックス
    palette = QPalette();
    // 背景色
    color = settings.value(IniKey_BoxColorBg).value<QColor>();
    palette.setColor(QPalette::Base, color);
    // 文字色
    color = settings.value(IniKey_BoxColorFg).value<QColor>();
    palette.setColor(QPalette::Text, color);
    // フォント
    font = settings.value(IniKey_BoxFont).value<QFont>();
    ui->boxFont->setCurrentText(font.family());
    ui->boxFontBold->setChecked(font.bold());
    ui->boxFontSize->setValue(font.pointSize());
    // サンプル表示
    ui->sampleEdit->setPalette(palette);
    ui->sampleEdit->setFont(font);
    //>>>> ファイルビュー
    // 背景色
    color = settings.value(IniKey_ViewColorBgMark).value<QColor>();
    m_colorMap["clrBgMark"] = color;
    color = settings.value(IniKey_ViewColorBgNormal).value<QColor>();
    m_colorMap["clrBgNormal"] = color;
    // 文字色
    color = settings.value(IniKey_ViewColorFgHidden).value<QColor>();
    m_colorMap["clrFgHidden"] = color;
    color = settings.value(IniKey_ViewColorFgMark).value<QColor>();
    m_colorMap["clrFgMark"] = color;
    color = settings.value(IniKey_ViewColorFgNormal).value<QColor>();
    m_colorMap["clrFgNormal"] = color;
    color = settings.value(IniKey_ViewColorFgReadonly).value<QColor>();
    m_colorMap["clrFgReadonly"] = color;
    color = settings.value(IniKey_ViewColorFgSystem).value<QColor>();
    m_colorMap["clrFgSystem"] = color;
    // フォント
    font = settings.value(IniKey_ViewFont).value<QFont>();
    ui->viewFont->setCurrentText(font.family());
    ui->viewFontBold->setChecked(font.bold());
    ui->viewFontSize->setValue(font.pointSize());
    // サンプル表示
    m_model.setFont(font);
    m_model.update();

}

void PreferenceDialog::changeFont()
{
    QFont font;

    if (sender() == ui->boxFont ||
        sender() == ui->boxFontBold ||
        sender() == ui->boxFontSize)
    {
        font.setBold(ui->boxFontBold->isChecked());
        font.setPointSize(ui->boxFontSize->value());
        font.setFamily(ui->boxFont->currentText());
        ui->sampleEdit->setFont(font);
    }
    else {
        font.setBold(ui->viewFontBold->isChecked());
        font.setPointSize(ui->viewFontSize->value());
        font.setFamily(ui->viewFont->currentText());
        m_model.setFont(font);
        m_model.update();
    }
}

void PreferenceDialog::setControlsEnabled(bool enabled)
{
    if (sender() == ui->bootSize) {
        ui->sizeAbsolute->setEnabled(enabled);
        ui->sizeLast->setEnabled(enabled);
        ui->sizeRelative->setEnabled(enabled);
        if (enabled) {
            emit ui->sizeAbsolute->toggled(ui->sizeAbsolute->isChecked());
            emit ui->sizeRelative->toggled(ui->sizeRelative->isChecked());
        }
        else {
            emit ui->sizeAbsolute->toggled(false);
            emit ui->sizeRelative->toggled(false);
        }
    }
    else if (sender() == ui->sizeAbsolute) {
        ui->absoluteHeight->setEnabled(enabled);
        ui->absoluteWidth->setEnabled(enabled);
    }
    else if (sender() == ui->sizeRelative) {
        ui->relativeHeight->setEnabled(enabled);
        ui->relativeWidth->setEnabled(enabled);
    }
    else if (sender() == ui->bootPos) {
        ui->posAbsolute->setEnabled(enabled);
        ui->posCenter->setEnabled(enabled);
        ui->posLast->setEnabled(enabled);
        ui->posRelative->setEnabled(enabled);
        if (enabled) {
            emit ui->posAbsolute->toggled(ui->posAbsolute->isChecked());
            emit ui->posRelative->toggled(ui->posRelative->isChecked());
        }
        else {
            emit ui->posAbsolute->toggled(false);
            emit ui->posRelative->toggled(false);
        }
    }
    else if (sender() == ui->posAbsolute) {
        ui->absoluteLeft->setEnabled(enabled);
        ui->absoluteTop->setEnabled(enabled);
    }
    else if (sender() == ui->posRelative) {
        ui->relativeLeft->setEnabled(enabled);
        ui->relativeTop->setEnabled(enabled);
    }
}

void PreferenceDialog::selectBoxColor()
{
    QColor color;
    QPalette palette = ui->sampleEdit->palette();
    if (sender() == ui->boxClrBg) {
        color = palette.background().color();
    }
    else if (sender() == ui->boxClrFg) {
        color = palette.text().color();
    }

    color = QColorDialog::getColor(color, this, tr("色選択"));
    if (!color.isValid()) {
        return;
    }

    if (sender() == ui->boxClrBg) {
        palette.setColor(QPalette::Base, color);
        ui->sampleEdit->setPalette(palette);
    }
    else if (sender() == ui->boxClrFg) {
        palette.setColor(QPalette::Text, color);
        ui->sampleEdit->setPalette(palette);
    }
}

void PreferenceDialog::selectViewColor()
{
    const QString objName = sender()->objectName();
    QColor color = m_colorMap[objName];

    color = QColorDialog::getColor(color, this, tr("色選択"));
    if (!color.isValid()) {
        return;
    }

    m_colorMap[objName] = color;
    m_model.update();
}

void PreferenceDialog::browseApp()
{
    QStringList list = QStandardPaths::standardLocations(
                QStandardPaths::ApplicationsLocation);
#ifdef Q_OS_WIN
    QString path = QFileDialog::getOpenFileName(
                this, tr("アプリケーションを選択"), list.at(0),
                tr("実行ファイル (*.exe *.com *.bat *.pif);;すべてのファイル (*)"));
#elif defined(Q_OS_MAC)
    QString path = QFileDialog::getOpenFileName(
                this, tr("アプリケーションを選択"), list.at(0),
                tr("実行ファイル (*.app);;すべてのファイル (*)"));
#else
    QString path = QFileDialog::getOpenFileName(
                this, tr("アプリケーションを選択"), list.at(0),
                tr("すべてのファイル (*)"));
#endif
    if (!path.isEmpty()) {
        if (sender() == ui->editorBrowse) {
            ui->editorPath->setText(path);
        }
        else if (sender() == ui->termBrowse) {
            ui->termPath->setText(path);
        }
    }
}

void PreferenceDialog::importAppearance()
{
    QStringList list = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(
                this, tr("ファイルを選択"), list.at(0),
                tr("設定ファイル (*.ini);;すべてのファイル (*)"));
    if (path.isEmpty()) {
        return;
    }

    QSettings settings(path, QSettings::IniFormat);
    loadAppearance(settings);
}

void PreferenceDialog::exportAppearance()
{
    QStringList list = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getSaveFileName(
                this, tr("ファイルを選択"), list.at(0) + "/gefu_appearance.ini",
                tr("設定ファイル (*.ini);;すべてのファイル (*)"));
    if (path.isEmpty()) {
        return;
    }

    QSettings settings(path, QSettings::IniFormat);
    saveAppearance(settings);
}

void PreferenceDialog::accept()
{
    QSettings settings;
    QAbstractButton *selected;

    //>>>>> 起動と終了
    // 終了時の確認ダイアログ
    settings.setValue(IniKey_ConfirmExit, ui->confirmExit->isChecked());
    // 起動時のサイズ
    if (!ui->bootSize->isChecked()) {
        settings.setValue(IniKey_BootSizeSpec, "");
    }
    else {
        selected = ui->sizeOptions->checkedButton();
        settings.setValue(IniKey_BootSizeSpec, selected->objectName());
        QSize size;
        // 絶対指定
        size = QSize(ui->absoluteWidth->value(), ui->absoluteHeight->value());
        settings.setValue(IniKey_BootSizeAbs, size);
        // 相対指定
        size = QSize(ui->relativeWidth->value(), ui->relativeHeight->value());
        settings.setValue(IniKey_BootSizeRel, size);
    }
    // 起動時の位置
    if (!ui->bootPos->isChecked()) {
        settings.setValue(IniKey_BootPosSpec, "");
    }
    else {
        selected = ui->posOptions->checkedButton();
        settings.setValue(IniKey_BootPosSpec, selected->objectName());
        // 絶対指定
        QPoint pos;
        pos = QPoint(ui->absoluteLeft->value(), ui->absoluteTop->value());
        settings.setValue(IniKey_BootPosAbs, pos);
        // 相対指定
        pos = QPoint(ui->relativeLeft->value(), ui->relativeTop->value());
        settings.setValue(IniKey_BootPosRel, pos);
    }
    // 起動時の設定削除
    settings.setValue(IniKey_ResetOnBoot, ui->resetOnBoot->isChecked());

    //>>>>> 色とフォント
    saveAppearance(settings);

    //>>>>> ファイル操作
    settings.setValue(IniKey_ConfirmCopy, ui->confirmCopy->isChecked());
    settings.setValue(IniKey_ConfirmDelete, ui->confirmDelete->isChecked());
    settings.setValue(IniKey_ConfirmMove, ui->confirmMove->isChecked());
    settings.setValue(IniKey_ConfirmRename, ui->confirmRename->isChecked());

    settings.setValue(IniKey_AutoCloseCopy, ui->autoCloseCopy->isChecked());
    settings.setValue(IniKey_AutoCloseDelete, ui->autoCloseDelete->isChecked());
    settings.setValue(IniKey_AutoCloseMove, ui->autoCloseMove->isChecked());
    settings.setValue(IniKey_AutoCloseRename, ui->autoCloseRename->isChecked());

    selected = ui->overwriteOptions->checkedButton();
    settings.setValue(IniKey_DefaultOnCopy, selected->objectName());

    settings.setValue(IniKey_MoveAfterCreateFolder, ui->moveAfterCreate->isChecked());
    settings.setValue(IniKey_OpenAfterCreateFile, ui->openAfterCreate->isChecked());

    //>>>>> パス設定
    settings.setValue(IniKey_EditorOption, ui->editorOpt->text().trimmed());
    settings.setValue(IniKey_EditorPath, ui->editorPath->text().trimmed());

    settings.setValue(IniKey_TerminalOption, ui->termOpt->text().trimmed());
    settings.setValue(IniKey_TerminalPath, ui->termPath->text().trimmed());

    QDialog::accept();
}
