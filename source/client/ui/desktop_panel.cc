//
// PROJECT:         Aspia
// FILE:            client/ui/desktop_panel.cc
// LICENSE:         GNU Lesser General Public License 2.1
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#include "client/ui/desktop_panel.h"

#include <QMenu>

#include "client/ui/key_sequence_dialog.h"

namespace aspia {

DesktopPanel::DesktopPanel(proto::auth::SessionType session_type, QWidget* parent)
    : QFrame(parent)
{
    ui.setupUi(this);

    connect(ui.button_settings, &QPushButton::pressed, this, &DesktopPanel::settingsButton);
    connect(ui.button_autosize, &QPushButton::pressed, this, &DesktopPanel::onAutosizeButton);
    connect(ui.button_full_screen, &QPushButton::clicked, this, &DesktopPanel::onFullscreenButton);

    if (session_type == proto::auth::SESSION_TYPE_DESKTOP_MANAGE)
    {
        keys_menu_ = new QMenu(this);
        keys_menu_->addAction(ui.action_alt_tab);
        keys_menu_->addAction(ui.action_alt_shift_tab);
        keys_menu_->addAction(ui.action_printscreen);
        keys_menu_->addAction(ui.action_alt_printscreen);
        keys_menu_->addAction(ui.action_custom);

        connect(ui.button_ctrl_alt_del, &QPushButton::pressed,
                this, &DesktopPanel::onCtrlAltDelButton);

        connect(ui.action_alt_tab, &QAction::triggered,
                this, &DesktopPanel::onAltTabAction);

        connect(ui.action_alt_shift_tab, &QAction::triggered,
                this, &DesktopPanel::onAltShiftTabAction);

        connect(ui.action_printscreen, &QAction::triggered,
                this, &DesktopPanel::onPrintScreenAction);

        connect(ui.action_alt_printscreen, &QAction::triggered,
                this, &DesktopPanel::onAltPrintScreenAction);

        connect(ui.action_custom, &QAction::triggered,
                this, &DesktopPanel::onCustomAction);

        ui.button_send_keys->setMenu(keys_menu_);
        ui.button_send_keys->setIcon(QIcon(":/icon/keyboard.png"));
    }
    else
    {
        Q_ASSERT(session_type == proto::auth::SESSION_TYPE_DESKTOP_VIEW);

        ui.button_send_keys->setHidden(true);
        ui.button_ctrl_alt_del->setHidden(true);
    }
}

DesktopPanel::~DesktopPanel() = default;

void DesktopPanel::onFullscreenButton(bool checked)
{
    if (checked)
        ui.button_full_screen->setIcon(QIcon(":/icon/application-resize-actual.png"));
    else
        ui.button_full_screen->setIcon(QIcon(":/icon/application-resize-full.png"));

    emit switchToFullscreen(checked);
}

void DesktopPanel::onAutosizeButton()
{
    if (ui.button_full_screen->isChecked())
    {
        ui.button_full_screen->setIcon(QIcon(":/icon/application-resize-full.png"));
        ui.button_full_screen->setChecked(false);
    }

    emit switchToAutosize();
}

void DesktopPanel::onCtrlAltDelButton()
{
    emit keySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Delete);
}

void DesktopPanel::onAltTabAction()
{
    emit keySequence(Qt::AltModifier | Qt::Key_Tab);
}

void DesktopPanel::onAltShiftTabAction()
{
    emit keySequence(Qt::AltModifier | Qt::ShiftModifier | Qt::Key_Tab);
}

void DesktopPanel::onPrintScreenAction()
{
    emit keySequence(Qt::Key_Print);
}

void DesktopPanel::onAltPrintScreenAction()
{
    emit keySequence(Qt::AltModifier | Qt::Key_Print);
}

void DesktopPanel::onCustomAction()
{
    QKeySequence key_sequence = KeySequenceDialog::keySequence(this);

    for (int i = 0; i < key_sequence.count(); ++i)
        emit keySequence(key_sequence[i]);
}

} // namespace aspia
