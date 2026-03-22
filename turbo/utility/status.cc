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
#include <turbo/utility/status.h>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <turbo/strings/str_split.h>
#include <turbo/base/checked_cast.h>

namespace turbo {

    // Map from a Windows error code to a `errno` value
    //
    // Most code in this function is taken from CPython's `PC/errmap.h`.
    // Unlike CPython however, we return 0 for unknown / unsupported values.
    int platform_error_to_errno(int winerror) {
#if _WIN32
        // Unwrap FACILITY_WIN32 HRESULT errors.
        if ((winerror & 0xFFFF0000) == 0x80070000) {
          winerror &= 0x0000FFFF;
        }

        // Winsock error codes (10000-11999) are errno values.
        if (winerror >= 10000 && winerror < 12000) {
          switch (winerror) {
            case WSAEINTR:
            case WSAEBADF:
            case WSAEACCES:
            case WSAEFAULT:
            case WSAEINVAL:
            case WSAEMFILE:
              // Winsock definitions of errno values. See WinSock2.h
              return winerror - 10000;
            default:
              return winerror;
          }
        }

        switch (winerror) {
          case ERROR_FILE_NOT_FOUND:        //    2
          case ERROR_PATH_NOT_FOUND:        //    3
          case ERROR_INVALID_DRIVE:         //   15
          case ERROR_NO_MORE_FILES:         //   18
          case ERROR_BAD_NETPATH:           //   53
          case ERROR_BAD_NET_NAME:          //   67
          case ERROR_BAD_PATHNAME:          //  161
          case ERROR_FILENAME_EXCED_RANGE:  //  206
            return ENOENT;

          case ERROR_BAD_ENVIRONMENT:  //   10
            return E2BIG;

          case ERROR_BAD_FORMAT:                 //   11
          case ERROR_INVALID_STARTING_CODESEG:   //  188
          case ERROR_INVALID_STACKSEG:           //  189
          case ERROR_INVALID_MODULETYPE:         //  190
          case ERROR_INVALID_EXE_SIGNATURE:      //  191
          case ERROR_EXE_MARKED_INVALID:         //  192
          case ERROR_BAD_EXE_FORMAT:             //  193
          case ERROR_ITERATED_DATA_EXCEEDS_64k:  //  194
          case ERROR_INVALID_MINALLOCSIZE:       //  195
          case ERROR_DYNLINK_FROM_INVALID_RING:  //  196
          case ERROR_IOPL_NOT_ENABLED:           //  197
          case ERROR_INVALID_SEGDPL:             //  198
          case ERROR_AUTODATASEG_EXCEEDS_64k:    //  199
          case ERROR_RING2SEG_MUST_BE_MOVABLE:   //  200
          case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:   //  201
          case ERROR_INFLOOP_IN_RELOC_CHAIN:     //  202
            return ENOEXEC;

          case ERROR_INVALID_HANDLE:         //    6
          case ERROR_INVALID_TARGET_HANDLE:  //  114
          case ERROR_DIRECT_ACCESS_HANDLE:   //  130
            return EBADF;

          case ERROR_WAIT_NO_CHILDREN:    //  128
          case ERROR_CHILD_NOT_COMPLETE:  //  129
            return ECHILD;

          case ERROR_NO_PROC_SLOTS:        //   89
          case ERROR_MAX_THRDS_REACHED:    //  164
          case ERROR_NESTING_NOT_ALLOWED:  //  215
            return EAGAIN;

          case ERROR_ARENA_TRASHED:      //    7
          case ERROR_NOT_ENOUGH_MEMORY:  //    8
          case ERROR_INVALID_BLOCK:      //    9
          case ERROR_NOT_ENOUGH_QUOTA:   // 1816
            return ENOMEM;

          case ERROR_ACCESS_DENIED:            //    5
          case ERROR_CURRENT_DIRECTORY:        //   16
          case ERROR_WRITE_PROTECT:            //   19
          case ERROR_BAD_UNIT:                 //   20
          case ERROR_NOT_READY:                //   21
          case ERROR_BAD_COMMAND:              //   22
          case ERROR_CRC:                      //   23
          case ERROR_BAD_LENGTH:               //   24
          case ERROR_SEEK:                     //   25
          case ERROR_NOT_DOS_DISK:             //   26
          case ERROR_SECTOR_NOT_FOUND:         //   27
          case ERROR_OUT_OF_PAPER:             //   28
          case ERROR_WRITE_FAULT:              //   29
          case ERROR_READ_FAULT:               //   30
          case ERROR_GEN_FAILURE:              //   31
          case ERROR_SHARING_VIOLATION:        //   32
          case ERROR_LOCK_VIOLATION:           //   33
          case ERROR_WRONG_DISK:               //   34
          case ERROR_SHARING_BUFFER_EXCEEDED:  //   36
          case ERROR_NETWORK_ACCESS_DENIED:    //   65
          case ERROR_CANNOT_MAKE:              //   82
          case ERROR_FAIL_I24:                 //   83
          case ERROR_DRIVE_LOCKED:             //  108
          case ERROR_SEEK_ON_DEVICE:           //  132
          case ERROR_NOT_LOCKED:               //  158
          case ERROR_LOCK_FAILED:              //  167
          case 35:                             //   35 (undefined)
            return EACCES;

          case ERROR_FILE_EXISTS:     //   80
          case ERROR_ALREADY_EXISTS:  //  183
            return EEXIST;

          case ERROR_NOT_SAME_DEVICE:  //   17
            return EXDEV;

          case ERROR_DIRECTORY:  //  267 (bpo-12802)
            return ENOTDIR;

          case ERROR_TOO_MANY_OPEN_FILES:  //    4
            return EMFILE;

          case ERROR_DISK_FULL:  //  112
            return ENOSPC;

          case ERROR_BROKEN_PIPE:  //  109
          case ERROR_NO_DATA:      //  232 (bpo-13063)
            return EPIPE;

          case ERROR_DIR_NOT_EMPTY:  //  145
            return ENOTEMPTY;

          case ERROR_NO_UNICODE_TRANSLATION:  // 1113
            return EILSEQ;

          case ERROR_INVALID_FUNCTION:   //    1
          case ERROR_INVALID_ACCESS:     //   12
          case ERROR_INVALID_DATA:       //   13
          case ERROR_INVALID_PARAMETER:  //   87
          case ERROR_NEGATIVE_SEEK:      //  131
            return EINVAL;
          default:
            return 0;
        }
#else
        return winerror;
#endif
    }

