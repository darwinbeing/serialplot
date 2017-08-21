/*
  Copyright © 2017 Hasan Yavuz Özderya

  This file is part of serialplot.

  serialplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  serialplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with serialplot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include "utils.h"
#include "setting_defines.h"

#include "asciireadersettings.h"
#include "ui_asciireadersettings.h"

AsciiReaderSettings::AsciiReaderSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AsciiReaderSettings)
{
    ui->setupUi(this);

    auto validator = new QRegularExpressionValidator(QRegularExpression("[^\\d]?"), this);
    ui->leDelimiter->setValidator(validator);

    connect(ui->rbComma, &QAbstractButton::toggled,
            this, &AsciiReaderSettings::delimiterToggled);
    connect(ui->rbSpace, &QAbstractButton::toggled,
            this, &AsciiReaderSettings::delimiterToggled);
    connect(ui->rbTab, &QAbstractButton::toggled,
            this, &AsciiReaderSettings::delimiterToggled);
    connect(ui->rbOtherDelimiter, &QAbstractButton::toggled,
            this, &AsciiReaderSettings::delimiterToggled);
    connect(ui->leDelimiter, &QLineEdit::textChanged,
            this, &AsciiReaderSettings::customDelimiterChanged);

    // Note: if directly connected we get a runtime warning on incompatible signal arguments
    connect(ui->spNumOfChannels, SELECT<int>::OVERLOAD_OF(&QSpinBox::valueChanged),
            [this](int value)
            {
                emit numOfChannelsChanged(value);
            });
}

AsciiReaderSettings::~AsciiReaderSettings()
{
    delete ui;
}

unsigned AsciiReaderSettings::numOfChannels() const
{
    return ui->spNumOfChannels->value();
}

QChar AsciiReaderSettings::delimiter() const
{
    if (ui->rbComma->isChecked())
    {
        return QChar(',');
    }
    else if (ui->rbSpace->isChecked())
    {
        return QChar(' ');
    }
    else if (ui->rbTab->isChecked())
    {
        return QChar('\t');
    }
    else                        // rbOther
    {
        auto t = ui->leDelimiter->text();
        return t.isEmpty() ? QChar() : t.at(0);
    }
}

void AsciiReaderSettings::delimiterToggled(bool checked)
{
    if (!checked) return;

    auto d = delimiter();
    if (!d.isNull())
    {
        emit delimiterChanged(d);
    }
}

void AsciiReaderSettings::customDelimiterChanged(const QString text)
{
    if (ui->rbOtherDelimiter->isChecked())
    {
        if (!text.isEmpty()) emit delimiterChanged(text.at(0));
    }
}

void AsciiReaderSettings::saveSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_ASCII);

    // save number of channels setting
    QString numOfChannelsSetting = QString::number(numOfChannels());
    if (numOfChannelsSetting == "0") numOfChannelsSetting = "auto";
    settings->setValue(SG_ASCII_NumOfChannels, numOfChannelsSetting);

    // save delimiter
    QString delimiterS;
    if (ui->rbOtherDelimiter->isChecked())
    {
        delimiterS = "other";
    }
    else if (ui->rbTab->isChecked())
    {
        // Note: \t is not correctly loaded
        delimiterS = "TAB";
    }
    else
    {
        delimiterS = delimiter();
    }

    settings->setValue(SG_ASCII_Delimiter, delimiterS);
    settings->setValue(SG_ASCII_CustomDelimiter, ui->leDelimiter->text());

    settings->endGroup();
}

void AsciiReaderSettings::loadSettings(QSettings* settings)
{
    settings->beginGroup(SettingGroup_ASCII);

    // load number of channels
    QString numOfChannelsSetting =
        settings->value(SG_ASCII_NumOfChannels, numOfChannels()).toString();

    if (numOfChannelsSetting == "auto")
    {
        ui->spNumOfChannels->setValue(0);
    }
    else
    {
        bool ok;
        int nc = numOfChannelsSetting.toInt(&ok);
        if (ok)
        {
            ui->spNumOfChannels->setValue(nc);
        }
    }

    // load delimiter
    auto delimiterS = settings->value(SG_ASCII_Delimiter, delimiter()).toString();
    auto customDelimiter = settings->value(SG_ASCII_CustomDelimiter, delimiter()).toString();
    if (!customDelimiter.isEmpty()) ui->leDelimiter->setText(customDelimiter);
    if (delimiterS == ",")
    {
        ui->rbComma->setChecked(true);
    }
    else if (delimiterS == " ")
    {
        ui->rbSpace->setChecked(true);
    }
    else if (delimiterS == "TAB")
    {
        ui->rbTab->setChecked(true);
    }
    else
    {
        ui->rbOtherDelimiter->setChecked(true);
    }

    settings->endGroup();
}
