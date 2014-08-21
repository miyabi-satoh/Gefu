#include "colorsamplemodel.h"
#include"common.h"
#include "preferencedialog.h"
#include "ui_preferencedialog.h"

#include <QAbstractTableModel>
#include <QColorDialog>
#include <QSettings>
#include <QDebug>
#include <QDir>



PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog),
    m_model(),
    m_colorMap()
{
    m_model.setColorMap(&m_colorMap);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setTabText(0, "起動と終了");
    ui->tabWidget->setTabText(1, "色とフォント");
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

    QSettings settings;
    QString strValue;
    QSize size;
    QPoint point;
    QColor color;
    QPalette palette;
    QFont font;
    QRadioButton *radioBtn;

    //>>>>> 起動と終了
    // 終了時の確認ダイアログ
    ui->confirmExit->setChecked(settings.value(IniKey_ConfirmExit, true).toBool());
    // 起動時のサイズ
    strValue = settings.value(IniKey_BootSizeSpec, "sizeLast").toString();
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
    size = settings.value(IniKey_BootSizeAbs, QSize(800, 600)).toSize();
    ui->absoluteWidth->setValue(size.width());
    ui->absoluteHeight->setValue(size.height());
    size = settings.value(IniKey_BootSizeRel, QSize(50, 50)).toSize();
    ui->relativeWidth->setValue(size.width());
    ui->relativeHeight->setValue(size.height());
    // 起動時の位置
    strValue = settings.value(IniKey_BootPosSpec, "posLast").toString();
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
    point = settings.value(IniKey_BootPosAbs, QPoint(0, 0)).toPoint();
    ui->absoluteLeft->setValue(point.x());
    ui->absoluteTop->setValue(point.y());
    point = settings.value(IniKey_BootPosRel, QPoint(0, 0)).toPoint();
    ui->relativeLeft->setValue(point.x());
    ui->relativeTop->setValue(point.y());
    // 起動時の設定削除
    ui->resetOnBoot->setChecked(settings.value(IniKey_ResetOnBoot, false).toBool());

    //>>>>> 色とフォント
    //>>>> アドレスボックス
    palette = QPalette();
    // 背景色
    color = settings.value(IniKey_BoxColorBg, QPalette().base().color()).value<QColor>();
    palette.setColor(QPalette::Base, color);
    // 文字色
    color = settings.value(IniKey_BoxColorFg, QPalette().text().color()).value<QColor>();
    palette.setColor(QPalette::Text, color);
    // フォント
    QFont defaultFont = ui->sampleEdit->font();
    font = settings.value(IniKey_BoxFont, defaultFont).value<QFont>();
    ui->boxFont->setCurrentText(font.family());
    ui->boxFontBold->setChecked(font.bold());
    ui->boxFontSize->setValue(font.pointSize());
    // サンプル表示
    ui->sampleEdit->setPalette(palette);
    ui->sampleEdit->setFont(font);
    //>>>> ファイルビュー
    // 背景色
    color = settings.value(IniKey_ViewColorBgMark, DefaultMarkBgColor).value<QColor>();
    m_colorMap["clrBgMark"] = color;
    color = settings.value(IniKey_ViewColorBgNormal, QPalette().base().color()).value<QColor>();
    m_colorMap["clrBgNormal"] = color;
    // 文字色
    color = settings.value(IniKey_ViewColorFgHidden, DefaultHiddenColor).value<QColor>();
    m_colorMap["clrFgHidden"] = color;
    color = settings.value(IniKey_ViewColorFgMark, DefaultMarkFgColor).value<QColor>();
    m_colorMap["clrFgMark"] = color;
    color = settings.value(IniKey_ViewColorFgNormal, QPalette().text().color()).value<QColor>();
    m_colorMap["clrFgNormal"] = color;
    color = settings.value(IniKey_ViewColorFgReadonly, DefaultReadonlyColor).value<QColor>();
    m_colorMap["clrFgReadonly"] = color;
    color = settings.value(IniKey_ViewColorFgSystem, DefaultSystemColor).value<QColor>();
    m_colorMap["clrFgSystem"] = color;
    // フォント
    defaultFont = ui->sampleTable->font();
    font = settings.value(IniKey_ViewFont, defaultFont).value<QFont>();
    ui->viewFont->setCurrentText(font.family());
    ui->viewFontBold->setChecked(font.bold());
    ui->viewFontSize->setValue(font.pointSize());
    // サンプル表示
    m_model.setFont(font);
    m_model.update();
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
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

    QDialog::accept();
}
