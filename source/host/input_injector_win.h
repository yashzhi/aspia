//
// Aspia Project
// Copyright (C) 2020 Dmitry Chapyshev <dmitry@aspia.ru>
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

#ifndef HOST__INPUT_INJECTOR_WIN_H
#define HOST__INPUT_INJECTOR_WIN_H

#include "base/win/scoped_thread_desktop.h"
#include "host/input_injector.h"

namespace host {

class InputInjectorWin : public InputInjector
{
public:
    InputInjectorWin();
    ~InputInjectorWin();

    // InputInjector implementation.
    void setScreenOffset(const base::Point& offset) override;
    void setBlockInput(bool enable) override;
    void injectKeyEvent(const proto::KeyEvent& event) override;
    void injectTextEvent(const proto::TextEvent& event) override;
    void injectMouseEvent(const proto::MouseEvent& event) override;

private:
    void switchToInputDesktop();
    bool isCtrlAndAltPressed();

    base::ScopedThreadDesktop desktop_;

    bool block_input_ = false;
    std::set<uint32_t> pressed_keys_;

    base::Point screen_offset_;
    base::Point last_mouse_pos_;
    uint32_t last_mouse_mask_ = 0;

    DISALLOW_COPY_AND_ASSIGN(InputInjectorWin);
};

} // namespace host

#endif // HOST__INPUT_INJECTOR_WIN_H
