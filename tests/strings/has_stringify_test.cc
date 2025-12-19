// Copyright (C) 2024 EA group inc.
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

#include <turbo/strings/has_stringify.h>
#include <turbo/strings/string_builder.h>
#include <string>

#include <gtest/gtest.h>
#include <optional>

namespace {

    struct TypeWithoutTurboStringify {
    };

    struct TypeWithTurboStringify {
        template<typename Sink>
        friend void turbo_stringify(Sink &, const TypeWithTurboStringify &) {}
    };

    TEST(HasTurboStringifyTest, Works) {
        EXPECT_FALSE(turbo::HasTurboStringify<int>::value);
        EXPECT_FALSE(turbo::HasTurboStringify<std::string>::value);
        EXPECT_FALSE(turbo::HasTurboStringify<TypeWithoutTurboStringify>::value);
        EXPECT_TRUE(turbo::HasTurboStringify<TypeWithTurboStringify>::value);
        EXPECT_FALSE(
                turbo::HasTurboStringify<std::optional<TypeWithTurboStringify>>::value);
    }

    class HasToStringMember {
    public:
        std::string to_string() const {
            return "has";
        }

    };

    class HasToStringMemberWithDefaultOption {
    public:
        std::string to_string(bool has = true) const {
            if (has)
                return "has";
            else
                return "no";
        }

    };

    class NoHasToStringMember {
    public:
        std::string ToString() const {
            return "has";
        }

    };

    struct Point {
        int x;
        int y;

        std::string to_string() const {
            return "[" + std::to_string(x) + "," + std::to_string(y) + "]";
        }
    };

    struct PointConst {
        int x;
        int y;

        const std::string &to_string() const {
            static std::string ins = "abc";
            return ins;
        }
    };

    struct PointView {
        int x;
        int y;

        std::string_view to_string() const {
            static std::string ins = "abc";
            return ins;
        }
    };

    struct Point1 {
        int x;
        int y;
    };

    template<typename Sink>
    void turbo_stringify(Sink &sink, const Point1 &p) {
        sink.Append("[" + std::to_string(p.x) + "," + std::to_string(p.y) + "]");
    }

    struct PointOs {
        int x;
        int y;
    };

    std::ostream &operator<<(std::ostream &os, const PointOs &p) {
        os << "[" << std::to_string(p.x) << "," << std::to_string(p.y) << "]";
        return os;
    }

    struct PointOsI {
        int x;
        int y;

        friend std::ostream &operator<<(std::ostream &os, const PointOsI &p) {
            os << "[" << std::to_string(p.x) << "," << std::to_string(p.y) << "]";
            return os;
        }
    };

    struct PointOsd {
        int x;
        int y;

        void describe(std::ostream &os) const {
            os << "[" << std::to_string(x) << "," << std::to_string(y) << "]";
        }
    };


    TEST(HasToStringTest, Works) {
        EXPECT_FALSE(turbo::HasToStringMember<int>::value);
        EXPECT_FALSE(turbo::HasToStringMember<std::string>::value);
        EXPECT_FALSE(turbo::HasToStringMember<NoHasToStringMember>::value);
        EXPECT_TRUE(turbo::HasToStringMember<HasToStringMember>::value);
        EXPECT_TRUE(turbo::HasToStringMember<Point>::value);
        EXPECT_TRUE(turbo::HasToStringMember<PointConst>::value);
        EXPECT_TRUE(turbo::HasToStringMember<PointView>::value);
        EXPECT_TRUE(turbo::HasToStringMember<HasToStringMemberWithDefaultOption>::value);
        EXPECT_FALSE(
                turbo::HasToStringMember<std::optional<HasToStringMember>>::value);
    }

    TEST(HasStdToStringTest, Works) {
        EXPECT_TRUE(turbo::HasStdToString<int>::value);
        EXPECT_FALSE(turbo::HasStdToString<std::string>::value);
        EXPECT_FALSE(turbo::HasStdToString<NoHasToStringMember>::value);
        EXPECT_FALSE(turbo::HasStdToString<HasToStringMember>::value);
        EXPECT_FALSE(turbo::HasStdToString<HasToStringMemberWithDefaultOption>::value);
        EXPECT_FALSE(
                turbo::HasStdToString<std::optional<HasToStringMember>>::value);
    }

    TEST(StringBuilder, simple) {
        EXPECT_EQ(turbo::StringBuilder::create("1", "3"), "13");
        EXPECT_EQ(turbo::StringBuilder::create("1"), "1");
        EXPECT_EQ(turbo::StringBuilder::create(), "");

        Point p{1, 2};
        EXPECT_EQ(turbo::StringBuilder::create("a", p), "a[1,2]");

        EXPECT_EQ(turbo::StringBuilder::create("b", Point1{1, 2}), "b[1,2]");
        EXPECT_EQ(turbo::StringBuilder::create("o", PointOs{1, 2}), "o[1,2]");
        EXPECT_EQ(turbo::StringBuilder::create("i", PointOsI{1, 2}), "i[1,2]");
        EXPECT_EQ(turbo::StringBuilder::create("d", PointOsd{1, 2}), "d[1,2]");

        /// builder
        turbo::StringBuilder b;
        b << p;
        b << "a"<<PointOs{1, 2};
        std::cout << b.str() << std::endl;

    }

    struct NoOstream : public turbo::ToStringOstreamable<NoOstream> {
        int a;
    };

    struct ToStringOstream : public turbo::ToStringOstreamable<ToStringOstream> {
        ToStringOstream(int a1) : a(a1) {}


        std::string to_string() const {
            return std::to_string(a);
        }

        int a;
    };

    struct ToDesOstream : public turbo::ToStringOstreamable<ToDesOstream> {
        ToDesOstream(int a1) : a(a1) {}


        void describe(std::ostream &os) const {
            os<<a;
        }

        int a;
    };
    struct ToCstringOstream : public turbo::ToStringOstreamable<ToCstringOstream> {

        std::string to_string() const {
            const std::string ins = "12";
            return ins;
        }

    };

    struct ToViewOstream : public turbo::ToStringOstreamable<ToViewOstream> {

        std::string_view to_string() const {
            const std::string_view ins = "12";
            return ins;
        }

    };

    TEST(StringBuilder, to_ostream) {
        {
            std::stringstream ss0;
            /// ss0<<NoOstream(); fail
            ToStringOstream a{1};
            ss0 << a << ToStringOstream{2};
            EXPECT_EQ(ss0.str(), "12");
        }
        {
            std::stringstream ss1;
            ToDesOstream d{1};
            ss1 << d << ToDesOstream{2};
            EXPECT_EQ(ss1.str(), "12");
        }

        {
            std::stringstream ss;
            ToViewOstream d;
            ss << d << ToViewOstream{};
            EXPECT_EQ(ss.str(), "1212");
        }
    }

}  // namespace
