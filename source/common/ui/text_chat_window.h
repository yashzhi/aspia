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

#ifndef COMMON__UI__TEXT_CHAT_WINDOW_H
#define COMMON__UI__TEXT_CHAT_WINDOW_H

#include "proto/text_chat.pb.h"

#include <QWidget>

namespace Ui {
class TextChatWindow;
} // namespace Ui

namespace common {

class TextChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TextChatWindow(QWidget* parent = nullptr);
    ~TextChatWindow();

    void readMessage(const proto::TextChatMessage& message);
    void readStatus(const proto::TextChatStatus& status);

signals:
    void sendMessage(const proto::TextChatMessage& message);
    void sendStatus(const proto::TextChatStatus& status);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void addOutgoingMessage(time_t timestamp, const QString& message);
    void onSendMessage();
    void onSendStatus(proto::TextChatStatus::Status status);
    void onClearHistory();
    void onSaveChat();
    void onUpdateSize();

    std::unique_ptr<Ui::TextChatWindow> ui;
    std::string host_name_;
    QTimer* status_clear_timer_;
};

} // namespace common

#endif // COMMON__UI__TEXT_CHAT_WINDOW_H
