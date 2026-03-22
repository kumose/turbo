//
// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
// Created by jeff on 24-6-7.
//
#pragma once

#include <memory>
#include <sstream>

#include <turbo/memory/memory.h>
#include <turbo/strings/string_builder.h>
#include <turbo/utility/internal/status.h>
#include <turbo/utility/internal/result.h>
#include <turbo/utility/errno.h>

namespace turbo {

    // aborted_error()
    // already_exists_error()
    // cancelled_error()
    // data_loss_error()
    // deadline_exceeded_error()
    // failed_precondition_error()
    // internal_error()
    // invalid_argument_error()
    // not_found_error()
    // out_of_range_error()
    // permission_denied_error()
    // resource_exhausted_error()
    // unauthenticated_error()
    // unavailable_error()
    // unimplemented_error()
    // unknown_error()
    //
    // These convenience functions create an `turbo::Status` object with an error
    // code as indicated by the associated function name, using the error message
    // passed in `message`.

    template<typename ...Args>
    Status aborted_error(const Args&...args) {
        return Status::aborted_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_aborted_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::aborted_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status already_exists_error(const Args &... args) {
        return Status::already_exists_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_already_exists_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::already_exists_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status cancelled_error(const Args &... args) {
        return Status::cancelled_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_cancelled_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::cancelled_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status data_loss_error(const Args &... args) {
        return Status::data_loss_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_data_loss_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::data_loss_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status format_deadline_exceeded_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::deadline_exceeded_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status deadline_exceeded_error(const Args &... args) {
        return Status::deadline_exceeded_error(StringBuilder::create(args...));
    }


    template<typename... Args>
    Status failed_precondition_error(const Args &... args) {
        return Status::failed_precondition_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_failed_precondition_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::failed_precondition_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status format_internal_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::internal_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status internal_error( const Args &... args) {
        return Status::internal_error(StringBuilder::create(args...));
    }


    template<typename... Args>
    Status invalid_argument_error(const Args &... args) {
        return Status::invalid_argument_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_invalid_argument_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::invalid_argument_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status not_found_error( const Args &... args) {
        return Status::not_found_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_not_found_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::not_found_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status out_of_range_error(const Args &... args) {
        return Status::out_of_range_error(StringBuilder::create(args...));
    }
    template<typename... Args>
    Status format_out_of_range_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::out_of_range_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status permission_denied_error(const Args &... args) {
        return Status::permission_denied_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_permission_denied_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::permission_denied_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status resource_exhausted_error(const Args &... args) {
        return Status::resource_exhausted_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_resource_exhausted_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::resource_exhausted_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status unauthenticated_error(const Args &... args) {
        return Status::unauthenticated_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_unauthenticated_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::unauthenticated_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status unavailable_error(const Args &... args) {
        return Status::unavailable_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_unavailable_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::unavailable_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status unimplemented_error(const Args &... args) {
        return Status::unimplemented_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_unimplemented_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::unimplemented_error(turbo::str_format(fmt, args...));
    }


    template<typename... Args>
    Status unknown_error(const Args &... args) {
        return Status::unknown_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_unknown_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::unknown_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status io_error(const Args &... args) {
        return Status::io_error(StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_io_error(const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::io_error(turbo::str_format(fmt, args...));
    }

    template<typename... Args>
    Status errno_to_status(int error_number, const Args &... args) {
        return Status::errno_to_status(error_number, StringBuilder::create(args...));
    }

    template<typename... Args>
    Status format_errno_to_status(int error_number, const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        return Status::errno_to_status(error_number, turbo::str_format(fmt, args...));
    }

    template<typename T = int, typename... Args>
    inline turbo::Status make_status(T code, const Args &... args) {
        static_assert(sizeof(code) <= sizeof(turbo::StatusCode),
                      "type size must less than sizeof(turbo::StatusCode)(4)");
        return Status::make_status(code, StringBuilder::create(args...));
    }

    template<typename T = int, typename... Args>
    inline turbo::Status format_status(T code, const turbo::FormatSpec<Args...> &fmt, const Args &... args) {
        static_assert(sizeof(code) <= sizeof(turbo::StatusCode),
                      "type size must less than sizeof(turbo::StatusCode)(4)");
        return Status::make_status(code, turbo::str_format(fmt, args...));
    }

    /// errno with payload
    TURBO_EXPORT
    std::shared_ptr<turbo::StatusPayload> status_payload_from_errno(int errnum);

    template <typename T, typename... Args>
    turbo::Status status_with_errno_payload(int errnum, T code, Args&&... args) {
        return make_status(code, std::forward<Args>(args)...).set_payload(status_payload_from_errno(errnum));
    }

    template <typename... Args>
    turbo::Status io_error_with_errno_payload(int errnum, Args&&... args) {
        return status_with_errno_payload(errnum, StatusCode::kIOError, std::forward<Args>(args)...);
    }

    TURBO_EXPORT
    int errno_from_status_payload(const turbo::Status&);

    /// signal
    std::shared_ptr<turbo::StatusPayload> status_payload_from_signal(int signum);

    template <typename T, typename... Args>
    turbo::Status status_with_signal_payload(int signum, T code, Args&&... args) {
        return make_status(code, std::forward<Args>(args)...).set_payload(status_payload_from_signal(signum));
    }

    template <typename... Args>
    turbo::Status cancelled_with_signal_payload(int signum, Args&&... args) {
        return status_with_signal_payload(signum, StatusCode::kCancelled, std::forward<Args>(args)...);
    }

    TURBO_EXPORT
    int signal_from_status_payload(const turbo::Status&);


    class TURBO_MUST_USE_RESULT StatusBuilder {
    public:
        explicit StatusBuilder(const turbo::Status &status);

        explicit StatusBuilder(turbo::Status &&status);

        explicit StatusBuilder(turbo::StatusCode code);

        StatusBuilder(const StatusBuilder &sb);

        template<typename T>
        StatusBuilder &operator<<(const T &value) &{
            if (status_.ok()) return *this;
            if (streamptr_ == nullptr)
                streamptr_ = std::make_unique<std::ostringstream>();
            *streamptr_ << value;
            return *this;
        }

        template<typename T>
        StatusBuilder &&operator<<(const T &value) &&{
            return std::move(operator<<(value));
        }

        StatusBuilder &log_error() &;

        StatusBuilder &&log_error() &&;

        operator turbo::Status() const &;

        operator turbo::Status() &&;

        template<typename T>
        inline operator turbo::Result<T>() const &{
            if (streamptr_ == nullptr) return turbo::Result<T>(status_);
            return turbo::Result<T>(StatusBuilder(*this).create_status());
        }

        template<typename T>
        inline operator turbo::Result<T>() &&{
            if (streamptr_ == nullptr) return turbo::Result<T>(status_);
            return turbo::Result<T>(StatusBuilder(*this).create_status());
        }

        template<typename Enum>
        StatusBuilder &set_error_code(Enum code) &{
            status_ =
                    turbo::Status(static_cast<turbo::StatusCode>(code), status_.message());
            return *this;
        }

        template<typename Enum>
        StatusBuilder &&set_error_code(Enum code) &&{
            return std::move(set_error_code(code));
        }

        turbo::Status create_status() &&;

    private:
        std::unique_ptr<std::ostringstream> streamptr_;

        turbo::Status status_;
    };

    StatusBuilder aborted_error_builder();

    StatusBuilder already_exists_error_builder();

    StatusBuilder cancelled_error_builder();

    StatusBuilder failed_precondition_error_builder();

    StatusBuilder internal_error_builder();

    StatusBuilder invalid_argument_error_builder();

    StatusBuilder not_found_error_builder();

    StatusBuilder out_of_range_error_builder();

    StatusBuilder unauthenticated_error_builder();

    StatusBuilder unavailable_error_builder();

    StatusBuilder unimplemented_error_builder();

    StatusBuilder unknown_error_builder();

    turbo::Status annotate_status(const turbo::Status &s, std::string_view msg);

    template<typename T>
    inline turbo::Status annotate_status(const turbo::Result<T> &s, std::string_view msg) {
        return annotate_status(s.status(), msg);
    }

    StatusBuilder ret_check_fail(std::string_view msg);

    struct __StatusLocation {
        explicit __StatusLocation(turbo::Nonnull<const char *> f, int l)
                : file(f), line(l) {

        }

        const char *file;
        int line;
    };

    std::ostream &operator<<(std::ostream &os, const __StatusLocation &l);
}  // namespace turbo

#ifndef STATUS_MACROS_IMPL
#define STATUS_MACROS_IMPL

#define STATUS_RET_CHECK(cond)         \
  while (TURBO_UNLIKELY(!(cond))) \
  return ret_check_fail(                \
      "STATUS_RET_CHECK failure ")

#define STATUS_RET_CHECK_EQ(lhs, rhs)         \
  while (TURBO_UNLIKELY((lhs) != (rhs))) \
  return ret_check_fail("STATUS_RET_CHECK_EQ failure ")

#define STATUS_RET_CHECK_NE(lhs, rhs)         \
  while (TURBO_UNLIKELY((lhs) == (rhs))) \
  return ret_check_fail("STATUS_RET_CHECK_NE failure ")

#define STATUS_RET_CHECK_GE(lhs, rhs)            \
  while (TURBO_UNLIKELY(!((lhs) >= (rhs)))) \
  return ret_check_fail("STATUS_RET_CHECK_GE failure ")

#define STATUS_RET_CHECK_LE(lhs, rhs)            \
  while (TURBO_UNLIKELY(!((lhs) <= (rhs)))) \
  return ret_check_fail("STATUS_RET_CHECK_LE failure ")

#define STATUS_RET_CHECK_GT(lhs, rhs)           \
  while (TURBO_UNLIKELY(!((lhs) > (rhs)))) \
  return ret_check_fail("STATUS_RET_CHECK_GT failure ")

#define STATUS_RET_CHECK_LT(lhs, rhs)           \
  while (TURBO_UNLIKELY(!((lhs) < (rhs)))) \
  return ret_check_fail("STATUS_RET_CHECK_LT failure ")
#define TURBO_EXTRA_ERROR_CONTEXT
#ifdef TURBO_EXTRA_ERROR_CONTEXT

/// \brief Return with given status if condition is met.
#define TURBO_RETURN_IF_(condition, status, bmsg, ...)   \
    do {                                              \
        if (TURBO_UNLIKELY(condition)) {           \
            ::turbo::Status _st = status;                    \
            auto msg = ::turbo::str_cat(__VA_ARGS__);    \
            _st.push_stack(TURBO_STATUS_TRACE_PARAM, (msg.empty() ? bmsg : msg));                  \
            return _st; \
        }                                                \
    }                                         \
    while (0)


#else

#define TURBO_RETURN_IF_(condition, status, bmsg, ...) \
  do {                                         \
    if (TURBO_UNLIKELY(condition)) {      \
      return (status);                         \
    }                                          \
  } while (0)

#endif  // TURBO_EXTRA_ERROR_CONTEXT

#define TURBO_RETURN_IF(condition, status) \
  TURBO_RETURN_IF_(condition, status, TURBO_STRINGIFY(status))

#define TURBO_RETURN_NOT_OK(status, ...)                                   \
  do {                                                                \
    ::turbo::Status __s = ::turbo::internal::generic_to_status(status);    \
                                                                           \
    TURBO_RETURN_IF_(!__s.ok(), __s, TURBO_STRINGIFY(status), __VA_ARGS__);        \
  } while (false)

#define TURBO_WARN_NOT_OK(expr, ...) \
  do {                                    \
    ::turbo::Status _s = ::turbo::internal::generic_to_status(expr);          \
    if (TURBO_UNLIKELY(!_s.ok())) {  \
        auto warn_msg = ::turbo::str_cat(__VA_ARGS__);  \
      _s.warn(warn_msg.empty() ? TURBO_STRINGIFY(expr) : warn_msg);                  \
    }                                     \
  } while (false)

#define TURBO_ABORT_NOT_OK(expr, ...) \
  do {                                    \
    ::turbo::Status _s = ::turbo::internal::generic_to_status(expr);          \
    if (TURBO_UNLIKELY(!_s.ok())) {  \
        auto warn_msg = ::turbo::str_cat(__VA_ARGS__);  \
      _s.abort(warn_msg.empty() ? TURBO_STRINGIFY(expr) : warn_msg);                  \
    }                                     \
  } while (false)

#define TURBO_RETURN_NOT_OK_ELSE(s, else_)                            \
  do {                                                          \
    ::turbo::Status _s = ::turbo::internal::generic_to_status(s); \
    if (!_s.ok()) {                                             \
      else_;                                                    \
      return _s;                                                \
    }                                                           \
  } while (false)


#define TURBO_MOVE_OR_RAISE_IMPL(result_name, lhs, rexpr, ...)                              \
  auto&& result_name = (rexpr);                                                             \
  TURBO_RETURN_IF_(!(result_name).ok(), (result_name).status(), TURBO_STRINGIFY(rexpr), __VA_ARGS__); \
  lhs = std::move(result_name).value_unsafe()

#define TURBO_COPY_OR_RAISE_IMPL(result_name, lhs, rexpr, ...)                              \
    auto&& result_name = (rexpr);                                                             \
    TURBO_RETURN_IF_(!(result_name).ok(), (result_name).status(), TURBO_STRINGIFY(rexpr), __VA_ARGS__); \
    lhs = result_name.value_unsafe();

#define TURBO_ASSIGN_OR_RAISE_NAME(x, y) TURBO_CONCAT(x, y)

/// \brief Execute an expression that returns a Result, extracting its value
/// into the variable defined by `lhs` (or returning a Status on error).
///
/// Example: Assigning to a new value:
///   TURBO_MOVE_OR_RAISE(auto value, MaybeGetValue(arg));
///
/// Example: Assigning to an existing value:
///   value_type value;
///   TURBO_MOVE_OR_RAISE(value, MaybeGetValue(arg));
///
/// WARNING: TURBO_MOVE_OR_RAISE expands into multiple statements;
/// it cannot be used in a single statement (e.g. as the body of an if
/// statement without {})!
///
/// WARNING: TURBO_MOVE_OR_RAISE `std::move`s its right operand. If you have
/// an lvalue Result which you *don't* want to move out of cast appropriately.
///
/// WARNING: TURBO_MOVE_OR_RAISE is not a single expression; it will not
/// maintain lifetimes of all temporaries in `rexpr` (e.g.
/// `TURBO_MOVE_OR_RAISE(auto x, MakeTemp().GetResultRef());`
/// will most likely segfault)!
#define TURBO_MOVE_OR_RAISE(lhs, rexpr, ...)   \
  TURBO_MOVE_OR_RAISE_IMPL(TURBO_ASSIGN_OR_RAISE_NAME(_error_or_value, __COUNTER__), \
                             lhs, rexpr, __VA_ARGS__)

#define TURBO_COPY_OR_RAISE(lhs, rexpr)                                              \
  TURBO_COPY_OR_RAISE_IMPL(TURBO_ASSIGN_OR_RAISE_NAME(_error_or_value, __COUNTER__), \
                             lhs, rexpr);

#endif  // STATUS_MACROS_IMPL
