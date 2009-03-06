/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "InputDialog.h"
#include "gui/widgets/LineEdit.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>


namespace Rosegarden
{

InputDialog::InputDialog(const QString &title, const QString &label,
                         QWidget *parent, QWidget *input, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    // All of these little ancillary dialogs should have the same background,
    // regardless of which parent widget they have (for popup proximity
    // purposes)
    //
    // NB: this will BREAK the "no stylesheet" case
    QString localStyle("QDialog {background-color: #000000} QLabel{background-color: transparent}");
    setStyleSheet(localStyle);

    // set the window title
    setWindowTitle(title);
    QVBoxLayout *vbox = new QVBoxLayout(this);

    // add the passed label to our layout
    QLabel *lbl = new QLabel(label, this);
    vbox->addWidget(lbl);
    vbox->addStretch(1);

    // add the passed input widget to our layout, and reparent it
    input->setParent(this);
    vbox->addWidget(input);
    vbox->addStretch(1);

    // I have no idea what this is for, but Qt had it, so we'll keep it in our
    // doctored version
//    lbl->setBuddy(input);

    // Put some clicky buttons and hook them up
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, this);
//    buttonBox->setObjectName(QLatin1String("qt_inputdlg_buttonbox"));
    QPushButton *okButton = static_cast<QPushButton *>(buttonBox->addButton(QDialogButtonBox::Ok));
    okButton->setDefault(true);
    vbox->addWidget(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // No size grip.  Size grips are stupid looking, and I detest them.
    // Rosegarden has a NO SIZE GRIP policy.
    setSizeGripEnabled(false);
}

InputDialog::~InputDialog()
{
}


QString
InputDialog::getText(QWidget *parent, const QString &title, const QString &label,
                     LineEdit::EchoMode mode, const QString &text,
                     bool *ok, Qt::WindowFlags f)
{
    LineEdit *le = new LineEdit;
    le->setText(text);
    le->setEchoMode(mode);
    le->setFocus();
    le->selectAll();

    InputDialog dlg(title, label, parent, le, f);

    QString result;
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    if (accepted)
        result = le->text();

    return result;
}

}

#include "InputDialog.moc"