    const char kErrnoDetailTypeId[] = "turbo::ErrnoDetail";

    class ErrnoDetail : public turbo::StatusPayload {
    public:
        explicit ErrnoDetail(int errnum) : errnum_(errnum) {}

        const char *type_id() const override { return kErrnoDetailTypeId; }

        std::string to_string() const override {
            std::stringstream ss;
            ss << "[errno " << errnum_ << "] " << km_error(errnum_);
            return ss.str();
        }

        int errnum() const { return platform_error_to_errno(errnum_); }

    protected:
        int errnum_;
    };


    std::shared_ptr<turbo::StatusPayload> status_payload_from_errno(int errnum) {
        if (!errnum) {
            return nullptr;
        }
        return std::make_shared<ErrnoDetail>(errnum);
    }

    int errno_from_status_payload(const turbo::Status &status) {
        const auto detail = status.get_payload(kErrnoDetailTypeId);
        if (detail != nullptr) {
            return turbo::checked_cast<const ErrnoDetail &>(*detail).errnum();
        }
        return 0;
    }

    const char kSignalDetailTypeId[] = "turbo::SignalDetail";

    class SignalPayload : public turbo::StatusPayload {
    public:
        explicit SignalPayload(int signum) : signum_(signum) {}

        [[nodiscard]] const char* type_id() const override { return kSignalDetailTypeId; }

        [[nodiscard]] std::string to_string() const override {
            std::stringstream ss;
            ss << "received signal " << signum_;
            return ss.str();
        }

        int signum() const { return signum_; }

    protected:
        int signum_;
    };

    std::shared_ptr<turbo::StatusPayload> status_payload_from_signal(int signum) {
        return std::make_shared<SignalPayload>(signum);
    }

