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
#include <QFontDialog>

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog),
    m_model(),
    m_colorMap()
{
    m_model.setColorMap(&m_colorMap);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    // アドレスボックスの外観サンプル
    ui->sampleEdit->setText(QDir::homePath());
    // ファイルビューの外観サンプル
    ui->sampleTable->setModel(&m_model);

    QHeaderView *header;
    header = ui->sampleTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header = ui->sampleTable->verticalHeader();
    header->setDefaultSectionSize(header->defaultSectionSize() * 0.75);

    // シグナル＆スロット
    connect(ui->bootSize, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->sizeAbsolute, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->sizeRelative, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->bootPos, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->posAbsolute, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->posRelative, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));

    connect(ui->boxClrBg, SIGNAL(clicked()), this, SLOT(selectBoxColor()));
    connect(ui->boxClrFg, SIGNAL(clicked()), this, SLOT(selectBoxColor()));
    connect(ui->chooseBoxFont, SIGNAL(clicked()), this, SLOT(chooseFont()));

    connect(ui->clrBgMark, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrBgNormal, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgHidden, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgMark, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgNormal, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgReadonly, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->clrFgSystem, SIGNAL(clicked()), this, SLOT(selectViewColor()));
    connect(ui->chooseViewFont, SIGNAL(clicked()), this, SLOT(chooseFont()));

    connect(ui->importAppearance, SIGNAL(clicked()), this, SLOT(importAppearance()));
    connect(ui->exportAppearance, SIGNAL(clicked()), this, SLOT(exportAppearance()));

    connect(ui->browseArchiver, SIGNAL(clicked()), this, SLOT(browseApp()));
    connect(ui->browseEditor, SIGNAL(clicked()), this, SLOT(browseApp()));
    connect(ui->browseTerminal, SIGNAL(clicked()), this, SLOT(browseApp()));

    connect(ui->chooseViewerFont, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(ui->viewerClrBg, SIGNAL(clicked()), this, SLOT(selectViewerColor()));
    connect(ui->viewerClrFg, SIGNAL(clicked()), this, SLOT(selectViewerColor()));
    connect(ui->viewerInherit, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->enableViewerIgnoreExt, SIGNAL(toggled(bool)), this, SLOT(setControlsEnabled(bool)));
    connect(ui->defaultIgnoreExt, SIGNAL(clicked()), this, SLOT(setIgnoreExtDefault()));

    // 現在の設定で各コントロールを初期化する
    QSettings settings;
    QString strValue;
    QSize size;
    QPoint point;
    QRadioButton *radioBtn;

    //>>>>> 起動と終了
    // 起動時のサイズ
    ui->bootSize->setChecked(true);
    strValue = settings.value(IniKey_BootSizeSpec).toString();
    radioBtn = findChild<QRadioButton*>(strValue);
    if (radioBtn == NULL) {
        radioBtn = ui->sizeRelative;
    }
    radioBtn->setChecked(true);
    if (strValue.isEmpty()) {
        ui->bootSize->setChecked(false);
    }
    size = settings.value(IniKey_BootSizeAbs).toSize();
    ui->absoluteWidth->setValue(size.width());
    ui->absoluteHeight->setValue(size.height());
    size = settings.value(IniKey_BootSizeRel).toSize();
    ui->relativeWidth->setValue(size.width());
    ui->relativeHeight->setValue(size.height());
    // 起動時の位置
    ui->bootPos->setChecked(true);
    strValue = settings.value(IniKey_BootPosSpec).toString();
    radioBtn = findChild<QRadioButton*>(strValue);
    if (radioBtn == NULL) {
        radioBtn = ui->posCenter;
    }
    radioBtn->setChecked(true);
    if (strValue.isEmpty()) {
        ui->bootPos->setChecked(false);
    }
    point = settings.value(IniKey_BootPosAbs).toPoint();
    ui->absoluteLeft->setValue(point.x());
    ui->absoluteTop->setValue(point.y());
    point = settings.value(IniKey_BootPosRel).toPoint();
    ui->relativeLeft->setValue(point.x());
    ui->relativeTop->setValue(point.y());
    // 起動時の設定削除
    ui->resetOnBoot->setChecked(settings.value(IniKey_ResetOnBoot).toBool());
    // アップデートの確認
    ui->checkUpdates->setChecked(settings.value(IniKey_CheckUpdates).toBool());
    // 終了時の確認ダイアログ
    ui->confirmExit->setChecked(settings.value(IniKey_ConfirmExit).toBool());

    //>>>>> 色とフォント、テキストビューア
    loadAppearance(settings, false);
    ui->dark->setValue(settings.value(IniKey_Darkness).toInt());

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
        radioBtn = ui->rbOverWriteIfNew;
    }
    radioBtn->setChecked(true);
    ui->moveAfterCreate->setChecked(settings.value(IniKey_MoveAfterCreateFolder).toBool());
    ui->openAfterCreate->setChecked(settings.value(IniKey_OpenAfterCreateFile).toBool());

    //>>>>> パス設定
    // エディタ
    ui->editorPath->setText(settings.value(IniKey_PathEditor).toString());
    // ターミナル
    ui->terminalPath->setText(settings.value(IniKey_PathTerminal).toString());
    // アーカイバ
    ui->archiverPath->setText(settings.value(IniKey_PathArchiver).toString());

    //>>>>> テキストビューア
    ui->enableViewerIgnoreExt->setChecked(true);
    ui->enableViewerIgnoreExt->setChecked(!settings.value(IniKey_ViewerForceOpen).toBool());
    ui->viewerIgnoreExt->setPlainText(settings.value(IniKey_ViewerIgnoreExt).toString());
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

