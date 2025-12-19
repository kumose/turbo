// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#include <turbo/threading/waitable_event.h>

#include <turbo/base/macros.h>
#include <turbo/threading/platform_thread.h>
#include <gtest/gtest.h>

namespace turbo {

    TEST(WaitableEventTest, ManualBasics) {
        WaitableEvent event(true, false);

        EXPECT_FALSE(event.IsSignaled());

        event.Signal();
        EXPECT_TRUE(event.IsSignaled());
        EXPECT_TRUE(event.IsSignaled());

        event.Reset();
        EXPECT_FALSE(event.IsSignaled());
        EXPECT_FALSE(event.TimedWait(turbo::Duration::milliseconds(10)));

        event.Signal();
        event.Wait();
        EXPECT_TRUE(event.TimedWait(turbo::Duration::milliseconds(10)));
    }

    TEST(WaitableEventTest, AutoBasics) {
        WaitableEvent event(false, false);

        EXPECT_FALSE(event.IsSignaled());

        event.Signal();
        EXPECT_TRUE(event.IsSignaled());
        EXPECT_FALSE(event.IsSignaled());

        event.Reset();
        EXPECT_FALSE(event.IsSignaled());
        EXPECT_FALSE(event.TimedWait(turbo::Duration::milliseconds(10)));

        event.Signal();
        event.Wait();
        EXPECT_FALSE(event.TimedWait(turbo::Duration::milliseconds(10)));

        event.Signal();
        EXPECT_TRUE(event.TimedWait(turbo::Duration::milliseconds(10)));
    }

    TEST(WaitableEventTest, WaitManyShortcut) {
        WaitableEvent *ev[5];
        for (unsigned i = 0; i < 5; ++i)
            ev[i] = new WaitableEvent(false, false);

        ev[3]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 3u);

        ev[3]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 3u);

        ev[4]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 4u);

        ev[0]->Signal();
        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 0u);

        for (unsigned i = 0; i < 5; ++i)
            delete ev[i];
    }

    class WaitableEventSignaler : public PlatformThread::Delegate {
    public:
        WaitableEventSignaler(double seconds, WaitableEvent *ev)
                : seconds_(seconds),
                  ev_(ev) {
        }

        virtual void ThreadMain() override {
            turbo::sleep_for(turbo::Duration::seconds(seconds_));
            ev_->Signal();
        }

    private:
        const double seconds_;
        WaitableEvent *const ev_;
    };

    TEST(WaitableEventTest, WaitMany) {
        WaitableEvent *ev[5];
        for (unsigned i = 0; i < 5; ++i)
            ev[i] = new WaitableEvent(false, false);

        WaitableEventSignaler signaler(0.1, ev[2]);
        PlatformThreadHandle thread;
        PlatformThread::Create(0, &signaler, &thread);

        EXPECT_EQ(WaitableEvent::WaitMany(ev, 5), 2u);

        PlatformThread::Join(thread);

        for (unsigned i = 0; i < 5; ++i)
            delete ev[i];
    }

}  // namespace turbo