    int signal_from_status_payload(const turbo::Status& status) {
        const auto detail = status.get_payload(kSignalDetailTypeId);
        if (detail != nullptr) {
            return turbo::checked_cast<const SignalPayload&>(*detail).signum();
        }
        return 0;
    }


    StatusBuilder::StatusBuilder(const turbo::Status &status) : status_(status) {}

    StatusBuilder::StatusBuilder(turbo::Status &&status) : status_(status) {}

    StatusBuilder::StatusBuilder(turbo::StatusCode code) : status_(code, "") {}

    StatusBuilder::StatusBuilder(const StatusBuilder &sb) : status_(sb.status_) {
        if (sb.streamptr_ != nullptr) {
            streamptr_ = std::make_unique<std::ostringstream>(sb.streamptr_->str());
        }
    }

    turbo::Status StatusBuilder::create_status() &&{
        auto result = [&] {
            if (streamptr_->str().empty()) return status_;
            std::string new_msg =
                    turbo::str_cat(status_.message(), "; ", streamptr_->str());
            return turbo::Status(status_.code(), new_msg);
        }();
        status_ = unknown_error("");
        streamptr_ = nullptr;
        return result;
    }

    StatusBuilder &StatusBuilder::log_error() &{ return *this; }

    StatusBuilder &&StatusBuilder::log_error() &&{ return std::move(log_error()); }

    StatusBuilder::operator turbo::Status() const & {
        if (streamptr_ == nullptr) return status_;
        return StatusBuilder(*this).create_status();
    }

    StatusBuilder::operator turbo::Status() && {
        if (streamptr_ == nullptr) return status_;
        return std::move(*this).create_status();
    }

    StatusBuilder aborted_error_builder() { return StatusBuilder(turbo::StatusCode::kAborted); }

    StatusBuilder already_exists_error_builder() {
        return StatusBuilder(turbo::StatusCode::kAlreadyExists);
    }

    StatusBuilder cancelled_error_builder() {
        return StatusBuilder(turbo::StatusCode::kCancelled);
    }

    StatusBuilder failed_precondition_error_builder() {
        return StatusBuilder(turbo::StatusCode::kFailedPrecondition);
    }

    StatusBuilder internal_error_builder() { return StatusBuilder(turbo::StatusCode::kInternal); }

    StatusBuilder invalid_argument_error_builder() {
        return StatusBuilder(turbo::StatusCode::kInvalidArgument);
    }

    StatusBuilder not_found_error_builder() { return StatusBuilder(turbo::StatusCode::kNotFound); }

    StatusBuilder out_of_range_error_builder() {
        return StatusBuilder(turbo::StatusCode::kOutOfRange);
    }

    StatusBuilder unauthenticated_error_builder() {
        return StatusBuilder(turbo::StatusCode::kUnauthenticated);
    }

    StatusBuilder unavailable_error_builder() {
        return StatusBuilder(turbo::StatusCode::kUnavailable);
    }

    StatusBuilder unimplemented_error_builder() {
        return StatusBuilder(turbo::StatusCode::kUnimplemented);
    }

    StatusBuilder unknown_error_builder() { return StatusBuilder(turbo::StatusCode::kUnknown); }

    turbo::Status annotate_status(const turbo::Status &s, std::string_view msg) {
        if (s.ok() || msg.empty()) return s;

        std::string_view new_msg = msg;
        std::string annotated;
        if (!s.message().empty()) {
            turbo::str_append(&annotated, s.message(), "; ", msg);
            new_msg = annotated;
        }
        return turbo::make_status(s.code(), new_msg);
    }

    StatusBuilder ret_check_fail(std::string_view msg) {
        return internal_error_builder() << msg;
    }

    std::ostream &operator<<(std::ostream &os, const __StatusLocation &l) {
        static const int kMaxDepth = 4;
        std::vector<std::string_view> segs = turbo::str_split(l.file, "/");
        auto offset = segs.size() > kMaxDepth ? segs.size() - kMaxDepth : 0;
        os << " [" << (offset == 0 ? "." : "...");
        for (; offset < segs.size(); offset++) {
            os << "/" << segs[offset];
        }
        os << ":" << l.line << "] ";
        return os;
    }

}  // namespace turbo
