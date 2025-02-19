//
// Aspia Project
// Copyright (C) 2021 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "common/ui/text_chat_window.h"

#include "base/logging.h"
#include "common/ui/text_chat_incoming_message.h"
#include "common/ui/text_chat_outgoing_message.h"
#include "ui_text_chat_window.h"

#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHostInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>

namespace common {

namespace {

const int kMaxMessageLength = 2048;

} // namespace

TextChatWindow::TextChatWindow(QWidget* parent)
    : QWidget(parent),
      ui(std::make_unique<Ui::TextChatWindow>()),
      host_name_(QHostInfo::localHostName().toStdString()),
      status_clear_timer_(new QTimer(this))
{
    ui->setupUi(this);
    ui->edit_message->installEventFilter(this);
    ui->list_messages->horizontalScrollBar()->installEventFilter(this);
    ui->list_messages->verticalScrollBar()->installEventFilter(this);

    connect(status_clear_timer_, &QTimer::timeout, ui->label_status, &QLabel::clear);
    connect(ui->button_send, &QToolButton::clicked, this, &TextChatWindow::onSendMessage);
    connect(ui->button_tools, &QToolButton::clicked, this, [=]()
    {
        QMenu menu;
        QAction* save_chat_action = menu.addAction(tr("Save chat..."));
        QAction* clear_history_action = menu.addAction(tr("Clear chat"));

        menu.show();

        QPoint pos = ui->button_tools->mapToGlobal(ui->button_tools->rect().topRight());
        pos.setX(pos.x() - menu.rect().width());
        pos.setY(pos.y() - menu.rect().height());

        QAction* action = menu.exec(pos);
        if (action == clear_history_action)
        {
            onClearHistory();
        }
        else if (action == save_chat_action)
        {
            onSaveChat();
        }
    });
}

TextChatWindow::~TextChatWindow() = default;

void TextChatWindow::readMessage(const proto::TextChatMessage& message)
{
    QListWidget* list_messages = ui->list_messages;
    TextChatIncomingMessage* message_widget = new TextChatIncomingMessage(list_messages);

    message_widget->setTimestamp(message.timestamp());
    message_widget->setSource(QString::fromStdString(message.source()));
    message_widget->setMessageText(QString::fromStdString(message.text()));

    QListWidgetItem* item = new QListWidgetItem(list_messages);

    list_messages->addItem(item);
    list_messages->setItemWidget(item, message_widget);

    onUpdateSize();
}

void TextChatWindow::readStatus(const proto::TextChatStatus& status)
{
    status_clear_timer_->stop();

    QString user_name = QString::fromStdString(status.source());
    QString status_message;

    switch (status.status())
    {
        case proto::TextChatStatus::STATUS_TYPING:
            status_message = tr("%1 is typing...").arg(user_name);
            break;

        case proto::TextChatStatus::STATUS_STARTED:
            status_message = tr("User %1 started a chat.").arg(user_name);
            break;

        case proto::TextChatStatus::STATUS_STOPPED:
            status_message = tr("User %1 finished a chat.").arg(user_name);
            break;

        default:
            LOG(LS_WARNING) << "Unhandled status code: " << static_cast<int>(status.status());
            return;
    }

    ui->label_status->setText(status_message);
    status_clear_timer_->start(std::chrono::seconds(3));
}

bool TextChatWindow::eventFilter(QObject* object, QEvent* event)
{
    if (object == ui->edit_message && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Return)
        {
            onSendMessage();
            return true;
        }

        onSendStatus(proto::TextChatStatus::STATUS_TYPING);
    }
    else if (object == ui->list_messages->horizontalScrollBar() ||
             object == ui->list_messages->verticalScrollBar())
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
        {
            onUpdateSize();
        }
    }

    return QWidget::eventFilter(object, event);
}

void TextChatWindow::resizeEvent(QResizeEvent* /* event */)
{
    onUpdateSize();
}