void PreferenceDialog::loadAppearance(QSettings &settings, bool import)
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
    ui->boxFont->setText(tr("%1, %2pt").arg(font.family()).arg(font.pointSize()));
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
    ui->viewFont->setText(tr("%1, %2pt").arg(font.family()).arg(font.pointSize()));
    // サンプル表示
    QHeaderView *header = ui->sampleTable->verticalHeader();
    header->setDefaultSectionSize(QFontMetrics(font).height() * 1.5);
    ui->sampleTable->setMinimumHeight(header->sectionSize(0) * 5);
    ui->sampleTable->setMaximumHeight(ui->sampleTable->minimumHeight() + 2);
    m_model.setFont(font);
    m_model.update();

    //>>>> テキストビューア
    // 文字色と背景色
    if (settings.value(IniKey_ViewerInherit).toBool()) {
        ui->viewerInherit->setChecked(true);
        color = settings.value(IniKey_ViewColorBgNormal).value<QColor>();
        palette.setColor(QPalette::Base, color);
        color = settings.value(IniKey_ViewColorFgNormal).value<QColor>();
        palette.setColor(QPalette::Text, color);
    }
    else if (!import){
        ui->viewerInherit->setChecked(false);
        color = settings.value(IniKey_ViewerColorBg).value<QColor>();
        palette.setColor(QPalette::Base, color);
        color = settings.value(IniKey_ViewerColorFg).value<QColor>();
        palette.setColor(QPalette::Text, color);
    }
    ui->viewerSample->setPalette(palette);
    // フォント
    if (!import) {
        font = settings.value(IniKey_ViewerFont).value<QFont>();
        ui->viewerFont->setText(tr("%1, %2pt").arg(font.family()).arg(font.pointSize()));
        ui->viewerSample->setFont(font);
    }
}

void PreferenceDialog::chooseFont()
{
    bool ok;
    QFont font;
    QLabel *label = NULL;

    if (sender() == ui->chooseViewerFont) {
        font = ui->viewerSample->font();
    }
    else if (sender() == ui->chooseBoxFont) {
        font = ui->sampleEdit->font();
    }
    else if (sender() == ui->chooseViewFont) {
        font = m_model.font();
    }

    font = QFontDialog::getFont(&ok, font, this);

    if (sender() == ui->chooseViewerFont) {
        ui->viewerSample->setFont(font);
        label = ui->viewerFont;
    }
    else if (sender() == ui->chooseBoxFont) {
        ui->sampleEdit->setFont(font);
        label = ui->boxFont;
    }
    else if (sender() == ui->chooseViewFont) {
        QHeaderView *header = ui->sampleTable->verticalHeader();
        header->setDefaultSectionSize(QFontMetrics(font).height() * 1.5);
        ui->sampleTable->setMinimumHeight(header->sectionSize(0) * 5);
        ui->sampleTable->setMaximumHeight(ui->sampleTable->minimumHeight() + 2);
        m_model.setFont(font);
        m_model.update();
        label = ui->viewFont;
    }
    label->setText(tr("%1, %2pt").arg(font.family()).arg(font.pointSize()));

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
    else if (sender() == ui->viewerInherit) {
        ui->viewerClrBg->setEnabled(!enabled);
        ui->viewerClrFg->setEnabled(!enabled);
        // サンプル表示も更新
        QPalette pal = ui->viewerSample->palette();
        if (enabled) {
            pal.setColor(QPalette::Base, m_colorMap["clrBgNormal"]);
            pal.setColor(QPalette::Text, m_colorMap["clrFgNormal"]);
        }
        else {
            QSettings settings;
            pal.setColor(QPalette::Base, settings.value(IniKey_ViewerColorBg).value<QColor>());
            pal.setColor(QPalette::Text, settings.value(IniKey_ViewerColorFg).value<QColor>());
        }
        ui->viewerSample->setPalette(pal);
    }
    else if (sender() == ui->enableViewerIgnoreExt) {
        ui->viewerIgnoreExt->setEnabled(enabled);
    }
}