void TextChatWindow::addOutgoingMessage(time_t timestamp, const QString& message)
{
    QListWidget* list_messages = ui->list_messages;
    TextChatOutgoingMessage* message_widget = new TextChatOutgoingMessage(list_messages);

    message_widget->setTimestamp(timestamp);
    message_widget->setMessageText(message);

    QListWidgetItem* item = new QListWidgetItem(list_messages);

    list_messages->addItem(item);
    list_messages->setItemWidget(item, message_widget);

    onUpdateSize();
}

void TextChatWindow::onSendMessage()
{
    QString message = ui->edit_message->text();
    if (message.isEmpty())
        return;

    if (message.length() > kMaxMessageLength)
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("The message is too long. The maximum message length is %n "
                                "characters.", "", kMaxMessageLength),
                             QMessageBox::Ok);
        return;
    }

    int64_t timestamp = QDateTime::currentSecsSinceEpoch();

    addOutgoingMessage(timestamp, message);
    ui->edit_message->clear();
    ui->edit_message->setFocus();

    proto::TextChatMessage text_chat_message;
    text_chat_message.set_timestamp(timestamp);
    text_chat_message.set_source(host_name_);
    text_chat_message.set_text(message.toStdString());

    emit sendMessage(text_chat_message);
}

void TextChatWindow::onSendStatus(proto::TextChatStatus::Status status)
{
    proto::TextChatStatus text_chat_status;
    text_chat_status.set_timestamp(QDateTime::currentSecsSinceEpoch());
    text_chat_status.set_source(host_name_);
    text_chat_status.set_status(status);

    emit sendStatus(text_chat_status);
}

void TextChatWindow::onClearHistory()
{
    QListWidget* list_messages = ui->list_messages;
    for (int i = list_messages->count() - 1; i >= 0; --i)
        delete list_messages->item(i);
}

void TextChatWindow::onSaveChat()
{
    QString selected_filter;
    QString file_path = QFileDialog::getSaveFileName(this,
                                                     tr("Save File"),
                                                     QString(),
                                                     tr("TXT files (*.txt)"),
                                                     &selected_filter);
    if (file_path.isEmpty() || selected_filter.isEmpty())
        return;

    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Could not open file for writing."),
                             QMessageBox::Ok);
        return;
    }

    QListWidget* list_messages = ui->list_messages;
    QTextStream stream(&file);

    for (int i = 0; i < list_messages->count(); ++i)
    {
        QListWidgetItem* item = list_messages->item(i);
        TextChatMessage* message_widget =
            static_cast<TextChatMessage*>(list_messages->itemWidget(item));

        if (message_widget->direction() == TextChatMessage::Direction::INCOMING)
        {
            TextChatIncomingMessage* incoming_message_widget =
                static_cast<TextChatIncomingMessage*>(message_widget);

            stream << "[" << incoming_message_widget->messageTime() << "] "
                   << incoming_message_widget->source() << Qt::endl;
            stream << incoming_message_widget->messageText() << Qt::endl;
            stream << Qt::endl;
        }
        else
        {
            DCHECK_EQ(message_widget->direction(), TextChatMessage::Direction::OUTGOING);

            TextChatOutgoingMessage* outgoing_message_widget =
                static_cast<TextChatOutgoingMessage*>(message_widget);

            stream << "[" << outgoing_message_widget->messageTime() << "] " << Qt::endl;
            stream << outgoing_message_widget->messageText() << Qt::endl;
            stream << Qt::endl;
        }
    }
}

void TextChatWindow::onUpdateSize()
{
    QListWidget* list_messages = ui->list_messages;
    int count = list_messages->count();

    for (int i = 0; i < count; ++i)
    {
        QListWidgetItem* item = list_messages->item(i);

        TextChatMessage* message_widget =
            static_cast<TextChatMessage*>(list_messages->itemWidget(item));
        int viewport_width = list_messages->viewport()->width();

        message_widget->setFixedWidth(viewport_width);
        message_widget->setFixedHeight(message_widget->heightForWidth(viewport_width));

        item->setSizeHint(message_widget->size());
    }

    list_messages->scrollToBottom();
}

} // namespace common