void PreferenceDialog::setIgnoreExtDefault()
{
    ui->viewerIgnoreExt->setPlainText(ViewerIgnoreExt());
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
    }
    else if (sender() == ui->boxClrFg) {
        palette.setColor(QPalette::Text, color);
    }
    ui->sampleEdit->setPalette(palette);
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

void PreferenceDialog::selectViewerColor()
{
    QColor color;
    QPalette palette = ui->viewerSample->palette();
    if (sender() == ui->viewerClrBg) {
        color = palette.background().color();
    }
    else if (sender() == ui->viewerClrFg) {
        color = palette.text().color();
    }

    color = QColorDialog::getColor(color, this, tr("色選択"));
    if (!color.isValid()) {
        return;
    }

    if (sender() == ui->viewerClrBg) {
        palette.setColor(QPalette::Base, color);
    }
    else if (sender() == ui->viewerClrFg) {
        palette.setColor(QPalette::Text, color);
    }
    ui->viewerSample->setPalette(palette);
}

void PreferenceDialog::browseApp()
{
    QStringList list = QStandardPaths::standardLocations(
                QStandardPaths::ApplicationsLocation);
#ifdef Q_OS_WIN
    QString path = QFileDialog::getOpenFileName(
                this, tr("アプリケーションを選択"), getenv("PROGRAMFILES"),
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
        if (path.indexOf(" ") != -1) {
            path = QQ(path);
        }

        if (sender() == ui->browseEditor) {
            ui->editorPath->setText(path);
        }
        else if (sender() == ui->browseTerminal) {
            ui->terminalPath->setText(path);
        }
        else if (sender() == ui->browseArchiver) {
            ui->archiverPath->setText(path);
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
    loadAppearance(settings, true);
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
    // 終了時の確認ダイアログ
    settings.setValue(IniKey_ConfirmExit, ui->confirmExit->isChecked());
    // アップデートのチェック
    settings.setValue(IniKey_CheckUpdates, ui->checkUpdates->isChecked());

    //>>>>> 色とフォント
    saveAppearance(settings);
    settings.setValue(IniKey_Darkness, ui->dark->value());

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
    settings.setValue(IniKey_PathEditor, ui->editorPath->text().trimmed());
    settings.setValue(IniKey_PathTerminal, ui->terminalPath->text().trimmed());
    settings.setValue(IniKey_PathArchiver, ui->archiverPath->text().trimmed());

    //>>>>> テキストビューア
    settings.setValue(IniKey_ViewerFont, ui->viewerSample->font());
    settings.setValue(IniKey_ViewerColorBg, ui->viewerSample->palette().base().color());
    settings.setValue(IniKey_ViewerColorFg, ui->viewerSample->palette().text().color());
    settings.setValue(IniKey_ViewerInherit, ui->viewerInherit->isChecked());
    settings.setValue(IniKey_ViewerForceOpen, !ui->enableViewerIgnoreExt->isChecked());
    QStringList list = ui->viewerIgnoreExt->toPlainText().split(",", QString::SkipEmptyParts);
    QStringList::iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        *it = it->trimmed();
    }
    settings.setValue(IniKey_ViewerIgnoreExt, list.join(","));

    QDialog::accept();
}
